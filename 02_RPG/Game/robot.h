#pragma once
#include "GameConfig.h"
class Player;

class Robot : public IGameObject
{
public:
    bool Start();
    void Update();
    void Render(RenderContext& rc);

    void TakeDamage(int damage, int bulletColor = -1);
    // 弾が start から end まで飛んだときに、体のどこかに当たったか。
    // 当たったら hitPos に当たった場所を入れる(いらなければ nullptr)
    bool HitTest(const Vector3& start, const Vector3& end, Vector3* hitPos = nullptr) const;

    // 色: 0=赤, 1=青, 2=黄(NewGO したあと、Start より前に呼ぶ)
    void SetColor(int color)
    {
        m_color = color;
    }

    int GetColor() const
    {
        return m_color;
    }

    // 出てくる場所(NewGO したあと、Start より前に呼ぶ)
    void SetStartPosition(const Vector3& pos)
    {
        m_position = pos;
    }

    bool IsDead() const
    {
        return m_hp <= 0;
    }

    const Vector3& GetPosition() const
    {
        return m_position;
    }

    int GetHP() const
    {
        return m_hp;
    }

    int GetMaxHP() const
    {
        return m_maxHP;
    }

private:
    ModelRender m_modelRender;
    Player* m_player = nullptr;
    void FireAtPlayer();     // プレイヤーに向けて弾を撃つ
    int m_gunSide = 0;       // 次に撃つ腕(0=右, 1=左)
    int m_color = 0;                                // 0=赤, 1=青, 2=黄
    Vector3 m_position = Vector3::Zero;
    Quaternion m_rotation = Quaternion::Identity;   // 今の向き(当たり判定でも使う)
    float m_scale = 1.3f;                           // 大きさ(当たり判定でも使う)
    // ---- 頭の上のHPバー
    void UpdateHpBar();

    SpriteRender m_hpBack;          // 黒い枠
    SpriteRender m_hpTrail;         // 削られた分の白い残像
    SpriteRender m_hpFill;          // 今のHP(ロボットの色)
    float m_hpTrailRate = 1.0f;     // 残像の長さ(0〜1)
    bool m_hpVisible = false;       // 画面に出すか
    Vector2 m_hpScreenPos;          // バーの画面の位置
    float m_moveSpeed = 100.0f;
    float m_stopDistance = 200.0f;
    bool CanSeePlayer();
    int m_hp = 100;
    int m_maxHP = 100;
    float m_attackDistance = GameConfig::ATTACK_RANGE;   // この距離より近いと撃ってくる
    float m_attackTimer = 0.0f;
    float m_attackInterval = 0.55f;
    int m_attackDamage = 10;
    int m_displayDamage = 0;
    float m_damageDisplayTimer = 0.0f;
    float m_damageOffsetY = 0.0f;
    float m_damageFontX = 200.0f;                   // ダメージ数字の横位置(色ごとにずらす)
    // ---- 倒れる演出
    void StartDeath();
    void UpdateDeath();
    Vector3 RandomBodyPoint() const;    // 体のどこかの位置(火花を出す場所)

    bool m_dying = false;               // 倒れてる途中
    bool m_exploded = false;            // 倒れたときの爆発を出したか
    float m_deathTimer = 0.0f;
    float m_deathSparkTimer = 0.0f;
    Vector3 m_deathForward;             // 倒れる前に向いていた方向
    Vector3 m_deathBasePos;             // 倒れる前の位置
    float m_deathBoomTimer = 0.0f;      // 連鎖爆発の間隔
    FontRender m_damageFont;
    Vector3 m_prevPlayerPos;            // プレイヤーの前のフレームの位置
    Vector3 m_playerVel;                // プレイヤーの動く速さ(先読み用)
};