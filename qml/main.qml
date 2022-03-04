import QtQuick 2.15
import QtQuick.Window 2.15

import QtQml 2.15

import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {

    id: thatWindow
    visible: true
    title: "aurals"

    x: 30
    y: 50

    Component.onCompleted: {
        if (Qt.platform.os === "android") {
            exitMenuItem.visible = false
            exitMenuItem.height = 0
            thatWindow.visibility = "FullScreen"
        }
        else {
            thatWindow.width = 1280
            thatWindow.height = 600
        }

        mainLoader.setSource("config.qml") // tablature audioHandler midiPlayer
    }


    onClosing: {
        if (Qt.platform.os == "android") {
            close.accepted = false
            mainMenu.open()
        }
        else
            close.accepted = true
    }


    ToolButton {
        id: mainMenuButton
        text: ":"
        x: parent.width - width - 5
        y: 5
        onClicked: {
            mainMenu.x = parent.width - mainMenu.width
            mainMenu.open()
        }
    }


    Menu {
        id: mainMenu
        MenuItem {
            text: "Console"
            onTriggered: {
                mainLoader.setSource("consoleLog.qml")
                mainLoader.focus = true
            }
        }
        MenuItem {
            text: "Config"
            onTriggered: {
                mainLoader.setSource("config.qml")
            }
        }
        MenuItem {
            text: "Audio"
            onTriggered: mainLoader.setSource("audioHandler.qml")
        }
        MenuItem {
            text: "Tab"
            onTriggered: {
                mainLoader.setSource("tablature.qml")
                mainLoader.focus = true
            }
        }
        MenuItem {
            text: "Midi"
            onTriggered: {
                mainLoader.setSource("midiPlayer.qml")
                mainLoader.focus = true
            }
        }
        MenuItem {
            text: "Tap"
            onTriggered: mainLoader.setSource("tapper.qml")
        }
        MenuItem {
            text: "Pattern"
            onTriggered: mainLoader.setSource("patternInput.qml")
        }
        MenuItem {
            id: exitMenuItem
            text: "Exit"
            onTriggered:  {
                if (Qt.platform.os !== "android")
                    Qt.exit(0)
            }

        }
    }


    function requestWaveshape(filename) {
        //Delayed: open many tabs, just like in athenum
        mainLoader.setSource("qrc:/qml/WaveshapeQML.qml", {"filename": filename});
        mainLoader.focus = true
    }

    function requestAFC(filename) {
        mainLoader.setSource("qrc:/qml/ACFQML.qml", {"filename": filename});
        mainLoader.focus = true
    }

    function requestSTFT(filename) {
        mainLoader.setSource("qrc:/qml/STFTQML.qml", {"filename": filename});
        mainLoader.focus = true
    }

    function requestCQT(filename) {
        mainLoader.setSource("qrc:/qml/CQTQML.qml", {"filename": filename});
        mainLoader.focus = true
    }

    function requestFilters(filename) {
        mainLoader.setSource("qrc:/qml/FiltersQML.qml", {"filename": filename});
        mainLoader.focus = true
    }

    function requestCepstrum(filename) {
        mainLoader.setSource("qrc:/qml/CepstrumQML.qml", {"filename": filename});
        mainLoader.focus = true
    }


    Loader {
    id:mainLoader
       anchors.fill: parent
       focus: true
       Keys.onPressed:  {

           if (event.key === Qt.Key_Home)
               mainMenu.open()

           mainLoader.item.keyboardEventSend(event.key, event.modifiers)
       }
    }



}
