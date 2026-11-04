import QtQuick 2.12

// 各实验 QML 的公共契约：描述实验、参数和管线阶段，默认显示占位视口。
Item {
    id: root
    // 模块身份与接入位置，仅作为展示和路由元数据。
    property string moduleId
    property string title
    property string subtitle
    property string entryPath
    // 只有真实渲染器接入后才设为 true；可编辑参数不代表算法已经运行。
    property bool rendererReady: false
    // 实验列表和当前选择，由工作台导航控件读取与修改。
    property var experiments: []
    property int experimentIndex: 0
    // 参数规格约束编辑范围，defaults 为初始值，parameters 为本次会话的当前值。
    property var parameterSpecs: []
    property var defaults: ({})
    property var parameters: ({})
    property var stages: []
    // 视口通过组件注入；具体模块可用真正的渲染 Item 替换占位实现。
    property Component viewportComponent: Component {
        ViewportPlaceholder { module: root }
    }
    readonly property string experimentTitle: experiments.length ? experiments[experimentIndex].title : ""
    readonly property string experimentDescription: experiments.length ? experiments[experimentIndex].description : ""

    // 查找参数规格并拒绝未知键、非数值和非有限值，然后限制到合法范围。
    function setParameter(key, value) {
        var spec = null
        for (var i = 0; i < parameterSpecs.length; ++i)
            if (parameterSpecs[i].key === key) spec = parameterSpecs[i]
        if (!spec || typeof value !== "number" || !isFinite(value)) return
        // 复制参数对象后整体赋值，触发 QML 属性通知，让输入框和滑块同步更新。
        var next = Object.assign({}, parameters)
        next[key] = Math.max(spec.from, Math.min(spec.to, value))
        parameters = next
    }
    // 复制默认值而非共享同一对象，避免编辑当前值时污染默认配置。
    function resetParameters() { parameters = Object.assign({}, defaults) }
    // 模块创建完成后初始化参数；模块常驻时可保留会话状态。
    Component.onCompleted: resetParameters()
}
