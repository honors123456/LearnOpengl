#include "PbrLab.h"

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLExtraFunctions>
#include <QQuickWindow>
#include <Shader/shader.h>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstddef>
#include <cmath>
#include <QMouseEvent>
#include <QWheelEvent>

namespace {
// 每个 lab 独立实现渲染，后续算法和 GL 资源放在本模块内部。
class PbrLabRenderer : public QQuickFramebufferObject::Renderer
{
public:
    ~PbrLabRenderer() override
    {
        // Qt Quick 在渲染线程销毁渲染器，此时上下文是当前的。
        auto *context = QOpenGLContext::currentContext();
        if (!context)
            return;
        auto *gl = context->extraFunctions();

        if(gl == nullptr)
        {
            qDebug()<<" initialized opengl function failed!";
            return;
        }

        gl->glDeleteVertexArrays(1, &m_sphereVao);
        gl->glDeleteBuffers(1, &m_sphereVbo);
        gl->glDeleteBuffers(1, &m_sphereEbo);

        if(m_sphereShader) delete m_sphereShader;
        if(m_lightShader) delete m_lightShader;
    }

    void synchronize(QQuickFramebufferObject *item) override
    {
        auto *lab = static_cast<PbrLab *>(item);
        m_parameters = lab->parameters();
        m_experimentIndex = lab->experimentIndex();
        m_orbit = lab->orbit();
        m_window = lab->window();
        sphereData = lab->getSphereData();
    }

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override
    {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        return new QOpenGLFramebufferObject(size, format);
    }

    void render() override
    {
        // Qt 已绑定本视口 FBO 并设置 viewport；这里只提交本模块的绘制。
        auto *gl = QOpenGLContext::currentContext()->extraFunctions();
        if(gl == nullptr)
        {
            qDebug()<<" initialized opengl function failed!";
            return;
        }

        gl->glClearColor(0.055f, 0.063f, 0.075f, 1.0f);
        gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        gl->glEnable(GL_DEPTH_TEST);

        // 初始化 GL 资源
        if(!m_bInitialized)
        {
            initializeResources(gl);
            m_bInitialized = true;
        }

        const QSize size = framebufferObject()->size();
        if (size.width() <= 0 || size.height() <= 0) {
            if (m_window)
            {
                gl->glBindVertexArray(0);
                m_window->resetOpenGLState();
            }
            return;
        }
        const float yaw = glm::radians(m_orbit.x());
        const float pitch = glm::radians(m_orbit.y());
        const float distance = m_orbit.z();
        m_cameraPosition = glm::vec3(
            distance * std::cos(pitch) * std::cos(yaw),
            distance * std::sin(pitch),
            distance * std::cos(pitch) * std::sin(yaw));
        m_view = glm::lookAt(m_cameraPosition,glm::vec3(0.0f),glm::vec3(0.0f, 1.0f, 0.0f));
        m_projection = glm::perspective(glm::radians(60.0f),static_cast<float>(size.width()) / size.height(),0.1f,100.0f);

        // 使用材质着色器
        m_sphereShader->use();
        m_sphereShader->setMat4("view",m_view);
        m_sphereShader->setMat4("projection",m_projection);

        // 材质参数
        const float metallic = parameterValue("metallic", sphereData.material.metallic);
        const float roughness = parameterValue("roughness", sphereData.material.roughness);
        const float exposure = parameterValue("exposure", 1.0f);

        m_sphereShader->setVec3("albedo",sphereData.material.albedo);
        m_sphereShader->setFloat("metallic",metallic);
        m_sphereShader->setFloat("roughness",roughness);
        m_sphereShader->setFloat("ao",sphereData.material.ao);
        m_sphereShader->setFloat("exposure",exposure);

        // 光源参数
        m_sphereShader->setVec3("lightPos",m_lightPosition);
        m_sphereShader->setVec3("lightColor",m_lightColor);

        // 相机位置
        m_sphereShader->setVec3("cameraPos",m_cameraPosition);

        gl->glBindVertexArray(m_sphereVao);
        for(int i=0;i<5;i++)
        {
            for(int j=0;j<5;j++)
            {
                int index = i * 5 + j;
                glm::mat4 model = glm::translate(glm::mat4(1.0f),m_spherePositions[index]);
                model = glm::scale(model, glm::vec3(0.82f));

                m_sphereShader->setMat4("model",model);
                gl->glDrawElements(GL_TRIANGLES,static_cast<GLsizei>(sphereData.indices.size()),GL_UNSIGNED_INT,nullptr);
            }
        }

        // 绘制光源小球
        m_lightShader->use();
        m_lightShader->setMat4("view",m_view);
        m_lightShader->setMat4("projection",m_projection);

        gl->glBindVertexArray(m_sphereVao);
        glm::mat4 model = glm::translate(glm::mat4(1.0f),m_lightPosition);
        model = glm::scale(model, glm::vec3(0.14f));
        m_lightShader->setMat4("model",model);
        m_lightShader->setVec3("lightColor",m_lightColor * 0.025f);
        gl->glDrawElements(GL_TRIANGLES,static_cast<GLsizei>(sphereData.indices.size()),GL_UNSIGNED_INT,nullptr);


        // 与 Qt Quick 共用上下文，返回前恢复其依赖的 GL 状态。
        if (m_window)
        {
            gl->glBindVertexArray(0);
            m_window->resetOpenGLState();
        }
    }

private:
    // 从界面参数读取数值，参数缺失时回退到网格默认材质。
    float parameterValue(const char *key, float fallback) const
    {
        const QVariant value = m_parameters.value(QLatin1String(key));
        return value.isValid() ? value.toFloat() : fallback;
    }

