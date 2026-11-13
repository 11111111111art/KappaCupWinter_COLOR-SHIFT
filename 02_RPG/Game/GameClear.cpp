
#include "stdafx.h"
#include "GameClear.h"
#include "Title.h"
#include "Game.h"
#include "Player.h"
#include "Robot.h"
#include "GameCamera.h"
#include "BackGround.h"
#include "EnemyHPBar.h"
#include "PlayerHPBar.h"
#include "DamageEffect.h"
#include "Crosshair.h"
bool GameClear::Start()
{
    m_spriteRender.Init(
        "Assets/sprite/gameClear.dds",
        static_cast<float>(FRAME_BUFFER_W),
        static_cast<float>(FRAME_BUFFER_H)
    );

    m_spriteRender.SetPosition(Vector3::Zero);
    m_spriteRender.SetPivot(Vector2(0.5f, 0.5f));
    m_spriteRender.Update();

    return true;
}

void GameClear::Update()
{
    if (g_pad[0] == nullptr)
    {
        return;
    }

    if (g_pad[0]->IsTrigger(enButtonA))
    {
        DamageEffect* effect =
            FindGO<DamageEffect>("damageEffect");
        Crosshair* crosshair = FindGO<Crosshair>("crosshair");
        if (effect != nullptr)
        {
            DeleteGO(effect);
        }
        EnemyHPBar* enemyHP = FindGO<EnemyHPBar>("enemyHPBar");
        PlayerHPBar* playerHP = FindGO<PlayerHPBar>("playerHPBar");
        Player* player = FindGO<Player>("player");
        Robot* robot = FindGO<Robot>("robot");
        GameCamera* camera = FindGO<GameCamera>("gameCamera");
        BackGround* background = FindGO<BackGround>("background");
        Game* game = FindGO<Game>("game");
        
        if (enemyHP != nullptr)
        {
            DeleteGO(enemyHP);
        }

        if (playerHP != nullptr)
        {
            DeleteGO(playerHP);
        }

        if (player != nullptr)
        {
            DeleteGO(player);
        }

        if (robot != nullptr)
        {
            DeleteGO(robot);
        }

        if (camera != nullptr)
        {
            DeleteGO(camera);
        }

        if (background != nullptr)
        {
            DeleteGO(background);
        }

        if (game != nullptr)
        {
            DeleteGO(game);
        }

        NewGO<Title>(0, "title");

        DeleteGO(this);
    }
}

void GameClear::Render(RenderContext& rc)
{
    m_spriteRender.Draw(rc);
}
