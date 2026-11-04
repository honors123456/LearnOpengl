import QtQuick 2.12
import QtQuick.Controls 2.5

// 工作台公共按钮：统一尺寸、键盘焦点、选中态与悬停样式。
Button {
    id: control
    // selected 由上层业务状态驱动，与鼠标按下时的 down 状态分开。
    property bool selected: false
    implicitHeight: 34
    implicitWidth: Math.max(72, label.implicitWidth + 24)
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus
    font.family: Theme.fontFamily
    font.pixelSize: Theme.bodySize
    // 文本颜色响应禁用和选中状态，过长文字显示省略号。
    contentItem: Text {
        id: label
        text: control.text
        font: control.font
        color: !control.enabled ? Theme.muted : control.selected ? Theme.accent : Theme.text
        opacity: control.enabled ? 1 : 0.6
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    // 背景区分按下、选中、悬停；键盘聚焦时用强调色边框提示。
    background: Rectangle {
        radius: 4
        color: control.down ? Theme.selected : control.selected ? Theme.selected : control.hovered ? Theme.elevated : "transparent"
        border.color: control.activeFocus ? Theme.accent : control.selected ? Theme.selected : Theme.border
    }
}
