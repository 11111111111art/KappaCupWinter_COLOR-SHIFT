#include "stdafx.h"
#include "GameCamera.h"
#include "Player.h"
#include <cmath>
#include "physics/Physics.h"
#include "GameInput.h"
#include "Robot.h"

namespace
{
    const float PI = 3.14159265f;

    // ---- 一人称カメラ
    const float EYE_HEIGHT = 100.0f;        // 目の高さ(プレイヤーの足元から)
    const float PITCH_MAX = 80.0f;          // 見下ろしの上限(度)
    const float PITCH_MIN = -80.0f;         // 見上げの上限(度)
    const float CROUCH_EYE_DROP = 40.0f;    // しゃがんだときに目線が下がる量

    // ---- 一人称の銃
    const float SCOPE_VIEW_ANGLE = 20.0f * PI / 180.0f;  // スコープを覗いたときの画角(小さいほどズーム)
    const float SCOPE_SPEED = 8.0f;         // 覗く速さ(約0.12秒で覗ききる)
    const float SPRINT_SPEED_LINE = 420.0f; // この速さを超えたらダッシュの構え
    const float BOB_SIZE_X = 0.6f;          // 走ったときの左右の揺れ
    const float BOB_SIZE_Y = 0.7f;          // 走ったときの上下の揺れ
    const float RECOIL_PITCH = 5.0f;        // 撃ったときに銃口が跳ねる角度(度)
    const float RECOIL_BACK = 3.0f;         // 撃ったときに銃が後ろに下がる量
    const float FLASH_TIME = 0.05f;         // 銃口の光を出す時間(秒)
    const float SLIDE_EYE_DROP = 45.0f;     // スライディング中に目線が下がる量
    const float SLIDE_WIDEN = 0.10f;        // スライディング中に画角を広げる割合

    // ---- パッドの右スティックで視点を回す速さ(度/秒)
    const float PAD_YAW_SPEED = 200.0f;     // 左右
    const float PAD_PITCH_SPEED = 140.0f;   // 上下
    const float PAD_SCOPE_RATE = 0.4f;      // スコープ中は遅くして狙いやすく

    // ---- マウスで視点
    const float MOUSE_SENS = 0.12f;          // 感度(マウス1ピクセルで何度回るか)
    const float MOUSE_SCOPE_RATE = 0.5f;     // スコープ中の感度(倍)

    bool s_mouseCaptured = false;            // マウスで視点操作中か
    bool s_mouseSkip = true;                 // つかんだ直後の1フレームは動かさない
    bool s_userReleased = false;             // Esc でカーソルを出したか
    bool s_escPrev = false;

    // ---- エイムアシスト(パッドだけ)
    const float ASSIST_RADIUS = 150.0f;      // 照準がロボットからこれ以内なら効く(ロボットの太さくらい)
    const float ASSIST_MAX_DIST = 4000.0f;   // これより遠いロボットには効かない
    const float ASSIST_FRICTION = 0.4f;      // 吸い付き: 一番近いとき視点の速さが何倍になるか
    const float ASSIST_MAGNET = 25.0f;       // 追従: 1秒に何度までロボットのほうへ寄せるか
    const float ROBOT_AIM_HEIGHT = 260.0f;   // ロボットの胸の高さ(足元から)

