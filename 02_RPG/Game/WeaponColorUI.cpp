#include "stdafx.h"
#include "WeaponColorUI.h"
#include "Player.h"

namespace
{
    const char* WHITE_SPRITE = "Assets/sprite/hp_white.dds";

    // ---- 照準の真ん中の点
    const float DOT_SIZE = 6.0f;

    // ---- 右下のチップ(画面の真ん中が 0、右と上がプラス)
    const float CHIP_X = 730.0f;        // 3つのチップの真ん中の位置
    const float CHIP_Y = -255.0f;
    const float CHIP_SIZE = 34.0f;      // 選んでないチップの大きさ
    const float CHIP_GAP = 52.0f;       // チップどうしの間隔
    const float SELECT_SCALE = 1.45f;   // 選んでるチップの大きさ(倍)
    const float POP_TIME = 0.15f;       // 切り替えたときにポンと大きくなる時間

    const Vector4 COLORS[3] =
    {
        Vector4(1.0f, 0.3f, 0.2f, 1.0f),    // 赤
        Vector4(0.25f, 0.7f, 1.0f, 1.0f),   // 青
        Vector4(1.0f, 0.88f, 0.15f, 1.0f),  // 黄
    };

    // 選んでないチップは暗くする
    Vector4 Dim(const Vector4& c)
    {
        return Vector4(c.x * 0.4f, c.y * 0.4f, c.z * 0.4f, 0.8f);
    }
}

bool WeaponColorUI::Start()
{
    m_player = FindGO<Player>("player");

    m_centerDot.Init(WHITE_SPRITE, DOT_SIZE, DOT_SIZE);

    m_chipBack.Init(WHITE_SPRITE, CHIP_GAP * 3.0f + 16.0f, CHIP_SIZE * SELECT_SCALE + 16.0f);
    m_chipBack.SetMulColor(Vector4(0.0f, 0.0f, 0.0f, 0.45f));
    m_chipBack.SetPosition(Vector3(CHIP_X, CHIP_Y, 0.0f));
    m_chipBack.Update();

    m_selectFrame.Init(WHITE_SPRITE, CHIP_SIZE + 6.0f, CHIP_SIZE + 6.0f);
    m_selectFrame.SetMulColor(Vector4(1.0f, 1.0f, 1.0f, 0.95f));

    for (int i = 0; i < 3; i++)
    {
        m_chips[i].Init(WHITE_SPRITE, CHIP_SIZE, CHIP_SIZE);
    }

    Update();
    return true;
}

void WeaponColorUI::Update()
{
    if (m_player == nullptr)
    {
        m_player = FindGO<Player>("player");

        if (m_player == nullptr)
        {
            return;
        }
    }

    const float dt = 1.0f / 60.0f;
    int color = m_player->GetWeaponColor();

    // 色が変わった瞬間
    if (color != m_color)
    {
        m_color = color;
        m_popTimer = POP_TIME;
    }

    if (m_popTimer > 0.0f)
    {
        m_popTimer -= dt;
    }

    // ---- 照準の真ん中の点
    m_centerDot.SetMulColor(COLORS[m_color]);
    m_centerDot.SetPosition(Vector3::Zero);
    m_centerDot.Update();

    // ---- 右下の3色
    for (int i = 0; i < 3; i++)
    {
        bool selected = (i == m_color);

        // 選んだチップはなめらかに大きく、ほかは小さく
        float target = selected ? SELECT_SCALE : 1.0f;
        m_chipScale[i] += (target - m_chipScale[i]) * 0.3f;

        float scale = m_chipScale[i];

        if (selected && m_popTimer > 0.0f)
        {
            scale += 0.35f * (m_popTimer / POP_TIME);   // ポンッ
        }

        Vector3 pos(CHIP_X + (i - 1) * CHIP_GAP, CHIP_Y, 0.0f);

        m_chips[i].SetMulColor(selected ? COLORS[i] : Dim(COLORS[i]));
        m_chips[i].SetPosition(pos);
        m_chips[i].SetScale(Vector3(scale, scale, 1.0f));
        m_chips[i].Update();

        if (selected)
        {
            m_selectFrame.SetPosition(pos);
            m_selectFrame.SetScale(Vector3(scale, scale, 1.0f));
            m_selectFrame.Update();
        }
    }
}

void WeaponColorUI::Render(RenderContext& rc)
{
    if (m_player == nullptr || m_color < 0)
    {
        return;
    }

    m_chipBack.Draw(rc);

    // 白い枠 → その上に色のチップ(枠が少しはみ出して縁取りになる)
    m_selectFrame.Draw(rc);

    for (int i = 0; i < 3; i++)
    {
        m_chips[i].Draw(rc);
    }

    m_centerDot.Draw(rc);
}