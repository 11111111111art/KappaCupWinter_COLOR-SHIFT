#include "stdafx.h"
#include "Robot.h"
#include "Player.h"
#include <cmath>
#include "GameClear.h"
#include "physics/Physics.h"
#include "HitFxManager.h"
#include "EnemySpawner.h"
#include "EnemyBulletManager.h"
#include <cstdlib>
namespace
{
    // ---- ロボットの体の当たり判定(カプセル = 太さのある線)
    //      ロボットのモデルの座標で書いてある。x = 横, y = 上, z = 前、足元が原点
    struct HitCapsule
    {
        Vector3 a;      // 片方の端
        Vector3 b;      // もう片方の端
        float radius;   // 太さ(半径)
    };

    const HitCapsule ROBOT_HITBOXES[] =
    {
        // 頭、胴体(縦)、胸(横)、腰、背中
        { Vector3(0.0f, 256.0f, 4.0f),    Vector3(0.0f, 292.0f, 4.0f),    22.0f },
        { Vector3(0.0f, 175.0f, 0.0f),    Vector3(0.0f, 248.0f, 0.0f),    36.0f },
        { Vector3(-32.0f, 218.0f, 0.0f),  Vector3(32.0f, 218.0f, 0.0f),   36.0f },
        { Vector3(-40.0f, 154.0f, 0.0f),  Vector3(40.0f, 154.0f, 0.0f),   22.0f },
        { Vector3(0.0f, 195.0f, -42.0f),  Vector3(0.0f, 245.0f, -42.0f),  25.0f },
        // 片方の腕: 肩、上腕、腕の銃
        { Vector3(74.0f, 232.0f, -4.0f),  Vector3(74.0f, 212.0f, -4.0f),  28.0f },
        { Vector3(74.0f, 212.0f, -4.0f),  Vector3(74.0f, 160.0f, -4.0f),  15.0f },
        { Vector3(74.0f, 160.0f, -18.0f), Vector3(74.0f, 166.0f, 110.0f), 18.0f },
        // もう片方の腕
        { Vector3(-74.0f, 232.0f, -4.0f),  Vector3(-74.0f, 212.0f, -4.0f),  28.0f },
        { Vector3(-74.0f, 212.0f, -4.0f),  Vector3(-74.0f, 160.0f, -4.0f),  15.0f },
        { Vector3(-74.0f, 160.0f, -18.0f), Vector3(-74.0f, 166.0f, 110.0f), 18.0f },
        // 片方の足: 太もも、すね、足
        { Vector3(40.0f, 150.0f, 0.0f),   Vector3(40.0f, 89.0f, 0.0f),    20.0f },
        { Vector3(40.0f, 89.0f, 0.0f),    Vector3(40.0f, 20.0f, -6.0f),   18.0f },
        { Vector3(40.0f, 10.0f, -25.0f),  Vector3(40.0f, 10.0f, 40.0f),   18.0f },
        // もう片方の足
        { Vector3(-40.0f, 150.0f, 0.0f),  Vector3(-40.0f, 89.0f, 0.0f),   20.0f },
        { Vector3(-40.0f, 89.0f, 0.0f),   Vector3(-40.0f, 20.0f, -6.0f),  18.0f },
        { Vector3(-40.0f, 10.0f, -25.0f), Vector3(-40.0f, 10.0f, 40.0f),  18.0f },
    };

    const float BULLET_RADIUS = 5.0f;   // 弾の太さ(少し当たりやすくする)

    // ---- 色の相性
    const float WEAK_RATE = 2.0f;       // 同じ色の弾: 弱点(2倍)
    const float RESIST_RATE = 0.25f;    // 違う色の弾: 耐性(1/4)

    // ロボットの色(ダメージ数字とHPバーで使う)
    const Vector4 WEAK_FONT_COLOR[3] =
    {
        Vector4(1.0f, 0.35f, 0.25f, 1.0f),   // 赤
        Vector4(0.3f, 0.75f, 1.0f, 1.0f),    // 青
        Vector4(1.0f, 0.9f, 0.2f, 1.0f),     // 黄
    };
    const Vector4 RESIST_FONT_COLOR(0.6f, 0.6f, 0.6f, 1.0f);

