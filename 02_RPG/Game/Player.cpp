#include "stdafx.h"
#include "Player.h"
#include "Bullet.h"
#include "GameCamera.h"
#include "GameOver.h"
#include "DamageEffect.h"
#include "GameInput.h"

Player::Player()
{
    animationClips[enAnimationClip_Idle].Load("Assets/animData/soldier_idle.tka");
    animationClips[enAnimationClip_Idle].SetLoopFlag(true);
    /*animationClips[enAnimationClip_Walk].Load("Assets/animData/soldier_walk.tka");
    animationClips[enAnimationClip_Walk].SetLoopFlag(true);*/
    animationClips[enAnimationClip_Dash].Load("Assets/animData/soldier_dash.tka");
    animationClips[enAnimationClip_Dash].SetLoopFlag(true);
    animationClips[enAnimationClip_Jump].Load("Assets/animData/soldier_jump.tka");
    animationClips[enAnimationClip_Jump].SetLoopFlag(false);
}


Player::~Player()
{
    // 最初に作った弾を全部消す
    for (int i = 0; i < BULLET_POOL_SIZE; i++)
    {
        if (m_redBullets[i] != nullptr)
        {
            DeleteGO(m_redBullets[i]);
            m_redBullets[i] = nullptr;
        }

        if (m_blueBullets[i] != nullptr)
        {
            DeleteGO(m_blueBullets[i]);
            m_blueBullets[i] = nullptr;
        }

        if (m_yellowBullets[i] != nullptr)
        {
            DeleteGO(m_yellowBullets[i]);
            m_yellowBullets[i] = nullptr;
        }
    }
}



bool Player::Start()
{
    m_position = { -77.4f,0.0f,3914.7f };

    m_modelRender.Init(
        "Assets/modelData/preset/soldier.tkm",
        animationClips,
        enAnimationClip_Num
    );
    m_modelRender.PlayAnimation(enAnimationClip_Idle);
    m_modelRender.SetPosition(m_position);
    /* Quaternion rot;
     rot.SetRotationDeg(Vector3::AxisX, 90.0f);

     m_modelRender.SetRotation(rot);
     m_modelRender.PlayAnimation(enAnimationClip_Idle);
     m_modelRender.SetPosition(m_position);*/

    m_characterController.Init(
        40.0f,
        120.0f,
        m_position
    );

    for (int i = 0; i < BULLET_POOL_SIZE; i++)
    {
        m_redBullets[i] = NewGO<Bullet>(0);
        m_redBullets[i]->SetBulletType(Bullet::BulletType::Red);

        m_blueBullets[i] = NewGO<Bullet>(0);
        m_blueBullets[i]->SetBulletType(Bullet::BulletType::Blue);

        m_yellowBullets[i] = NewGO<Bullet>(0);
        m_yellowBullets[i]->SetBulletType(Bullet::BulletType::Yellow);
    }

    m_ammo = m_maxAmmo;
    m_isReloading = false;
    m_reloadTimer = 0.0f;
    m_positionFont.SetPosition(-800.0f, 450.0f, 0.0f);
    m_positionFont.SetScale(1.0f);
    m_positionFont.SetColor(g_vec4White);
    m_positionFont.SetText(L"X: 0.0  Y: 0.0  Z: 0.0");
    return true;
}

