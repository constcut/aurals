import QtQuick 2.15
import QtQml 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1
import mther.app 1.0


Item {

    id: midiPlayerItem

    FileDialog {
        id: fileDialog
        title: "Please choose a midi file"
        folder: shortcuts.home
        onAccepted: {
            audio.openMidiFile(fileDialog.fileUrls[0].substring(7))
            audio.startMidiPlayer()
            fileDialog.visible = false
        }
        onRejected: {
            fileDialog.visible = false
        }
    }


    ColumnLayout {

        y: 30
        x: 10

        spacing:  10

        RowLayout {
            spacing: 10
            MidiRenderSettings {
            }
        }
        RowLayout {
            spacing: 10
            ComboBox {
                id: testFilesCombo
                model: ["test1.mid", "test2.mid", "test3.mid", "test4.mid", "test5.mid", "test6.mid"]
            }
            ToolButton {
                text: "Load and play"
                onClicked:  {
                    audio.openMidiFile(testFilesCombo.currentText)
                    audio.startMidiPlayer()
                }
            }
            ToolButton {
                text: "Stop"
                onClicked:  audio.stopMidiPlayer()
            }
            ToolButton {
                text: "Load test file"
                onClicked: audio.loadMidiFile(testFilesCombo.currentText)
            }
            ToolButton {
                text: "Play loaded file"
                onClicked: audio.startMidiPlayer()
            }
            ToolButton {
                text: "Load from file"
                onClicked: fileDialog.visible = true
            }
        }
    }

}
