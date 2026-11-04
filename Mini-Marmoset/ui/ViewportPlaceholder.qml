import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12

// 渲染器尚未接入时的说明视口；网格和图标只用于布局示意。
Rectangle {
    id: root
    property var module
    property bool showGuides: true
    color: Theme.background

    // 绘制二维辅助网格，尺寸改变时请求重绘，不创建 OpenGL 场景。
    Canvas {
        id: guides
        anchors.fill: parent
        visible: root.showGuides
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.strokeStyle = "#252b31"
            ctx.lineWidth = 1
            // 用汇聚线和非线性分布的横线模拟透视感，仅为视觉引导。
            var horizon = height * 0.58
            for (var x = -width * 2; x <= width * 3; x += 100) {
                ctx.beginPath(); ctx.moveTo(width / 2 + (x - width / 2) * 0.12, horizon)
                ctx.lineTo(x, height); ctx.stroke()
            }
            for (var i = 1; i <= 9; ++i) {
                var y = horizon + (height - horizon) * Math.pow(i / 9, 2)
                ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(width, y); ctx.stroke()
            }
        }
    }

    // 居中显示实验名称、接入状态和说明，宽度随视口收缩。
    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(440, parent.width - 64)
        spacing: 14
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            width: 52; height: 52
            radius: 10
            color: Theme.surface
            border.color: Theme.border
            Canvas {
                anchors.fill: parent
                onPaint: {
                    var c = getContext("2d")
                    c.strokeStyle = "#80cbb7"; c.lineWidth = 1.5
                    c.beginPath(); c.moveTo(26, 12); c.lineTo(39, 19); c.lineTo(39, 33)
                    c.lineTo(26, 40); c.lineTo(13, 33); c.lineTo(13, 19); c.closePath()
                    c.moveTo(13, 19); c.lineTo(26, 26); c.lineTo(39, 19)
                    c.moveTo(26, 26); c.lineTo(26, 40); c.stroke()
                }
            }
        }
        Label {
            text: root.module ? root.module.experimentTitle : "渲染视口"
            color: Theme.text; font.pixelSize: 22; font.weight: Font.DemiBold
            Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
        Label {
            text: "视口已就绪，等待接入渲染器"
            color: Theme.accent; font.pixelSize: 13
            Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter
        }
        Label {
            text: root.module ? root.module.experimentDescription : ""
            color: Theme.muted; font.pixelSize: 13; lineHeight: 1.5
            Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }
    // 明确标注网格不是渲染结果，并随辅助网格开关一起显示。
    Label {
        anchors.left: parent.left; anchors.bottom: parent.bottom; anchors.margins: 18
        text: "布局辅助网格 · 非渲染结果"
        color: Theme.muted; font.pixelSize: 11
        visible: root.showGuides
    }
}
