import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Euterpe

Button {
    id: actionButtonID

    property string iconPath: ""
    property int buttonSize: 32
    property real iconScale: 0.6
    property real cornerRadius: 0.25
    property string borderColor: "white"

    implicitWidth: buttonSize
    implicitHeight: buttonSize

    Layout.preferredWidth: buttonSize
    Layout.preferredHeight: buttonSize

    checkable: false
    autoRepeat: false

    background: Rectangle {
        anchors.fill: parent
        radius: buttonSize * cornerRadius
        border.color: actionButtonID.borderColor
        border.width: 1

        gradient: Gradient {
            GradientStop { position: 0.0; color: actionButtonID.down ? Colors.gradientStart : "transparent"}
            GradientStop { position: 0.5; color: actionButtonID.down ? Colors.gradientMiddle : "transparent" }
            GradientStop { position: 1.0; color: actionButtonID.down ? Colors.gradientEnd : "transparent" }
        }
    }

    contentItem: Item {
        anchors.fill: parent

        Image {
            anchors.centerIn: parent
            source: actionButtonID.iconPath
            width: buttonSize * iconScale
            height: buttonSize * iconScale
            fillMode: Image.PreserveAspectFit
        }
    }

    scale: actionButtonID.down ? 1.2 : 1.0
    Behavior on scale {
        NumberAnimation { duration: 240; easing.type: Easing.OutQuad }
    }
}

