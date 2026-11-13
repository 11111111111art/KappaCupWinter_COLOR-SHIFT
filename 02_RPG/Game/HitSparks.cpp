#include "stdafx.h"
#include "HitSparks.h"
#include <cstdlib>
#include <cmath>

namespace
{
    const float SPARK_SPEED_MIN = 350.0f;   // 飛び散る速さ
    const float SPARK_SPEED_MAX = 800.0f;
    const float SPARK_LIFE_MIN = 0.18f;     // 消えるまでの時間(秒)
    const float SPARK_LIFE_MAX = 0.35f;
    const float SPARK_GRAVITY = 1200.0f;    // 下に落ちる強さ
    const float SPARK_WIDTH = 0.25f;        // 火花の太さ(弾モデルに対する倍率)
    const float SPARK_LENGTH = 0.5f;        // 火花の長さ(弾モデルに対する倍率)

    float RandomRange(float lo, float hi)
    {
        return lo + (hi - lo) * (float)rand() / (float)RAND_MAX;
    }

    // dir の方向を向く回転を作る(弾と同じやり方)
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

void HitSparks::Init(const Vector3& pos, const Vector3& bulletDir, const char* modelPath)
{
    // 弾が来た方向に跳ね返るように飛ばす
    Vector3 back = bulletDir * -1.0f;
    back.Normalize();

    for (Spark& s : m_sparks)
    {
        Vector3 v(RandomRange(-1.0f, 1.0f), RandomRange(-1.0f, 1.0f), RandomRange(-1.0f, 1.0f));

        if (v.Length() < 0.01f)
        {
            v = back;
        }

        v.Normalize();

        // 壁の向こう側に飛ばないよう、手前側に折り返す
        if (v.Dot(back) < 0.0f)
        {
            v -= back * (2.0f * v.Dot(back));
        }

        Vector3 dir = back * 0.5f + v;
        dir.Normalize();

        s.pos = pos;
        s.vel = dir * RandomRange(SPARK_SPEED_MIN, SPARK_SPEED_MAX);
        s.maxLife = RandomRange(SPARK_LIFE_MIN, SPARK_LIFE_MAX);
        s.life = s.maxLife;

        s.render.Init(modelPath);
    }

    m_initialized = true;
}

void HitSparks::Update()
{
    if (!m_initialized)
    {
        return;
    }

    const float dt = 1.0f / 60.0f;
    bool alive = false;

    for (Spark& s : m_sparks)
    {
        if (s.life <= 0.0f)
        {
            continue;
        }

        s.life -= dt;

        if (s.life <= 0.0f)
        {
            continue;
        }

        alive = true;

        // 重力で落ちて、空気でちょっとずつ遅くなる
        s.vel.y -= SPARK_GRAVITY * dt;
        s.vel *= 0.97f;
        s.pos += s.vel * dt;

        // 残り時間に合わせて細く・短くなって消える
        float t = s.life / s.maxLife;
        float speedRate = s.vel.Length() / SPARK_SPEED_MAX;
        float w = SPARK_WIDTH * t;
        float l = SPARK_LENGTH * (0.4f + speedRate) * t;

        s.render.SetPosition(s.pos);
        s.render.SetRotation(LookRotation(s.vel));
        s.render.SetScale(Vector3(w, w, l));
        s.render.Update();
    }

    // 全部消えたら自分も消える
    if (!alive)
    {
        DeleteGO(this);
    }
}

void HitSparks::Render(RenderContext& rc)
{
    if (!m_initialized)
    {
        return;
    }

    for (Spark& s : m_sparks)
    {
        if (s.life > 0.0f)
        {
            s.render.Draw(rc);
        }
    }
}