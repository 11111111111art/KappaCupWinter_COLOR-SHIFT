
#include "stdafx.h"
#include "EnemyHPBar.h"
#include "Robot.h"

bool EnemyHPBar::Start()
{
    m_robot = FindGO<Robot>("robot");

    m_spriteRender.Init(
        "Assets/sprite/enemyHP.dds",
        200.0f,
        30.0f
    );

    m_spriteRender.SetPivot(Vector2(0.0f, 0.5f));
    m_spriteRender.SetPosition(Vector3::Zero);
    m_spriteRender.Update();

    return true;
}

void EnemyHPBar::Update()
{
    if (m_robot == nullptr)
    {
        return;
    }

    Vector3 hpWorldPos = m_robot->GetPosition();
    hpWorldPos.y += 180.0f;

    Vector2 screenPos;

    g_camera3D->CalcScreenPositionFromWorldPosition(
        screenPos,
        hpWorldPos
    );

    float hpRate =
        static_cast<float>(m_robot->GetHP()) /
        static_cast<float>(m_robot->GetMaxHP());

    m_spriteRender.SetScale(
        Vector3(hpRate, 1.0f, 1.0f)
    );

    m_spriteRender.SetPosition(
        Vector3(
            screenPos.x - 100.0f,
            screenPos.y,
            0.0f
        )
    );

    m_spriteRender.Update();
}

void EnemyHPBar::Render(RenderContext& rc)
{
    if (m_robot != nullptr)
    {
        m_spriteRender.Draw(rc);
    }
}

