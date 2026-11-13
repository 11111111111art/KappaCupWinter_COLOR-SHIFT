#pragma once

class Bullet : public IGameObject
{
public:
    enum class BulletType
    {
        Red,
        Blue,
        Yellow
    };

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

    void SetBulletType(BulletType type)
    {
        m_bulletType = type;
    }

    void Activate(Vector3 pos, Vector3 dir)
    {
        m_position = pos;
        m_direction = dir;
        m_lifeTime = 3.0f;
        m_traveled = 0.0f;
        m_isActive = true;
    }

    bool IsActive() const
    {
        return m_isActive;
    }

private:
    ModelRender m_modelRender;
    Vector3 m_position = Vector3::Zero;
    Vector3 m_direction = Vector3::Zero;

    BulletType m_bulletType = BulletType::Red;

    float m_speed = 15000.0f;
    float m_lifeTime = 3.0f;
    Quaternion  m_hitRot = Quaternion::Identity; // ランダムな向き
    bool m_modelInitialized = false;
    bool m_isActive = false;
    float m_traveled = 0.0f;    // 撃ってから飛んだ距離
};
