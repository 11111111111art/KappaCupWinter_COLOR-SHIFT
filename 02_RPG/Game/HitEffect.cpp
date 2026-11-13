#include "stdafx.h"
#include "HitEffect.h"

bool HitEffect::Start()
{
    m_duration = 0.3f;

    m_spriteRender.Init("Assets/sprite/hitEffect.dds",180.0f,180.0f );

    m_spriteRender.SetPivot(Vector2(0.5f, 0.5f));
    m_spriteRender.SetPosition(Vector3::Zero);
    m_spriteRender.Update();

    Play();

    return true;
}

void HitEffect::Update()
{
    EffectBase::Update();

    if (!m_isPlaying)
    {
        DeleteGO(this);
        return;
    }

    Vector2 screenPos;

    g_camera3D->CalcScreenPositionFromWorldPosition(
        screenPos,
        m_position
    );

    float alpha = m_timer / m_duration;

    m_spriteRender.SetPosition(
        Vector3(screenPos.x, screenPos.y, 0.0f)
    );

    m_spriteRender.SetMulColor(
        Vector4(1.0f, 1.0f, 1.0f, alpha)
    );

    m_spriteRender.Update();
}

void HitEffect::Render(RenderContext& rc)
{
    /*if (m_isPlaying)
    {
        m_spriteRender.Draw(rc);
    }*/
}

void HitEffect::SetPosition(Vector3 pos)
{
    m_position = pos;
}

void HitEffect::Play()
{
    EffectBase::Play();

    m_spriteRender.SetMulColor(
        Vector4(1.0f, 1.0f, 1.0f, 1.0f)
    );
}

