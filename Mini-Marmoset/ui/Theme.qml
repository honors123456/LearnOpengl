pragma Singleton
import QtQuick 2.12
import QtQml 2.12

// 全局主题单例，公共控件统一引用这里的颜色和字体，避免组件间样式漂移。
QtObject {
    // 背景色分为主背景、面板和抬高层；文字、强调和选中状态使用独立色值。
    readonly property color background: "#16191d"
    readonly property color surface: "#20252b"
    readonly property color elevated: "#292f37"
    readonly property color header: "#232e38"
    readonly property color border: "#38414b"
    readonly property color text: "#edf0f2"
    readonly property color muted: "#adb8c3"
    readonly property color accent: "#80cbb7"
    readonly property color selected: "#2b4945"
    readonly property color warning: "#e5bd7b"
    // 统一字体与基础字号；具体组件可按信息层级覆盖字号。
    readonly property string fontFamily: "Microsoft YaHei UI"
    readonly property int bodySize: 13
}
