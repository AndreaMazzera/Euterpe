import QtQuick
import QtQuick.Layouts
import Euterpe

Item {
    id: listSongSection
    clip: true

    ListView {
        id: songListView
        anchors.fill: parent
        model: playbackController.filterModel
        clip: true
        currentIndex: playbackController.currentProxyIndex

        delegate: SongItem {
            width: songListView.width
            height: 60
            songData: model
            isSelected: index === playbackController.currentProxyIndex

            MouseArea {
                anchors.fill: parent
                onClicked: playbackController.playSong(index)
            }
        }
    }
}
