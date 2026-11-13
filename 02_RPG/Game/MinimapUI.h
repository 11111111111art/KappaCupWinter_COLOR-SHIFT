#pragma once

// 左上のミニマップ(北が上の固定マップ。自分とロボットを点で出す)
class MinimapUI : public IGameObject
{
public:
    bool Start() override;
    void Update() override;
    void Render(RenderContext& rc) override;

private:
    static const int MAX_ROBOT_DOTS = 8;

    // ゲームの位置 → ミニマップ上の画面の位置
    Vector3 WorldToMap(const Vector3& pos) const;

    SpriteRender m_frame;                       // 黒いふち
    SpriteRender m_map;                         // 地図
    SpriteRender m_playerDot;                   // 自分
    SpriteRender m_playerDir;                   // 自分の向き
    SpriteRender m_robotDots[MAX_ROBOT_DOTS];   // ロボット
    int m_robotDotCount = 0;
};

