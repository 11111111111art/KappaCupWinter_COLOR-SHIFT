#include "stdafx.h"
#include "Bullet.h"
#include "Robot.h"
#include <cmath>
#include "Player.h"
#include "HitFxManager.h"
#include <cfloat>
#include "GameConfig.h"
bool Bullet::Start()
{
    return true;
}

void Bullet::Update()
{
    if (!m_modelInitialized)
    {
        const char* modelPath = "Assets/modelData/stage/bullet.tkm";

        switch (m_bulletType)
        {
        case BulletType::Red:
            modelPath = "Assets/modelData/stage/bulletRed.tkm";
            break;

        case BulletType::Blue:
            modelPath = "Assets/modelData/stage/bulletBlue.tkm";
            break;

        case BulletType::Yellow:
            modelPath = "Assets/modelData/stage/bullet.tkm";
            break;
        }

        m_modelRender.Init(modelPath);
        m_modelInitialized = true;
    }

    if (!m_isActive)
    {
        //DeleteGO(this);   // 使い終わった弾は消す
        return;
    }

    const float deltaTime = 1.0f / 60.0f;

    Vector3 oldPosition = m_position;

    m_position += m_direction * m_speed * deltaTime;
    // ---- 届く距離を超えたら消える(ロボットの攻撃範囲と同じ)
    m_traveled += m_speed * deltaTime;

    if (m_traveled > GameConfig::ATTACK_RANGE)
    {
        m_isActive = false;
        return;
    }
    // ---- 当たり判定: 全部のロボットを調べて、一番手前で当たったやつ
    Robot* robot = nullptr;
    Vector3 robotHitPos;
    float nearestRobot = FLT_MAX;

    QueryGOs<Robot>("robot", [&](Robot* r)
        {
            Vector3 p;

            if (!r->IsDead() && r->HitTest(oldPosition, m_position, &p))
            {
                float d = (p - oldPosition).Length();

                if (d < nearestRobot)
                {
                    nearestRobot = d;
                    robot = r;
                    robotHitPos = p;
                }
            }

            return true;
        });

    bool hitRobot = robot != nullptr;

    Vector3 wallHitPos;
    bool hitWall = PhysicsWorld::GetInstance()->RayTest(oldPosition, m_position, wallHitPos);
    if (hitRobot || hitWall)
    {
        // 弾の色(0=赤, 1=青, 2=黄)
        int color = 2;

        if (m_bulletType == BulletType::Red)
        {
            color = 0;
        }
        else if (m_bulletType == BulletType::Blue)
        {
            color = 1;
        }

        HitFxManager* fx = FindGO<HitFxManager>("hitFx");

        if (fx == nullptr)
        {
            fx = NewGO<HitFxManager>(10, "hitFx");
        }

        if (hitRobot)
        {
            const int BULLET_DAMAGE = 20;   // 全色共通。相性はロボット側で計算する

            robot->TakeDamage(BULLET_DAMAGE, color);
            fx->Spawn(robotHitPos, m_direction, color, false);
        }
        else
        {
            fx->Spawn(wallHitPos, m_direction, color, true);
        }

        m_isActive = false;
        return;
    }

    m_lifeTime -= deltaTime;

    if (m_lifeTime <= 0.0f)
    {
        m_isActive = false;
        return;
    }

    // ---- 弾を飛んでいく向きに向ける(光の尾が後ろに伸びる)
    Vector3 fwd = m_direction;
    fwd.Normalize();
    Vector3 right;
    right.Cross(Vector3::AxisY, fwd);

    if (right.Length() < 0.001f)
    {
        // 真上・真下に撃ったとき
        right = Vector3::AxisX;
    }

    right.Normalize();
    Vector3 up;
    up.Cross(fwd, right);

    Matrix rotMat = Matrix::Identity;
    rotMat.m[0][0] = right.x; rotMat.m[0][1] = right.y; rotMat.m[0][2] = right.z;
    rotMat.m[1][0] = up.x;    rotMat.m[1][1] = up.y;    rotMat.m[1][2] = up.z;
    rotMat.m[2][0] = fwd.x;   rotMat.m[2][1] = fwd.y;   rotMat.m[2][2] = fwd.z;

    Quaternion rot;
    rot.SetRotation(rotMat);

    m_modelRender.SetRotation(rot);
    m_modelRender.SetPosition(m_position);
    m_modelRender.Update();
}

void Bullet::Render(RenderContext& rc)
{
    if (m_modelInitialized && m_isActive)
    {
        m_modelRender.Draw(rc);
    }
}