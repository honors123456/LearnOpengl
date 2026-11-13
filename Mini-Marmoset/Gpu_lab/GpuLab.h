#pragma once

#include <QQuickFramebufferObject>
#include <QVariantMap>

// 本模块的 QML 视口入口；GUI 线程只保存属性，GL 操作交给 Renderer。
class GpuLab : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap parameters READ parameters WRITE setParameters NOTIFY parametersChanged)
    Q_PROPERTY(int experimentIndex READ experimentIndex WRITE setExperimentIndex NOTIFY experimentIndexChanged)

public:
    explicit GpuLab(QQuickItem *parent = nullptr);
    Renderer *createRenderer() const override;

    QVariantMap parameters() const { return m_parameters; }
    void setParameters(const QVariantMap &parameters);
    int experimentIndex() const { return m_experimentIndex; }
    void setExperimentIndex(int experimentIndex);

signals:
    void parametersChanged();
    void experimentIndexChanged();

private:
    QVariantMap m_parameters;
    int m_experimentIndex = 0;
};
