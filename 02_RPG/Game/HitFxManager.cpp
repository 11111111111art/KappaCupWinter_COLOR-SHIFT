#include "stdafx.h"
#include "HitFxManager.h"
#include <cstdlib>
#include <cmath>
#include "HitFxManager.h"
#include "GameCamera.h"
namespace
{
    // ---- 閃光(一瞬だけ光る)
    const float FLASH_TIME = 0.07f;
    const float FLASH_SCALE_ROBOT = 1.6f;
    const float FLASH_SCALE_WALL = 1.0f;

    // ---- 火花
    const int SPARKS_ROBOT = 8;
    const int SPARKS_WALL = 4;
    const float SPARK_SPEED_MIN = 600.0f;
    const float SPARK_SPEED_MAX = 1300.0f;
    const float SPARK_LIFE_MIN = 0.08f;
    const float SPARK_LIFE_MAX = 0.2f;
    const float SPARK_GRAVITY = 1500.0f;
    const float SPARK_WIDTH = 0.15f;
    const float SPARK_LENGTH = 0.45f;

    // ---- 土ぼこり
    const float DUST_TIME = 0.45f;
    const float DUST_SCALE = 1.4f;
    const float DUST_RISE = 40.0f;

    // ---- 爆発の火の玉
    const float FIREBALL_TIME = 0.55f;
    const float FIREBALL_SCALE = 2.2f;
    const float FIREBALL_RISE = 80.0f;
    const int EXPLOSION_SPARKS = 6;

    // ---- 画面の揺れ
    const float SHAKE_DECAY = 0.86f;        // 1フレームごとに弱くなる割合

    const char* SPARK_MODEL[3] =
    {
        "Assets/modelData/stage/bulletRed.tkm",
        "Assets/modelData/stage/bulletBlue.tkm",
        "Assets/modelData/stage/bullet.tkm",
    };

    const char* FLASH_MODEL[3] =
    {
        "Assets/modelData/stage/hit_red.tkm",
        "Assets/modelData/stage/hit_blue.tkm",
        "Assets/modelData/stage/hit_yellow.tkm",
    };

    const char* FIREBALL_MODEL[3] =
    {
        "Assets/modelData/stage/explo_red.tkm",
        "Assets/modelData/stage/explo_blue.tkm",
        "Assets/modelData/stage/explo_yellow.tkm",
    };

    const char* DUST_MODEL = "Assets/modelData/stage/hit_dust.tkm";

    float RandomRange(float lo, float hi)
    {
        return lo + (hi - lo) * (float)rand() / (float)RAND_MAX;
    }

    Vector3 RandomDir()
    {
        Vector3 v(RandomRange(-1.0f, 1.0f), RandomRange(-1.0f, 1.0f), RandomRange(-1.0f, 1.0f));

        if (v.Length() < 0.01f)
        {
            v = Vector3::AxisY;
        }

        v.Normalize();
        return v;
    }

    // fwd の方向を向いて、その軸まわりに roll だけ回した回転
    Quaternion LookRotation(Vector3 fwd, float roll)
    {
        if (fwd.Length() < 0.001f)
        {
            fwd = Vector3::AxisZ;
        }

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

        float c = cosf(roll);
        float s = sinf(roll);
        Vector3 r2 = right * c + up * s;
        Vector3 u2 = up * c - right * s;

        Matrix m = Matrix::Identity;
        m.m[0][0] = r2.x;  m.m[0][1] = r2.y;  m.m[0][2] = r2.z;
        m.m[1][0] = u2.x;  m.m[1][1] = u2.y;  m.m[1][2] = u2.z;
        m.m[2][0] = fwd.x; m.m[2][1] = fwd.y; m.m[2][2] = fwd.z;

        Quaternion q;
        q.SetRotation(m);
        return q;
    }

    Vector3 ToCamera(const Vector3& pos)
    {
        Vector3 v = g_camera3D->GetPosition() - pos;

        if (v.Length() < 0.001f)
        {
            return Vector3::AxisZ;
        }

        v.Normalize();
        return v;
    }
}

bool HitFxManager::Start()
{
    if (FindGO<HitFxManager>("hitFx") != this)
    {
        DeleteGO(this);
        return true;
    }

    InitPool();
    return true;
}

void HitFxManager::InitPool()
{
    if (m_ready)
    {
        return;
    }

    // ここで全部読み込んでおく(ゲーム中は Init しない)
    for (int c = 0; c < COLOR_COUNT; c++)
    {
        for (Spark& s : m_sparks[c])
        {
            s.render.Init(SPARK_MODEL[c]);
        }

        for (Flash& f : m_flashes[c])
        {
            f.render.Init(FLASH_MODEL[c]);
        }

        for (Fireball& f : m_fireballs[c])
        {
            f.render.Init(FIREBALL_MODEL[c]);
        }
    }

    for (Dust& d : m_dusts)
    {
        d.render.Init(DUST_MODEL);
    }

    m_ready = true;
}

// ================= 出す =================

