import QtQuick 2.15
import QtQuick.Window 2.15

import QtQml 2.15

import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {

    id: thatWindow
    visible: true
    visibility: "FullScreen"
    title: "Mtherapp"

    onClosing: {
        if (Qt.platform.os == "android")
            close.accepted = false
        else
            close.accepted = true
    }


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
                text: "Midi"
                onClicked: mainLoader.setSource("midiPlayer.qml")
            }
            ToolButton {
                text: "Tap"
                onClicked: mainLoader.setSource("tapper.qml")
            }
            ToolButton {
                text: "Exit"
                onClicked:  {
                    if (Qt.platform.os !== "android")
                        Qt.exit(0)
                }
                visible: Qt.platform.os !== "android"
            }
        }
    }


    function requestWaveshape(filename) {
        //Delayed: open many tabs, just like in athenum
        mainLoader.setSource("qrc:/qml/WaveshapeQML.qml", {"filename": filename});
    }

    Loader {
    id:mainLoader
       anchors.fill: parent
    }

    Component.onCompleted: {
        onClicked: mainLoader.setSource("audioHandler.qml")
    }
}
