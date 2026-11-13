#pragma once
#include "EffectBase.h"

class HitEffect : public EffectBase
{
public:
    bool Start();
    void Update();
    void Render(RenderContext& rc);

    void SetPosition(Vector3 pos);
    void Play() override;

private:
    SpriteRender m_spriteRender;
    Vector3 m_position = Vector3::Zero;
};

