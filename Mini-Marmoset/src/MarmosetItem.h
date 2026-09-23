#pragma once
#include <QQuickFramebufferObject>

class MarmosetItem : public QQuickFramebufferObject
{
    Q_OBJECT

public:
    Q_PROPERTY(float orbitYaw READ orbitYaw WRITE setOrbitYaw NOTIFY orbitYawChanged)
    Q_PROPERTY(float orbitPitch READ orbitPitch WRITE setOrbitPitch NOTIFY orbitPitchChanged)
    Q_PROPERTY(float orbitDistance READ orbitDistance WRITE setOrbitDistance NOTIFY orbitDistanceChanged)
    Q_PROPERTY(float leftSphereRoughness READ leftSphereRoughness WRITE setLeftSphereRoughness NOTIFY leftSphereRoughnessChanged)
    Q_PROPERTY(float leftSphereMetallic READ leftSphereMetallic WRITE setLeftSphereMetallic NOTIFY leftSphereMetallicChanged)
    Q_PROPERTY(float leftSphereAlbedoR READ leftSphereAlbedoR WRITE setLeftSphereAlbedoR NOTIFY leftSphereAlbedoRChanged)
    Q_PROPERTY(float leftSphereAlbedoG READ leftSphereAlbedoG WRITE setLeftSphereAlbedoG NOTIFY leftSphereAlbedoGChanged)
    Q_PROPERTY(float leftSphereAlbedoB READ leftSphereAlbedoB WRITE setLeftSphereAlbedoB NOTIFY leftSphereAlbedoBChanged)

    Renderer *createRenderer() const override;

    float orbitYaw() const;
    void setOrbitYaw(float value);
    float orbitPitch() const;
    void setOrbitPitch(float value);
    float orbitDistance() const;
    void setOrbitDistance(float value);
    float leftSphereRoughness() const;
    void setLeftSphereRoughness(float value);
    float leftSphereMetallic() const;
    void setLeftSphereMetallic(float value);
    float leftSphereAlbedoR() const;
    void setLeftSphereAlbedoR(float value);
    float leftSphereAlbedoG() const;
    void setLeftSphereAlbedoG(float value);
    float leftSphereAlbedoB() const;
    void setLeftSphereAlbedoB(float value);

signals:
    void orbitYawChanged();
    void orbitPitchChanged();
    void orbitDistanceChanged();
    void leftSphereRoughnessChanged();
    void leftSphereMetallicChanged();
    void leftSphereAlbedoRChanged();
    void leftSphereAlbedoGChanged();
    void leftSphereAlbedoBChanged();

private:
    float orbitYawValue = 90.0f;
    float orbitPitchValue = 2.0f;
    float orbitDistanceValue = 11.0f;
    float leftSphereRoughnessValue = 0.23f;
    float leftSphereMetallicValue = 0.0f;
    float leftSphereAlbedoRValue = 0.52f;
    float leftSphereAlbedoGValue = 0.055f;
    float leftSphereAlbedoBValue = 0.025f;
};
