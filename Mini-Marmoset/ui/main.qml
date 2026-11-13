import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import QtQml 2.12
import "."

// 工作台主窗口，组合模块导航、实验列表、视口、诊断区和参数检查器。
ApplicationWindow {
    id: window
    visible: true
    // 启动参数由 C++ 上下文传入；缺省时使用标准窗口尺寸。
    width: typeof compactWindow !== "undefined" && compactWindow ? 1080 : 1440
    height: typeof compactWindow !== "undefined" && compactWindow ? 720 : 900
    minimumWidth: 960
    minimumHeight: 640
    title: "Mini-Marmoset · " + (currentModule ? currentModule.title : "OpenGL Lab")
    color: Theme.background
    background: Rectangle { color: Theme.background }
    font.family: Theme.fontFamily
    font.pixelSize: Theme.bodySize
    palette.window: Theme.surface
    palette.windowText: Theme.text
    palette.base: Theme.background
    palette.text: Theme.text
    palette.button: Theme.elevated
    palette.buttonText: Theme.text
    palette.highlight: Theme.accent
    palette.highlightedText: Theme.background

    // 窗口状态：当前模块/阶段、诊断页签、面板开关和短时提示。
    property int moduleIndex: typeof initialModuleIndex !== "undefined" ? initialModuleIndex : 0
    property int stageIndex: 0
    property int diagnosticTab: 0
    property bool diagnosticsOpen: height >= 780
    property bool focusViewport: false
    property real experimentPanelWidth: 216
    property real inspectorPanelWidth: 292
    property real diagnosticPanelHeight: 190
    property bool experimentsOpen: width >= 1200
    property bool inspectorOpen: true
    property bool showGuides: true
    property string notice: ""
    readonly property var moduleNames: ["PBR / IBL", "Deferred", "SSAO", "GPU Simulation"]
    // 模块文件顺序与 --module 索引一致，加载后的对象保存在 loadedModules。
    readonly property var moduleFiles: ["PbrModule.qml", "DeferredModule.qml", "SsaoModule.qml", "GpuModule.qml"]
    property var loadedModules: [null, null, null, null]
    readonly property var currentModule: loadedModules[moduleIndex]

    // 切换模块时清空阶段选择和提示，不重建模块，因此保留各模块参数。
    function selectModule(index) {
        if (index < 0 || index >= 4) return
        moduleIndex = index
        stageIndex = 0
        notice = ""
    }
    // 显示短时操作反馈；重复通知重新启动清除定时器。
    function notify(message) { notice = message; noticeTimer.restart() }

    // 常驻 Loader 创建四个模块状态对象，实际视口由下方独立 Loader 创建。
    Repeater {
        id: moduleLoaders
        model: window.moduleFiles
        Loader {
            source: modelData
            visible: false
            onLoaded: {
                // 复制数组后赋回属性，触发依赖 currentModule 的界面更新。
                var modules = window.loadedModules.slice()
                modules[index] = item
                window.loadedModules = modules
            }
        }
    }

    // 提示自动清除定时器，不参与渲染计算。
    Timer
    {
        id: noticeTimer;
        interval: 3500;
        onTriggered:
            window.notice = ""
    }
    // 快捷键和鼠标操作共用状态逻辑，分别切换模块与诊断面板。
    Shortcut { sequence: "Ctrl+1"; onActivated: window.selectModule(0) }
    Shortcut { sequence: "Ctrl+2"; onActivated: window.selectModule(1) }
    Shortcut { sequence: "Ctrl+3"; onActivated: window.selectModule(2) }
    Shortcut { sequence: "Ctrl+4"; onActivated: window.selectModule(3) }
    Shortcut { sequence: "Ctrl+Shift+F"; onActivated: window.focusViewport = !window.focusViewport }
    Shortcut { sequence: "Escape"; enabled: window.focusViewport; onActivated: window.focusViewport = false }
    Shortcut { sequence: "Ctrl+D"; onActivated: window.diagnosticsOpen = !window.diagnosticsOpen }

    // 主布局纵向分为顶部导航、中间工作区和底部状态栏。
    ColumnLayout {
        objectName: "workbenchLayout"
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 68
            color: Theme.header
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 22; anchors.rightMargin: 20
                spacing: 24
                ColumnLayout {
                    spacing: 2
                    Label { text: "Mini-Marmoset"; color: Theme.text; font.pixelSize: 19; font.weight: Font.DemiBold }
                }
                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 28; color: Theme.border }
                RowLayout {
                    spacing: 6
                    Repeater {
                        model: window.moduleNames
                        LabButton {
                            objectName: "moduleButton" + index
                            text: modelData
                            checkable: true
                            selected: window.moduleIndex === index
                            onClicked: window.selectModule(index)
                            ToolTip.visible: hovered
                            ToolTip.text: "切换模块 · Ctrl+" + (index + 1)
                        }
                    }
                }
                Item { Layout.fillWidth: true }
                Label { text: "工作台骨架"; color: Theme.warning; font.pixelSize: 12; visible: window.width >= 1200 }
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                // 左侧列表由当前模块 experiments 提供，按面板开关控制显示。
                visible: window.experimentsOpen && !window.focusViewport
                Layout.preferredWidth: Math.min(window.experimentPanelWidth, window.width - 650)
                Layout.fillHeight: true
                color: Theme.surface
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 16
                    spacing: 8
                    Label { text: "实验"; color: Theme.text; font.pixelSize: 15; font.weight: Font.DemiBold; Layout.topMargin: 8 }
                    Label { text: window.currentModule ? window.currentModule.subtitle : ""; color: Theme.muted; font.pixelSize: 11; Layout.bottomMargin: 18 }
                    Repeater {
                        model: window.currentModule ? window.currentModule.experiments : []
                        delegate: LabButton {
                            objectName: "experimentButton" + index
                            Layout.fillWidth: true
                            implicitHeight: 42
                            text: modelData.title
                            checkable: true
                            selected: window.currentModule && window.currentModule.experimentIndex === index
                            onClicked: {
                                window.currentModule.experimentIndex = index
                                window.notify("已选择实验：" + modelData.title)
                            }
                        }
                    }
                    Item { Layout.fillHeight: true }
                    Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }
                    Label {
                        text: "同一工作台切换四个模块，\n共享界面与交互组件。"
                        color: Theme.muted; font.pixelSize: 11; lineHeight: 1.6
                        Layout.topMargin: 10; Layout.bottomMargin: 8
                    }
                    LabButton {
                        text: "查看接入说明"
                        Layout.fillWidth: true
                        onClicked: { window.diagnosticsOpen = true; window.diagnosticTab = 1 }
                    }
                }
            }
            Rectangle {
                visible: window.experimentsOpen && !window.focusViewport
                Layout.fillHeight: true; Layout.preferredWidth: 7; color: Theme.background
                Rectangle { anchors.centerIn: parent; width: 1; height: parent.height; color: Theme.border }
                MouseArea {
                    objectName: "experimentResizeHandle"
                    anchors.fill: parent; cursorShape: Qt.SizeHorCursor
                    property real startX
                    property real startWidth
                    activeFocusOnTab: true
                    Accessible.role: Accessible.Slider
                    Accessible.name: "实验列表宽度，左右方向键调整"
                    Keys.onLeftPressed: window.experimentPanelWidth = Math.max(190, window.experimentPanelWidth - 10)
                    Keys.onRightPressed: window.experimentPanelWidth = Math.min(280, window.experimentPanelWidth + 10)
                    Rectangle { anchors.fill: parent; color: parent.activeFocus ? Theme.accent : "transparent" }
                    onPressed: { forceActiveFocus(); startX = mapToItem(window.contentItem, mouse.x, 0).x; startWidth = window.experimentPanelWidth }
                    onPositionChanged: if (pressed) window.experimentPanelWidth = Math.max(190, Math.min(280, startWidth + (mapToItem(window.contentItem, mouse.x, 0).x - startX)))
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 370
                spacing: 0

                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 52
                    color: Theme.surface
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12; spacing: 8
                        LabButton {
                            objectName: "experimentsToggle"
                            enabled: !window.focusViewport
                            checkable: true
                            text: "实验列表"; selected: window.experimentsOpen
                            onClicked: window.experimentsOpen = !window.experimentsOpen
                        }
                        ComboBox {
                            // 视口上方的选择器与左侧实验列表共用 experimentIndex。
                            id: experimentSelect
                            Layout.fillWidth: true; Layout.maximumWidth: 220
                            implicitHeight: 34
                            textRole: "title"
                            model: window.currentModule ? window.currentModule.experiments : []
                            currentIndex: window.currentModule ? window.currentModule.experimentIndex : 0
                            onActivated: if (window.currentModule) window.currentModule.experimentIndex = index
                            Accessible.name: "当前实验"
                        }
                        Item { Layout.fillWidth: true }
                        LabButton {
                            objectName: "focusViewportToggle"
                            text: window.focusViewport ? "退出专注" : "专注视口"
                            checkable: true
                            selected: window.focusViewport
                            onClicked: window.focusViewport = !window.focusViewport
                            ToolTip.visible: hovered || activeFocus
                            ToolTip.text: "专注视口 · Ctrl+Shift+F；Esc 退出"
                        }
                        LabButton {
                            objectName: "inspectorToggle"
                            enabled: !window.focusViewport
                            checkable: true
                            text: "参数"; selected: window.inspectorOpen
                            onClicked: window.inspectorOpen = !window.inspectorOpen
                        }
                    }
                }
                Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

                Label {
                    Layout.fillWidth: true
                    Layout.leftMargin: 16; Layout.rightMargin: 16
                    Layout.topMargin: 10; Layout.bottomMargin: 10
                    visible: !window.focusViewport
                    text: window.currentModule ? window.currentModule.experimentDescription : "正在加载实验…"
                    background: Rectangle { color: Theme.background }
                    color: Theme.muted; font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }

                Item {
                    id: viewportArea
                    Layout.fillWidth: true;
                    Layout.fillHeight: true;
                    Layout.minimumHeight: 180

                    clip: true

                    Loader {
                        // 仅实例化当前模块的视口组件，切换组件时销毁旧视口。
                        id: viewportLoader
                        anchors.fill: parent
                        sourceComponent: window.currentModule ? window.currentModule.viewportComponent : null
                        onLoaded: if (item && "showGuides" in item) item.showGuides = Qt.binding(function() { return window.showGuides })
                    }

                    Row {
                        anchors.top: parent.top; anchors.left: parent.left; anchors.margins: 16
                        spacing: 8
                        Rectangle {
                            width: viewportLabel.implicitWidth + 20; height: 28; radius: 4
                            color: Theme.surface; border.color: Theme.border
                            Label { id: viewportLabel; anchors.centerIn: parent; text: "视口 / " + (window.currentModule && window.currentModule.rendererReady ? "已接入" : "待接入"); color: Theme.muted; font.pixelSize: 11 }
                        }
                    }

                    LabButton {
                        anchors.top: parent.top;
                        anchors.right: parent.right;
                        anchors.margins: 16
                        visible: viewportLoader.item !== null && "showGuides" in viewportLoader.item
                        text: "辅助网格";
                        checkable: true
                        selected: window.showGuides
                        onClicked: window.showGuides = !window.showGuides
                    }
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }
            }

            Rectangle {
                visible: window.inspectorOpen && !window.focusViewport
                Layout.fillHeight: true; Layout.preferredWidth: 7; color: Theme.background
                Rectangle { anchors.centerIn: parent; width: 1; height: parent.height; color: Theme.border }
                MouseArea {
                    objectName: "inspectorResizeHandle"
                    anchors.fill: parent; cursorShape: Qt.SizeHorCursor
                    property real startX
                    property real startWidth
                    activeFocusOnTab: true
                    Accessible.role: Accessible.Slider
                    Accessible.name: "参数面板宽度，左右方向键调整"
                    Keys.onLeftPressed: window.inspectorPanelWidth = Math.max(250, window.inspectorPanelWidth - 10)
                    Keys.onRightPressed: window.inspectorPanelWidth = Math.min(400, window.inspectorPanelWidth + 10)
                    Rectangle { anchors.fill: parent; color: parent.activeFocus ? Theme.accent : "transparent" }
                    onPressed: { forceActiveFocus(); startX = mapToItem(window.contentItem, mouse.x, 0).x; startWidth = window.inspectorPanelWidth }
                    onPositionChanged: if (pressed) window.inspectorPanelWidth = Math.max(250, Math.min(400, startWidth - (mapToItem(window.contentItem, mouse.x, 0).x - startX)))
                }
            }
            // 右侧按参数规格动态创建编辑控件，重置仅影响当前模块。
            Rectangle {
                visible: window.inspectorOpen && !window.focusViewport
                Layout.preferredWidth: Math.min(window.inspectorPanelWidth, window.width < 1200 ? 280 : 400)
                Layout.fillHeight: true
                color: Theme.surface
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 20
                    spacing: 14
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "实验参数"; color: Theme.text; font.pixelSize: 15; font.weight: Font.DemiBold; Layout.fillWidth: true }
                        LabButton {
                            objectName: "resetParameters"
                            text: "重置"; implicitWidth: 54; implicitHeight: 28
                            onClicked: {
                                if (window.currentModule) window.currentModule.resetParameters()
                                window.notify("当前模块参数已恢复默认")
                            }
                        }
                    }
                    Label {
                        text: window.currentModule && window.currentModule.rendererReady ? "调整当前实验的渲染参数。" : "参数可编辑，渲染逻辑待接入。"
                        color: Theme.muted; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true
                    }
                    Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

                    ScrollView {
                        Layout.fillWidth: true;
                        Layout.fillHeight: true;
                        clip: true;
                        contentWidth: availableWidth
                        ColumnLayout {
                            width: parent.width
                            spacing: 22
                            Repeater {
                                model: window.currentModule ? window.currentModule.parameterSpecs : []
                                ParameterControl { Layout.fillWidth: true; module: window.currentModule; spec: modelData }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true;
                        height: 1;
                        color: Theme.border
                    }

                    Label {
                        text: "参数保留在当前会话中。\n切换模块不会丢失，关闭程序后重置。"
                        color: Theme.muted; font.pixelSize: 11; lineHeight: 1.5
                        wrapMode: Text.WordWrap; Layout.fillWidth: true
                    }
                }
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }
        Rectangle {
            // 底栏展示提示、视口逻辑尺寸和 GPU 数据接入状态，不伪造性能指标。
            Layout.fillWidth: true; Layout.preferredHeight: 30; color: Theme.header
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16; spacing: 20
                Label { text: window.notice || (window.currentModule && window.currentModule.rendererReady ? "就绪 · 渲染器已接入" : "就绪 · 渲染器待接入"); color: window.notice ? Theme.accent : Theme.muted; font.pixelSize: 11; elide: Text.ElideRight; Layout.fillWidth: true }
                Label { text: Math.round(viewportLoader.width) + " × " + Math.round(viewportLoader.height) + " 逻辑像素"; color: Theme.muted; font.pixelSize: 11 }
                Label { text: "GPU 数据：未接入"; color: Theme.muted; font.pixelSize: 11 }
            }
        }
    }
}