    // ---- HPバー
    const char* HP_SPRITE = "Assets/sprite/hp_white.dds";
    const float HP_BAR_W = 140.0f;            // 横の長さ(画面のピクセル)
    const float HP_BAR_H = 12.0f;             // 太さ
    const float HP_BAR_HEIGHT = 320.0f;       // 足元からどれくらい上に出すか(大きさ1のとき)
    const float HP_BAR_MAX_DIST = 6000.0f;    // これより遠いと出さない
    const float HP_TRAIL_SPEED = 0.6f;        // 白い残像が減る速さ(1秒あたり)
    // ---- 倒れる演出
    const float DEATH_FALL_START = 0.4f;     // 倒れ始める時間
    const float DEATH_FALL_END = 1.3f;       // 地面に倒れ切る時間(ここで爆発)
    const float DEATH_TIME = 2.0f;           // 消える時間
    const float DEATH_FALL_ANGLE = 85.0f;    // 倒れる角度
    const float DEATH_SHAKE = 4.0f;          // 震える強さ
    const float DEATH_SPARK_INTERVAL = 0.12f;// 火花が出る間隔
    const float DEATH_SINK = 50.0f;          // 最後に沈む量
    const float ENEMY_SPREAD = 0.02f;   // 弾のバラつき(大きいほど外れやすい)
    const float ENEMY_LEAD = 0.6f;      // 動く先をどれだけ読むか(0=読まない, 1=完全に読む)
    float RandomRange(float lo, float hi)
    {
        return lo + (hi - lo) * (float)rand() / (float)RAND_MAX;
    }
    float Clamp01(float v)
    {
        return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
    }

    // 2本の線分の、一番近いところの距離の2乗。
    // outS には、1本目の線分の上で一番近いところの割合(0〜1)を入れる
    float SegmentDistanceSq(const Vector3& p1, const Vector3& q1,
        const Vector3& p2, const Vector3& q2, float* outS = nullptr)
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
            if (outS != nullptr) *outS = 0.0f;
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

        if (outS != nullptr) *outS = s;
        Vector3 diff = (p1 + d1 * s) - (p2 + d2 * t);
        return diff.Dot(diff);
    }
}

bool Robot::Start()
{
    // 色に合わせてモデルを選ぶ
    const char* modelPath = "Assets/modelData/Enemy/enemy_red.tkm";

    if (m_color == 1)
    {
        modelPath = "Assets/modelData/Enemy/enemy_blue.tkm";
    }
    else if (m_color == 2)
    {
        modelPath = "Assets/modelData/Enemy/enemy_yellow.tkm";
    }

    m_modelRender.Init(modelPath);
    m_modelRender.SetScale(Vector3(m_scale, m_scale, m_scale));

    m_rotation.SetRotationDeg(Vector3::AxisY, 180.0f);
    m_modelRender.SetRotation(m_rotation);

    // 位置は SetStartPosition で決める
    m_modelRender.SetPosition(m_position);
    m_modelRender.Update();

    m_player = FindGO<Player>("player");

    m_damageFont.SetScale(1.5f);
    m_damageFont.SetColor(g_vec4White);
    m_damageFont.SetText(L"");

    // ---- HPバー
    m_hpBack.Init(HP_SPRITE, HP_BAR_W + 6.0f, HP_BAR_H + 6.0f);
    m_hpBack.SetMulColor(Vector4(0.05f, 0.05f, 0.07f, 0.85f));

    m_hpTrail.Init(HP_SPRITE, HP_BAR_W, HP_BAR_H);
    m_hpTrail.SetPivot(Vector2(0.0f, 0.5f));   // 左はしを基準にして、右から縮める
    m_hpTrail.SetMulColor(Vector4(1.0f, 1.0f, 1.0f, 0.9f));

    m_hpFill.Init(HP_SPRITE, HP_BAR_W, HP_BAR_H);
    m_hpFill.SetPivot(Vector2(0.0f, 0.5f));
    m_hpFill.SetMulColor(WEAK_FONT_COLOR[m_color]);   // ロボットの色

    UpdateHpBar();
    return true;
}

