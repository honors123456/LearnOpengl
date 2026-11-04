import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12

// 单个数值参数编辑器：文本输入和滑块共用模块中的同一参数值。
ColumnLayout {
    id: root
    property var module
    property var spec
    // 参数未就绪时显示 0，正常情况下以模块参数为唯一数据源。
    readonly property real currentValue: module && spec && module.parameters[spec.key] !== undefined ? module.parameters[spec.key] : 0
    spacing: 5

    // 上方显示参数名称和精确数值输入，输入验证范围来自参数规格。
    RowLayout {
        Layout.fillWidth: true
        Label { text: root.spec.label; color: Theme.text; font.pixelSize: 12; Layout.fillWidth: true }
        TextField {
            id: input
            objectName: "parameterInput_" + root.spec.key
            Layout.preferredWidth: 82
            implicitHeight: 29
            leftPadding: 6
            rightPadding: 6
            selectByMouse: true
            color: Theme.text
            font.pixelSize: 12
            horizontalAlignment: Text.AlignRight
            text: root.currentValue.toFixed(root.spec.decimals || 0)
            validator: DoubleValidator { bottom: root.spec.from; top: root.spec.to; decimals: root.spec.decimals || 0; locale: "C" }
            background: Rectangle { color: Theme.background; radius: 3; border.color: input.activeFocus ? Theme.accent : Theme.border }
            // 完成编辑或失焦时提交合法数值，非法/空输入恢复当前值。
            function commitOrRestore() {
                var number = Number(text)
                if (text.trim().length && acceptableInput && isFinite(number)) root.module.setParameter(root.spec.key, number)
                // 手动编辑会打断 text 绑定，提交后重新绑定，继续响应滑块和重置。
                text = Qt.binding(function() { return root.currentValue.toFixed(root.spec.decimals || 0) })
            }
            onEditingFinished: commitOrRestore()
            onActiveFocusChanged: if (!activeFocus) commitOrRestore()
            Accessible.name: root.spec.label
        }
    }
    // 拖动时通过模块的 setParameter() 更新数值，统一执行边界检查。
    Slider {
        id: slider
        objectName: "parameterSlider_" + root.spec.key
        Layout.fillWidth: true
        from: root.spec.from; to: root.spec.to; stepSize: root.spec.step
        value: root.currentValue
        onMoved: root.module.setParameter(root.spec.key, value)
        Accessible.name: root.spec.label
        // 自定义滑轨和已选进度，位置按控件可用尺寸计算。
        background: Rectangle {
            x: slider.leftPadding
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            width: slider.availableWidth; height: 3; radius: 1
            color: Theme.border
            Rectangle { width: slider.visualPosition * parent.width; height: parent.height; color: Theme.accent; radius: 1 }
        }
        // 滑块把手响应按下和键盘焦点；visualPosition 支持控件方向。
        handle: Rectangle {
            x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            width: 12; height: 12; radius: 6
            color: slider.pressed ? Theme.text : Theme.accent
            border.width: slider.activeFocus ? 2 : 0; border.color: Theme.text
        }
    }
}
