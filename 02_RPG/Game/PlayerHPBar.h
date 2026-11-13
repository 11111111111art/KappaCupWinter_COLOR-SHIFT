#pragma once

class Player;

// 左下のプレイヤーのHPバー(ネオンのわく + 右から減る + 白い残像 + 少ないと赤く点滅)
class PlayerHPBar : public IGameObject
{
public:
    bool Start();
    void Update();
    void Render(RenderContext& rc);

private:
    Player* m_player = nullptr;

    SpriteRender m_back;        // バーの下の暗い板
    SpriteRender m_trail;       // 削られた分の白い残像
    SpriteRender m_fill;        // 今のHP
    SpriteRender m_frame;       // ネオンのわく(いちばん上に重ねる)
    FontRender m_number;        // 「75 / 100」

    float m_trailRate = 1.0f;   // 残像の長さ(0〜1)
    int m_prevHP = -1;          // 前のフレームのHP(減ったかどうかを見る)
    float m_hitTimer = 0.0f;    // ダメージを受けた瞬間の光
    float m_time = 0.0f;        // 点滅用
};