import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Euterpe

Item {
    id: root
    anchors.fill: parent
    visible: opacity > 0
    opacity: 0

    property int deleteCounter: 5
    property bool isDeleting: false
    property var songData
    signal close()
    signal saveResult(bool success)

    onSongDataChanged: resetFields()

    Timer {
        id: deleteTimer
        interval: 1000
        repeat: true
        onTriggered: {
            if (deleteCounter > 0) {
                deleteCounter--;
            } else {
                deleteTimer.stop();
                performFinalDelete();
            }
        }
    }

    function performFinalDelete() {
        if (songData) {
            let pathToDelete = songData.filePath;
            MetadataService.removeSongFromAndroidDatabase(pathToDelete)
            songModel.deleteSong(pathToDelete);
            playbackController.next();
            root.hide();
            toast.show("Song permanently deleted", "#D50000");
        }
    }

    Behavior on opacity {
        NumberAnimation { duration: 200 }
    }

    NumberAnimation {
        id: slideIn
        target: card
        property: "y"
        from: -card.height
        to: 0
        duration: 260
        easing.type: Easing.OutCubic
    }

    NumberAnimation {
        id: slideOut
        target: card
        property: "y"
        from: card.y
        to: -card.height
        duration: 200
        easing.type: Easing.InCubic
        onFinished: {
            opacity = 0
            MetadataService.clearPendingData()
            root.close()
        }
    }

    function resetFields() {
        if (songData) {
            titleFieldComp.text = songData.title
            artistFieldComp.text = songData.artist
            albumFieldComp.text = songData.album
        }
    }

    function open() {
        resetFields()
        MetadataService.setInitialState(songData.title, songData.artist, songData.album)
        opacity = 1
        card.y = -card.height
        slideIn.start()
    }

    function hide() {
        Qt.inputMethod.hide()
        slideOut.start()
    }

    Rectangle {
        id: backgroundOverlay
        anchors.fill: parent
        color: "#AA000000"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.hide()
    }

    Rectangle {
        id: card
        width: parent.width
        height: mainLayout.implicitHeight + 40
        y: -height
        bottomLeftRadius: 20
        bottomRightRadius: 20
        color: "#1E1E1E"

        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            anchors.margins: 20
            spacing: 5

            Text {
                id: sectionLabel
                text: "Metadata"
                color: "white"
                font.pixelSize: 20
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            FloatingLabelTextField {
                id: titleFieldComp
                label: "Title"
                Layout.fillWidth: true
                text: songData?.title || ""
            }

            FloatingLabelTextField {
                id: artistFieldComp
                label: "Artist"
                Layout.fillWidth: true
                text: songData?.artist || ""
            }

            FloatingLabelTextField {
                id: albumFieldComp
                label: "Album"
                Layout.fillWidth: true
                text: songData?.album || ""
            }

            RowLayout {
                id: row1
                Layout.fillWidth: true
                Layout.topMargin: 5
                Layout.bottomMargin: 5
                spacing: 15

                Button {
                    id: loadCoverBtn
                    text: "Load Cover"
                    icon.source: "qrc:/assets/icons/ic_add_cover.svg"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    onClicked: MetadataService.pickCoverImage()
                    background: Rectangle { radius: 10 }
                }

                Button {
                    id: loadLyricsBtn
                    text: "Load Lyrics"
                    icon.source: "qrc:/assets/icons/ic_lyrics.svg"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    background: Rectangle { radius: 10 }
                    onClicked: MetadataService.pickLyricsFile()
                }
            }

            RowLayout {
                id: row2
                Layout.fillWidth: true
                spacing: 15

                Button {
                    id: closeBtn
                    text: "Close"
                    Layout.preferredWidth: loadCoverBtn.width
                    Layout.preferredHeight: loadCoverBtn.Layout.preferredHeight
                    onClicked: root.hide()
                    background: Rectangle { radius: 10 }
                }

                Button {
                    id: saveBtn
                    text: "Save"
                    Layout.preferredWidth: loadLyricsBtn.width
                    Layout.preferredHeight: loadLyricsBtn.Layout.preferredHeight
                    enabled: titleFieldComp.text.length > 0
                    onClicked: {
                        MetadataService.setMetadataTitle(titleFieldComp.text)
                        MetadataService.setMetadataArtist(artistFieldComp.text)
                        MetadataService.setMetadataAlbum(albumFieldComp.text)
                        MetadataService.updateMetadata(songData.filePath, songData.uri)
                    }
                    background: Rectangle { radius: 10 }
                }
            }

            Button {
                id: deleteButton
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                Layout.topMargin: 5

                background: Rectangle {
                    color: isDeleting ? "#444444" : "#D50000"
                    radius: 10
                }
                contentItem: Row {
                    spacing: 8
                    anchors.centerIn: parent
                    width: childrenRect.width

                    Image {
                        source: "qrc:/assets/icons/ic_trash.svg"
                        width: 24
                        height: 24
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: isDeleting ? "Cancel Delete (" + deleteCounter + ")" : "Delete Song"
                        color: "white"
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
                onClicked: {
                    if (!isDeleting) {
                        isDeleting = true;
                        deleteCounter = 5;
                        deleteTimer.start();
                    } else {
                        isDeleting = false;
                        deleteTimer.stop();
                        deleteCounter = 5;
                    }
                }
            }
        }
    }

    Connections {
        target: MetadataService
        function onMetadataSaved(success) {
            console.log("[MetadataEditor] Signal received - metadataSaved:", success)
            root.saveResult(success)
            root.hide()
        }
    }
}