void Robot::Update()
{
        // 倒れてる途中は演出だけ
        if (m_dying)
        {
            UpdateDeath();
            return;
        }
        // ---- プレイヤーの動きを覚えておく(先読みに使う)
        if (m_player != nullptr)
        {
            Vector3 p = m_player->GetPosition();
            Vector3 v = (p - m_prevPlayerPos) * 60.0f;
            v.y = 0.0f;

            // 急な値の変化(ワープなど)をならす
            m_playerVel = m_playerVel * 0.8f + v * 0.2f;
            m_prevPlayerPos = p;
        }
        // ...ここから下は今のまま
        if (m_player != nullptr && !m_player->IsDead())
        {
            Vector3 targetPos = m_player->GetPosition();
            Vector3 direction = targetPos - m_position;
            direction.y = 0.0f;

            float distance = direction.Length();

            // ---- いつもプレイヤーのほうを向く(攻撃範囲の外でも)
            if (distance > 0.01f)
            {
                float angle = atan2f(direction.x, direction.z);

                // 向きを覚えておく(当たり判定も一緒に回すため)
                m_rotation.SetRotationDeg(
                    Vector3::AxisY,
                    angle * 180.0f / 3.14159265f
                );

                m_modelRender.SetRotation(m_rotation);
            }

            // ---- 攻撃は範囲内のときだけ
            if (distance <= m_attackDistance)
            {
                m_attackTimer += 1.0f / 60.0f;

                if (m_attackTimer >= m_attackInterval)
                {
                    m_attackTimer = 0.0f;

                    if (CanSeePlayer())
                    {
                        FireAtPlayer();   // 弾を撃つ(当たるかどうかは弾しだい)
                    }
                }
            }
            else
            {
                m_attackTimer = 0.0f;
            }
        }
        else
        {
            m_attackTimer = 0.0f;
        }
    

    // ---- ダメージの数字
    if (m_damageDisplayTimer > 0.0f)
    {
        m_damageDisplayTimer -= 1.0f / 60.0f;
        m_damageOffsetY += 50.0f / 60.0f;

        // 表示が終わったら数字を消して、合計もリセット
        if (m_damageDisplayTimer <= 0.0f)
        {
            m_damageFont.SetText(L"");
            m_displayDamage = 0;
        }
    }

    m_modelRender.SetPosition(m_position);
    m_modelRender.Update();

    UpdateHpBar();
}

void Robot::Render(RenderContext& rc)
{
    m_modelRender.Draw(rc);

    if (m_hpVisible)
    {
        m_hpBack.Draw(rc);
        m_hpTrail.Draw(rc);
        m_hpFill.Draw(rc);

        if (m_damageDisplayTimer > 0.0f)
        {
            m_damageFont.Draw(rc);
        }
    }
}

void Robot::TakeDamage(int damage, int bulletColor)
{
    // もう倒れてたら何もしない
    if (m_hp <= 0)
    {
        return;
    }

    // ---- 色の相性
    Vector4 fontColor = g_vec4White;

    if (bulletColor == m_color)
    {
        // 同じ色: 弱点
        damage = (int)(damage * WEAK_RATE);
        fontColor = WEAK_FONT_COLOR[m_color];
    }
    else if (bulletColor >= 0)
    {
        // 違う色: 耐性
        damage = (int)(damage * RESIST_RATE);
        fontColor = RESIST_FONT_COLOR;
    }

    if (damage < 1)
    {
        damage = 1;
    }

    m_hp -= damage;

    m_displayDamage += damage;
    m_damageDisplayTimer = 1.0f;
    m_damageOffsetY = 0.0f;

    wchar_t damageText[32];
    swprintf_s(damageText, L"%d", m_displayDamage);

    m_damageFont.SetText(damageText);
    m_damageFont.SetColor(fontColor);
    // 数字の位置は UpdateHpBar でバーの横に合わせる

    if (m_hp <= 0)
    {
        m_hp = 0;
        StartDeath();   // すぐ消さずに、倒れる演出を始める
    }
}

