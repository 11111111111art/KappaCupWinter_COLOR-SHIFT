#pragma once

class Bullet : public IGameObject
{
public:
    bool Start();
    void Update();
    void Render(RenderContext& rc);

    void SetPosition(Vector3 pos)
    {
        m_position = pos;
    }

    void SetDirection(Vector3 dir)
    {
        m_direction = dir;
    }

private:
    ModelRender m_modelRender;

    Vector3 m_position = Vector3::Zero;
    Vector3 m_direction = Vector3::Zero;

    float m_speed = 1000.0f;
    float m_lifeTime = 3.0f;
};