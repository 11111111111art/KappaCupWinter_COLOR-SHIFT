#include "stdafx.h"
#include "Game.h"
#include "Player.h"
#include "GameCamera.h"
#include "BackGround.h"
#include "Robot.h"
#include "PlayerHPBar.h"
#include "DamageEffect.h"
#include "Crosshair.h"
#include "AmmoUI.h"
#include "HitFxManager.h"
#include "EnemySpawner.h"
#include "WeaponColorUI.h"
#include "EnemyBulletManager.h"
#include "MinimapUI.h"
#include "Sky.h"
#include "Sound.h"
bool Game::Start()
{
    Sound::Init();
    Sound::PlayBGM(BGM_BATTLE, 0.35f);
   /* NewGO<Player>(0, "player");
    NewGO<GameCamera>(0, "gameCamera");
    NewGO<BackGround>(0, "background");
    NewGO<PlayerHPBar>(0, "playerHPBar");
    NewGO<Crosshair>(0, "crosshair");
    NewGO<WeaponColorUI>(0, "weaponColorUI");
    NewGO<EnemyBulletManager>(0, "enemyBullet");
    NewGO<MinimapUI>(0, "minimapUI");
    NewGO<Sky>(0, "sky");*/

    // エフェクト(カメラのあとに動かしたいので 10)
    //NewGO<HitFxManager>(10, "hitFx");

    //// ロボットを出して、倒した数を数える(最初の3体もここで出る)
    //NewGO<EnemySpawner>(0, "enemySpawner");

    if (FindGO<AmmoUI>("ammoUI") == nullptr)
    {
        /*NewGO<AmmoUI>(0, "ammoUI");*/
    }

    return true;
}

Game::~Game()
{
    Sound::StopBGM();
    // Game が作ったものを、名前で探して全部消す(2個以上あっても全部)
    const char* names[] =
    {
        "player",
        "gameCamera",
        "background",
        "playerHPBar",
        "crosshair",
        "ammoUI",
        "hitFx",
        "enemySpawner",
        "enemyBullet",
        "weaponColorUI",
        "minimapUI",
    };

    for (const char* name : names)
    {
        QueryGOs<IGameObject>(name, [&](IGameObject* go)
            {
                DeleteGO(go);
                return true;
            });
    }

    // ロボットも全部
    QueryGOs<Robot>("robot", [&](Robot* robot)
        {
            DeleteGO(robot);
            return true;
        });
}
void Game::Update()
{
}

void Game::Render(RenderContext& rc)
{
}
