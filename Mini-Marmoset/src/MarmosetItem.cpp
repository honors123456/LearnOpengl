#include "MarmosetItem.h"

#include "../shader/shader.h"

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <memory>
#include <vector>

namespace {

void *getProcAddress(const char *name)
{
    QOpenGLContext *context = QOpenGLContext::currentContext();
    return context ? reinterpret_cast<void *>(context->getProcAddress(name)) : nullptr;
}

std::vector<float> makeSphereVertices()
{
    constexpr int segments = 64;
    constexpr int rings = 32;
    std::vector<float> vertices;
    vertices.reserve((segments + 1) * (rings + 1) * 6);
    for (int ring = 0; ring <= rings; ++ring) {
        const float v = static_cast<float>(ring) / rings;
        const float phi = v * glm::pi<float>();
        for (int segment = 0; segment <= segments; ++segment) {
            const float u = static_cast<float>(segment) / segments;
            const float theta = u * glm::two_pi<float>();
            const glm::vec3 p(std::sin(phi) * std::cos(theta), std::cos(phi),
                              std::sin(phi) * std::sin(theta));
            vertices.insert(vertices.end(), {p.x, p.y, p.z, p.x, p.y, p.z});
        }
    }
    return vertices;
}

std::vector<unsigned int> makeSphereIndices()
{
    constexpr int segments = 64;
    constexpr int rings = 32;
    std::vector<unsigned int> indices;
    indices.reserve(segments * rings * 6);
    for (int ring = 0; ring < rings; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {
            const unsigned int current = ring * (segments + 1) + segment;
            const unsigned int next = current + segments + 1;
            indices.insert(indices.end(), {current, next, current + 1,
                                           current + 1, next, next + 1});
        }
    }
    return indices;
}

class MarmosetRenderer final : public QQuickFramebufferObject::Renderer
{
public:
    ~MarmosetRenderer() override
    {
        if (sphereVAO != 0) {
            glDeleteVertexArrays(1, &sphereVAO);
            glDeleteBuffers(1, &sphereVBO);
            glDeleteBuffers(1, &sphereEBO);
        }
    }

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override
    {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        format.setInternalTextureFormat(GL_RGBA8);
        return new QOpenGLFramebufferObject(size, format);
    }

    void synchronize(QQuickFramebufferObject *item) override
    {
        const MarmosetItem *marmosetItem = static_cast<const MarmosetItem *>(item);
        orbitYaw = marmosetItem->orbitYaw();
        orbitPitch = marmosetItem->orbitPitch();
        orbitDistance = marmosetItem->orbitDistance();
        leftSphereRoughness = marmosetItem->leftSphereRoughness();
        leftSphereMetallic = marmosetItem->leftSphereMetallic();
        leftSphereAlbedoR = marmosetItem->leftSphereAlbedoR();
        leftSphereAlbedoG = marmosetItem->leftSphereAlbedoG();
        leftSphereAlbedoB = marmosetItem->leftSphereAlbedoB();
    }
    void render() override
    {
        if (!initialized) {
            if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(getProcAddress)))
                throw std::runtime_error("Failed to load OpenGL functions from Qt context");

            //创建着色器
            pbrShader = std::make_unique<Shader>(SHADER_DIR "/pbrShader.vert",SHADER_DIR "/pbrShader.frag");
            lightShader = std::make_unique<Shader>(SHADER_DIR "/lightShader.vert",SHADER_DIR "/lightShader.frag");

            //创建球体数据
            const std::vector<float> vertices = makeSphereVertices();
            const std::vector<unsigned int> indices = makeSphereIndices();
            sphereIndexCount = static_cast<GLsizei>(indices.size());

            glGenVertexArrays(1, &sphereVAO);
            glGenBuffers(1, &sphereVBO);
            glGenBuffers(1, &sphereEBO);

