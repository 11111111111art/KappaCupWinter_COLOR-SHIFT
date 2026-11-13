#pragma once

// ゲーム全体で使う数字
namespace GameConfig
{
    // 攻撃が届く距離(ロボットが撃ってくる距離 = 自分の弾が届く距離 = ロボットの弾が届く距離)
    constexpr float ATTACK_RANGE = 3000.0f;
    // ロボットの弾の速さ
    constexpr float ENEMY_SHOT_SPEED = 2400.0f;
}