void Player::Update()
{
    if (IsDead())
    {
        m_modelRender.Update();
        return;
    }

    const float deltaTime = 1.0f / 60.0f;

    if (GetAsyncKeyState('1') & 0x8000)
    {
        m_weaponType = WeaponType::Red;
    }
    else if (GetAsyncKeyState('2') & 0x8000)
    {
        m_weaponType = WeaponType::Blue;
    }
    else if (GetAsyncKeyState('3') & 0x8000)
    {
        m_weaponType = WeaponType::Yellow;
    }

    // パッド: RB で次の色、LB で前の色(赤 → 青 → 黄 → 赤 …)
    if (GameInput::PadTrigger(enButtonRB1))
    {
        if (m_weaponType == WeaponType::Red)       m_weaponType = WeaponType::Blue;
        else if (m_weaponType == WeaponType::Blue) m_weaponType = WeaponType::Yellow;
        else                                       m_weaponType = WeaponType::Red;
    }
    else if (GameInput::PadTrigger(enButtonLB1))
    {
        if (m_weaponType == WeaponType::Red)         m_weaponType = WeaponType::Yellow;
        else if (m_weaponType == WeaponType::Yellow) m_weaponType = WeaponType::Blue;
        else                                         m_weaponType = WeaponType::Red;
    }

    if (m_isReloading)
    {
        m_reloadTimer -= deltaTime;

        if (m_reloadTimer <= 0.0f)
        {
            m_ammo = m_maxAmmo;
            m_isReloading = false;
            m_reloadTimer = 0.0f;
        }
    }

    // リロード: R / パッドの X(ダッシュ中に押したら、ダッシュをやめてリロード)
    if (((GetAsyncKeyState('R') & 0x0001) || GameInput::PadTrigger(enButtonX)) &&
        !m_isReloading &&
        m_ammo < m_maxAmmo)
    {
        m_isReloading = true;
        m_reloadTimer = m_reloadTime;
        m_padSprint = false;
    }

    Vector3 moveSpeed = Vector3::Zero;

    float x = 0.0f;
    float y = 0.0f;

    if (GetAsyncKeyState('W') & 0x8000)
    {
        y += 1.0f;
    }

    if (GetAsyncKeyState('S') & 0x8000)
    {
        y -= 1.0f;
    }

    if (GetAsyncKeyState('A') & 0x8000)
    {
        x -= 1.0f;
    }

    if (GetAsyncKeyState('D') & 0x8000)
    {
        x += 1.0f;
    }

    // キーボード入力がないときだけパッドの左スティックを使う(少しの傾きは無視)
    if (x == 0.0f && y == 0.0f)
    {
        x = GameInput::LStickX();
        y = GameInput::LStickY();
    }
    // カメラの向きを基準にした前方向と右方向を作る。
    Vector3 forward = Vector3::AxisZ;
    Vector3 right = Vector3::AxisX;

    GameCamera* moveCamera = FindGO<GameCamera>("gameCamera");

    if (moveCamera != nullptr)
    {
        Quaternion moveRotY;
        moveRotY.SetRotationDeg(
            Vector3::AxisY,
            moveCamera->GetAngle()
        );

        moveRotY.Apply(forward);
        moveRotY.Apply(right);
    }

    forward.y = 0.0f;
    right.y = 0.0f;
    forward.Normalize();
    right.Normalize();

    Vector3 direction = right * x + forward * y;

    if (direction.Length() > 1.0f)
    {
        direction.Normalize();
    }

    bool isMoving = direction.Length() > 0.01f;

    // ダッシュ: Shift を押している間 / パッドは左スティック押し込みで ON(止まると OFF)
    const float DASH_RATE = 1.8f;   // ダッシュ時の速さの倍率

    if (GameInput::PadTrigger(enButtonLB3))
    {
        m_padSprint = !m_padSprint;
    }

    if (!isMoving)
    {
        m_padSprint = false;
    }

    // 撃つボタン
    bool fireHeld = ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0) || GameInput::PadPress(enButtonRB2);

    // リロード中はダッシュしない
    if (m_isReloading)
    {
        m_padSprint = false;
    }

    // ダッシュ中は撃てない(撃つボタンを押してもダッシュは止まらない)
    bool isDashing = isMoving && !m_isReloading &&
        (((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0) || m_padSprint);
    m_isDashing = isDashing;   // カメラの構えと同じ判定を使う
    // ---- ダッシュの度合い(走り出しは速く、止まるときはだんだん遅く)
    const float SPRINT_IN_TIME = 0.15f;    // 歩き → 全力ダッシュまで(秒)
    const float SPRINT_OUT_TIME = 0.35f;   // 全力ダッシュ → 歩きまで(秒)

    if (isDashing)
    {
        m_dashBlend += deltaTime / SPRINT_IN_TIME;
    }
    else
    {
        m_dashBlend -= deltaTime / SPRINT_OUT_TIME;
    }

    if (m_dashBlend < 0.0f) m_dashBlend = 0.0f;
    if (m_dashBlend > 1.0f) m_dashBlend = 1.0f;
    // スライディング中はダッシュあつかいにしない(銃を前に構えて撃てる)
    if (m_isSliding)
    {
        m_dashBlend = 0.0f;
    }
    float dashSmooth = m_dashBlend * m_dashBlend * (3.0f - 2.0f * m_dashBlend);   // なめらかに
    // ---- しゃがみ(ダッシュしていないときに C / Ctrl を押している間 / パッドは B で切り替え)
    const float CROUCH_RATE = 0.5f;   // しゃがみ中の移動の速さ(倍)

    bool crouchKey = (GetAsyncKeyState('C') & 0x8000) || (GetAsyncKeyState(VK_CONTROL) & 0x8000);

    if (isDashing)
    {
        m_padCrouch = false;          // 走り出したら立つ
    }
    else if (GameInput::PadTrigger(enButtonB))
    {
        m_padCrouch = !m_padCrouch;
    }

    m_isCrouching = !m_isSliding && ((crouchKey && !isDashing) || m_padCrouch);
    // 歩きの速さ 〜 ダッシュの速さ を、なめらかに切り替える
    moveSpeed = direction * m_moveSpeed * (1.0f + (DASH_RATE - 1.0f) * dashSmooth);
    if (m_isCrouching)
    {
        moveSpeed *= CROUCH_RATE;
    }
    // ---- スライディング(ダッシュ中に C か Ctrl / パッドの B)
    const float SLIDE_TIME = 1.0f;                     // 滑る時間(秒)
    const float SLIDE_START_SPEED = m_moveSpeed * 3.0f; // 滑り出しの速さ
    const float SLIDE_END_SPEED = m_moveSpeed * 1.0f;   // 滑り終わりの速さ
    const float SLIDE_COOLDOWN = 0.4f;                  // 次に滑れるまでの時間(秒)

    bool slideKey = (GetAsyncKeyState('C') & 0x8000) || (GetAsyncKeyState(VK_CONTROL) & 0x8000) ||
        GameInput::PadPress(enButtonB);
    bool slidePressed = slideKey && !m_prevSlideKey;   // 押した瞬間だけ
    m_prevSlideKey = slideKey;
    m_slideCooldown -= deltaTime;

    if (!m_isSliding && slidePressed && isDashing && m_isGrounded && m_slideCooldown <= 0.0f)
    {
        m_isSliding = true;
        m_slideTimer = 0.0f;
        m_slideDir = direction;
        m_slideDir.y = 0.0f;
        m_slideDir.Normalize();
    }

    if (m_isSliding)
    {
        m_slideTimer += deltaTime;
        float t = m_slideTimer / SLIDE_TIME;

        if (t >= 1.0f)
        {
            m_isSliding = false;
            m_slideCooldown = SLIDE_COOLDOWN;
            m_padSprint = false;
        }
        else
        {
            // 最初は速く、だんだん遅くなる
            float ease = 1.0f - (1.0f - t) * (1.0f - t);
            float slideSpeed = SLIDE_START_SPEED + (SLIDE_END_SPEED - SLIDE_START_SPEED) * ease;
            moveSpeed.x = m_slideDir.x * slideSpeed;
            moveSpeed.z = m_slideDir.z * slideSpeed;
        }
    }

    // カメラ用に、なめらかに 0〜1 を変える
    m_slideRate += (m_isSliding ? 8.0f : -6.0f) * deltaTime;
    if (m_slideRate < 0.0f) m_slideRate = 0.0f;
    if (m_slideRate > 1.0f) m_slideRate = 1.0f;
    m_isWalking = isMoving;
    m_crouchRate += (m_isCrouching ? 8.0f : -8.0f) * deltaTime;
    if (m_crouchRate < 0.0f) m_crouchRate = 0.0f;
    if (m_crouchRate > 1.0f) m_crouchRate = 1.0f;
    // ジャンプ: Space / パッドの A
    if (((GetAsyncKeyState(VK_SPACE) & 0x8000) || GameInput::PadPress(enButtonA)) && m_isGrounded)
    {
        m_verticalSpeed = m_jumpPower;
        m_isGrounded = false;
        m_padCrouch = false;          // ジャンプしたら立つ

        if (m_isSliding)
        {
            m_isSliding = false;
            m_slideCooldown = 0.4f;
        }
    }

    m_verticalSpeed += m_gravity * deltaTime;

    moveSpeed.y = m_verticalSpeed;

    m_position = m_characterController.Execute(
        moveSpeed,
        deltaTime
    );

    // 床だけでなく、足場や階段の上でも着地扱いにする。
    if (m_characterController.IsOnGround())
    {
        m_verticalSpeed = 0.0f;
        m_isGrounded = true;
    }
    else
    {
        m_isGrounded = false;
    }

    // アニメーションの切り替え。
    // 空中 → ジャンプ、ダッシュ中 → 走り、移動中 → 歩き、それ以外 → 待機。
    // 同じアニメーションを指定し続けても、再生し直しにはならない。
    // 段差を降りた一瞬でジャンプに切り替わらないよう、空中が少し続いてから切り替える。
    static float airTime = 0.0f;

    if (m_isGrounded)
    {
        airTime = 0.0f;
    }
    else
    {
        airTime += deltaTime;
    }

    if (!m_isGrounded && (m_verticalSpeed > 0.0f || airTime > 0.15f))
    {
        m_modelRender.PlayAnimation(enAnimationClip_Jump, 0.1f);
    }
    else if (isMoving)
    {
        m_modelRender.PlayAnimation(enAnimationClip_Dash, 0.15f);
    }
    /*  else if (isMoving)
      {
          m_modelRender.PlayAnimation(enAnimationClip_Walk, 0.2f);*/
          /* }*/
    else
    {
        m_modelRender.PlayAnimation(enAnimationClip_Idle, 0.2f);
    }
    // ---- 連射(押しっぱなしで撃ち続ける)
    const float FIRE_INTERVAL = 0.1f;   // 撃つ間隔(秒)。0.1で1秒に10発
    m_fireTimer -= deltaTime;

    // 撃つ: 左クリック / パッドの RT
        // 撃つ: 左クリック / パッドの RT(ダッシュ中と、構え直し中は撃てない)
    if (fireHeld &&
        (m_isSliding || (!isDashing && m_dashBlend <= 0.05f)) &&   // 滑ってる間 or 構え直し終わったら撃てる
        !m_isReloading &&
        m_ammo > 0 &&
        m_fireTimer <= 0.0f)
    {
        m_fireTimer = FIRE_INTERVAL;
        GameCamera* camera = FindGO<GameCamera>("gameCamera");

        Vector3 bulletDirection = Vector3::AxisZ;
        Vector3 bulletPos = m_position;
        bulletPos.y += 80.0f;

        if (camera != nullptr)
        {
            // 照準の先で最初に当たる点
            Vector3 aimDir = camera->GetAimDirection();
            Vector3 aimTarget = camera->GetAimTarget();

            // 弾は目の前の、銃口のあたり(少し右下)から出す
            Vector3 eye = g_camera3D->GetPosition();
            Vector3 right;
            right.Cross(Vector3::AxisY, aimDir);
            right.Normalize();
            bulletPos = eye + aimDir * 60.0f + right * 8.0f;
            bulletPos.y -= 6.0f;

            // 出た場所から、照準の先の点に向かって飛ばす
            bulletDirection = aimTarget - bulletPos;

            // 壁に近すぎて向きがおかしくなるときは、照準の向きそのままにする
            if (bulletDirection.Length() < 80.0f)
            {
                bulletDirection = aimDir;
            }
        }

        bulletDirection.Normalize();
        Bullet** bulletPool = m_redBullets;

        switch (m_weaponType)
        {
        case WeaponType::Red:
            bulletPool = m_redBullets;
            break;

        case WeaponType::Blue:
            bulletPool = m_blueBullets;
            break;

        case WeaponType::Yellow:
            bulletPool = m_yellowBullets;
            break;
        }

        for (int i = 0; i < BULLET_POOL_SIZE; i++)
        {
            if (bulletPool[i] != nullptr && !bulletPool[i]->IsActive())
            {
                bulletPool[i]->Activate(
                    bulletPos,
                    bulletDirection
                );

                m_ammo--;

                // 一人称の銃の反動と銃口の光
                if (camera != nullptr)
                {
                    camera->OnFire();
                }

                if (m_ammo <= 0)
                {
                    m_isReloading = true;
                    m_reloadTimer = m_reloadTime;
                }

                break;
            }
        }

    }

    GameCamera* camera = FindGO<GameCamera>("gameCamera");

    if (camera != nullptr)
    {
        float angle = camera->GetAngle();

        Quaternion rotY;
        rotY.SetRotationDeg(
            Vector3::AxisY,
            angle
        );

        /*  Quaternion rotX;
          rotX.SetRotationDeg(
              Vector3::AxisX,
              90.0f
          );

          Quaternion rot = rotY * rotX;*/

        m_modelRender.SetRotation(rotY);
    }

    m_modelRender.SetPosition(m_position);
    m_modelRender.Update();
    wchar_t positionText[128];

    swprintf_s(
        positionText,
        L"X: %.1f  Y: %.1f  Z: %.1f",
        m_position.x,
        m_position.y,
        m_position.z
    );

    m_positionFont.SetText(positionText);
}

