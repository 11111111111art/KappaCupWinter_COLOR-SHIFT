#include "stdafx.h"
#include "Sky.h"

namespace
{
    const float SKY_BASE_Y = -300.0f;   // ドームの中心の高さ(地平線が少し下に来るように)
}

bool Sky::Start()
{
    m_model.Init("Assets/modelData/stage/sky.tkm");
    Update();
    return true;
}

void Sky::Update()
{
    // 横はカメラについていく(高さは固定)
    Vector3 pos = g_camera3D->GetPosition();
    pos.y = SKY_BASE_Y;

    m_model.SetPosition(pos);
    m_model.Update();
}

void Sky::Render(RenderContext& rc)
{
    m_model.Draw(rc);
}
