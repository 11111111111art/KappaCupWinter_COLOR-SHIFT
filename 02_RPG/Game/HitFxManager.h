#pragma once

// 当たったとき・爆発したときのエフェクトをまとめて管理する(最初に読み込んで使い回す)
class HitFxManager : public IGameObject
{
public:
    // 着弾: color 0=赤, 1=青, 2=黄 / hitWall: 壁・床なら true(土ぼこり) / power: 大きさ
    void Spawn(const Vector3& pos, const Vector3& bulletDir, int color, bool hitWall, float power = 1.0f);

    // 爆発: 火の玉 + 閃光 + 全方向の火花 + 土ぼこり
    void SpawnExplosion(const Vector3& pos, int color, float power = 1.0f);

    // 画面を揺らす(大きいほど強い)
    void AddShake(float power);

    bool Start() override;
    void Update() override;
    void Render(RenderContext& rc) override;
    // 閃光だけ出す(銃口の光など)
    void SpawnFlash(const Vector3& pos, int color, float size)
    {
        InitPool();

        if (color < 0 || color >= COLOR_COUNT)
        {
            color = 2;
        }

        StartFlash(color, pos, size);
    }
private:
    static const int COLOR_COUNT = 3;
    static const int SPARKS_PER_COLOR = 12;
    static const int FLASHES_PER_COLOR = 4;
    static const int FIREBALLS_PER_COLOR = 3;
    static const int DUST_COUNT = 6;

    struct Spark
    {
        ModelRender render;
        Vector3 pos;
        Vector3 vel;
        float life = 0.0f;
        float maxLife = 0.0f;
        float size = 1.0f;
    };

    struct Flash
    {
        ModelRender render;
        Vector3 pos;
        float timer = 0.0f;
        float roll = 0.0f;
        float size = 1.0f;
    };

    struct Fireball
    {
        ModelRender render;
        Vector3 pos;
        float timer = 0.0f;
        float roll = 0.0f;
        float spin = 0.0f;
        float size = 1.0f;
    };

    struct Dust
    {
        ModelRender render;
        Vector3 pos;
        float timer = 0.0f;
        float roll = 0.0f;
        float size = 1.0f;
    };

    void InitPool();
    void ShootSpark(int color, const Vector3& pos, const Vector3& dir, float speedMul, float size);
    void StartFlash(int color, const Vector3& pos, float size);
    void StartDust(const Vector3& pos, float size);
    void ApplySpark(Spark& s);
    void ApplyFlash(Flash& f);
    void ApplyFireball(Fireball& f);
    void ApplyDust(Dust& d);

    Spark m_sparks[COLOR_COUNT][SPARKS_PER_COLOR];
    Flash m_flashes[COLOR_COUNT][FLASHES_PER_COLOR];
    Fireball m_fireballs[COLOR_COUNT][FIREBALLS_PER_COLOR];
    Dust m_dusts[DUST_COUNT];
    float m_shake = 0.0f;
    bool m_ready = false;
};