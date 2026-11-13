#pragma once

// 弾が当たったところから飛び散る火花(弾のモデルを細くして使い回す)
class HitSparks : public IGameObject
{
public:
    // pos: 当たった場所 / bulletDir: 弾が飛んできた向き / modelPath: 火花に使うモデル
    void Init(const Vector3& pos, const Vector3& bulletDir, const char* modelPath);
    void Update() override;
    void Render(RenderContext& rc) override;

private:
    static const int SPARK_COUNT = 8;

    struct Spark
    {
        ModelRender render;
        Vector3 pos;
        Vector3 vel;
        float life = 0.0f;
        float maxLife = 0.0f;
    };

    Spark m_sparks[SPARK_COUNT];
    bool m_initialized = false;
};