void Robot::UpdateHpBar()
{
    // 頭の上の位置
    Vector3 head = m_position;
    head.y += HP_BAR_HEIGHT * m_scale;

    // カメラの後ろ、または遠すぎるときは出さない
    Vector3 toHead = head - g_camera3D->GetPosition();
    m_hpVisible = toHead.Dot(g_camera3D->GetForward()) > 0.0f
        && toHead.Length() < HP_BAR_MAX_DIST;

    if (!m_hpVisible)
    {
        // ---- 壁の向こうにいるときは出さない(胸か頭のどちらかが見えていれば出す)
        {
            Vector3 eye = g_camera3D->GetPosition();
            Vector3 chest = m_position;
            chest.y += 200.0f * m_scale;

            auto blocked = [&](const Vector3& to)
                {
                    Vector3 hitPos;

                    if (PhysicsWorld::GetInstance()->RayTest(eye, to, hitPos))
                    {
                        return (hitPos - eye).Length() < (to - eye).Length() - 30.0f;
                    }

                    return false;
                };

            if (blocked(chest) && blocked(head))
            {
                m_hpVisible = false;
                return;
            }
        }
    }

    // 3Dの位置 → 画面の位置(真ん中が0、上がプラス)
    g_camera3D->CalcScreenPositionFromWorldPosition(m_hpScreenPos, head);

    float rate = Clamp01((float)m_hp / (float)m_maxHP);

    // 白い残像は少し遅れて追いかける
    if (m_hpTrailRate > rate)
    {
        m_hpTrailRate -= HP_TRAIL_SPEED / 60.0f;

        if (m_hpTrailRate < rate)
        {
            m_hpTrailRate = rate;
        }
    }
    else
    {
        m_hpTrailRate = rate;
    }

    float left = m_hpScreenPos.x - HP_BAR_W * 0.5f;

    m_hpBack.SetPosition(Vector3(m_hpScreenPos.x, m_hpScreenPos.y, 0.0f));
    m_hpBack.Update();

    m_hpTrail.SetPosition(Vector3(left, m_hpScreenPos.y, 0.0f));
    m_hpTrail.SetScale(Vector3(m_hpTrailRate, 1.0f, 1.0f));
    m_hpTrail.Update();

    m_hpFill.SetPosition(Vector3(left, m_hpScreenPos.y, 0.0f));
    m_hpFill.SetScale(Vector3(rate, 1.0f, 1.0f));
    m_hpFill.Update();

    // ダメージの数字はバーの右上に出して、ふわっと上がる
    m_damageFont.SetPosition(
        m_hpScreenPos.x + HP_BAR_W * 0.5f + 12.0f,
        m_hpScreenPos.y + 30.0f + m_damageOffsetY,
        0.0f
    );
}

bool Robot::HitTest(const Vector3& start, const Vector3& end, Vector3* hitPos) const
{
    bool hit = false;
    float nearest = 2.0f;   // 弾の線分の上で、一番手前で当たったところ(0〜1)

    for (const HitCapsule& cap : ROBOT_HITBOXES)
    {
        Vector3 a = cap.a * m_scale;
        Vector3 b = cap.b * m_scale;
        m_rotation.Apply(a);
        m_rotation.Apply(b);
        a += m_position;
        b += m_position;

        float r = cap.radius * m_scale + BULLET_RADIUS;
        float s = 0.0f;

        if (SegmentDistanceSq(start, end, a, b, &s) < r * r)
        {
            hit = true;

            if (s < nearest)
            {
                nearest = s;
            }
        }
    }

    if (hit && hitPos != nullptr)
    {
        *hitPos = start + (end - start) * nearest;
    }

    return hit;
}

