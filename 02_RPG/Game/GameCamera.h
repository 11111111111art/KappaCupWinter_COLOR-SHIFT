#pragma once

class Player;

class GameCamera : public IGameObject
{
public:

    GameCamera();
    ~GameCamera();

    bool Start();
    void Update();

    void ResetCamera();

private:

    Player* m_player = nullptr;

    // カメラの位置
    Vector3 m_toCameraPos;

    // 初期位置
    Vector3 startToCameraPos;
};