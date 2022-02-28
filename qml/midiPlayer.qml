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
                    pianoRoll.setHeightStretch(stretchHeight.checked)
                    pianoRoll.loadMidi(testFilesCombo.currentText)
                    pianoRollTrack.model = pianoRoll.getTrackCount()

                    var maxWidth = pianoRoll.getContentWidth()
                    flick.contentWidth = maxWidth
                    pianoRoll.width = maxWidth
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
            CheckBox {
                id: stretchHeight
                checked: false
                text: "Stretch"
            }
            ToolButton {
                text: "+"
                onClicked: pianoRoll.zoomIn();
            }
            ToolButton {
                text: "-"
                onClicked: pianoRoll.zoomOut();
            }
            ToolButton {
                text: ">"
                onClicked:  {
                    pianoRoll.saveAs("pianoroll.mid")
                    audio.openMidiFile("pianoroll.mid")
                    audio.startMidiPlayer()

                    //Compare with previous load
                    "tests/regression/3.1.mid"
                }
            }
        }
    }


    ScrollView {
        y: mainLayout.y + mainLayout.height
        width: parent.width
        height: parent.height - y

        Flickable {
            id: flick
            width: parent.width
            height: parent.height
            contentWidth: 3000
            contentHeight:  parent.height

            PianoRoll {
                id: pianoRoll
                y: 0
                width: parent.width
                height: parent.height

                MouseArea {
                    width: parent.width
                    height: parent.height

                    property int pressX: 0
                    property int pressY: 0
                    property bool pressed: false

                    preventStealing: true
                    onPressed: {
                        pressX = mouseX
                        pressY = mouseY
                        pressed = true
                    }
                    onMouseYChanged: {
                        if (pressed) {
                            var diffY = mouseY - pressY
                            if (Math.abs(diffY) > 5)
                                pianoRoll.onMoveVertical(mouseY)
                        }
                    }
                    onMouseXChanged: {
                        if (pressed) {
                            var diffX = mouseX - pressX
                            if (Math.abs(diffX) > 2) //TODO not the best option, as we can't make small moves, need to compare diffX and diffY
                                pianoRoll.onMoveHorizontal(mouseX)
                        }
                    }

                    onReleased: {
                        pressed = false

                    }

                    onDoubleClicked: {
                        pianoRoll.onDoubleClick(mouseX, mouseY)
                    }

                    onPressAndHold: {
                        pianoRoll.onPressAndHold(mouseX, mouseY)
                    }

                }
            }
        }
    }


    Component.onCompleted: {
        pianoRoll.reset()
        pianoRoll.setHeightStretch(true) //stretchHeight.checked

        pianoRoll.zoomIn()
        pianoRoll.zoomIn()
        pianoRoll.zoomIn()

        pianoRoll.loadMidi("tests/regression/3.1.mid") //1.1
        pianoRollTrack.model = pianoRoll.getTrackCount()

        var maxWidth = pianoRoll.getContentWidth() * 8 //TODO zoom
        flick.contentWidth = maxWidth
        pianoRoll.width = maxWidth

        pianoRollTrack.currentIndex = 1

        pianoRoll.saveAs("pianoCheck.mid")
    }

    function keyboardEventSend(key, mode) {
        //Заглушка, но можно реализовать логику здесь
    }

}
