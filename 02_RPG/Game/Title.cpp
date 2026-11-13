#include "stdafx.h"
#include "Title.h"
#include "Game.h"

bool Title::Start()
{
    m_spriteRender.Init(
        "Assets/sprite/title.dds",
        static_cast<float>(FRAME_BUFFER_W),
        static_cast<float>(FRAME_BUFFER_H)
    );

    m_spriteRender.SetPosition(Vector3::Zero);
    m_spriteRender.SetPivot(Vector2(0.5f, 0.5f));
    m_spriteRender.Update();

    return true;
}


void Title::Update()
{
    if (m_isStarting)
    {
        return;
    }

    if (g_pad[0] != nullptr)
    {
        if (g_pad[0]->IsTrigger(enButtonA))
        {
            m_isStarting = true;

            NewGO<Game>(0, "game");

            DeleteGO(this);
        }
    }
}

void Title::Render(RenderContext& rc)
{
    m_spriteRender.Draw(rc);
}