void HitFxManager::ShootSpark(int color, const Vector3& pos, const Vector3& dir, float speedMul, float size)
{
    // 空いてるやつ(一番早く消えるやつ)を使う
    Spark* spark = &m_sparks[color][0];

    for (Spark& s : m_sparks[color])
    {
        if (s.life < spark->life)
        {
            spark = &s;
        }
    }

    Vector3 d = dir;
    d.Normalize();

    spark->pos = pos;
    spark->vel = d * RandomRange(SPARK_SPEED_MIN, SPARK_SPEED_MAX) * speedMul;
    spark->maxLife = RandomRange(SPARK_LIFE_MIN, SPARK_LIFE_MAX) * (0.6f + 0.4f * speedMul);
    spark->life = spark->maxLife;
    spark->size = size;
    ApplySpark(*spark);
}

void HitFxManager::StartFlash(int color, const Vector3& pos, float size)
{
    Flash* flash = &m_flashes[color][0];

    for (Flash& f : m_flashes[color])
    {
        if (f.timer < flash->timer)
        {
            flash = &f;
        }
    }

    flash->pos = pos;
    flash->timer = FLASH_TIME;
    flash->roll = RandomRange(0.0f, 6.2831853f);
    flash->size = size;
    ApplyFlash(*flash);
}

void HitFxManager::StartDust(const Vector3& pos, float size)
{
    Dust* dust = &m_dusts[0];

    for (Dust& d : m_dusts)
    {
        if (d.timer < dust->timer)
        {
            dust = &d;
        }
    }

    dust->pos = pos;
    dust->timer = DUST_TIME;
    dust->roll = RandomRange(0.0f, 6.2831853f);
    dust->size = size;
    ApplyDust(*dust);
}

void HitFxManager::Spawn(const Vector3& pos, const Vector3& bulletDir, int color, bool hitWall, float power)
{
    InitPool();

    if (color < 0 || color >= COLOR_COUNT)
    {
        color = 2;
    }

    Vector3 back = bulletDir * -1.0f;

    if (back.Length() < 0.001f)
    {
        back = Vector3::AxisY;
    }

    back.Normalize();

    StartFlash(color, pos, (hitWall ? FLASH_SCALE_WALL : FLASH_SCALE_ROBOT) * power);

    if (hitWall)
    {
        StartDust(pos + back * 4.0f, power);
    }

    // 火花: 弾が来た方向に跳ね返る
    int count = hitWall ? SPARKS_WALL : SPARKS_ROBOT;

    for (int n = 0; n < count; n++)
    {
        Vector3 v = RandomDir();

        // 壁の向こう側に飛ばないよう、手前側に折り返す
        if (v.Dot(back) < 0.0f)
        {
            v -= back * (2.0f * v.Dot(back));
        }

        ShootSpark(color, pos, back * 0.3f + v, 1.0f, 1.0f);
    }
}

void HitFxManager::SpawnExplosion(const Vector3& pos, int color, float power)
{
    InitPool();

    if (color < 0 || color >= COLOR_COUNT)
    {
        color = 2;
    }

    // ---- 火の玉
    Fireball* ball = &m_fireballs[color][0];

    for (Fireball& f : m_fireballs[color])
    {
        if (f.timer < ball->timer)
        {
            ball = &f;
        }
    }

    ball->pos = pos;
    ball->timer = FIREBALL_TIME;
    ball->roll = RandomRange(0.0f, 6.2831853f);
    ball->spin = RandomRange(-2.0f, 2.0f);
    ball->size = power;
    ApplyFireball(*ball);

    // ---- 真ん中の強い閃光
    StartFlash(color, pos, 2.5f * power);

    // ---- 全方向に火花(少し上向き)
    int count = (int)(EXPLOSION_SPARKS * power);

    for (int n = 0; n < count; n++)
    {
        ShootSpark(color, pos, RandomDir() + Vector3(0.0f, 0.5f, 0.0f), 1.3f, 1.6f);
    }

    // ---- 土ぼこり
    StartDust(pos, 1.8f * power);

    AddShake(6.0f * power);
}

void HitFxManager::AddShake(float power)
{
    if (power > m_shake)
    {
        m_shake = power;
    }
}

// ================= 毎フレーム =================

