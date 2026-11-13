#pragma once

#include <QQuickFramebufferObject>
#include <QVariantMap>
#include <Geometry/MeshGenerator.h>
#include <Camera/camera.h>
#include <QVector3D>
#include <QVector2D>

// 本模块的 QML 视口入口；GUI 线程只保存属性，GL 操作交给 Renderer。
class PbrLab : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap parameters READ parameters WRITE setParameters NOTIFY parametersChanged)
    Q_PROPERTY(int experimentIndex READ experimentIndex WRITE setExperimentIndex NOTIFY experimentIndexChanged)

public:
    explicit PbrLab(QQuickItem *parent = nullptr);
    Renderer *createRenderer() const override;
    QVector3D orbit() const { return m_orbit; }

    QVariantMap parameters() const { return m_parameters; }
    void setParameters(const QVariantMap &parameters);
    int experimentIndex() const { return m_experimentIndex; }
    void setExperimentIndex(int experimentIndex);

    //一些数据
    marmoset::MeshData getSphereData() const;
    glm::mat4 model() const;
    glm::mat4 view() const;
    glm::mat4 projection() const;


protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseUngrabEvent() override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

signals:
    void parametersChanged();
    void experimentIndexChanged();

private:
    QVariantMap m_parameters;
    int m_experimentIndex = 0;
    marmoset::MeshData sphereData;

    //视图矩阵
    glm::mat4 m_model;
    glm::mat4 m_view;
    glm::mat4 m_projection;

    //交互参数
    QVector2D m_lastPos;
    bool m_pressed;
    bool m_rotating;
    QVector3D m_orbit;  //orbitYaw水平环绕角，orbitPitch俯仰角，orbitDistance相机到目标点得距离
};
