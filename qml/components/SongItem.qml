import QtQuick
import QtQuick.Layouts
import Euterpe

Rectangle {
    id: itemRoot
    clip: true
    radius: 10
    height: 70

    property var songData
    property bool isSelected: false

    signal clicked()

    function formatTime(ms) {
        if (!ms || ms <= 0) return "0:00";
        let totalSeconds = Math.floor(ms / 1000);
        let minutes = Math.floor(totalSeconds / 60);
        let seconds = totalSeconds % 60;
        // Zero padding (3.5 -> 3:05)
        return minutes + ":" + (seconds < 10 ? "0" + seconds : seconds);
    }

    color: isSelected ? "#404040" : "transparent"

    Behavior on color {
        ColorAnimation { duration: 150 }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 15

        ImageItem {
            id: imageContainer
            Layout.preferredWidth: 50
            Layout.preferredHeight: 50
            Layout.alignment: Qt.AlignVCenter
            imagePath: songData ? songData.songCover : ""
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 2

            Text {
                id: titleText
                Layout.fillWidth: true
                color: "white"
                font.pixelSize: 16
                font.bold: true
                elide: Text.ElideRight
                text: {
                    if (!songData) return "";
                    return (songData.title === "Unknown Title")
                           ? songData.fileName
                           : songData.artist + " - " + songData.title
                }
            }

            Text {
                id: subtitleText
                Layout.fillWidth: true
                color: "#B0B0B0"
                font.pixelSize: 13
                elide: Text.ElideRight
                text: {
                    if (!songData) return "";
                    let album = songData.album;
                    let duration = formatTime(songData.duration);
                    return album + "  •  " + duration;
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: itemRoot.clicked()
        onPressed: itemRoot.opacity = 0.7
        onReleased: itemRoot.opacity = 1.0
        onCanceled: itemRoot.opacity = 1.0
    }
}
