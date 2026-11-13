#pragma once
#include "sound/SoundEngine.h"
#include "sound/SoundSource.h"

// 鳴らす音の番号
enum SoundID
{
    SE_SHOT,        // 0: 自分の射撃
    SE_HIT,         // 1: ロボットに当たった(普通)
    SE_HIT_WEAK,    // 2: 弱点に当たった
    SE_HIT_RESIST,  // 3: 耐性で弾かれた
    SE_HIT_WALL,    // 4: 壁に当たった
    SE_ENEMY_SHOT,  // 5: ロボットの射撃
    SE_DAMAGE,      // 6: 自分が被弾
    SE_RELOAD_OUT,  // 7: マガジンを抜く
    SE_RELOAD_IN,   // 8: マガジンを差す
    SE_SWITCH,      // 9: 弾の色を切り替え
    SE_SPAWN,       // 10: ロボットが出てくる
    SE_ROBOT_DOWN,  // 11: ロボットが倒れる
    SE_CLEAR,       // 12: クリア
    BGM_BATTLE,     // 13: 戦闘BGM
    BGM_TITLE,      // 14: タイトルBGM
    SOUND_NUM       // 15: 音の数(これは鳴らさない。数を数えるためのもの)
};

namespace Sound
{
    void Init();                                                        // 最初に1回
    void Play(SoundID id, float volume = 1.0f);                         // 1回鳴らす
    void Play3D(SoundID id, const Vector3& pos, float volume = 1.0f);   // 場所から鳴らす
    void PlayBGM(SoundID id, float volume = 0.4f);                      // BGM(ループ)
    void StopBGM();                                                     // BGMを止める
}