void Player::Render(RenderContext& rc)
{
    /* m_modelRender.Draw(rc);*/
     /* m_positionFont.Draw(rc);*/
}

void Player::TakeDamage(int damage)
{
    if (m_hp <= 0)
    {
        return;
    }

    m_hp -= damage;

    if (m_hp < 0)
    {
        m_hp = 0;
    }

    DamageEffect* effect =
        FindGO<DamageEffect>("damageEffect");

    if (effect != nullptr && !effect->IsPlaying())
    {
        effect->Play();
    }

    if (m_hp <= 0)
    {
        NewGO<GameOver>(0, "gameOver");
    }
}

void Player::RemoveBullet(Bullet* bullet)
{
    if (bullet == nullptr)
    {
        return;
    }

    for (int i = 0; i < BULLET_POOL_SIZE; i++)
    {
        if (m_redBullets[i] == bullet)
        {
            m_redBullets[i] = nullptr;
            DeleteGO(bullet);
            return;
        }

        if (m_blueBullets[i] == bullet)
        {
            m_blueBullets[i] = nullptr;
            DeleteGO(bullet);
            return;
        }

        if (m_yellowBullets[i] == bullet)
        {
            m_yellowBullets[i] = nullptr;
            DeleteGO(bullet);
            return;
        }
    }
}