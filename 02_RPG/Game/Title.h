#pragma once

class Title : public IGameObject
{
public:
    bool Start();
    void Update();
    void Render(RenderContext& rc);

private:
    SpriteRender m_spriteRender;
    bool m_isStarting = false;
};
