#pragma once

// ロボットを出して、倒した数を数える
class EnemySpawner : public IGameObject
{
public:
    bool Start() override;
    void Update() override;
    void Render(RenderContext& rc) override;

    // ロボットが倒れ終わったときに呼ぶ
    void OnRobotKilled();

private:
    void SpawnRobot(const Vector3& pos, int color, bool withEffect);
    int PickSpawnPoint() const;     // 空いてる出現ポイントを選ぶ(なければ -1)
    void UpdateText();
    FontRender m_missionText;       // 開始時の目的
    float m_missionTimer = 4.0f;    // 目的を出しておく時間(秒)
    int m_killCount = 0;            // 倒した数
    int m_spawnCount = 0;           // 出した数
    float m_respawnTimer = 0.0f;    // 次を出すまでの時間
    int m_waiting = 0;              // 出す予定の数
    bool m_cleared = false;
    FontRender m_text;
};

