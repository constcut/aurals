import QtQuick 2.15
import QtQml 2.15
import QtQuick.Controls 2.15
import mther.app 1.0

Item {
    id: tapperItem

    Tapper {
        id: tapper
    }

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
                tapper.pressed(0);
            }
            onReleased: {
                tapper.released(0);
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
                tapper.pressed(1);
            }
            onReleased: {
                tapper.released(1);
            }
        }
    }

    ToolButton {
        id: resetButton
        text: "Reset"
        y: 5
        x: leftTap.width + 10
        onClicked: {
            tapper.reset()
        }
    }
    ToolButton {
        id: saveButton
        text: "Save"
        y: 5
        x: resetButton.x + resetButton.width + 10
        onClicked: {
            tapper.saveAsMidi("tapCheck.mid")
        }
    }
    ToolButton {
        id: playButton
        text: "Play"
        y: 5
        x: saveButton.x + saveButton.width + 10
    }

}
