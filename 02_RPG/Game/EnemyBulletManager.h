#pragma once

// ロボットが撃つ弾をまとめて動かす(最初に読み込んで使い回す)
class EnemyBulletManager : public IGameObject
{
public:
    // pos から dir の向きに撃つ。color: 0=赤, 1=青, 2=黄
    void Fire(const Vector3& pos, const Vector3& dir, int color, int damage);

    bool Start() override;
    void Update() override;
    void Render(RenderContext& rc) override;

private:
    static const int COLOR_COUNT = 3;
    static const int SHOTS_PER_COLOR = 8;   // 色ごとに用意しておく弾の数

    struct Shot
    {
        ModelRender render;
        Vector3 pos;
        Vector3 dir;
        float life = 0.0f;
        int damage = 0;
    };

    void InitPool();
    void ApplyShot(Shot& s);

    Shot m_shots[COLOR_COUNT][SHOTS_PER_COLOR];
    bool m_ready = false;
};

