import QtQuick 2.10
import QtQuick.Window 2.10

import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

ApplicationWindow {

    id: thatWindow
    visible: true
    width: 1280
    height: 800
    title: qsTr("ideas")

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: "Console"
                onClicked: mainLoader.setSource("ConsoleLog.qml")
            }
            /*ToolButton {
                text: "Piano"
                onClicked: mainLoader.setSource("PianoMap.qml")
            }*/
        }
    }

    Loader {
    id:mainLoader
       anchors.fill: parent
    }

    Component.onCompleted: {
        onClicked: mainLoader.setSource("ConsoleLog.qml")
    }
}
