import QtQuick
import QtQuick.Layouts
import Euterpe

Rectangle {
    id: topBar
    property int iconSize: Math.max(32, topBar.height * 0.6)
    property int titleSize: Math.max(20, topBar.height * 0.35)

    signal setupMetadataClicked()
    signal returnMainPage()

    RowLayout {
        id: topBarLayout
        anchors.fill: parent

        ActionButton {
            id: backButton
            iconPath: "qrc:/assets/icons/ic_left_arrow.svg"
            buttonSize: iconSize
            onClicked: {
                returnMainPage()
            }
        }

        Item { Layout.fillWidth: true }

        Text {
            id: titleApp
            text: "Now Playing"
            color: "white"
            font.pixelSize: titleSize
            font.bold: true
            Layout.alignment: Qt.AlignVCenter
        }

        Item { Layout.fillWidth: true }

        ActionButton {
            id: modifySongMetadataButton
            iconPath: "qrc:/assets/icons/ic_edit.svg"
            buttonSize: iconSize
            onClicked: {
                setupMetadataClicked()
            }
        }
    }
}
