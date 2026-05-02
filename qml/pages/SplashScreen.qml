import QtQuick
import QtQuick.Controls
import QtMultimedia
import Euterpe

Rectangle {
    id: splash
    color: "black"

    signal finished()

    MediaPlayer {
        id: player
        source: "qrc:/assets/videos/splash_screen_video.mp4"
        autoPlay: true
        videoOutput: videoOutput
    }

    VideoOutput {
        id: videoOutput
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectCrop
        z: 0
    }

    Text {
        id: splashText
        text: "Euterpe"
        color: "white"
        font.family: spaceGroteskBold.name
        font.pixelSize: Math.min(parent.width * 0.09, 46)
        font.letterSpacing: 3
        anchors.centerIn: parent
        opacity: 0
        style: Text.Outline
        styleColor: "black"
        scale: 0.92
        renderType: Text.NativeRendering
    }

    Rectangle {
        id: fadeRect
        anchors.fill: parent
        color: "black"
        opacity: 1
    }

    Component.onCompleted: {
        splashAnim.start()
        player.play()
    }

    Timer {
        interval: 3400
        running: true
        repeat: false
        onTriggered: {
            player.stop()
        }
    }

    SequentialAnimation {
        id: splashAnim

        // Fade in
        ParallelAnimation {

            NumberAnimation {
                target: fadeRect
                property: "opacity"
                from: 1
                to: 0
                duration: 900
                easing.type: Easing.InOutQuad
            }

            NumberAnimation {
                target: splashText
                property: "opacity"
                from: 0
                to: 1
                duration: 900
                easing.type: Easing.OutCubic
            }

            NumberAnimation {
                target: splashText
                property: "scale"
                from: 0.92
                to: 1.0
                duration: 900
                easing.type: Easing.OutCubic
            }
        }

        // Pause
        PauseAnimation { duration: 1800 }

        // Fade out
        ParallelAnimation {

            NumberAnimation {
                target: fadeRect
                property: "opacity"
                from: 0
                to: 1
                duration: 700
                easing.type: Easing.InQuad
            }

            NumberAnimation {
                target: splashText
                property: "opacity"
                from: 1
                to: 0
                duration: 700
                easing.type: Easing.InQuad
            }

            NumberAnimation {
                target: splashText
                property: "scale"
                from: 1.0
                to: 1.05
                duration: 700
            }
        }

        ScriptAction { script: splash.finished() }
    }
}
