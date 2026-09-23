import QtQuick 2.12
import QtQml 2.12
import QtQuick.Controls 2.5
import MiniMarmoset 1.0
import QtQuick.Layouts 1.12

ApplicationWindow {
    visible: true
    width: 1000
    height: 650
    title: qsTr("Mini-Marmoset · Qt Quick PBR")
    color: "#17191f"

    MarmosetView {
        id: marmosetView
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: materialPanel.left

        MouseArea {
            anchors.fill: marmosetView
            acceptedButtons: Qt.LeftButton

            property bool rotating: false
            property real lastX: 0
            property real lastY: 0

            onPressed: {
                rotating = true
                lastX = mouse.x
                lastY = mouse.y
            }

            onPositionChanged: {
                if (!rotating)
                    return

                marmosetView.orbitYaw += (mouse.x - lastX) * 0.2
                marmosetView.orbitPitch += (lastY - mouse.y) * 0.2
                lastX = mouse.x
                lastY = mouse.y
            }

            onReleased: {
                rotating = false
            }

            onCanceled: {
                rotating = false
            }

            onWheel: {
                marmosetView.orbitDistance -= wheel.angleDelta.y / 120 * 0.5
            }
        }
    }

    Rectangle {
        id: materialPanel
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width / 4
        color: "#cc171a21"
        border.color: "#44535d6b"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 18
            spacing: 8

            Label {
                text: qsTr("左侧球体材质")
                color: "#f4f7fb"
                font.pixelSize: 18
                font.bold: true
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("Roughness：%1").arg(roughnessSlider.value.toFixed(2))
                color: "#aeb8c4"
                font.pixelSize: 12
                Layout.fillWidth: true
            }

            Slider {
                id: roughnessSlider
                from: 0.05
                to: 1.0
                stepSize: 0.01
                value: marmosetView.leftSphereRoughness
                Layout.fillWidth: true
                onMoved: marmosetView.leftSphereRoughness = value
            }

            Label {
                text: qsTr("Metallic：%1").arg(metallicSlider.value.toFixed(2))
                color: "#aeb8c4"
                font.pixelSize: 12
                Layout.fillWidth: true
            }

            Slider {
                id: metallicSlider
                from: 0.0
                to: 1.0
                stepSize: 0.01
                value: marmosetView.leftSphereMetallic
                Layout.fillWidth: true
                onMoved: marmosetView.leftSphereMetallic = value
            }

            Label {
                text: qsTr("Albedo R：%1").arg(albedoRSlider.value.toFixed(2))
                color: "#aeb8c4"
                font.pixelSize: 12
                Layout.fillWidth: true
            }

            Slider {
                id: albedoRSlider
                from: 0.0
                to: 1.0
                stepSize: 0.01
                value: marmosetView.leftSphereAlbedoR
                Layout.fillWidth: true
                onMoved: marmosetView.leftSphereAlbedoR = value
            }

            Label {
                text: qsTr("Albedo G：%1").arg(albedoGSlider.value.toFixed(2))
                color: "#aeb8c4"
                font.pixelSize: 12
                Layout.fillWidth: true
            }

            Slider {
                id: albedoGSlider
                from: 0.0
                to: 1.0
                stepSize: 0.01
                value: marmosetView.leftSphereAlbedoG
                Layout.fillWidth: true
                onMoved: marmosetView.leftSphereAlbedoG = value
            }

            Label {
                text: qsTr("Albedo B：%1").arg(albedoBSlider.value.toFixed(2))
                color: "#aeb8c4"
                font.pixelSize: 12
                Layout.fillWidth: true
            }

            Slider {
                id: albedoBSlider
                from: 0.0
                to: 1.0
                stepSize: 0.01
                value: marmosetView.leftSphereAlbedoB
                Layout.fillWidth: true
                onMoved: marmosetView.leftSphereAlbedoB = value
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
