import QtQuick 2.15
import QtQuick.Layouts
import QtQuick.Controls 2.15
import Euterpe

Item {
    id: root

    property alias text: searchField.text
    property bool hasText: false
    property real barHeight: parent ? parent.height * 0.065 : 60
    property int iconSize: Math.max(32, barHeight * 0.6)
    signal searchClicked()
    signal searchTextChanged(string value)

    width: parent ? parent.width : 300
    height: barHeight

    Rectangle {
        id: glassBackground
        anchors.fill: parent
        radius: height / 2
        color: "#22FFFFFF"
        border.color: "#4DFFFFFF"
        border.width: 1

        layer.enabled: true
        layer.smooth: true
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 10
        spacing: 10

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Text {
                id: placeholder
                text: "Search songs, artists..."
                color: "#88FFFFFF"
                font.pixelSize: searchField.font.pixelSize
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 0
                visible: !root.hasText && !searchField.inputMethodComposing
            }

            TextField {
                id: searchField
                anchors.fill: parent
                verticalAlignment: TextInput.AlignVCenter
                leftPadding: 0
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                background: Item {}

                cursorDelegate: Rectangle {
                    width: 2
                    color: "white"
                    visible: searchField.activeFocus && searchField.cursorVisible
                }

                color: "white"
                font.pixelSize: parent.height * 0.35

                onDisplayTextChanged: {
                    root.hasText = displayText.length > 0
                    root.searchTextChanged(displayText)
                }
            }
        }

        ActionButton {
            id: clearSearchBarButton
            borderColor: "transparent"
            cornerRadius: 20
            iconPath: root.hasText
                      ? "qrc:/assets/icons/ic_search_clear.svg"
                      : "qrc:/assets/icons/ic_search_open.svg"
            buttonSize: iconSize + 12
            onClicked: {
                if (root.hasText) {
                    searchField.text = ""
                    searchField.forceActiveFocus()
                } else {
                    searchField.forceActiveFocus()
                    root.searchClicked()
                }
            }
        }
    }

    states: [
        State {
            name: "focused"
            when: searchField.activeFocus

            PropertyChanges {
                target: glassBackground
                color: "#33FFFFFF"
                border.color: "#AAFFFFFF"
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"; to: "focused"
            ColorAnimation { properties: "color, border.color"; duration: 150 }
        },
        Transition {
            from: "focused"; to: "*"
            ColorAnimation { properties: "color, border.color"; duration: 200 }
        }
    ]
}
