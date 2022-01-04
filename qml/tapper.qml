import QtQuick 2.15
import QtQml 2.15
import QtQuick.Controls 2.15

Item {
    id: tapperItem

    Rectangle {
        x: 0
        y: 0
        height: parent.height
        width: parent.width / 8
        color: "darkgray"
        id: leftTap
        MouseArea {
            anchors.fill: parent
            onPressed: {
                console.log("pressed 1")
            }
            onReleased: {
                console.log("released 1")
            }
        }
    }
    Rectangle {
        x: parent.width - width
        y: 0
        height: parent.height
        width: parent.width / 8
        color: "darkgray"
        id: rightTap
        MouseArea {
            anchors.fill: parent
            onPressed: {
                console.log("pressed 2")
            }
            onReleased: {
                console.log("released 2")
            }
        }
    }

    ToolButton {
        id: resetButton
        text: "Reset"
        y: 5
        x: leftTap.width + 10
    }
    ToolButton {
        id: saveButton
        text: "Save"
        y: 5
        x: resetButton.x + resetButton.width + 10
    }
    ToolButton {
        id: playButton
        text: "Play"
        y: 5
        x: saveButton.x + saveButton.width + 10
    }

}
