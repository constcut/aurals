import QtQuick 2.0
import mther.app 1.0

Item {

    id:item
        ConsoleLog {

            id: consoleLog
            y: 100
            width: parent.width
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
