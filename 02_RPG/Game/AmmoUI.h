#pragma once

class Player;

class AmmoUI : public IGameObject
{
public:
	bool Start();
	void Update();
	void Render(RenderContext& rc);

private:
	FontRender m_ammoFont;
	FontRender m_reloadFont;

	Player* m_player = nullptr;
};

