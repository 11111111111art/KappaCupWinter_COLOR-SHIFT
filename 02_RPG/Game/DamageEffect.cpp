
#include "stdafx.h"
#include "DamageEffect.h"

bool DamageEffect::Start()
{
    m_spriteRender.Init(
        "Assets/sprite/damageEffect.dds",
        static_cast<float>(FRAME_BUFFER_W),
        static_cast<float>(FRAME_BUFFER_H)
    );

    m_spriteRender.SetPivot(Vector2(0.5f, 0.5f));
    m_spriteRender.SetPosition(Vector3::Zero);

    m_spriteRender.SetMulColor(
        Vector4(1.0f, 1.0f, 1.0f, 0.0f)
    );

    m_spriteRender.Update();

    return true;
}



void DamageEffect::Update()
{
    EffectBase::Update();

    float alpha = 0.0f;

    if (m_isPlaying)
    {
        alpha = m_timer / m_duration;
    }

    m_spriteRender.SetMulColor(
        Vector4(1.0f, 1.0f, 1.0f, alpha)
    );

    m_spriteRender.Update();
}

void DamageEffect::Render(RenderContext& rc)
{
    if (m_isPlaying)
    {
        m_spriteRender.Draw(rc);
    }
}

void DamageEffect::Play()
{
    EffectBase::Play();

    m_spriteRender.SetMulColor(
        Vector4(1.0f, 1.0f, 1.0f, 1.0f)
    );

    m_spriteRender.Update();
}