    void initializeResources(QOpenGLExtraFunctions* gl)
    {
        // 相机

        // 球体顶点数据
        // 生成顶点数组对象、顶点缓冲和索引缓冲
        gl->glGenVertexArrays(1,&m_sphereVao);
        gl->glGenBuffers(1,&m_sphereVbo);
        gl->glGenBuffers(1,&m_sphereEbo);

        // 绑定顶点数组对象
        gl->glBindVertexArray(m_sphereVao);

        // 绑定顶点缓冲
        gl->glBindBuffer(GL_ARRAY_BUFFER,m_sphereVbo);
        gl->glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(sphereData.vertices.size() * sizeof(marmoset::Vertex)),sphereData.vertices.data(),GL_STATIC_DRAW);

        // 绑定索引缓冲并上传索引数据
        gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_sphereEbo);
        gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,static_cast<GLsizeiptr>(sphereData.indices.size() * sizeof(unsigned int)),sphereData.indices.data(),GL_STATIC_DRAW);

        // 指定顶点属性布局
        // 位置属性
        gl->glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8 * sizeof(float),(void*)0);
        gl->glEnableVertexAttribArray(0);

        // 法线属性
        gl->glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8 * sizeof(float),(void*)(3 * sizeof(float)));
        gl->glEnableVertexAttribArray(1);

        // 纹理坐标属性
        gl->glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8 * sizeof(float),(void*)(6 * sizeof(float)));
        gl->glEnableVertexAttribArray(2);


        // 球体实例位置
        for(int i=0;i<5;i++)
        {
            for(int j=0;j<5;j++)
            {
                glm::vec3 origin(-3.2f,-3.2f,0.0f);
                m_spherePositions.push_back(glm::vec3(origin.x+i*1.6,origin.y+j*1.6,0.0f));
            }
        }

        // 光源位置
        m_lightPosition = glm::vec3(4.5f, 4.5f, 4.0f);

        // 光源颜色
        m_lightColor = glm::vec3(130.0f, 118.0f, 94.0f);

        // 着色器
        m_sphereShader = new Shader("../Pbr_lab/pbrShader.vert","../Pbr_lab/pbrShader.frag");
        m_lightShader = new Shader("../Pbr_lab/lightShader.vert","../Pbr_lab/lightShader.frag");


    }

private:
    // 渲染数据
    GLuint m_sphereVao = 0;
    GLuint m_sphereVbo = 0;
    GLuint m_sphereEbo = 0;
    marmoset::MeshData sphereData;

    // 着色器实例
    Shader* m_sphereShader{nullptr};
    Shader* m_lightShader{nullptr};

    // 球体实例位置
    QVector<glm::vec3> m_spherePositions;

    // 光源位置
    glm::vec3 m_lightPosition;
    glm::vec3 m_lightColor;

    // 渲染矩阵
    glm::mat4 m_view;
    glm::mat4 m_projection;

    // 相机
    glm::vec3 m_cameraPosition{0.0f, 0.0f, 11.0f};
    QVector3D m_orbit{90.0f, 2.0f, 11.0f};

    QVariantMap m_parameters;
    int m_experimentIndex = 0;
    QQuickWindow *m_window = nullptr;

    bool m_bInitialized{false};
};
}

PbrLab::PbrLab(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
    setMirrorVertically(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setKeepMouseGrab(true);

    //static MeshData sphere(float radius = 1.0f, std::uint32_t segments = 64,std::uint32_t rings = 32, const Material &mat = {})
    sphereData = marmoset::MeshGenerator::sphere(0.8f);
}

QQuickFramebufferObject::Renderer *PbrLab::createRenderer() const
{
    return new PbrLabRenderer;
}

void PbrLab::setParameters(const QVariantMap &parameters)
{
    if (m_parameters == parameters)
        return;
    m_parameters = parameters;
    emit parametersChanged();
    update();
}

void PbrLab::setExperimentIndex(int experimentIndex)
{
// 无效索引不进入渲染线程；实验上界由对应 QML 模块的列表约束。
    if (experimentIndex < 0 || m_experimentIndex == experimentIndex)
        return;
    m_experimentIndex = experimentIndex;
    emit experimentIndexChanged();
    update();
}

marmoset::MeshData PbrLab::getSphereData() const
{
    return sphereData;
}

void PbrLab::keyPressEvent(QKeyEvent *event)
{

}

void PbrLab::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_rotating || !(event->buttons() & Qt::LeftButton)) {
        event->ignore();
        return;
    }

    float currentX = static_cast<float>(event->localPos().x());
    float currentY = static_cast<float>(event->localPos().y());

    const float deltaX = currentX - m_lastPos.x();
    const float deltaY = currentY - m_lastPos.y();
    m_orbit.setX(m_orbit.x() + deltaX * 0.2f);
    m_orbit.setY(glm::clamp(m_orbit.y() + deltaY * 0.2f, -89.0f, 89.0f));

    m_lastPos.setX(currentX);
    m_lastPos.setY(currentY);
    event->accept();
    update();
}

void PbrLab::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }
    m_rotating = true;
    m_lastPos = QVector2D(event->localPos());
    event->accept();
}

void PbrLab::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }
    m_rotating = false;
    event->accept();
}

void PbrLab::mouseUngrabEvent()
{
    m_rotating = false;
    QQuickFramebufferObject::mouseUngrabEvent();
}

void PbrLab::wheelEvent(QWheelEvent *event)
{
    float factor = event->angleDelta().y() / 120.0f;
    float orbitDistance = m_orbit.z();
    orbitDistance -= factor * 0.5f;
    orbitDistance = glm::clamp(orbitDistance, 2.0f, 20.0f);
    m_orbit.setZ(orbitDistance);
    event->accept();

    update();
}

void PbrLab::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickFramebufferObject::geometryChanged(newGeometry,oldGeometry);
    update();
}
