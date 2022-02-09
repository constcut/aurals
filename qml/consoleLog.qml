import QtQuick 2.0
import aurals 1.0

Item {

    anchors.fill: parent
    id:item

    ConsoleLogView {

        id: consoleLog
        y: 10
        width: parent.width - 10
        height: 400
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

    function keyboardEventSend(key, mode) {
        console.log("Pressed key: ", key)
    }

}
