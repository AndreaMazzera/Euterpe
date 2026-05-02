import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Euterpe

Rectangle {
    id: playbackControls
    radius: 20
    color: "#121212"

    property bool lyricsVisible: false
    signal lyricsClicked(bool enabled)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 5

        SeekBar {
            Layout.fillWidth: true
            Layout.preferredHeight: 20
        }

        Item {
            id: timeIndicators
            Layout.fillWidth: true
            Layout.fillHeight: true

            RowLayout {
                anchors.fill: parent

                Text { text: formatTime(playbackController.position); color: "white" }
                Item { Layout.fillWidth: true }
                Text { text: formatTime(playbackController.duration); color: "white" }
            }
        }

        // Controls
        Item {
            id: controlButtons
            Layout.fillWidth: true
            Layout.fillHeight: true

            RowLayout {
                anchors.centerIn: parent
                spacing: 20

                ActionButton {
                    id: repeatSongButton
                    iconPath: "qrc:/assets/icons/ic_repeat.svg";
                    borderColor: "transparent"
                    onClicked: {
                        playbackController.restart()
                    }
                }

                ActionButton {
                    id: previousSongButton
                    iconPath: "qrc:/assets/icons/ic_previous.svg";
                    borderColor: "transparent";
                    onClicked: playbackController.previous()
                }

                Rectangle {
                    id: playStopButton
                    width: 50; height: 50; radius: 25
                    gradient: Gradient { GradientStop { position:0;color:"#A1A2FC" } GradientStop { position:0.5;color:"#FEAEB7" } GradientStop { position:1;color:"#F6B9FB" } }
                    MouseArea { anchors.fill: parent
                        onClicked: {
                            if (playbackController.playing) playbackController.pause()
                            else playbackController.resume()
                        }
                    }
                    Image {
                        anchors.centerIn: parent
                        width: 24; height: 24
                        source: playbackController.playing ? "qrc:/assets/icons/ic_pause.svg" : "qrc:/assets/icons/ic_play.svg"
                    }
                }

                ActionButton {
                    id: nextSongButton
                    iconPath: "qrc:/assets/icons/ic_next.svg";
                    borderColor: "transparent";
                    onClicked: playbackController.next()
                }

                ActionButton {
                    id: lyricsSongButton
                    iconPath: "qrc:/assets/icons/ic_lyrics.svg";
                    borderColor: "transparent";
                    onClicked: {
                        lyricsVisible = !lyricsVisible
                        lyricsClicked(lyricsVisible)
                    }
                }
            }
        }
    }

    function formatTime(ms) {
        var totalSeconds = Math.floor(ms / 1000)
        var minutes = Math.floor(totalSeconds / 60)
        var seconds = totalSeconds % 60
        return minutes + ":" + (seconds < 10 ? "0" + seconds : seconds)
    }
}
