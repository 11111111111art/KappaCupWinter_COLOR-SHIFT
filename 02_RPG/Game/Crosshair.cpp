#include "stdafx.h"
#include "Crosshair.h"

bool Crosshair::Start()
{
    m_spriteRender.Init("Assets/sprite/crosshair.dds",80.0f,80.0f );
    m_spriteRender.SetPivot( Vector2(0.5f, 0.5f) );
    m_spriteRender.SetPosition(Vector3(0.0f, 0.0f, 0.0f) );
    m_spriteRender.Update();
    return true;
}

void Crosshair::Update()
{

}

void Crosshair::Render(RenderContext& rc)
{
	m_spriteRender.Draw(rc);
}