bool Robot::CanSeePlayer()
{
    if (m_player == nullptr)
    {
        return false;
    }

    Vector3 start = m_position;
    Vector3 end = m_player->GetPosition();

    start.y += 100.0f;
    end.y += 180.0f - 70.0f * m_player->GetLowRate();   // しゃがむと頭が低くなる

    btVector3 rayStart(start.x, start.y, start.z);
    btVector3 rayEnd(end.x, end.y, end.z);

    btCollisionWorld::ClosestRayResultCallback callback(
        rayStart,
        rayEnd
    );

    PhysicsWorld::GetInstance()->GetDynamicWorld()->rayTest(
        rayStart,
        rayEnd,
        callback
    );

    if (!callback.hasHit())
    {
        return true;
    }

    if (callback.m_collisionObject->isStaticObject())
    {
        return false;
    }

    return true;
}

void Robot::StartDeath()
{
    m_dying = true;
    m_exploded = false;
    m_deathTimer = 0.0f;
    m_deathSparkTimer = 0.0f;
    m_hpVisible = false;

    // 今向いている方向(横だけ)を覚えておく
    m_deathForward = Vector3::AxisZ;
    m_rotation.Apply(m_deathForward);
    m_deathForward.y = 0.0f;

    if (m_deathForward.Length() < 0.001f)
    {
        m_deathForward = Vector3::AxisZ;
    }

    m_deathForward.Normalize();
    m_deathBasePos = m_position;
}

Vector3 Robot::RandomBodyPoint() const
{
    // 当たり判定のカプセルのどれかの上の、どこか1点
    const int count = sizeof(ROBOT_HITBOXES) / sizeof(ROBOT_HITBOXES[0]);
    const HitCapsule& cap = ROBOT_HITBOXES[rand() % count];

    Vector3 p = cap.a + (cap.b - cap.a) * RandomRange(0.0f, 1.0f);
    p *= m_scale;
    m_rotation.Apply(p);
    p += m_position;
    return p;
}

