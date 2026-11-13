#pragma once

class Player;

// 今の弾の色を画面に出す(照準の真ん中の点 + 右下の3色のチップ)
class WeaponColorUI : public IGameObject
{
public:
    bool Start() override;
    void Update() override;
    void Render(RenderContext& rc) override;

private:
    Player* m_player = nullptr;

    SpriteRender m_centerDot;       // 照準の真ん中の点
    SpriteRender m_chipBack;        // チップの後ろの黒い帯
    SpriteRender m_chips[3];        // 赤・青・黄のチップ
    SpriteRender m_selectFrame;     // 選んでる色の白い枠

    int m_color = -1;               // 今の色(0=赤, 1=青, 2=黄)
    float m_chipScale[3] = { 1.0f, 1.0f, 1.0f };
    float m_popTimer = 0.0f;        // 切り替えた瞬間に少し大きくする
};

