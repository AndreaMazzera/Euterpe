import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Euterpe

Slider {
    id: seekSlider

    property real barRadius: 4

    from: 0
    to: playbackController.duration
    value: playbackController.position

    onMoved: playbackController.setPosition(value)

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    background: Rectangle {
        x: 0
        y: seekSlider.height / 2 - height / 2
        width: seekSlider.width
        height: 8

        radius: seekSlider.barRadius
        color: "#303030"

        Rectangle {
            width: seekSlider.visualPosition * parent.width
            height: parent.height

            radius: Math.min(seekSlider.barRadius, width / 2)

            gradient: Gradient {
                orientation: Gradient.Horizontal

                GradientStop { position: 0.0; color: Colors.gradientStart }
                GradientStop { position: 0.5; color: Colors.gradientMiddle }
                GradientStop { position: 1.0; color: Colors.gradientEnd }
            }
        }
    }

    handle: Rectangle {
        x: seekSlider.visualPosition * (seekSlider.width - width)
        y: seekSlider.height / 2 - height / 2

        width: 14
        height: 14
        radius: 7

        color: "#A1A2FC"

        opacity: seekSlider.pressed ? 1 : 0
        scale: seekSlider.pressed ? 1 : 0.8

        Behavior on opacity { NumberAnimation { duration: 120 } }
        Behavior on scale { NumberAnimation { duration: 120 } }
    }
}
