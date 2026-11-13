#include "stdafx.h"
#include "AmmoUI.h"
#include "Player.h"
#include <cstdio>

bool AmmoUI::Start()
{
    m_player = FindGO<Player>("player");

    m_ammoFont.SetPosition(Vector3(600.0f, -300.0f, 0.0f));
    m_ammoFont.SetScale(1.5f);
    m_ammoFont.SetColor(g_vec4White);

    m_reloadFont.SetPosition(Vector3(600.0f, -400.0f, 0.0f));
    m_reloadFont.SetScale(0.8f);
    m_reloadFont.SetColor(g_vec4White);

    return true;
}


void AmmoUI::Update()
{
    m_player = FindGO<Player>("player");

    if (m_player == nullptr)
    {
        return;
    }

    wchar_t ammoText[32];

    swprintf_s(
        ammoText,
        32,
        L"%d / %d",
        m_player->GetAmmo(),
        m_player->GetMaxAmmo()
    );

    m_ammoFont.SetText(ammoText);

    if (m_player->IsReloading())
    {
        m_reloadFont.SetText(L"RELOADING...");
    }
    else
    {
        m_reloadFont.SetText(L"");
    }
}


void AmmoUI::Render(RenderContext& rc)
{
    m_ammoFont.Draw(rc);
    m_reloadFont.Draw(rc);
}

