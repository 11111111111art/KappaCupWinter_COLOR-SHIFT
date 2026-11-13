#pragma once
#include <cmath>

// ゲームパッドの入力をまとめたもの(パッドがつながってなくても安全に動く)
namespace GameInput
{
    const float STICK_DEADZONE = 0.15f;   // スティックの遊び(これより小さい傾きは無視)

    inline float Deadzone(float v)
    {
        if (fabsf(v) < STICK_DEADZONE)
        {
            return 0.0f;
        }

        // 遊びの分を引いて、0〜1 に戻す
        float s = (fabsf(v) - STICK_DEADZONE) / (1.0f - STICK_DEADZONE);
        return v < 0.0f ? -s : s;
    }

    inline float LStickX() { return g_pad[0] != nullptr ? Deadzone(g_pad[0]->GetLStickXF()) : 0.0f; }
    inline float LStickY() { return g_pad[0] != nullptr ? Deadzone(g_pad[0]->GetLStickYF()) : 0.0f; }
    inline float RStickX() { return g_pad[0] != nullptr ? Deadzone(g_pad[0]->GetRStickXF()) : 0.0f; }
    inline float RStickY() { return g_pad[0] != nullptr ? Deadzone(g_pad[0]->GetRStickYF()) : 0.0f; }

    // 押している間 true
    inline bool PadPress(EnButton button)
    {
        return g_pad[0] != nullptr && g_pad[0]->IsPress(button);
    }

    // 押した瞬間だけ true
    inline bool PadTrigger(EnButton button)
    {
        return g_pad[0] != nullptr && g_pad[0]->IsTrigger(button);
    }
}
