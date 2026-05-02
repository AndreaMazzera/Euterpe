import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Euterpe

Rectangle {
    id: control

    property string label: "Placeholder"
    property alias text: titleField.text
    property alias textField: titleField
    property alias font: titleField.font
    property alias inputHints: titleField.inputMethodHints

    property color activeColor: "#FFFFFF"
    property color idleColor: "#4DFFFFFF"
    property color backgroundColor: "#1E1E1E"

    property int preferredHeight: 42
    property int borderRadius: 6

    signal inputChanged(string newText)

    readonly property bool isFloating: titleField.activeFocus || titleField.text.length > 0

    height: preferredHeight
    Layout.fillWidth: true
    Layout.topMargin: 10
    radius: borderRadius
    color: "transparent"

    border.width: titleField.activeFocus ? 2 : 1
    border.color: titleField.activeFocus ? activeColor : idleColor

    Behavior on border.color { ColorAnimation { duration: 150 } }

    Rectangle {
        id: labelWrapper
        x: 10
        y: control.isFloating ? -height / 2 : (parent.height - height) / 2
        height: 14
        width: titleLabel.implicitWidth + 8
        z: 2
        color: control.isFloating ? backgroundColor : "transparent"

        Text {
            id: titleLabel
            anchors.centerIn: parent
            text: control.label
            font.pixelSize: control.isFloating ? 10 : 14
            font.bold: control.isFloating

            color: titleField.activeFocus ? activeColor :
                   (control.isFloating ? "#CCFFFFFF" : idleColor)

            Behavior on font.pixelSize { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }
            Behavior on color { ColorAnimation { duration: 200 } }
        }

        Behavior on y {
            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
        }
    }

    TextField {
        id: titleField
        anchors.fill: parent
        anchors.topMargin: control.isFloating ? 4 : 0

        verticalAlignment: TextInput.AlignVCenter
        leftPadding: 12
        rightPadding: 12
        color: "white"
        background: Item {}

        selectByMouse: true
        activeFocusOnTab: true

        onTextChanged: control.inputChanged(text)

        cursorDelegate: Rectangle {
            width: 1
            color: "white"
            visible: titleField.activeFocus && titleField.cursorVisible
        }
    }

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: true
        onPressed: (mouse) => {
            titleField.forceActiveFocus();
            mouse.accepted = false;
        }
    }
}
