import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import Euterpe

Rectangle {
    id: imageItemRoot
    radius: 10

    // ====================================================
    // Properties
    // ====================================================
    property string imagePath: ""
    property color defaultColor: "black"

    // Limiti opzionali (∞ di default)
    property real maxWidth: Infinity
    property real maxHeight: Infinity

    readonly property bool hasPath: imagePath !== ""

    border.color: hasPath ? "transparent" : "#33FFFFFF"
    border.width: 1

    implicitWidth: imageSource.status === Image.Ready
        ? Math.min(imageSource.sourceSize.width, maxWidth)
        : 0

    implicitHeight: imageSource.status === Image.Ready
        ? Math.min(imageSource.sourceSize.height, maxHeight)
        : 0

    color: imageSource.status === Image.Ready ? "transparent" : defaultColor

    Image {
        id: fallbackIcon
        anchors.centerIn: parent
        width: parent.width * 0.5
        height: parent.height * 0.5
        source: "qrc:/assets/icons/ic_music_note.svg"
        fillMode: Image.PreserveAspectFit
        visible: !imageItemRoot.hasPath
        opacity: 0.5
    }

    Image {
        id: imageSource
        anchors.fill: parent
        source: hasPath ? "file://" + imageItemRoot.imagePath : ""
        fillMode: Image.PreserveAspectCrop
        visible: false
        cache: false
    }

    Rectangle {
        id: imageMask
        anchors.fill: parent
        visible: false
        radius: imageItemRoot.radius
        color: "black"
        layer.enabled: true
    }

    MultiEffect {
        id: imageMaskEffect
        anchors.fill: parent
        source: imageSource
        maskSource: imageMask
        maskEnabled: true
        maskThresholdMin: 0.5
        maskSpreadAtMin: 0.0
    }
}
