#pragma once
#include "Bullet.h"

class Player : public IGameObject
{
public:
    Player();
    ~Player();
    bool IsReloading() const
    {
        return m_isReloading;
    }
    // ダッシュ中か(このときは撃てない、銃を横に抱える)
    bool IsDashing() const
    {
        return m_isDashing;
    }
    // リロードの進み具合(0〜1)
    float GetReloadProgress() const
    {
        if (!m_isReloading || m_reloadTime <= 0.0f)
        {
            return 0.0f;
        }

        return 1.0f - m_reloadTimer / m_reloadTime;
    }
    // ダッシュの度合い(0=歩き 〜 1=全力ダッシュ)。速さと銃の構えに使う
    float GetDashBlend() const
    {
        return m_dashBlend;
    }
    bool Start();
    void Update();
    void Render(RenderContext& rc);
    void PlayAnimation();
    void DeleteBullets();
    void TakeDamage(int damage);
    FontRender m_positionFont;
    bool IsSliding() const { return m_isSliding; }
    float GetSlideRate() const { return m_slideRate; }   
    Vector3 GetPosition()
    {
        return m_position;
    }

    int GetHP() const
    {
        return m_hp;
    }

    int GetMaxHP() const
    {
        return m_maxHP;
    }

    bool IsDead() const
    {
        return m_hp <= 0;
    }
    enum class WeaponType
    {
        Red,
        Blue,
        Yellow
    };
    int GetAmmo() const
    {
        return m_ammo;
    }
    int GetMaxAmmo() const
    {
        return m_maxAmmo;
    }
    WeaponType GetWeaponType() const
    {
        return m_weaponType;
    }
    ModelRender modelRender;

    enum EnAnimationClip
    {
        enAnimationClip_Idle,
       /* enAnimationClip_Walk,*/
        enAnimationClip_Dash,
        enAnimationClip_Run,
        enAnimationClip_Jump,
        enAnimationClip_Num,
    };

    AnimationClip animationClips[enAnimationClip_Num];
    void RemoveBullet(Bullet* bullet);
    int GetWeaponColor() const
    {
        if (m_weaponType == WeaponType::Red) return 0;
        if (m_weaponType == WeaponType::Blue) return 1;
        return 2;
    }
    // しゃがみ・スライディングで体が低くなっている度合い(0=立ってる, 1=一番低い)
    float GetLowRate() const
    {
        return m_crouchRate > m_slideRate ? m_crouchRate : m_slideRate;
    }

    float GetCrouchRate() const
    {
        return m_crouchRate;
    }
private:
    ModelRender m_modelRender;
    Vector3 m_position = Vector3::Zero;
    float m_fireTimer = 0.0f;
    bool m_isWalking = false;
    float m_moveSpeed = 500.0f;
    float m_verticalSpeed = 0.0f;
    float m_jumpPower = 350.0f;
    float m_gravity = -980.0f;
    bool m_isGrounded = false;
    int m_ammo = 30;
    int m_maxAmmo = 30;
    bool m_isReloading = false;
    float m_reloadTimer = 0.0f;
    float m_reloadTime = 2.0f;
    int m_hp = 100;
    int m_maxHP = 100;
    WeaponType m_weaponType = WeaponType::Red;
    CharacterController m_characterController;
    static const int BULLET_POOL_SIZE = 10;
    bool m_isSliding = false;           // スライディング中か
    float m_slideTimer = 0.0f;          // スライディングを始めてからの時間
    float m_slideCooldown = 0.0f;       // 次にスライディングできるまでの時間
    float m_slideRate = 0.0f;           // カメラ用(0 = 普通、1 = スライディング中)
    bool m_prevSlideKey = false;        // 前のフレームでキーを押していたか
    Vector3 m_slideDir;                 // 滑る向き
    Bullet* m_redBullets[BULLET_POOL_SIZE] = {};
    Bullet* m_blueBullets[BULLET_POOL_SIZE] = {};
    Bullet* m_yellowBullets[BULLET_POOL_SIZE] = {};
    bool m_padSprint = false;   // パッドのダッシュ(左スティック押し込みで ON)
    bool m_isCrouching = false;     // しゃがんでいるか
    bool m_padCrouch = false;       // パッドのしゃがみ(B で切り替え)
    float m_crouchRate = 0.0f;      // カメラ用に、なめらかに 0〜1
    float m_sprintRecover = 0.0f;   // ダッシュをやめてから撃てるまでの残り時間
    bool m_isDashing = false;
    float m_dashBlend = 0.0f;
};
