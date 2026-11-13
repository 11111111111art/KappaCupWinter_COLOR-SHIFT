#pragma once
#include "EffectBase.h"
class DamageEffect : public EffectBase
{
public:
    bool IsPlaying() const
    {
        return m_isPlaying;
    }
    bool Start();
    void Update();
    void Render(RenderContext& rc);

    void Play();

private:
    SpriteRender m_spriteRender;

    float m_timer = 0.0f;
    float m_duration = 0.5f;
    bool m_isPlaying = false;
};
