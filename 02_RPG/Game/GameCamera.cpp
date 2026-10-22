#include "stdafx.h"
#include "GameCamera.h"
#include "Player.h"

GameCamera::GameCamera()
{
}

GameCamera::~GameCamera()
{
}

bool GameCamera::Start()
{
    // カメラの初期位置
    m_toCameraPos.Set(0.0f, 200.0f, -500.0f);

    startToCameraPos = m_toCameraPos;

    // プレイヤーを取得
    m_player = FindGO<Player>("player");

    // カメラの描画範囲
    g_camera3D->SetNear(1.0f);
    g_camera3D->SetFar(10000.0f);

    return true;
}

void GameCamera::Update()
{
    if (m_player == nullptr)
    {
        return;
    }

    // =====================================
    // プレイヤーの位置を取得
    // =====================================

    Vector3 target = m_player->GetPosition();

    target.y += 80.0f;

    // 回転前の位置を保存
    Vector3 toCameraPosOld = m_toCameraPos;

    // =====================================
    // 右スティックでカメラ回転
    // =====================================

    float x = g_pad[0]->GetRStickXF();
    float y = g_pad[0]->GetRStickYF();

    Quaternion qRot;

    // 左右回転
    qRot.SetRotationDeg(
        Vector3::AxisY,
        2.0f * x
    );

    qRot.Apply(m_toCameraPos);

    // 上下回転
    Vector3 axisX;

    axisX.Cross(
        Vector3::AxisY,
        m_toCameraPos
    );

    axisX.Normalize();

    qRot.SetRotationDeg(
        axisX,
        2.0f * y
    );

    qRot.Apply(m_toCameraPos);

    // =====================================
    // カメラの上下回転制限
    // =====================================

    Vector3 toPosDir = m_toCameraPos;

    toPosDir.Normalize();

    if (toPosDir.y < -0.2f)
    {
        m_toCameraPos = toCameraPosOld;
    }
    else if (toPosDir.y > 0.9f)
    {
        m_toCameraPos = toCameraPosOld;
    }

    // =====================================
    // カメラの位置を設定
    // =====================================

    Vector3 pos = target + m_toCameraPos;

    g_camera3D->SetTarget(target);
    g_camera3D->SetPosition(pos);

    g_camera3D->Update();
}

void GameCamera::ResetCamera()
{
    m_toCameraPos.Set(
        0.0f,
        200.0f,
        -500.0f
    );
}