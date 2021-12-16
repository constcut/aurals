import QtQuick 2.0
import mther.app 1.0

Item {

    anchors.fill: parent
    id:item
        ConsoleLogView {

            id: consoleLog
            y: 10
            width: parent.width - 10
            height: 400

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    consoleLog.update()
                }
            }
        }

        Timer {
            id: refTimer
            interval: 1000
            onTriggered: {
                consoleLog.update()
            }
            repeat: true
            running: false
        }

        Component.onCompleted:{
            refTimer.running = true
        }
}
