#pragma once

class GameOver : public IGameObject
{
public:
    bool Start();
    void Update();
    void Render(RenderContext& rc);

private:
    SpriteRender m_spriteRender;
};