            glBindVertexArray(sphereVAO);
            glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
            glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),vertices.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),indices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),reinterpret_cast<void *>(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glBindVertexArray(0);
            initialized = true;
        }

        // 设置视口，清空屏幕
        const QSize size = framebufferObject()->size();
        glViewport(0, 0, size.width(), size.height());
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.56f, 0.43f, 0.31f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        //视图矩阵和投影矩阵
        const float yaw = glm::radians(orbitYaw);
        const float pitch = glm::radians(orbitPitch);
        const glm::vec3 orbitOffset(
            orbitDistance * std::cos(pitch) * std::cos(yaw),
            orbitDistance * std::sin(pitch),
            orbitDistance * std::cos(pitch) * std::sin(yaw));
        const glm::vec3 cameraPosition = orbitOffset;

        const float aspect = size.height() > 0 ? static_cast<float>(size.width()) / size.height() : 1.0f;
        const glm::mat4 view = glm::lookAt(cameraPosition, glm::vec3(0.0f),glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect,0.1f, 100.0f);

        //绘制球体
        pbrShader->use();
        pbrShader->setMat4("view", view);
        pbrShader->setMat4("projection", projection);
        pbrShader->setVec3("cameraPos", cameraPosition);
        pbrShader->setFloat("ao", 1.0f);

        //上传光源数据
        for (int i = 0; i < 4; ++i) {
            pbrShader->setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
            pbrShader->setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
        }

        //绘制球体
        glBindVertexArray(sphereVAO);
        for (int i = 0; i < materials.size(); ++i) {
            const glm::vec3 &position = spherePositions[i];
            Material material = materials[i];
            if (i == 0) {
                material.roughness = leftSphereRoughness;
                material.metallic = leftSphereMetallic;
                material.albedo = glm::vec3(leftSphereAlbedoR, leftSphereAlbedoG, leftSphereAlbedoB);
            }
            glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
            model = glm::scale(model, glm::vec3(0.82f));

            pbrShader->setMat4("model", model);
            pbrShader->setVec3("albedo", material.albedo);
            pbrShader->setFloat("metallic", material.metallic);
            pbrShader->setFloat("roughness", material.roughness);
            pbrShader->setInt("materialType", material.type);
            glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, nullptr);
        }

        glBindVertexArray(0);

        //绘制光源
        lightShader->use();
        lightShader->setMat4("view", view);
        lightShader->setMat4("projection", projection);
        glBindVertexArray(sphereVAO);
        for (int i = 2; i < lightPositions.size(); ++i) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), lightPositions[i]);
            model = glm::scale(model, glm::vec3(0.14f));
            lightShader->setMat4("model", model);
            lightShader->setVec3("lightColor", lightColors[i] * 0.025f);
            glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, nullptr);
        }
        glBindVertexArray(0);
    }

private:
    struct Material {
        glm::vec3 albedo;
        float metallic;
        float roughness;
        int type;
    };

    bool initialized = false;
    GLuint sphereVAO = 0;
    GLuint sphereVBO = 0;
    GLuint sphereEBO = 0;
    GLsizei sphereIndexCount = 0;
    std::unique_ptr<Shader> pbrShader;
    std::unique_ptr<Shader> lightShader;

    float orbitYaw = 90.0f;
    float orbitPitch = 2.0f;
    float orbitDistance = 11.0f;
    float leftSphereRoughness = 0.23f;
    float leftSphereMetallic = 0.0f;
    float leftSphereAlbedoR = 0.52f;
    float leftSphereAlbedoG = 0.055f;
    float leftSphereAlbedoB = 0.025f;

    std::array<glm::vec3, 4> lightPositions = {
        glm::vec3(-4.5f, 4.5f, 4.0f),
        glm::vec3(4.5f, 4.5f, 4.0f),
        glm::vec3(-3.0f, -1.65f, 2.8f),
        glm::vec3(3.0f, -1.65f, 2.8f)
    };

    std::array<glm::vec3, 4> lightColors = {
        glm::vec3(150.0f, 116.0f, 78.0f),
        glm::vec3(130.0f, 118.0f, 94.0f),
        glm::vec3(48.0f, 32.0f, 23.0f),
        glm::vec3(48.0f, 32.0f, 23.0f)
    };

    std::array<glm::vec3, 26> spherePositions = {
        glm::vec3(-7.0f, 0.0f, 0.0f),

        glm::vec3(-4.0f, -4.0f, 0.0f),
        glm::vec3(-2.0f, -4.0f, 0.0f),
        glm::vec3(0.0f, -4.0f, 0.0f),
        glm::vec3(2.0f, -4.0f, 0.0f),
        glm::vec3(4.0f, -4.0f, 0.0f),

        glm::vec3(-4.0f, -2.0f, 0.0f),
        glm::vec3(-2.0f, -2.0f, 0.0f),
        glm::vec3(0.0f, -2.0f, 0.0f),
        glm::vec3(2.0f, -2.0f, 0.0f),
        glm::vec3(4.0f, -2.0f, 0.0f),

        glm::vec3(-4.0f, 0.0f, 0.0f),
        glm::vec3(-2.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f),
        glm::vec3(4.0f, 0.0f, 0.0f),

        glm::vec3(-4.0f, 2.0f, 0.0f),
        glm::vec3(-2.0f, 2.0f, 0.0f),
        glm::vec3(0.0f, 2.0f, 0.0f),
        glm::vec3(2.0f, 2.0f, 0.0f),
        glm::vec3(4.0f, 2.0f, 0.0f),

        glm::vec3(-4.0f, 4.0f, 0.0f),
        glm::vec3(-2.0f, 4.0f, 0.0f),
        glm::vec3(0.0f, 4.0f, 0.0f),
        glm::vec3(2.0f, 4.0f, 0.0f),
        glm::vec3(4.0f, 4.0f, 0.0f),
    };

    std::array<Material, 26> materials = {
        // 最左侧独立球体：右侧 QML 滑条覆盖它的 roughness、metallic 和 albedo。
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.00f, 0.05f,   1},

        // 5×5 材质测试矩阵：每行 metallic 递增，每列 roughness 递增。
        // roughness: 0.05, 0.2875, 0.525, 0.7625, 1.0
        // metallic:   0.0,  0.25,  0.5,    0.75,   1.0
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.00f, 0.05f,   1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.00f, 0.25f, 1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.00f, 0.5f,  1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.00f, 0.75f, 1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.00f, 1.0f,    1},

        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.25f, 0.05f,   1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.25f, 0.25f, 1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.25f, 0.5f,  1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.25f, 0.75f, 1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.25f, 1.0f,    1},

        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.50f, 0.05f,   1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.50f, 0.25f, 1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.50f, 0.5f,  1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.50f, 0.75f, 1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.50f, 1.0f,    1},

        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.75f, 0.05f,   1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.75f, 0.25f, 1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.75f, 0.5f,  1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.75f, 0.75f, 1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 0.75f, 1.0f,    1},

        Material{glm::vec3(0.5f, 0.5f, 0.5f), 1.00f, 0.05f,   1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 1.00f, 0.25f, 1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 1.00f, 0.5f,  1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 1.00f, 0.75f, 1},
        Material{glm::vec3(0.5f, 0.5f, 0.5f), 1.00f, 1.0f,    1},
    };
};

} // namespace
float MarmosetItem::leftSphereRoughness() const
{
    return leftSphereRoughnessValue;
}

