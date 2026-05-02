import QtQuick 2.15
import QtQuick.Controls 2.15

Window {
    id: mainWindow
    visible: true
    color: Colors.backgroundColor
    title: qsTr("Euterpe")

    FontLoader {
        id: spaceGroteskRegular
        source: "qrc:/assets/fonts/SpaceGrotesk-Regular.ttf"
    }

    FontLoader {
        id: spaceGroteskBold
        source: "qrc:/assets/fonts/SpaceGrotesk-Bold.ttf"
    }

    StackView {
        id: stack
        anchors.fill: parent
        focus: true

        initialItem: SplashScreen {
            onFinished: {
                stack.replace(mainPageComponent)
            }
        }

        pushEnter: Transition { PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 200 } }
        pushExit: Transition { PropertyAnimation { property: "opacity"; from: 1; to: 0; duration: 200 } }
        popEnter: Transition { PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 200 } }
        popExit: Transition { PropertyAnimation { property: "opacity"; from: 1; to: 0; duration: 200 } }
    }

    Component {
        id: mainPageComponent
        MainPage {
            onOpenDetails: (songData) => {
                stack.push(songDetailsComponent, {
                    "showLyrics": false,
                    "showSetupMetadata": false
                })
            }
        }
    }

    Component {
        id: songDetailsComponent
        SongDetailsPage {
            onBackRequested: {
                stack.pop()
            }
        }
    }

    Keys.onReleased: (event) => {
        if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
            if (stack.depth > 1) {
                stack.pop()
                event.accepted = true
            }
        }
    }
}
