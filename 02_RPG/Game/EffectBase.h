#pragma once

class EffectBase : public IGameObject
{
public:
    virtual ~EffectBase() = default;

    virtual void Play()
    {
        m_timer = m_duration;
        m_isPlaying = true;
    }


    virtual void Update() override
    {
        if (!m_isPlaying)
        {
            return;
        }

        m_timer -= 1.0f / 60.0f;

        char debugText[128];
        sprintf_s(debugText, "Effect Timer: %.3f\n", m_timer);
        OutputDebugStringA(debugText);

        if (m_timer <= 0.0f)
        {
            m_timer = 0.0f;
            m_isPlaying = false;

            OutputDebugStringA("Effect Stopped!\n");
        }
    }


protected:
    float m_timer = 0.0f;
    float m_duration = 0.5f;
    bool m_isPlaying = false;
};

