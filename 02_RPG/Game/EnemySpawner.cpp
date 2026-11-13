#include "stdafx.h"
#include "EnemySpawner.h"
#include "Robot.h"
#include "Player.h"
#include "GameClear.h"
#include "HitFxManager.h"
#include <cstdlib>

namespace
{
    const int CLEAR_COUNT = 10;          // これだけ倒したらクリア
    const int MAX_ALIVE = 3;             // 同時に出ている最大数
    const float RESPAWN_DELAY = 1.5f;    // 倒してから次が出るまで(秒)
    const float MIN_PLAYER_DIST = 800.0f;// プレイヤーにこれより近い場所には出さない

    // 最初の3体(真ん中の土台)
    const Vector3 FIRST_POS[3] =
    {
        Vector3(861.4f, 120.0f, 582.1f),     // 赤
        Vector3(-840.0f, 120.0f, 600.0f),    // 青
        Vector3(0.0f, 120.0f, 1440.0f),      // 黄
    };

    // 出現ポイント(木箱・コンテナの上)
    const Vector3 SPAWN_POINTS[] =
    {
        Vector3(-2100.0f, 260.0f,   700.0f),   // 左のコンテナ
        Vector3(0.0f, 260.0f, -1200.0f),   // 奥のコンテナ(通路の下)
        Vector3(-2200.0f, 390.0f, -2000.0f),   // 左奥の、コンテナの上の木箱
        Vector3(800.0f, 260.0f, -3200.0f),   // 一番奥のコンテナ
        Vector3(2300.0f, 560.0f,  -250.0f),   // 右の高台のコンテナ
        Vector3(2400.0f, 430.0f,  1000.0f),   // 右の高台の木箱
        Vector3(-600.0f, 130.0f,  -850.0f),   // 真ん中左の木箱
        Vector3(-450.0f, 130.0f, -1550.0f),   // 奥の木箱
        Vector3(-700.0f, 130.0f, -3100.0f),   // 一番奥の木箱
        Vector3(1660.0f, 260.0f, -2510.0f),   // 右奥の積んだ木箱
    };
    const int SPAWN_POINT_COUNT = sizeof(SPAWN_POINTS) / sizeof(SPAWN_POINTS[0]);
}

bool EnemySpawner::Start()
{
    // 一番最初に作られたやつだけ残る。2個目以降は消える
    if (FindGO<EnemySpawner>("enemySpawner") != this)
    {
        DeleteGO(this);
        return true;
    }

    

        // ...ここから下は今のまま(最初の3体を出す)
    // 最初の3体
    for (int i = 0; i < 3; i++)
    {
        SpawnRobot(FIRST_POS[i], i, false);
    }

    m_text.SetPosition(-120.0f, 480.0f, 0.0f);
    m_text.SetScale(1.2f);
    m_text.SetColor(g_vec4White);
    // 「占領された研究所を奪還せよ」
    m_missionText.SetText(L"\u5360\u9818\u3055\u308C\u305F\u7814\u7A76\u6240\u3092\u596A\u9084\u305B\u3088");
    m_missionText.SetPosition(-420.0f, 260.0f, 0.0f);      // ロボットにかぶらないよう上へ
    m_missionText.SetScale(1.6f);
    m_missionText.SetColor(Vector4(1.0f, 0.75f, 0.4f, 1.0f));
    m_missionText.SetShadowParam(true, 2.0f, Vector4(0.0f, 0.0f, 0.0f, 1.0f));   // 黒いふち
    UpdateText();
    return true;
}

void EnemySpawner::SpawnRobot(const Vector3& pos, int color, bool withEffect)
{
    Robot* robot = NewGO<Robot>(0, "robot");
    robot->SetColor(color);
    robot->SetStartPosition(pos);
    m_spawnCount++;

    if (withEffect)
    {
        HitFxManager* fx = FindGO<HitFxManager>("hitFx");

        if (fx != nullptr)
        {
            Vector3 p = pos;
            p.y += 150.0f;
            fx->SpawnExplosion(p, color, 0.6f);
        }
    }
}

int EnemySpawner::PickSpawnPoint() const
{
    Player* player = FindGO<Player>("player");

    // 使える場所を集める
    int candidates[SPAWN_POINT_COUNT];
    int num = 0;

    for (int i = 0; i < SPAWN_POINT_COUNT; i++)
    {
        const Vector3& p = SPAWN_POINTS[i];

        // プレイヤーの近くはダメ
        if (player != nullptr)
        {
            Vector3 d = player->GetPosition() - p;
            d.y = 0.0f;

            if (d.Length() < MIN_PLAYER_DIST)
            {
                continue;
            }
        }

        // ほかのロボットがいる場所はダメ(倒れてる途中のやつも)
        bool used = false;

        QueryGOs<Robot>("robot", [&](Robot* robot)
            {
                Vector3 d = robot->GetPosition() - p;
                d.y = 0.0f;

                if (d.Length() < 200.0f)
                {
                    used = true;
                    return false;
                }

                return true;
            });

        if (!used)
        {
            candidates[num] = i;
            num++;
        }
    }

    if (num == 0)
    {
        return -1;
    }

    return candidates[rand() % num];
}

void EnemySpawner::OnRobotKilled()
{
    if (m_cleared)
    {
        return;
    }

    m_killCount++;
    UpdateText();

    if (m_killCount >= CLEAR_COUNT)
    {
        m_cleared = true;
        NewGO<GameClear>(0, "gameClear");
        return;
    }

    // まだ出せるなら、次を出す予定にする
    if (m_spawnCount + m_waiting < CLEAR_COUNT)
    {
        m_waiting++;

        if (m_respawnTimer <= 0.0f)
        {
            m_respawnTimer = RESPAWN_DELAY;
        }
    }
}

void EnemySpawner::Update()
{
    // ---- 開始時の目的(だんだん消える)
    if (m_missionTimer > 0.0f)
    {
        m_missionTimer -= 1.0f / 60.0f;

        float alpha = m_missionTimer < 1.0f ? m_missionTimer : 1.0f;   // 最後の1秒で消える
        m_missionText.SetColor(Vector4(1.0f, 0.75f, 0.4f, alpha));
    }
    if (m_cleared || m_waiting <= 0)
    {
        return;
    }

    m_respawnTimer -= 1.0f / 60.0f;

    if (m_respawnTimer > 0.0f)
    {
        return;
    }

    // 同時に出てる数がいっぱいなら待つ
    int alive = 0;

    QueryGOs<Robot>("robot", [&](Robot* robot)
        {
            if (!robot->IsDead())
            {
                alive++;
            }

            return true;
        });

    if (alive >= MAX_ALIVE)
    {
        m_respawnTimer = 0.5f;
        return;
    }

    int index = PickSpawnPoint();

    if (index < 0)
    {
        // 空いてる場所がなければ、少し待ってもう一回
        m_respawnTimer = 0.5f;
        return;
    }

    SpawnRobot(SPAWN_POINTS[index], rand() % 3, true);
    m_waiting--;

    // まだ待ってるのがいれば、少しずらして出す
    m_respawnTimer = m_waiting > 0 ? 0.8f : 0.0f;
}

void EnemySpawner::UpdateText()
{
    wchar_t text[64];
     swprintf_s(text, L"\u6B8B\u308A %d \u4F53", CLEAR_COUNT - m_killCount);
    m_text.SetText(text);
}

void EnemySpawner::Render(RenderContext& rc)
{
    m_text.Draw(rc);
    if (m_missionTimer > 0.0f)
    {
        m_missionText.Draw(rc);
    }
}
