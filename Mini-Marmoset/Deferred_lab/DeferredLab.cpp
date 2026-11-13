#include "DeferredLab.h"

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQuickWindow>

namespace {
// 每个 lab 独立实现渲染，后续算法和 GL 资源放在本模块内部。
class DeferredLabRenderer : public QQuickFramebufferObject::Renderer
{
public:
    void synchronize(QQuickFramebufferObject *item) override
    {
        // 此时 GUI 线程被阻塞；复制快照后，render() 不再访问 QML 对象。
        auto *lab = static_cast<DeferredLab *>(item);
        m_parameters = lab->parameters();
        m_experimentIndex = lab->experimentIndex();
        m_window = lab->window();
    }

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override
    {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        return new QOpenGLFramebufferObject(size, format);
    }

    void render() override
    {
        // Qt 已绑定本视口 FBO 并设置 viewport；当前只清屏，不模拟实验结果。
        auto *gl = QOpenGLContext::currentContext()->functions();
        gl->glDisable(GL_SCISSOR_TEST);
        gl->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        gl->glDepthMask(GL_TRUE);
        gl->glStencilMask(0xff);
        gl->glClearColor(0.055f, 0.063f, 0.075f, 1.0f);
        gl->glClearDepthf(1.0f);
        gl->glClearStencil(0);
        gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        // 与 Qt Quick 共用上下文，返回前恢复其依赖的 GL 状态。
        if (m_window)
            m_window->resetOpenGLState();
    }

private:
    QVariantMap m_parameters;
    int m_experimentIndex = 0;
    QQuickWindow *m_window = nullptr;
};
}

DeferredLab::DeferredLab(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
    setMirrorVertically(true);
}

QQuickFramebufferObject::Renderer *DeferredLab::createRenderer() const
{
    return new DeferredLabRenderer;
}

void DeferredLab::setParameters(const QVariantMap &parameters)
{
    if (m_parameters == parameters)
        return;
    m_parameters = parameters;
    emit parametersChanged();
    update();
}

void DeferredLab::setExperimentIndex(int experimentIndex)
{
    // 无效索引不进入渲染线程；实验上界由对应 QML 模块的列表约束。
    if (experimentIndex < 0 || m_experimentIndex == experimentIndex)
        return;
    m_experimentIndex = experimentIndex;
    emit experimentIndexChanged();
    update();
}
