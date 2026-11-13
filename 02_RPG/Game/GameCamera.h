#pragma once

class Player;

class GameCamera : public IGameObject
{
public:
    GameCamera();
    ~GameCamera();

    bool Start();
    void Update();
    void Render(RenderContext& rc);
    void ResetCamera();

    float GetAngle() const
    {
        return m_angle;
    }
    Vector3 GetAimDirection() const;
    Vector3 GetAimTarget() const;

    // 撃ったときに Player から呼ぶ(反動と銃口の光)
    void OnFire();
    // スコープを覗いている度合い(0=覗いてない 〜 1=覗いてる)
    float GetScopeRate() const
    {
        return m_scopeRate;
    }
private:
    // 一人称の銃の位置・向きを更新する
    void UpdateFpGun(const Vector3& cameraPos, const Vector3& forward);

    Player* m_player = nullptr;
    ModelRender m_fpMag;            // 一人称の銃のマガジン(リロードで抜き差しする)
    bool m_magVisible = true;
    float m_angle = 0.0f;
    float m_pitch = 0.0f;

    float m_cameraSpeed = 2.0f;

    // ---- 一人称の銃
    ModelRender m_fpGun;                // 銃
    ModelRender m_muzzleFlash;          // 銃口の光
    float m_scopeRate = 0.0f;           // 0 = 腰だめ、1 = スコープを覗ききった
    float m_sprintRate = 0.0f;          // 0 = 普通、1 = ダッシュの構え
    float m_bobPhase = 0.0f;            // 走ったときの揺れ
    float m_bobAmp = 0.0f;              //走ったときの揺れの大きさ
    float m_swayTime = 0.0f;            // 呼吸の揺れ
    float m_recoil = 0.0f;              // 反動
    float m_flashTimer = 0.0f;          // 銃口の光を出す残り時間
    float m_turnSwayYaw = 0.0f;         // 視点を回したときの遅れ(左右)
    float m_turnSwayPitch = 0.0f;       // 視点を回したときの遅れ(上下)
    float m_jumpSway = 0.0f;            // ジャンプ・着地の揺れ
    float m_prevAngle = 0.0f;           // 前のフレームの m_angle
    float m_prevPitch = 0.0f;           // 前のフレームの m_pitch
    float m_defaultViewAngle = 0.0f;    // 元の画角
    Vector3 m_prevPlayerPos;            // 前のフレームのプレイヤーの位置
   
};