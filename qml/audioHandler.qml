import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {

    RowLayout {
        spacing: 10

        ToolButton {
            text: "Start record"
            onClicked: audio.startRecord()
        }
        ToolButton {
            text: "Stop record"
            onClicked: audio.stopRecord()
        }
        ToolButton {
            text: "Start player"
            onClicked: audio.startPlayback()
        }
        ToolButton {
            text: "Stop player"
            onClicked: audio.stopPlayback()
        }
        ToolButton {
            text: "Delete default"
             onClicked: audio.deleteDump();
        }
    }

}
