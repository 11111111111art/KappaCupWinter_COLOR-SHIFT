#include "stdafx.h"
#include "PlayerHPBar.h"
#include "Player.h"
#include <cmath>

namespace
{
    const char* WHITE_SPRITE = "Assets/sprite/hp_white.dds";
    const char* FILL_SPRITE = "Assets/sprite/hp_fill.dds";
    const char* FRAME_SPRITE = "Assets/sprite/hp_frame.dds";

    // わくの絵の大きさと、その中のバーの場所(絵に合わせてあるので変えないこと)
    const float FRAME_W = 520.0f;
    const float FRAME_H = 80.0f;
    const float BAR_OFFSET_X = 85.0f;       // わくの左はし → バーの左はし
    const float BAR_W = 420.0f;
    const float BAR_H = 25.0f;

    // 画面のどこに出すか
    const float MARGIN_X = 30.0f;           // 画面の左はしからのすき間
    const float MARGIN_Y = 70.0f;           // 画面の下はしからのすき間

    const float TRAIL_WAIT = 0.35f;         // ダメージのあと、残像が減り始めるまで(秒)
    const float TRAIL_SPEED = 0.8f;         // 残像が減る速さ(1秒あたり)
    const float HIT_FLASH_TIME = 0.12f;     // ダメージを受けたときに光る時間
    const float DANGER_RATE = 0.3f;         // これより少ないと赤く点滅

    float Clamp01(float v)
    {
        return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
    }

    // HPの割合 → 色(緑 → 黄 → 赤)
    Vector4 HpColor(float rate)
    {
        Vector4 green(0.25f, 0.95f, 0.45f, 1.0f);
        Vector4 yellow(1.0f, 0.85f, 0.2f, 1.0f);
        Vector4 red(1.0f, 0.25f, 0.2f, 1.0f);

        if (rate > 0.5f)
        {
            float t = (rate - 0.5f) / 0.5f;
            return Vector4(
                yellow.x + (green.x - yellow.x) * t,
                yellow.y + (green.y - yellow.y) * t,
                yellow.z + (green.z - yellow.z) * t, 1.0f);
        }

        float t = rate / 0.5f;
        return Vector4(
            red.x + (yellow.x - red.x) * t,
            red.y + (yellow.y - red.y) * t,
            red.z + (yellow.z - red.z) * t, 1.0f);
    }
}

bool PlayerHPBar::Start()
{
    m_player = FindGO<Player>("player");

    m_back.Init(WHITE_SPRITE, BAR_W, BAR_H);
    m_back.SetPivot(Vector2(0.0f, 0.5f));
    m_back.SetMulColor(Vector4(0.03f, 0.05f, 0.08f, 0.85f));

    m_trail.Init(FILL_SPRITE, BAR_W, BAR_H);
    m_trail.SetPivot(Vector2(0.0f, 0.5f));      // 左はしを基準にして、右から減る
    m_trail.SetMulColor(Vector4(1.0f, 1.0f, 1.0f, 0.9f));

    m_fill.Init(FILL_SPRITE, BAR_W, BAR_H);
    m_fill.SetPivot(Vector2(0.0f, 0.5f));

    m_frame.Init(FRAME_SPRITE, FRAME_W, FRAME_H);
    m_frame.SetPivot(Vector2(0.0f, 0.5f));

    m_number.SetScale(0.7f);
    m_number.SetColor(g_vec4White);

    Update();
    return true;
}

void PlayerHPBar::Update()
{
    if (m_player == nullptr)
    {
        m_player = FindGO<Player>("player");

        if (m_player == nullptr)
        {
            return;
        }
    }

    const float dt = 1.0f / 60.0f;
    m_time += dt;

    int hp = m_player->GetHP();
    int maxHp = m_player->GetMaxHP();
    float rate = Clamp01((float)hp / (float)maxHp);

    // ---- ダメージを受けた瞬間
    if (m_prevHP >= 0 && hp < m_prevHP)
    {
        m_hitTimer = HIT_FLASH_TIME + TRAIL_WAIT;   // 光る + 残像が待つ時間
    }

    m_prevHP = hp;

    if (m_hitTimer > 0.0f)
    {
        m_hitTimer -= dt;
    }

    // ---- 白い残像(少し待ってから、追いかけて減る)
    if (m_trailRate > rate)
    {
        if (m_hitTimer <= 0.0f)
        {
            m_trailRate -= TRAIL_SPEED * dt;
        }

        if (m_trailRate < rate)
        {
            m_trailRate = rate;
        }
    }
    else
    {
        m_trailRate = rate;     // 回復したときはすぐ合わせる
    }

    // ---- 位置(ダメージを受けた瞬間は少し揺れる)
    float frameLeft = -(float)FRAME_BUFFER_W * 0.5f + MARGIN_X;
    float y = -(float)FRAME_BUFFER_H * 0.5f + MARGIN_Y;

    if (m_hitTimer > TRAIL_WAIT)
    {
        frameLeft += sinf(m_time * 90.0f) * 4.0f;
    }

    float barLeft = frameLeft + BAR_OFFSET_X;

    m_back.SetPosition(Vector3(barLeft, y, 0.0f));
    m_back.Update();

    m_trail.SetPosition(Vector3(barLeft, y, 0.0f));
    m_trail.SetScale(Vector3(m_trailRate, 1.0f, 1.0f));
    m_trail.Update();

    // ---- 色(少ないと赤く点滅、ダメージの瞬間は白く光る)
    Vector4 color = HpColor(rate);

    if (rate < DANGER_RATE)
    {
        float blink = 0.6f + 0.4f * (0.5f + 0.5f * sinf(m_time * 10.0f));
        color = Vector4(color.x * blink, color.y * blink, color.z * blink, 1.0f);
    }

    if (m_hitTimer > TRAIL_WAIT)
    {
        color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    m_fill.SetPosition(Vector3(barLeft, y, 0.0f));
    m_fill.SetScale(Vector3(rate, 1.0f, 1.0f));
    m_fill.SetMulColor(color);
    m_fill.Update();

    m_frame.SetPosition(Vector3(frameLeft, y, 0.0f));
    m_frame.Update();

    // ---- 数字(わくの右上)
    wchar_t text[32];
    swprintf_s(text, L"%d / %d", hp, maxHp);
    m_number.SetText(text);
    m_number.SetPosition(frameLeft + FRAME_W - 130.0f, y + 70.0f, 0.0f);
    m_number.SetColor(rate < DANGER_RATE ? Vector4(1.0f, 0.4f, 0.35f, 1.0f) : g_vec4White);
}

void PlayerHPBar::Render(RenderContext& rc)
{
    if (m_player == nullptr)
    {
        return;
    }

    // 下から順に: 暗い板 → 白い残像 → HP → わく(区切り線と「HP」の文字はわくの絵に入ってる)
    m_back.Draw(rc);
    m_trail.Draw(rc);
    m_fill.Draw(rc);
    m_frame.Draw(rc);
    m_number.Draw(rc);
}