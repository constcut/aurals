import QtQuick 2.15
import QtQuick.Window 2.15

import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {

    id: thatWindow
    visible: true
    //width: 1280
    //height: 800
    visibility: "FullScreen"
    title: "Mtherapp"

    header: ToolBar {

        RowLayout {
            spacing:  10
            //anchors.fill: parent
            ToolButton {
                text: "Console"
                onClicked: mainLoader.setSource("consoleLog.qml")
            }
            ToolButton {
                text: "Audio"
                onClicked: mainLoader.setSource("audioHandler.qml")
            }
            ToolButton {
                text: "Wave"
                onClicked: mainLoader.setSource("WaveshapeQML.qml")
            }

            ToolButton {
                text: "                                "
                //TODO spacing:
            }
            ToolButton {
                text: "Exit"
                onClicked: Qt.exit(0)
            }
        }
    }

    Loader {
    id:mainLoader
       anchors.fill: parent
    }

    Component.onCompleted: {
        onClicked: mainLoader.setSource("audioHandler.qml")
    }
}