void HitFxManager::Update()
{
    if (!m_ready)
    {
        return;
    }

    const float dt = 1.0f / 60.0f;

    for (int c = 0; c < COLOR_COUNT; c++)
    {
        for (Spark& s : m_sparks[c])
        {
            if (s.life <= 0.0f)
            {
                continue;
            }

            s.life -= dt;
            s.vel.y -= SPARK_GRAVITY * dt;
            s.vel *= 0.94f;
            s.pos += s.vel * dt;

            if (s.life > 0.0f)
            {
                ApplySpark(s);
            }
        }

        for (Flash& f : m_flashes[c])
        {
            if (f.timer <= 0.0f)
            {
                continue;
            }

            f.timer -= dt;

            if (f.timer > 0.0f)
            {
                ApplyFlash(f);
            }
        }

        for (Fireball& f : m_fireballs[c])
        {
            if (f.timer <= 0.0f)
            {
                continue;
            }

            f.timer -= dt;
            f.pos.y += FIREBALL_RISE * f.size * dt;
            f.roll += f.spin * dt;

            if (f.timer > 0.0f)
            {
                ApplyFireball(f);
            }
        }
    }

    for (Dust& d : m_dusts)
    {
        if (d.timer <= 0.0f)
        {
            continue;
        }

        d.timer -= dt;
        d.pos.y += DUST_RISE * dt;

        if (d.timer > 0.0f)
        {
            ApplyDust(d);
        }
    }

    // ---- 画面の揺れ(カメラの位置と注視点を同じだけずらす)
    if (m_shake > 0.05f)
    {
        // スコープを覗いている間は揺らさない(覗いている度合いに合わせて弱くする)
        float shakeScale = 1.0f;
        GameCamera* camera = FindGO<GameCamera>("gameCamera");

        if (camera != nullptr)
        {
            shakeScale = 1.0f - camera->GetScopeRate();
        }

        if (shakeScale > 0.01f)
        {
            float s = m_shake * shakeScale;

            Vector3 offset(
                RandomRange(-s, s),
                RandomRange(-s, s),
                RandomRange(-s, s)
            );

            g_camera3D->SetPosition(g_camera3D->GetPosition() + offset);
            g_camera3D->SetTarget(g_camera3D->GetTarget() + offset);
            g_camera3D->Update();
        }

        // 覗いていても、揺れの残りはちゃんと減らしていく
        m_shake *= SHAKE_DECAY;
    }
}

void HitFxManager::ApplySpark(Spark& s)
{
    // 残り時間に合わせて細く・短くなって消える
    float t = s.life / s.maxLife;
    float speedRate = s.vel.Length() / SPARK_SPEED_MAX;
    float w = SPARK_WIDTH * s.size * (0.5f + 0.5f * t);
    float l = SPARK_LENGTH * s.size * (0.3f + speedRate) * t;

    s.render.SetPosition(s.pos);
    s.render.SetRotation(LookRotation(s.vel, 0.0f));
    s.render.SetScale(Vector3(w, w, l));
    s.render.Update();
}

void HitFxManager::ApplyFlash(Flash& f)
{
    // 出た瞬間が一番大きくて、すぐしぼむ
    float t = 1.0f - f.timer / FLASH_TIME;
    float scale = f.size * (1.0f - t * 0.7f);

    Vector3 toCamera = ToCamera(f.pos);

    f.render.SetPosition(f.pos + toCamera * 8.0f);
    f.render.SetRotation(LookRotation(toCamera, f.roll));
    f.render.SetScale(Vector3(scale, scale, scale));
    f.render.Update();
}

void HitFxManager::ApplyFireball(Fireball& f)
{
    // ボンッと一気にふくらんで、少し保ってから、しぼんで消える
    float t = 1.0f - f.timer / FIREBALL_TIME;
    float scale = 0.0f;

    if (t < 0.2f)
    {
        float k = t / 0.2f;
        scale = 0.3f + 0.7f * (1.0f - (1.0f - k) * (1.0f - k));
    }
    else if (t < 0.6f)
    {
        scale = 1.0f + 0.15f * (t - 0.2f) / 0.4f;
    }
    else
    {
        float k = (t - 0.6f) / 0.4f;
        scale = 1.15f * (1.0f - k * k);
    }

    scale *= FIREBALL_SCALE * f.size;

    Vector3 toCamera = ToCamera(f.pos);

    f.render.SetPosition(f.pos + toCamera * 5.0f);
    f.render.SetRotation(LookRotation(toCamera, f.roll));
    f.render.SetScale(Vector3(scale, scale, scale));
    f.render.Update();
}

void HitFxManager::ApplyDust(Dust& d)
{
    // ふわっと広がって、最後に小さくなって消える
    float t = 1.0f - d.timer / DUST_TIME;
    float grow = 0.5f + 0.8f * sqrtf(t);
    float fade = t < 0.6f ? 1.0f : 1.0f - (t - 0.6f) / 0.4f;
    float scale = DUST_SCALE * d.size * grow * fade;

    Vector3 toCamera = ToCamera(d.pos);

    d.render.SetPosition(d.pos + toCamera * 3.0f);
    d.render.SetRotation(LookRotation(toCamera, d.roll + t * 0.8f));
    d.render.SetScale(Vector3(scale, scale, scale));
    d.render.Update();
}

void HitFxManager::Render(RenderContext& rc)
{
    if (!m_ready)
    {
        return;
    }

    for (Dust& d : m_dusts)
    {
        if (d.timer > 0.0f)
        {
            d.render.Draw(rc);
        }
    }

    for (int c = 0; c < COLOR_COUNT; c++)
    {
        for (Fireball& f : m_fireballs[c])
        {
            if (f.timer > 0.0f)
            {
                f.render.Draw(rc);
            }
        }

        for (Flash& f : m_flashes[c])
        {
            if (f.timer > 0.0f)
            {
                f.render.Draw(rc);
            }
        }

        for (Spark& s : m_sparks[c])
        {
            if (s.life > 0.0f)
            {
                s.render.Draw(rc);
            }
        }
    }
}