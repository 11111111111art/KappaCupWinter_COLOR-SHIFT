#include "stdafx.h"
#include "Bullet.h"

bool Bullet::Start()
{
    m_modelRender.Init("Assets/modelData/stage/bullet.tkm");

    m_modelRender.SetPosition(m_position);
    m_modelRender.Update();

    return true;
}

void Bullet::Update()
{
    // 弾を移動
    m_position += m_direction * m_speed * (1.0f / 60.0f);

    // 3秒後に消す
    m_lifeTime -= 1.0f / 60.0f;

    if (m_lifeTime <= 0.0f)
    {
        DeleteGO(this);
        return;
    }

    m_modelRender.SetPosition(m_position);
    m_modelRender.Update();
}

void Bullet::Render(RenderContext& rc)
{
    m_modelRender.Draw(rc);
}