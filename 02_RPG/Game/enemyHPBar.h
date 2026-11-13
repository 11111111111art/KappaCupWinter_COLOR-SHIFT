#pragma once

class Robot;

class EnemyHPBar : public IGameObject
{
public:
    bool Start();
    void Update();
    void Render(RenderContext& rc);

private:
    SpriteRender m_spriteRender;
    Robot* m_robot = nullptr;
};



