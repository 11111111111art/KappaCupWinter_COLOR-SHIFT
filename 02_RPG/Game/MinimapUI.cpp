#include "stdafx.h"
#include "MinimapUI.h"
#include "Player.h"
#include "Robot.h"
#include "GameCamera.h"
#include <cmath>

namespace
{
    const char* MAP_SPRITE = "Assets/sprite/minimap.dds";
    const char* WHITE_SPRITE = "Assets/sprite/hp_white.dds";

    // ---- 画面に出す場所と大きさ(画面の真ん中が 0、右と上がプラス)
    const float MAP_W = 240.0f;
    const float MAP_H = 320.0f;
    const float MAP_CENTER_X = 820.0f;     // 左上
    const float MAP_CENTER_Y = 340.0f;

    // ---- 地図に入っているステージの範囲(ゲームの座標)
    const float WORLD_X_MIN = -3000.0f;
    const float WORLD_X_MAX = 3000.0f;
    const float WORLD_Z_MIN = -4000.0f;
    const float WORLD_Z_MAX = 4000.0f;

    const float PLAYER_DOT = 10.0f;
    const float DIR_DOT = 5.0f;
    const float DIR_DIST = 11.0f;           // 向きの点を自分からどれだけ離すか
    const float ROBOT_DOT = 9.0f;

    const Vector4 ROBOT_COLORS[3] =
    {
        Vector4(1.0f, 0.3f, 0.2f, 1.0f),    // 赤
        Vector4(0.25f, 0.7f, 1.0f, 1.0f),   // 青
        Vector4(1.0f, 0.88f, 0.15f, 1.0f),  // 黄
    };

    float Clamp(float v, float lo, float hi)
    {
        return v < lo ? lo : (v > hi ? hi : v);
    }
}

bool MinimapUI::Start()
{
    m_frame.Init(WHITE_SPRITE, MAP_W + 8.0f, MAP_H + 8.0f);
    m_frame.SetMulColor(Vector4(0.0f, 0.0f, 0.0f, 0.6f));
    m_frame.SetPosition(Vector3(MAP_CENTER_X, MAP_CENTER_Y, 0.0f));
    m_frame.Update();

    m_map.Init(MAP_SPRITE, MAP_W, MAP_H);
    m_map.SetMulColor(Vector4(1.0f, 1.0f, 1.0f, 0.85f));   // ちょっとだけ透ける
    m_map.SetPosition(Vector3(MAP_CENTER_X, MAP_CENTER_Y, 0.0f));
    m_map.Update();

    m_playerDot.Init(WHITE_SPRITE, PLAYER_DOT, PLAYER_DOT);
    m_playerDir.Init(WHITE_SPRITE, DIR_DOT, DIR_DOT);
    m_playerDir.SetMulColor(Vector4(1.0f, 1.0f, 1.0f, 0.9f));

    for (int i = 0; i < MAX_ROBOT_DOTS; i++)
    {
        m_robotDots[i].Init(WHITE_SPRITE, ROBOT_DOT, ROBOT_DOT);
    }

    return true;
}

Vector3 MinimapUI::WorldToMap(const Vector3& pos) const
{
    // 地図は「スタートで前を向いたときに上が前」の向き
    //   右 = ゲームの -x、上 = ゲームの -z
    float u = (WORLD_X_MAX - pos.x) / (WORLD_X_MAX - WORLD_X_MIN);   // 0=左, 1=右
    float v = (pos.z - WORLD_Z_MIN) / (WORLD_Z_MAX - WORLD_Z_MIN);   // 0=上, 1=下

    u = Clamp(u, 0.0f, 1.0f);
    v = Clamp(v, 0.0f, 1.0f);

    float left = MAP_CENTER_X - MAP_W * 0.5f;
    float top = MAP_CENTER_Y + MAP_H * 0.5f;

    return Vector3(left + u * MAP_W, top - v * MAP_H, 0.0f);
}

void MinimapUI::Update()
{
    // ---- 自分
    Player* player = FindGO<Player>("player");

    if (player != nullptr)
    {
        Vector3 p = WorldToMap(player->GetPosition());
        m_playerDot.SetPosition(p);
        m_playerDot.Update();

        // 向いている方向(カメラの向き)
        GameCamera* camera = FindGO<GameCamera>("gameCamera");

        if (camera != nullptr)
        {
            float a = camera->GetAngle() * 3.14159265f / 180.0f;
            float fx = sinf(a);
            float fz = cosf(a);

            // 地図の上では 右 = -x、上 = -z
            Vector3 d(-fx, -fz, 0.0f);
            m_playerDir.SetPosition(p + d * DIR_DIST);
            m_playerDir.Update();
        }
    }

    // ---- ロボット
    m_robotDotCount = 0;

    QueryGOs<Robot>("robot", [&](Robot* robot)
        {
            if (m_robotDotCount >= MAX_ROBOT_DOTS)
            {
                return false;
            }

            SpriteRender& dot = m_robotDots[m_robotDotCount];
            Vector4 c = ROBOT_COLORS[robot->GetColor()];

            // 倒れてる途中は暗く
            if (robot->IsDead())
            {
                c = Vector4(c.x * 0.35f, c.y * 0.35f, c.z * 0.35f, 0.7f);
            }

            dot.SetMulColor(c);
            dot.SetPosition(WorldToMap(robot->GetPosition()));
            dot.Update();

            m_robotDotCount++;
            return true;
        });
}

void MinimapUI::Render(RenderContext& rc)
{
    m_frame.Draw(rc);
    m_map.Draw(rc);

    for (int i = 0; i < m_robotDotCount; i++)
    {
        m_robotDots[i].Draw(rc);
    }

    m_playerDir.Draw(rc);
    m_playerDot.Draw(rc);
}
