#pragma once

// 空のドーム(いつもカメラを中心にして、空の端に届かないようにする)
class Sky : public IGameObject
{
public:
    bool Start() override;
    void Update() override;
    void Render(RenderContext& rc) override;

private:
    ModelRender m_model;
};

