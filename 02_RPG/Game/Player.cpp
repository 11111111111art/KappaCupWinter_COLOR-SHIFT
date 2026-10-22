#include "stdafx.h"
#include "Player.h"
#include "Bullet.h"

Player::Player()
{
    animationClips[enAnimationClip_Idle]. Load("Assets/animData/idle.tka");
    animationClips[enAnimationClip_Idle]. SetLoopFlag(true);
    animationClips[enAnimationClip_Walk].Load("Assets/animData/walk.tka");
    animationClips[enAnimationClip_Walk].SetLoopFlag(true);
    animationClips[enAnimationClip_Run].Load("Assets/animData/run.tka");
    animationClips[enAnimationClip_Run].SetLoopFlag(true);
    animationClips[enAnimationClip_Jump].Load("Assets/animData/jump.tka");
    animationClips[enAnimationClip_Jump].SetLoopFlag(false);
}

Player::~Player()
{
}

bool Player::Start()
{
    m_position = Vector3::Zero;

    // プレイヤーモデル
    m_modelRender.Init(
        "Assets/modelData/unityChan.tkm"
    );

    m_modelRender.SetPosition(m_position);

    // 当たり判定
    m_characterController.Init(
        40.0f,
        120.0f,
        m_position
    );

    return true;
}

void Player::Update()
{
    Vector3 moveSpeed = Vector3::Zero;

    // =====================================
    // WASD + 左スティック入力
    // =====================================

    float x = 0.0f;
    float y = 0.0f;

    // キーボード入力
    if (GetAsyncKeyState('W') & 0x8000)
        y += 1.0f;

    if (GetAsyncKeyState('S') & 0x8000)
        y -= 1.0f;

    if (GetAsyncKeyState('A') & 0x8000)
        x -= 1.0f;

    if (GetAsyncKeyState('D') & 0x8000)
        x += 1.0f;

    // ゲームパッドの左スティック入力
    if (g_pad[0] != nullptr)
    {
        x += g_pad[0]->GetLStickXF();
        y += g_pad[0]->GetLStickYF();
    }

    // =====================================
    // 移動方向
    // =====================================

    Vector3 forward = Vector3::AxisZ;
    Vector3 right = Vector3::AxisX;

    // カメラの向きに合わせて移動
    // カメラの前方向を取得
    forward = g_camera3D->GetForward();

    // 上下方向には移動しない
    forward.y = 0.0f;
    forward.Normalize();

    // 右方向を計算
    right.Cross(Vector3::AxisY, forward);
    right.Normalize();

    // 入力を移動方向に変換
    Vector3 direction =
        right * x + forward * y;

    // 斜め移動が速くならないようにする
    if (direction.Length() > 1.0f)
    {
        direction.Normalize();
    }

    // 移動速度
    moveSpeed = direction * m_moveSpeed;

    // =====================================
    // 重力
    // =====================================

    moveSpeed.y = -5.0f;

    // =====================================
    // プレイヤー移動
    // =====================================

    m_position = m_characterController.Execute(
        moveSpeed,
        1.0f / 60.0f
    );

    // =====================================
    // 左クリックで弾を発射
    // =====================================

    if (GetAsyncKeyState(VK_LBUTTON) & 0x0001)
    {
        Bullet* bullet = NewGO<Bullet>(0);

        Vector3 bulletPos = m_position;
        bulletPos.y += 80.0f;
        bulletPos.z += 100.0f;

        bullet->SetPosition(bulletPos);

        bullet->SetDirection(
            Vector3(0.0f, 0.0f, 1.0f)
        );
    }

    // =====================================
    // モデル更新
    // =====================================

    m_modelRender.SetPosition(m_position);
    m_modelRender.Update();
}

void Player::Render(RenderContext& rc)
{
    m_modelRender.Draw(rc);
}