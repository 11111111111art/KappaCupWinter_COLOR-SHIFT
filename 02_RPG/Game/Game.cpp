#include "stdafx.h"
#include "Game.h"
#include "Player.h"
#include "GameCamera.h"
#include "BackGround.h"


bool Game::Start()
{
	NewGO<Player>(0, "player");
	NewGO<GameCamera>(0, "gameCamera");
	NewGO<BackGround>(0, "background");
	
	return true;
}

void Game::Update()
{
	
}

void Game::Render(RenderContext& rc)
{
	
}