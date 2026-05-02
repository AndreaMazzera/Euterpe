import QtQuick 2.15
import QtQuick.Layouts
import QtQuick.Controls 2.15
import QtQuick.Effects
import Euterpe

Rectangle {
    id: root
    color: Colors.backgroundColor
    signal backRequested()
    property real screenH: height
    property real screenW: width
    property real topBarHeight: screenH * 0.07
    property real playerHeight: screenH * 0.20

    property var currentSongData: playbackController.currentSong
    property bool showLyrics: false
    property bool showSetupMetadata: false

    property int coverCacheKey: 0

    onCurrentSongDataChanged: {
        showLyrics = false
        showSetupMetadata = false
    }

    Image {
        id: backgroundBlur
        anchors.fill: parent
        source: (currentSongData && currentSongData.songCover) ? "file://" + currentSongData.songCover : ""
        fillMode: Image.PreserveAspectCrop
        layer.enabled: true
        layer.effect: MultiEffect {
            blurEnabled: true
            blur: 1.0
            brightness: -0.2
            saturation: 0.2
        }
    }

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 20

        SongDetailsTopBar {
            id: topBar
            Layout.preferredHeight: topBarHeight
            Layout.fillWidth: true
            radius: 0
            color: "transparent"
            onReturnMainPage : {
                root.backRequested()
            }
            onSetupMetadataClicked: {
                showSetupMetadata = true
                playbackController.pause()
                metadataEditor.open()
            }
        }

        ImageItem {
            id: coverArtistImage
            Layout.fillWidth: true
            Layout.preferredHeight: coverArtistImage.width
            Layout.margins: 10
            imagePath: currentSongData && currentSongData.songCover
                       ? currentSongData.songCover + "?v=" + coverCacheKey
                       : ""
            radius: 20

            Rectangle {
                id: lyricsSection
                anchors.fill: parent
                visible: showLyrics
                radius: coverArtistImage.radius
                color: "#F2000000"

                Text {
                    anchors.centerIn: parent
                    visible: !currentSongData || currentSongData.lyrics.length === 0
                    text: "No lyrics available for this track"
                    color: "#88FFFFFF"
                    font.pixelSize: 16
                    font.italic: true
                }

                ListView {
                    id: lyricsListView
                    anchors.fill: parent
                    anchors.margins: 20
                    clip: true
                    model: currentSongData ? currentSongData.lyrics : []
                    spacing: 15

                    highlightMoveDuration: 500
                    highlightRangeMode: ListView.ApplyRange
                    preferredHighlightBegin: height / 3
                    preferredHighlightEnd: height / 2

                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

                    delegate: Item {
                        width: lyricsListView.width
                        height: lyricText.implicitHeight
                        readonly property bool isCurrent: lyricsListView.currentIndex === index

                        Text {
                            id: lyricText
                            text: modelData.text
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.Wrap

                            color: isCurrent ? "white" : "#88FFFFFF"
                            font.pixelSize: isCurrent ? 22 : 18
                            font.bold: isCurrent

                            Behavior on color { ColorAnimation { duration: 300 } }
                            Behavior on font.pixelSize { NumberAnimation { duration: 300 } }
                            Behavior on opacity { NumberAnimation { duration: 300 } }

                            opacity: (modelData.time === -1 || playbackController.position >= modelData.time) ? 1.0 : 0.4
                        }
                    }

                    Connections {
                        target: playbackController
                        function onPositionChanged() {
                            if (!showLyrics) return; // Non loggare se la sezione lyrics è chiusa

                            let pos = playbackController.position;
                            let model = lyricsListView.model;

                            if (!model || model.length === 0) {
                                console.log("[Lyrics] Modello vuoto o non caricato");
                                return;
                            }

                            let targetIndex = -1;

                            // Scansione per trovare la riga attiva
                            for (let i = 0; i < model.length; i++) {
                                let lyricTime = model[i].time;

                                // Log di debug per la prima riga o ogni 50 righe per non intasare la console
                                if (i === 0 && pos % 1000 < 50) {
                                    console.log("[Lyrics] Posizione Audio: " + pos + "ms | Tempo prima riga: " + lyricTime + "ms");
                                }

                                if (lyricTime !== -1 && lyricTime <= pos) {
                                    targetIndex = i;
                                } else {
                                    break;
                                }
                            }

                            // Se cambiamo riga, stampiamo il testo della nuova frase
                            if (targetIndex !== -1 && lyricsListView.currentIndex !== targetIndex) {
                                console.log("[Lyrics] SWITCH! Riga: " + targetIndex + " | Testo: " + model[targetIndex].text);

                                lyricsListView.currentIndex = targetIndex;
                                lyricsListView.positionViewAtIndex(targetIndex, ListView.Center);
                            }
                        }
                    }
                }
            }
        }

        Text {
            id: songTitle
            Layout.fillWidth: true
            text: currentSongData && currentSongData.title
                  ? currentSongData.title
                  : "Unknown Title"
            color: "white"
            font.bold: true
            font.pixelSize: 22
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            id: songArtist
            Layout.fillWidth: true
            text: currentSongData && currentSongData.artist
                  ? currentSongData.artist
                  : "Unknown Artist"
            color: "#CCCCCC"
            font.pixelSize: 18
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            id: songAlbum
            Layout.fillWidth: true
            text: currentSongData && currentSongData.album
                  ? currentSongData.album
                  : "Unknown Album"
            color: "#AAAAAA"
            font.pixelSize: 16
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }

        MusicPlayerControls {
            id: player
            Layout.preferredHeight: playerHeight
            Layout.fillWidth: true
            onLyricsClicked:function(enabled) {
                showLyrics = enabled
            }
        }
    }

    MetadataEditor {
        id: metadataEditor
        anchors.fill: parent
        songData: currentSongData
        onSaveResult: function(success)
        {
            if (success) {
                toast.show("Operazione completata", "success")
            } else {
                toast.show("Nessuna modifica rilevata", "error")
            }
        }
        onClose: {
            showSetupMetadata = false
            playbackController.resume()
        }
    }

    Connections {
        target: MetadataService
            function onMetadataSaved(success, filePath, title, artist, album, lyrics, coverPath) {
            if (success)
            {
                songModel.updateFullMetadata(filePath, title, artist, album, lyrics, coverPath)
                coverCacheKey++
                playbackController.currentSongChanged()

                if(coverPath!=="")
                {
                    if (typeof backgroundBlur !== "undefined") {
                        backgroundBlur.source = ""
                        backgroundBlur.source = "file://" + coverPath + "?v=" + coverCacheKey
                    }

                    console.log("Load Cover Path: " + coverPath)
                    coverArtistImage.imagePath = coverPath
                }
            }
        }
    }

    Toast {
        id: toast
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 48
        z: 999
    }
}
