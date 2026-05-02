import QtQuick
import QtQuick.Layouts
import Euterpe

Rectangle {
    id: topBar
    property int iconSize: Math.max(32, topBar.height * 0.6)
    property int titleSize: Math.max(20, topBar.height * 0.4)
    property bool searchModeActive: false
    signal searchToogle(bool enabled)

    RowLayout {
        anchors.fill: parent

        Text {
            text: "Your Playlist"
            color: "white"
            font.pixelSize: titleSize
            font.bold: true
            Layout.alignment: Qt.AlignVCenter
        }

        Item { Layout.fillWidth: true }

        ActionButton {
            iconPath: playbackController.shuffle
                        ? "qrc:/assets/icons/ic_shuffle.svg"
                        : "qrc:/assets/icons/ic_sort.svg"
            buttonSize: iconSize
            onClicked: playbackController.shuffle = !playbackController.shuffle
        }

        ActionButton {
            iconPath: searchModeActive
                        ? "qrc:/assets/icons/ic_search_close.svg"
                        : "qrc:/assets/icons/ic_search_open.svg"
            buttonSize: iconSize
            onClicked: {
                searchModeActive = !searchModeActive
                searchToogle(searchModeActive)
            }
        }
    }
}
