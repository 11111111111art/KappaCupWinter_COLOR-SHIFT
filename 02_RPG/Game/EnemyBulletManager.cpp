#include "stdafx.h"
#include "EnemyBulletManager.h"
#include "Player.h"
#include "HitFxManager.h"
#include <cmath>
#include "GameConfig.h"
namespace
{
    const float PLAYER_LOW_SHRINK = 60.0f;  // しゃがむと頭の高さがこれだけ下がる
    const float SHOT_SPEED = 2500.0f;      // 弾の速さ(プレイヤーの弾より遅く、避けられるくらい)
    const float SHOT_LIFE = GameConfig::ATTACK_RANGE / SHOT_SPEED + 0.1f;   // 届く距離ぶんだけ飛ぶ
    const float SHOT_SCALE = 1.6f;         // 弾の見た目の大きさ
    const float SHOT_RADIUS = 10.0f;       // 弾の当たりの太さ

    // プレイヤーの体(CharacterController と同じ大きさ: 半径40、足元から高さ200)
    const float PLAYER_RADIUS = 40.0f;
    const float PLAYER_BOTTOM = 40.0f;     // カプセルの下の芯の高さ
    const float PLAYER_TOP = 160.0f;       // カプセルの上の芯の高さ

    const char* SHOT_MODEL[3] =
    {
        "Assets/modelData/stage/bulletRed.tkm",
        "Assets/modelData/stage/bulletBlue.tkm",
        "Assets/modelData/stage/bullet.tkm",
    };

    float Clamp01(float v)
    {
        return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
    }

    // 2本の線分の、一番近いところの距離の2乗
    float SegmentDistanceSq(const Vector3& p1, const Vector3& q1,
        const Vector3& p2, const Vector3& q2)
    {
        Vector3 d1 = q1 - p1;
        Vector3 d2 = q2 - p2;
        Vector3 r = p1 - p2;
        float a = d1.Dot(d1);
        float e = d2.Dot(d2);
        float f = d2.Dot(r);
        float s = 0.0f;
        float t = 0.0f;

        if (a <= 1.0e-6f && e <= 1.0e-6f)
        {
            return r.Dot(r);
        }

        if (a <= 1.0e-6f)
        {
            t = Clamp01(f / e);
        }
        else
        {
            float c = d1.Dot(r);

            if (e <= 1.0e-6f)
            {
                s = Clamp01(-c / a);
            }
            else
            {
                float b = d1.Dot(d2);
                float denom = a * e - b * b;
                s = (denom != 0.0f) ? Clamp01((b * f - c * e) / denom) : 0.0f;
                t = (b * s + f) / e;

                if (t < 0.0f)
                {
                    t = 0.0f;
                    s = Clamp01(-c / a);
                }
                else if (t > 1.0f)
                {
                    t = 1.0f;
                    s = Clamp01((b - c) / a);
                }
            }
        }

        Vector3 diff = (p1 + d1 * s) - (p2 + d2 * t);
        return diff.Dot(diff);
    }

    // 飛んでいく向きに向ける(プレイヤーの弾と同じやり方)
    Quaternion LookRotation(Vector3 fwd)
    {
        fwd.Normalize();
        Vector3 right;
        right.Cross(Vector3::AxisY, fwd);

        if (right.Length() < 0.001f)
        {
            right = Vector3::AxisX;
        }

        right.Normalize();
        Vector3 up;
        up.Cross(fwd, right);

        Matrix m = Matrix::Identity;
        m.m[0][0] = right.x; m.m[0][1] = right.y; m.m[0][2] = right.z;
        m.m[1][0] = up.x;    m.m[1][1] = up.y;    m.m[1][2] = up.z;
        m.m[2][0] = fwd.x;   m.m[2][1] = fwd.y;   m.m[2][2] = fwd.z;

        Quaternion q;
        q.SetRotation(m);
        return q;
    }
}

bool EnemyBulletManager::Start()
{
    // 2個目以降は消える
    if (FindGO<EnemyBulletManager>("enemyBullet") != this)
    {
        DeleteGO(this);
        return true;
    }

    InitPool();
    return true;
}

void EnemyBulletManager::InitPool()
{
    if (m_ready)
    {
        return;
    }

    for (int c = 0; c < COLOR_COUNT; c++)
    {
        for (Shot& s : m_shots[c])
        {
            s.render.Init(SHOT_MODEL[c]);
        }
    }

    m_ready = true;
}

void EnemyBulletManager::Fire(const Vector3& pos, const Vector3& dir, int color, int damage)
{
    InitPool();

    if (color < 0 || color >= COLOR_COUNT)
    {
        color = 2;
    }

    // 空いてる弾(一番早く消えるやつ)を使う
    Shot* shot = &m_shots[color][0];

    for (Shot& s : m_shots[color])
    {
        if (s.life < shot->life)
        {
            shot = &s;
        }
    }

    shot->pos = pos;
    shot->dir = dir;
    shot->dir.Normalize();
    shot->life = SHOT_LIFE;
    shot->damage = damage;
    ApplyShot(*shot);
}

void EnemyBulletManager::Update()
{
    if (!m_ready)
    {
        return;
    }

    const float dt = 1.0f / 60.0f;
    Player* player = FindGO<Player>("player");
    HitFxManager* fx = FindGO<HitFxManager>("hitFx");

    for (int c = 0; c < COLOR_COUNT; c++)
    {
        for (Shot& s : m_shots[c])
        {
            if (s.life <= 0.0f)
            {
                continue;
            }

            s.life -= dt;

            Vector3 oldPos = s.pos;
            s.pos += s.dir * SHOT_SPEED * dt;

            // ---- 壁・床
            Vector3 wallHitPos;
            bool hitWall = PhysicsWorld::GetInstance()->RayTest(oldPos, s.pos, wallHitPos);
            float wallDist = hitWall ? (wallHitPos - oldPos).Length() : 1.0e9f;

            // ---- プレイヤー
            if (player != nullptr && !player->IsDead())
            {
                Vector3 bottom = player->GetPosition();
                Vector3 top = bottom;
                bottom.y += PLAYER_BOTTOM;
                top.y += PLAYER_TOP - PLAYER_LOW_SHRINK * player->GetLowRate();

                float r = PLAYER_RADIUS + SHOT_RADIUS;

                if (SegmentDistanceSq(oldPos, s.pos, bottom, top) < r * r)
                {
                    // 壁より手前でプレイヤーに当たった
                    Vector3 toPlayer = (bottom + top) * 0.5f - oldPos;

                    if (toPlayer.Length() < wallDist + r)
                    {
                        player->TakeDamage(s.damage);
                        s.life = 0.0f;
                        continue;
                    }
                }
            }

            if (hitWall)
            {
                if (fx != nullptr)
                {
                    fx->Spawn(wallHitPos, s.dir, c, true, 0.7f);
                }

                s.life = 0.0f;
                continue;
            }

            if (s.life > 0.0f)
            {
                ApplyShot(s);
            }
        }
    }
}

void EnemyBulletManager::ApplyShot(Shot& s)
{
    s.render.SetPosition(s.pos);
    s.render.SetRotation(LookRotation(s.dir));
    s.render.SetScale(Vector3(SHOT_SCALE, SHOT_SCALE, SHOT_SCALE));
    s.render.Update();
}

void EnemyBulletManager::Render(RenderContext& rc)
{
    if (!m_ready)
    {
        return;
    }

    for (int c = 0; c < COLOR_COUNT; c++)
    {
        for (Shot& s : m_shots[c])
        {
            if (s.life > 0.0f)
            {
                s.render.Draw(rc);
            }
        }
    }
}