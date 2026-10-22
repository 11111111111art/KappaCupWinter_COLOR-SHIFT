#pragma once

//class GameCamera;

class Player : public IGameObject
{
public:
    Player();
    ~Player();

    bool Start();
    void Update();
    void Render(RenderContext& rc);
    void PlayAnimation();

    Vector3 GetPosition()
    {
        return m_position;
    }
    ModelRender modelRender;
    enum EnAnimationClip {
        enAnimationClip_Idle,
        enAnimationClip_Walk,
        enAnimationClip_Run,
        enAnimationClip_Jump,
        enAnimationClip_Num,
    };
    AnimationClip animationClips[enAnimationClip_Num];
private:
    ModelRender m_modelRender;
    Vector3 m_position = Vector3::Zero;

    float m_moveSpeed = 300.0f;

    CharacterController m_characterController;

    /*GameCamera* m_gameCamera = nullptr;*/
};