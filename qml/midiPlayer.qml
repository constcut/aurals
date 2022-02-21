import QtQuick 2.15
import QtQml 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1
import aurals 1.0


Item {

    id: midiPlayerItem

    FileDialog {
        id: openFileDialog
        title: "Please choose a midi file"
        folder: shortcuts.home
        selectMultiple: false
        onAccepted: {
            audio.openMidiFile(openFileDialog.fileUrls[0].substring(7))
            audio.startMidiPlayer()
            openFileDialog.visible = false
        }
        onRejected: {
            openFileDialog.visible = false
        }
        nameFilters: [ "Midi file (*.mid *.midi)" ]
    }



    FileDialog {
        id: saveFileDialog
        title: "Save midi file to wav"
        folder: shortcuts.home
        selectExisting: false
        selectMultiple: false
        onAccepted: {
            audio.saveMidiToWav(saveFileDialog.fileUrls[0].substring(7))
            saveFileDialog.visible = false
        }
        onRejected: {
            saveFileDialog.visible = false
        }
        nameFilters: [ "Wav file (*.wav)" ]
    }


    ColumnLayout {

        id: mainLayout
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
                onClicked:   {
                    audio.stopMidiPlayer()

                }
            }
            ToolButton {
                text: "Load test file"
                onClicked: {
                    pianoRoll.reset()
                    pianoRoll.loadMidi(testFilesCombo.currentText)
                    pianoRollTrack.model = pianoRoll.getTrackCount()

                    var maxWidth = pianoRoll.getContentWidth()
                    console.log("Content max width: ", maxWidth)
                    //audio.openMidiFile(testFilesCombo.currentText)
                }
            }
            ToolButton {
                text: "Play loaded file"
                onClicked: audio.startMidiPlayer()
            }
            ToolButton {
                text: "Load and play from file"
                onClicked: openFileDialog.visible = true
            }
            ToolButton {
                text: "Export pcm"
                onClicked: saveFileDialog.visible = true
            }

            ComboBox {
                id: pianoRollTrack

                onCurrentTextChanged: {
                    var curTrack = parseInt(currentText)
                    console.log("setting current track: ", curTrack)
                    pianoRoll.setCurrentTrack(curTrack)
                }
            }
        }
    }

    PianoRoll {
        id: pianoRoll
        y: mainLayout.y + mainLayout.height
        width: parent.width
        height: parent.height - y
    }

    Component.onCompleted: {
       // pianoRoll.loadMidi(testFilesCombo.currentText)
    }

    function keyboardEventSend(key, mode) {
        //Заглушка, но можно реализовать логику здесь
    }

}