void MarmosetItem::setLeftSphereRoughness(float value)
{
    const float clampedValue = std::max(0.05f, std::min(1.0f, value));
    if (leftSphereRoughnessValue == clampedValue)
        return;
    leftSphereRoughnessValue = clampedValue;
    emit leftSphereRoughnessChanged();
    update();
}

float MarmosetItem::leftSphereMetallic() const
{
    return leftSphereMetallicValue;
}

void MarmosetItem::setLeftSphereMetallic(float value)
{
    const float clampedValue = std::max(0.0f, std::min(1.0f, value));
    if (leftSphereMetallicValue == clampedValue)
        return;
    leftSphereMetallicValue = clampedValue;
    emit leftSphereMetallicChanged();
    update();
}

float MarmosetItem::leftSphereAlbedoR() const
{
    return leftSphereAlbedoRValue;
}

void MarmosetItem::setLeftSphereAlbedoR(float value)
{
    const float clampedValue = std::max(0.0f, std::min(1.0f, value));
    if (leftSphereAlbedoRValue == clampedValue)
        return;
    leftSphereAlbedoRValue = clampedValue;
    emit leftSphereAlbedoRChanged();
    update();
}

float MarmosetItem::leftSphereAlbedoG() const
{
    return leftSphereAlbedoGValue;
}

void MarmosetItem::setLeftSphereAlbedoG(float value)
{
    const float clampedValue = std::max(0.0f, std::min(1.0f, value));
    if (leftSphereAlbedoGValue == clampedValue)
        return;
    leftSphereAlbedoGValue = clampedValue;
    emit leftSphereAlbedoGChanged();
    update();
}

float MarmosetItem::leftSphereAlbedoB() const
{
    return leftSphereAlbedoBValue;
}

void MarmosetItem::setLeftSphereAlbedoB(float value)
{
    const float clampedValue = std::max(0.0f, std::min(1.0f, value));
    if (leftSphereAlbedoBValue == clampedValue)
        return;
    leftSphereAlbedoBValue = clampedValue;
    emit leftSphereAlbedoBChanged();
    update();
}
float MarmosetItem::orbitYaw() const
{
    return orbitYawValue;
}

void MarmosetItem::setOrbitYaw(float value)
{
    if (orbitYawValue == value)
        return;
    orbitYawValue = value;
    emit orbitYawChanged();
    update();
}

float MarmosetItem::orbitPitch() const
{
    return orbitPitchValue;
}

void MarmosetItem::setOrbitPitch(float value)
{
    const float clampedValue = std::max(-65.0f, std::min(65.0f, value));
    if (orbitPitchValue == clampedValue)
        return;
    orbitPitchValue = clampedValue;
    emit orbitPitchChanged();
    update();
}

float MarmosetItem::orbitDistance() const
{
    return orbitDistanceValue;
}

void MarmosetItem::setOrbitDistance(float value)
{
    const float clampedValue = std::max(7.0f, std::min(17.0f, value));
    if (orbitDistanceValue == clampedValue)
        return;
    orbitDistanceValue = clampedValue;
    emit orbitDistanceChanged();
    update();
}
QQuickFramebufferObject::Renderer *MarmosetItem::createRenderer() const
{
    return new MarmosetRenderer;
}
