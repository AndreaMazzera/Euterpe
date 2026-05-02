import QtQuick 2.15
import QtQuick.Layouts 1.15
import Euterpe

Rectangle {
    id: root
    color: Colors.backgroundColor

    property real screenH: height
    property real screenW: width
    property real topBarHeight: screenH * 0.07
    property real playerHeight: screenH * 0.12

    property bool selectedSong: false
    property bool shuffle: false
    property bool searchModeActive: false

    signal openDetails()

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        TopBar {
            id: topBar
            Layout.preferredHeight: topBarHeight
            Layout.fillWidth: true
            radius: 0
            color: "transparent"
            onSearchToogle:function(enabled) {
                searchBar.text = ""
                root.searchModeActive = enabled
                console.log("Search state changed:", searchModeActive)
            }
        }

        HorizontalDivider {
            Layout.fillWidth: true
            Layout.preferredHeight: 2
            Layout.leftMargin: 5
            Layout.rightMargin: 5
        }

        SearchBar {
            id: searchBar
            Layout.fillWidth: true
            Layout.preferredHeight: screenH * 0.065
            visible: root.searchModeActive
            opacity: root.searchModeActive ? 1 : 0
            onSearchClicked: {
                console.log("Ricerca cliccata:", text)
            }
            onSearchTextChanged: function(t) {
                playbackController.filterModel.filterText = t
                console.log("Digitato:", t)
            }
        }

        SongListSection {
            id: listSong
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        SongQuickView {
            id: player
            Layout.preferredHeight: playerHeight
            Layout.fillWidth: true
            visible: playbackController.currentIndex >= 0
            onOpenDetailsSection: {
                console.log("Click to open song details page")
                songModel.updateSongMetadataAt(playbackController.currentIndex)
                root.openDetails()
            }
        }
    }
}
