import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Euterpe

Rectangle {
    id: itemRoot
    clip: true
    radius: 10
    border.color: "#4DFFFFFF"
    border.width: 1

    property var currentSongData: playbackController.currentSong
    property bool isSelected: false
    signal openDetailsSection()

    color: isSelected ? "#404040" : "transparent"

    Behavior on color {
        ColorAnimation { duration: 120 }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 1
        spacing: 0
        clip: true

        Rectangle {
            id: seekBar
            color: "transparent"
            Layout.preferredHeight: 10
            Layout.fillWidth: true
            topRightRadius: 10
            topLeftRadius: 10
            clip: true

            Rectangle {
                anchors.fill: parent
                color: "#303030"
                topRightRadius: itemRoot.radius
                topLeftRadius: itemRoot.radius
                clip: true

                Rectangle {
                    id: currentTime
                    anchors.bottom: parent.bottom
                    width: parent.width * (
                        playbackController.duration > 0
                            ? playbackController.position / playbackController.duration
                            : 0
                    )
                    height: Math.min(parent.height, width)
                    topRightRadius: itemRoot.radius
                    topLeftRadius: itemRoot.radius
                    color: "#A1A2FC"
                }
            }
        }

        Rectangle {
            color: "transparent"
            Layout.fillHeight: true
            Layout.fillWidth: true
            bottomRightRadius: 10
            bottomLeftRadius: 10

            RowLayout {
                id: dataContainerLayout
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 10

                ImageItem {
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 50
                    imagePath: playbackController.currentSong.songCover ? playbackController.currentSong.songCover : ""
                }

                Rectangle {
                    id: trackData
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignVCenter
                    color: "transparent"
                    clip: true

                    MouseArea {
                        anchors.fill: parent
                        onClicked: itemRoot.openDetailsSection()
                        focus: false
                        acceptedButtons: Qt.LeftButton
                        hoverEnabled: false
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 6
                        spacing: 2

                        Item { Layout.fillHeight: true }

                        Text {
                            text: playbackController.currentSong.title || ""
                            color: "white"
                            font.pixelSize: 16
                            font.bold: true
                            elide: Text.ElideRight
                            maximumLineCount: 1
                            Layout.fillWidth: true
                        }

                        Text {
                            text: playbackController.currentSong.artist || ""
                            color: "#A0A0A0"
                            font.pixelSize: 13
                            elide: Text.ElideRight
                            maximumLineCount: 1
                            Layout.fillWidth: true
                        }

                        Item { Layout.fillHeight: true }
                    }
                }

                Rectangle {
                    id: playerButton
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 50
                    radius: 25

                    gradient: Gradient {
                        GradientStop { position: 0; color: "#A1A2FC" }
                        GradientStop { position: 0.5; color: "#FEAEB7" }
                        GradientStop { position: 1; color: "#F6B9FB" }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (!playbackController.currentSong)
                                return

                            if (playbackController.playing)
                                playbackController.pause()
                            else
                                playbackController.resume()
                        }
                    }

                    Image {
                        anchors.centerIn: parent
                        width: 24
                        height: 24
                        source: audioPlayer.playing
                                ? "qrc:/assets/icons/ic_pause.svg"
                                : "qrc:/assets/icons/ic_play.svg"
                        fillMode: Image.PreserveAspectFit
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: "white"
        border.width: 2
        radius: 10
    }
}
