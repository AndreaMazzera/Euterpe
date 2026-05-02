import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Item {
    id: root
    width: Math.min(parent.width * 0.9, contentLayout.width + 64)
    height: 56
    z: 1000

    property string message: ""
    property string type: "success"
    property int duration: 2500

    readonly property color currentAccentColor: type === "success" ? "#3ECC63" : "#DF7366"
    readonly property color currentBgColor: type === "success" ? "#1D2A1F" : "#321F1C"

    signal shown()
    signal hidden()

    opacity: 0
    visible: opacity > 0
    enabled: visible

    function show(msg, toastType) {
        animIn.stop()
        animOut.stop()
        hideTimer.stop()

        message = msg
        type = toastType || "success"

        animIn.start()
        hideTimer.restart()
        shown()
    }

    Rectangle {
        id: background
        anchors.fill: parent
        radius: height / 2
        color: root.currentBgColor
        opacity: 0.98
        border.color: "#15FFFFFF"
        border.width: 1

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: "#80000000"
            shadowBlur: 0.6
            shadowVerticalOffset: 3
        }

        Row {
            id: contentLayout
            anchors.centerIn: parent
            spacing: 12

            Image {
                id: icon
                width: 22
                height: 22
                anchors.verticalCenter: parent.verticalCenter
                source: root.type === "success"
                        ? "qrc:/assets/icons/ic_success.svg"
                        : "qrc:/assets/icons/ic_fail.svg"
                fillMode: Image.PreserveAspectFit
            }

            Text {
                text: root.message
                color: root.currentAccentColor
                font.pixelSize: 16
                font.weight: Font.Medium
                antialiasing: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Timer {
        id: hideTimer
        interval: root.duration
        onTriggered: animOut.start()
    }

    NumberAnimation {
        id: animIn
        target: root
        property: "opacity"
        from: 0; to: 1
        duration: 200
        easing.type: Easing.OutCubic
    }

    NumberAnimation {
        id: animOut
        target: root
        property: "opacity"
        from: root.opacity; to: 0
        duration: 250
        easing.type: Easing.InCubic
        onFinished: root.hidden()
    }
}
