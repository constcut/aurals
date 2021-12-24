import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {

    ColumnLayout {
        spacing:  10
        y: 10
        x: 10

        RowLayout {
            spacing:  10
            Text {
                text: "Sample rate:"
            }
            ComboBox {
                model: ["22050", "44100", "48000"]
                currentIndex: 0
                onCurrentTextChanged: {
                    var sr = parseInt(currentText)
                    audio.setSampleRate(sr)
                }
            }
        }

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
                text: "Reset"
                 onClicked: audio.resetBufer();
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
                text: "Save dump"
                onClicked: audio.saveFile("rec.dump")
            }
            ToolButton {
                text: "Load dump"
                onClicked: audio.loadFile("rec.dump")
            }
            //ToolButton {
            //    text: "Request permission"
            //    onClicked: audio.requestPermission()
            //}

        }
    }



}
