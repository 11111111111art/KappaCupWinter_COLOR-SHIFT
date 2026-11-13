#include "stdafx.h"
#include "Sound.h"

namespace
{
    // 番号表(Sound.h の enum)と同じ順番で並べる
    const char* SOUND_FILES[SOUND_NUM] =
    {
        "Assets/sound/se_shot.wav",
        "Assets/sound/se_hit.wav",
        "Assets/sound/se_hit_weak.wav",
        "Assets/sound/se_hit_resist.wav",
        "Assets/sound/se_hit_wall.wav",
        "Assets/sound/se_enemy_shot.wav",
        "Assets/sound/se_damage.wav",
        "Assets/sound/se_reload_out.wav",
        "Assets/sound/se_reload_in.wav",
        "Assets/sound/se_switch.wav",
        "Assets/sound/se_spawn.wav",
        "Assets/sound/se_robot_down.wav",
        "Assets/sound/se_clear.wav",
        "Assets/sound/bgm_battle.wav",
        "Assets/sound/bgm_title.wav",
    };

    const float SOUND_3D_RANGE = 4000.0f;   // これより遠い音は聞こえない
    bool s_initialized = false;             // もう倉庫に登録したか
}

void Sound::Init()
{
    if (s_initialized)
    {
        return;     // 2回目からは何もしない
    }

    for (int i = 0; i < SOUND_NUM; i++)
    {
        g_soundEngine->ResistWaveFileBank(i, SOUND_FILES[i]);
    }

    s_initialized = true;
}

void Sound::Play(SoundID id, float volume)
{
    Init();

    SoundSource* se = NewGO<SoundSource>(0);
    se->Init(id);
    se->SetVolume(volume);
    se->Play(false);    // ループしない音は、鳴り終わったら勝手に消える
}

void Sound::Play3D(SoundID id, const Vector3& pos, float volume)
{
    float dist = (pos - g_camera3D->GetPosition()).Length();
    float k = 1.0f - dist / SOUND_3D_RANGE;

    if (k <= 0.02f)
    {
        return;     // 遠すぎて聞こえない
    }

    Play(id, volume * k * k);
}

void Sound::PlayBGM(SoundID id, float volume)
{
    Init();
    StopBGM();      // 前のBGMが鳴ってたら止める

    SoundSource* bgm = NewGO<SoundSource>(0, "bgm");
    bgm->Init(id);
    bgm->SetVolume(volume);
    bgm->Play(true);    // ループ
}

void Sound::StopBGM()
{
    QueryGOs<SoundSource>("bgm", [&](SoundSource* bgm)
        {
            bgm->Stop();
            DeleteGO(bgm);
            return true;
        });
}