void Robot::UpdateDeath()
{
    const float dt = 1.0f / 60.0f;
    m_deathTimer += dt;
    m_hpVisible = false;

    HitFxManager* fx = FindGO<HitFxManager>("hitFx");

    if (fx == nullptr)
    {
        fx = NewGO<HitFxManager>(10, "hitFx");
    }

    // ---- 倒れる角度(だんだん速くなる)
    float angle = 0.0f;

    if (m_deathTimer > DEATH_FALL_START)
    {
        float k = (m_deathTimer - DEATH_FALL_START) / (DEATH_FALL_END - DEATH_FALL_START);

        if (k > 1.0f)
        {
            k = 1.0f;
        }

        angle = DEATH_FALL_ANGLE * k * k;
    }

    // ---- 後ろ向きに倒す(足元を軸に回す)
    float rad = angle * 3.14159265f / 180.0f;
    Vector3 f = m_deathForward;
    Vector3 up = Vector3::AxisY;
    Vector3 right;
    right.Cross(up, f);
    right.Normalize();

    Vector3 up2 = up * cosf(rad) - f * sinf(rad);
    Vector3 f2 = f * cosf(rad) + up * sinf(rad);

    Matrix rotMat = Matrix::Identity;
    rotMat.m[0][0] = right.x; rotMat.m[0][1] = right.y; rotMat.m[0][2] = right.z;
    rotMat.m[1][0] = up2.x;   rotMat.m[1][1] = up2.y;   rotMat.m[1][2] = up2.z;
    rotMat.m[2][0] = f2.x;    rotMat.m[2][1] = f2.y;    rotMat.m[2][2] = f2.z;
    m_rotation.SetRotation(rotMat);

    // ---- 位置: 倒れるまではガタガタ震えて、倒れたあとは沈む
    m_position = m_deathBasePos;

    if (m_deathTimer < DEATH_FALL_END)
    {
        float shake = DEATH_SHAKE * m_scale;
        m_position.x += RandomRange(-shake, shake);
        m_position.z += RandomRange(-shake, shake);
    }
    else
    {
        float k = (m_deathTimer - DEATH_FALL_END) / (DEATH_TIME - DEATH_FALL_END);
        m_position.y -= DEATH_SINK * k;
    }

    // 体の真ん中(火花の向きと最後の爆発で使う)
    Vector3 center = Vector3(0.0f, 200.0f * m_scale, 0.0f);
    m_rotation.Apply(center);
    center += m_position;

    if (m_deathTimer < DEATH_FALL_END)
    {
        // ---- 体のあちこちから火花
        m_deathSparkTimer -= dt;

        if (m_deathSparkTimer <= 0.0f)
        {
            m_deathSparkTimer = DEATH_SPARK_INTERVAL;

            Vector3 p = RandomBodyPoint();
            fx->Spawn(p, center - p, m_color, false, 1.3f);
        }

        // ---- 小さな爆発が連鎖する
        m_deathBoomTimer -= dt;

        if (m_deathBoomTimer <= 0.0f)
        {
            m_deathBoomTimer = RandomRange(0.25f, 0.4f);
            fx->SpawnExplosion(RandomBodyPoint(), m_color, 0.55f);
        }
    }

    // ---- 倒れた瞬間に大爆発(体にそって3か所)
    if (!m_exploded && m_deathTimer >= DEATH_FALL_END)
    {
        m_exploded = true;

        for (int i = 0; i < 2; i++)
        {
            Vector3 p = m_deathBasePos - f * (60.0f + 110.0f * i) * m_scale;
            p.y += 40.0f * m_scale;
            fx->SpawnExplosion(p, m_color, 1.1f);
        }

        fx->AddShake(12.0f);
    }

    m_modelRender.SetRotation(m_rotation);
    m_modelRender.SetPosition(m_position);
    m_modelRender.Update();

    // ---- 最後にもう一発、一番大きい爆発で消える。最後の1体ならクリア
        // ---- 最後にもう一発、一番大きい爆発で消える
    if (m_deathTimer >= DEATH_TIME)
    {
        fx->SpawnExplosion(center, m_color, 1.8f);
        fx->AddShake(16.0f);

        // 倒した数はスポナーが数える(クリアの判定もスポナー)
        EnemySpawner* spawner = FindGO<EnemySpawner>("enemySpawner");

        if (spawner != nullptr)
        {
            spawner->OnRobotKilled();
        }

        DeleteGO(this);
    }
}

void Robot::FireAtPlayer()
{
    EnemyBulletManager* bullets = FindGO<EnemyBulletManager>("enemyBullet");

    if (bullets == nullptr || m_player == nullptr)
    {
        return;
    }

    // 右と左の腕の銃口から、交互に撃つ
    float side = (m_gunSide == 0) ? 1.0f : -1.0f;
    m_gunSide = 1 - m_gunSide;

    Vector3 muzzle(74.0f * side, 166.0f, 125.0f);   // 腕の銃の先(ロボットのモデルの座標)
    muzzle *= m_scale;
    m_rotation.Apply(muzzle);
    muzzle += m_position;

    // プレイヤーの胸を狙う。弾が届くまでに動く先を、少しだけ読む
    Vector3 target = m_player->GetPosition();
    target.y += 100.0f;

    float flightTime = (target - muzzle).Length() / GameConfig::ENEMY_SHOT_SPEED;
    target += m_playerVel * flightTime * ENEMY_LEAD;

    Vector3 dir = target - muzzle;
    dir.Normalize();

    // 少しだけバラつかせる
    dir.x += RandomRange(-ENEMY_SPREAD, ENEMY_SPREAD);
    dir.y += RandomRange(-ENEMY_SPREAD, ENEMY_SPREAD);
    dir.z += RandomRange(-ENEMY_SPREAD, ENEMY_SPREAD);
    dir.Normalize();

    bullets->Fire(muzzle, dir, m_color, m_attackDamage);

    // 銃口がピカッと光る
    HitFxManager* fx = FindGO<HitFxManager>("hitFx");

    if (fx != nullptr)
    {
        fx->SpawnFlash(muzzle, m_color, 0.8f);
    }
}