    float Lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    float Clamp(float v, float lo, float hi)
    {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    // 0〜1 をなめらかにする
    float Smooth(float t)
    {
        return t * t * (3.0f - 2.0f * t);
    }

    // ゲームのウィンドウが一番手前にあるときだけ、そのウィンドウを返す
    HWND GetActiveGameWindow()
    {
        HWND hwnd = GetForegroundWindow();

        if (hwnd == nullptr)
        {
            return nullptr;
        }

        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        return pid == GetCurrentProcessId() ? hwnd : nullptr;
    }

    // ウィンドウの真ん中(画面の座標)
    POINT GetWindowCenter(HWND hwnd)
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        POINT center = { (rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2 };
        ClientToScreen(hwnd, &center);
        return center;
    }

    // カーソルを隠して、ウィンドウの外に出ないようにする / 元に戻す
    void SetMouseCapture(bool on, HWND hwnd)
    {
        if (on == s_mouseCaptured)
        {
            return;
        }

        s_mouseCaptured = on;

        if (on)
        {
            RECT rc;
            GetClientRect(hwnd, &rc);
            POINT tl = { rc.left, rc.top };
            POINT br = { rc.right, rc.bottom };
            ClientToScreen(hwnd, &tl);
            ClientToScreen(hwnd, &br);
            RECT clip = { tl.x, tl.y, br.x, br.y };
            ClipCursor(&clip);

            while (ShowCursor(FALSE) >= 0) {}
            s_mouseSkip = true;
        }
        else
        {
            ClipCursor(nullptr);

            while (ShowCursor(TRUE) < 0) {}
        }
    }

    // 照準の近くにいるロボットを探す。効く強さ(0〜1)を返して、狙う場所を outTarget に入れる
    float FindAssistTarget(const Vector3& eye, const Vector3& aimDir, Vector3& outTarget)
    {
        float best = 0.0f;

        QueryGOs<Robot>("robot", [&](Robot* robot)
            {
                if (robot->IsDead())
                {
                    return true;
                }

                Vector3 target = robot->GetPosition();
                target.y += ROBOT_AIM_HEIGHT;

                Vector3 toTarget = target - eye;
                float dist = toTarget.Length();

                if (dist < 1.0f || dist > ASSIST_MAX_DIST)
                {
                    return true;
                }

                // 照準の線から、ロボットまでどれくらい離れているか
                float along = toTarget.Dot(aimDir);

                if (along <= 0.0f)
                {
                    return true;   // 後ろにいる
                }

                Vector3 closest = eye + aimDir * along;
                float off = (target - closest).Length();

                if (off > ASSIST_RADIUS)
                {
                    return true;
                }

                float strength = 1.0f - off / ASSIST_RADIUS;

                if (strength <= best)
                {
                    return true;
                }

                // 壁の向こうなら効かない
                Vector3 hitPos;

                if (PhysicsWorld::GetInstance()->RayTest(eye, target, hitPos))
                {
                    if ((hitPos - eye).Length() < dist - 50.0f)
                    {
                        return true;
                    }
                }

                best = strength;
                outTarget = target;
                return true;
            });

        return best;
    }
}

GameCamera::GameCamera()
{
}

GameCamera::~GameCamera()
{
    // カメラが消えたらカーソルを返す
    SetMouseCapture(false, nullptr);
}

bool GameCamera::Start()
{
    m_player = FindGO<Player>("player");

    m_angle = 180.0f;
    m_pitch = 0.0f;

    g_camera3D->SetNear(1.0f);
    g_camera3D->SetFar(10000.0f);

    // 一人称の銃
    m_fpGun.Init("Assets/modelData/preset/fp_rifle.tkm");
    m_muzzleFlash.Init("Assets/modelData/preset/muzzle_flash.tkm");
    m_fpMag.Init("Assets/modelData/preset/fp_mag.tkm");
    m_defaultViewAngle = g_camera3D->GetViewAngle();
    m_prevAngle = m_angle;
    m_prevPitch = m_pitch;

    if (m_player != nullptr)
    {
        m_prevPlayerPos = m_player->GetPosition();
    }

    return true;
}

void GameCamera::Update()
{
    if (m_player == nullptr)
    {
        return;
    }

    // ================= カメラの回転 =================
    if (GetAsyncKeyState(VK_LEFT) & 0x8000)
    {
        m_angle -= m_cameraSpeed;
    }

    if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
    {
        m_angle += m_cameraSpeed;
    }

    // m_pitch はプラスで下を向く
    if (GetAsyncKeyState(VK_UP) & 0x8000)
    {
        m_pitch -= m_cameraSpeed;
    }

    if (GetAsyncKeyState(VK_DOWN) & 0x8000)
    {
        m_pitch += m_cameraSpeed;
    }

    // ---- マウスで視点
    {
        HWND hwnd = GetActiveGameWindow();
        bool esc = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;

        if (hwnd == nullptr)
        {
            // 別のウィンドウに切り替えたら、カーソルを返す
            SetMouseCapture(false, nullptr);
        }
        else if (esc && !s_escPrev)
        {
            // Esc でカーソルを出す
            s_userReleased = true;
            SetMouseCapture(false, hwnd);
        }
        else if (s_userReleased && (GetAsyncKeyState(VK_LBUTTON) & 0x8000))
        {
            // クリックで視点操作に戻る
            s_userReleased = false;
        }

        if (hwnd != nullptr && !s_userReleased)
        {
            SetMouseCapture(true, hwnd);
        }

        s_escPrev = esc;

        if (s_mouseCaptured && hwnd != nullptr)
        {
            POINT center = GetWindowCenter(hwnd);
            POINT cursor;
            GetCursorPos(&cursor);

            if (!s_mouseSkip)
            {
                float sens = MOUSE_SENS * Lerp(1.0f, MOUSE_SCOPE_RATE, Smooth(m_scopeRate));

                m_angle += (cursor.x - center.x) * sens;
                m_pitch += (cursor.y - center.y) * sens;   // マウスを下げると下を向く
            }

            s_mouseSkip = false;

            // 毎フレーム真ん中に戻す
            SetCursorPos(center.x, center.y);
        }
    }

    // ---- パッドの右スティック(小さく倒すとゆっくり、大きく倒すと速く) + エイムアシスト
    {
        float rx = GameInput::RStickX();
        float ry = GameInput::RStickY();
        float k = Lerp(1.0f, PAD_SCOPE_RATE, Smooth(m_scopeRate));

        // 照準の近くのロボットを探す
        Vector3 assistTarget;
        float assist = FindAssistTarget(g_camera3D->GetPosition(), GetAimDirection(), assistTarget);

        // 吸い付き: ロボットの近くでは視点がゆっくりになる
        float friction = Lerp(1.0f, ASSIST_FRICTION, assist);

        m_angle += rx * fabsf(rx) * PAD_YAW_SPEED * k * friction / 60.0f;
        m_pitch -= ry * fabsf(ry) * PAD_PITCH_SPEED * k * friction / 60.0f;   // 上に倒すと上を向く

        // 追従: スティックを動かしている間だけ、少しロボットのほうへ寄せる
        bool padActive =
            fabsf(rx) + fabsf(ry) > 0.05f ||
            fabsf(GameInput::LStickX()) + fabsf(GameInput::LStickY()) > 0.1f;

        if (assist > 0.0f && padActive)
        {
            Vector3 dir = assistTarget - g_camera3D->GetPosition();
            dir.Normalize();

            // 狙いたい向き(m_angle と m_pitch と同じ決め方)
            float targetYaw = atan2f(dir.x, dir.z) * 180.0f / PI;
            float targetPitch = -asinf(Clamp(dir.y, -1.0f, 1.0f)) * 180.0f / PI;

            // 一番近い回り方にする(-180〜180度)
            float dYaw = targetYaw - m_angle;
            while (dYaw > 180.0f)  dYaw -= 360.0f;
            while (dYaw < -180.0f) dYaw += 360.0f;
            float dPitch = targetPitch - m_pitch;

            float step = ASSIST_MAGNET * assist / 60.0f;
            m_angle += Clamp(dYaw, -step, step);
            m_pitch += Clamp(dPitch, -step * 0.5f, step * 0.5f);   // 上下は弱め
        }
    }

    m_pitch = Clamp(m_pitch, PITCH_MIN, PITCH_MAX);

    // ================= 一人称カメラ =================
    Vector3 forward = GetAimDirection();

    Vector3 cameraPos = m_player->GetPosition();

    // スライディング・しゃがみ中は目線を下げる(低いほうに合わせる)
    float slideDrop = SLIDE_EYE_DROP * Smooth(m_player->GetSlideRate());
    float crouchDrop = CROUCH_EYE_DROP * Smooth(m_player->GetCrouchRate());
    cameraPos.y += EYE_HEIGHT - (slideDrop > crouchDrop ? slideDrop : crouchDrop);

    Vector3 target = cameraPos + forward * 100.0f;

    g_camera3D->SetPosition(cameraPos);
    g_camera3D->SetTarget(target);
    g_camera3D->Update();

    // ================= 一人称の銃 =================
    UpdateFpGun(cameraPos, forward);
}

void GameCamera::UpdateFpGun(const Vector3& cameraPos, const Vector3& forward)
{
    const float dt = 1.0f / 60.0f;

    // ---- プレイヤーの動き
    Vector3 playerPos = m_player->GetPosition();
    Vector3 move = playerPos - m_prevPlayerPos;
    float vy = move.y / dt;
    move.y = 0.0f;
    float speed = move.Length() / dt;
    m_prevPlayerPos = playerPos;

    // ---- 右クリック / パッドの LT で覗く。速く走るとダッシュの構え
    bool reloading = m_player->IsReloading();
    bool scoping = (((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0) || GameInput::PadPress(enButtonLB2)) && !reloading;
    
    m_scopeRate = Clamp(m_scopeRate + (scoping ? SCOPE_SPEED : -SCOPE_SPEED) * dt, 0.0f, 1.0f);
    float scope = Smooth(m_scopeRate);
    // 銃の構えも、プレイヤーのダッシュの度合いに合わせる(減速しながら構え直す)
    float sprint = Smooth(m_player->GetDashBlend());
    float hip = 1.0f - scope;

    // スライディング中はダッシュの構えをやめる
    float slide = Smooth(m_player->GetSlideRate());
    sprint *= 1.0f - slide;

    // ---- ズーム(スライディング中は少し広げてスピード感を出す)
    g_camera3D->SetViewAngle(
        Lerp(m_defaultViewAngle * (1.0f + SLIDE_WIDEN * slide), SCOPE_VIEW_ANGLE, scope));

    // ---- 構え(位置: カメラから見て 右, 上, 前 / 角度: 度)。左が普段、右がダッシュ
    //      ダッシュ中は銃を体の前で横向きに抱え、銃口を左へ向ける
    float ox = Lerp(14.0f, 24.0f, sprint) * hip;
    float oy = Lerp(-14.0f, -13.0f, sprint) * hip;
    float oz = Lerp(15.0f, 24.0f, sprint) * hip;
    float yaw = Lerp(12.0f, 66.0f, sprint) * hip;      // 銃口を左へ(大きいほど真横)
    float pitch = Lerp(11.0f, 3.0f, sprint) * hip;     // 銃口の上下
    float roll = Lerp(-6.0f, 14.0f, sprint) * hip;     // 銃を傾ける

    // スライディング中は銃を少し下げて傾ける
    ox -= 2.0f * slide * hip;
    oy -= 2.0f * slide * hip;
    yaw += 6.0f * slide * hip;
    roll += 18.0f * slide * hip;
    // ---- リロード(銃を持ち上げて傾け、マガジンを抜いて差す)
    float magY = 0.0f;      // マガジンの位置(銃から見て 上, 前)
    float magZ = 0.0f;
    m_magVisible = true;

    if (reloading)
    {
        float t = m_player->GetReloadProgress();

        // 持ち上げて傾ける → 最後に戻す
        float tilt = Smooth(Clamp(t / 0.18f, 0.0f, 1.0f)) *
            (1.0f - Smooth(Clamp((t - 0.82f) / 0.18f, 0.0f, 1.0f)));

        ox += -6.0f * tilt;
        oy += 4.0f * tilt;
        oz += 3.0f * tilt;
        yaw += 14.0f * tilt;
        pitch += 6.0f * tilt;
        roll += -38.0f * tilt;
        sprint *= 1.0f - tilt;

        // マガジン
        if (t >= 0.2f && t < 0.4f)
        {
            // 古いのが落ちる
            float s = (t - 0.2f) / 0.2f;
            magY = -40.0f * s * s;
            magZ = -3.0f * s;
        }
        else if (t >= 0.4f && t < 0.43f)
        {
            m_magVisible = false;   // 持ち替え中
        }
        else if (t >= 0.43f && t < 0.68f)
        {
            // 新しいのが下から来る
            float s = (t - 0.43f) / 0.25f;
            float e = 1.0f - (1.0f - s) * (1.0f - s);
            magY = -30.0f + 28.0f * e;
            magZ = -6.0f + 6.0f * e;
        }
        else if (t >= 0.68f && t < 0.72f)
        {
            // カチッと差し込む
            magY = -2.0f + 2.0f * (t - 0.68f) / 0.04f;
        }

        // 差し込んだ瞬間に銃が少し跳ねる
        if (t >= 0.68f && t <= 0.78f)
        {
            float k = sinf(PI * (t - 0.68f) / 0.10f);
            oy += 1.2f * k;
            pitch += 3.0f * k;
        }
    }
    // ---- 歩き・走りの揺れ(一歩ごとに1回沈む)と呼吸
    //      揺れの速さは歩数に合わせる: 速さ300で約2.5歩/秒、540で約3歩/秒
    bool moving = speed > 10.0f;
    if (moving)
    {
        m_bobPhase += dt * (5.5f + speed * 0.0075f);
    }
    float targetAmp = moving ? Clamp(speed / 300.0f, 0.0f, 1.5f) : 0.0f;
    m_bobAmp = Lerp(m_bobAmp, targetAmp, 0.1f);        // 止まったり走り出したりしたときに急に変わらないように
    m_swayTime += dt;
    float amp = m_bobAmp * (1.0f - 0.9f * scope) * (1.0f - slide);   // 滑っている間は足音の揺れを止める
    ox += sinf(m_bobPhase) * BOB_SIZE_X * amp;
    oy += (0.5f - fabsf(cosf(m_bobPhase))) * BOB_SIZE_Y * amp;
    roll += sinf(m_bobPhase) * 1.5f * amp * hip;       // 少し左右に傾く
    oy += sinf(m_swayTime * 1.6f) * 0.25f * hip;

    // ---- 視点を回したとき、銃が少し遅れてついてくる
    float dYaw = m_angle - m_prevAngle;
    float dPitch = m_pitch - m_prevPitch;
    m_prevAngle = m_angle;
    m_prevPitch = m_pitch;
    m_turnSwayYaw = Clamp(Lerp(m_turnSwayYaw, -dYaw * 1.5f, 0.15f), -5.0f, 5.0f);
    m_turnSwayPitch = Clamp(Lerp(m_turnSwayPitch, dPitch * 1.5f, 0.15f), -4.0f, 4.0f);
    float swayK = 1.0f - 0.8f * scope;
    yaw -= m_turnSwayYaw * swayK;
    pitch += m_turnSwayPitch * swayK;
    ox += m_turnSwayYaw * 0.25f * swayK;

    // ---- ジャンプで下がって、着地で沈む
    m_jumpSway = Lerp(m_jumpSway, Clamp(-vy * 0.006f, -3.0f, 3.0f), 0.15f);
    oy += m_jumpSway * (1.0f - 0.7f * scope);

    // ---- 反動
    m_recoil = Clamp(m_recoil - 9.0f * dt, 0.0f, 1.0f);
    float kick = m_recoil * m_recoil;
    oz -= RECOIL_BACK * kick * (1.0f - 0.5f * scope);
    oy += 0.4f * kick;
    pitch += RECOIL_PITCH * kick * hip + 1.0f * kick * scope;
    m_flashTimer -= dt;

    // ---- カメラの向きの軸
    Vector3 fwd = forward;
    fwd.Normalize();
    Vector3 right;
    right.Cross(Vector3::AxisY, fwd);
    right.Normalize();
    Vector3 up;
    up.Cross(fwd, right);

    Vector3 gunPos = cameraPos + right * ox + up * oy + fwd * oz;

    // ---- 銃の向き = カメラの向きを yaw → pitch → roll の順に回したもの
    float a = yaw * PI / 180.0f;
    Vector3 gf = fwd * cosf(a) - right * sinf(a);
    Vector3 gr = right * cosf(a) + fwd * sinf(a);
    Vector3 gu = up;

    a = pitch * PI / 180.0f;
    Vector3 tf = gf * cosf(a) + gu * sinf(a);
    gu = gu * cosf(a) - gf * sinf(a);
    gf = tf;

    a = roll * PI / 180.0f;
    Vector3 tr = gr * cosf(a) + gu * sinf(a);
    gu = gu * cosf(a) - gr * sinf(a);
    gr = tr;

    Matrix rotMat = Matrix::Identity;
    rotMat.m[0][0] = gr.x; rotMat.m[0][1] = gr.y; rotMat.m[0][2] = gr.z;
    rotMat.m[1][0] = gu.x; rotMat.m[1][1] = gu.y; rotMat.m[1][2] = gu.z;
    rotMat.m[2][0] = gf.x; rotMat.m[2][1] = gf.y; rotMat.m[2][2] = gf.z;

    Quaternion gunRot;
    gunRot.SetRotation(rotMat);

    m_fpGun.SetPosition(gunPos);
    m_fpGun.SetRotation(gunRot);
    m_fpGun.Update();

    m_muzzleFlash.SetPosition(gunPos);
    m_muzzleFlash.SetRotation(gunRot);
    m_muzzleFlash.Update();
    // マガジンは銃と同じ向きで、銃から見た位置だけずらす
    Vector3 magPos = gunPos + gu * magY + gf * magZ;
    m_fpMag.SetPosition(magPos);
    m_fpMag.SetRotation(gunRot);
    m_fpMag.Update();
}

void GameCamera::Render(RenderContext& rc)
{
    m_fpGun.Draw(rc);

    if (m_flashTimer > 0.0f)
    {
        m_muzzleFlash.Draw(rc);
    }

    if (m_magVisible)
    {
        m_fpMag.Draw(rc);
    }
}

void GameCamera::OnFire()
{
    m_recoil = 1.0f;
    m_flashTimer = FLASH_TIME;
}

void GameCamera::ResetCamera()
{
    m_angle = 180.0f;
    m_pitch = 0.0f;
}

Vector3 GameCamera::GetAimDirection() const
{
    float yaw = m_angle * PI / 180.0f;
    float pitch = m_pitch * PI / 180.0f;

    Vector3 direction;

    direction.x = sinf(yaw) * cosf(pitch);
    direction.y = -sinf(pitch);
    direction.z = cosf(yaw) * cosf(pitch);

    direction.Normalize();

    return direction;
}

Vector3 GameCamera::GetAimTarget() const
{
    Vector3 cameraPos = g_camera3D->GetPosition();
    Vector3 direction = GetAimDirection();

    Vector3 rayEnd = cameraPos + direction * 10000.0f;

    Vector3 hitPos;

    if (PhysicsWorld::GetInstance()->RayTest(
        cameraPos,
        rayEnd,
        hitPos
    ))
    {
        return hitPos;
    }

    return rayEnd;
}