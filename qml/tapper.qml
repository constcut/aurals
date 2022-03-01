import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.15
import aurals 1.0

Item {
    id: tapperItem

    Tapper {
        id: tapper
    }

    FileDialog {
        id: saveMidiFileDialog
        title: "Export tapper midi file"
        folder: shortcuts.home
        selectExisting: false
        selectMultiple: false
        onAccepted: {
            tapper.saveTapsAsMidi(saveMidiFileDialog.fileUrls[0].substring(7))
            saveMidiFileDialog.visible = false
        }
        onRejected: {
            saveMidiFileDialog.visible = false
        }
        nameFilters: [ "Midi file (*.mid)" ]
    }

    FileDialog {
        id: saveWavFileDialog
        title: "Export tapper into wav file"
        folder: shortcuts.home
        selectExisting: false
        selectMultiple: false
        onAccepted: {
            tapper.saveTapsAsMidi("tapper.mid")
            audio.openMidiFile("tapper.mid")
            var filename = saveWavFileDialog.fileUrls[0].substring(7)
            audio.saveMidiToWav(filename)
            saveWavFileDialog.visible = false
        }
        onRejected: {
            saveWavFileDialog.visible = false
        }
        nameFilters: [ "Wav file (*.wav)" ]
    }



    TextField {
        placeholderText: "Midi note 1"
        x: 0
        y: 5
        id: firstMidiNoteField
    }

    Rectangle {
        x: 0
        y: 50
        height: parent.height
        width: parent.width / 8
        color: "darkgray"
        id: leftTap

        TapHandler {
            acceptedButtons: Qt.AllButtons
            onTapped: {
                var midiNote = 40
                if (firstMidiNoteField.text != "")
                    midiNote = parseInt(firstMidiNoteField.text)
                tapper.tapped(midiNote)
            }
        }
        /*
        property int clickCount: 0
        property int releaseCount: 0
        MouseArea {
            anchors.fill: parent
            onPressed: {
                tapper.pressed(0);
                leftTap.clickCount += 1
            }
            onReleased: {
                tapper.released(0);
            }
        }*/ //This is the way to capture also note duration, but then we fail on fast taps :(
    }

    TextField {
        placeholderText: "Midi note 2"
        x: parent.width - width
        y: 5
        id: secondMidiNoteField
    }

    Rectangle {
        x: parent.width - width
        y: 50
        height: parent.height
        width: parent.width / 8
        color: "darkgray"
        id: rightTap

        TapHandler {
            acceptedButtons: Qt.AllButtons
            onTapped: {
                var midiNote = 36
                if (secondMidiNoteField.text != "")
                    midiNote = parseInt(secondMidiNoteField.text)
                tapper.tapped(midiNote)
            }
        }
    }

    RowLayout {
        id : buttonsLayout
        y: secondMidiNoteField.y + secondMidiNoteField.height + 5
        x: leftTap.width + (parent.width - leftTap.width - rightTap.width - width) / 2
        spacing: 10
        ToolButton {
            text: "Reset"
            onClicked: {
                tapper.reset()
            }
        }
        /*
        ToolButton {
            id: saveClicksButton
            text: "Save clicks"
            y: 5
            x: resetButton.x + resetButton.width + 10
            onClicked: {
                tapper.saveClicksAsMidi("tapper.mid")
            }
        }*/ //Used in case above
        ToolButton {
            text: "Play"
            onClicked: {
                tapper.saveTapsAsMidi("tapper.mid") //tapper.mid
                audio.openMidiFile("tapper.mid")
                audio.startMidiPlayer()
            }
        }
        ToolButton{
            text: "Stop"
            onClicked: {
                audio.stopMidiPlayer()
            }
        }
        Slider {
            from: 0.25
            to: 4.0
            stepSize: 0.05
            value: 1.0
            id: speedCoefSlider
            ToolTip {
                parent: speedCoefSlider.handle
                visible: speedCoefSlider.hovered
                text: speedCoefSlider.value.toFixed(2)
            }
            onValueChanged: {
                tapper.setSpeedCoef(value)
            }
        }
        ToolButton {
            text: "Export midi file"
            onClicked: saveMidiFileDialog.open()
        }
        ToolButton {
            text: "Export wav file"
            onClicked: saveWavFileDialog.open()
        }
    }

    MidiRenderSettings {
        y: buttonsLayout.y + buttonsLayout.height + 10
        x: leftTap.width + 10
    }

    function keyboardEventSend(key, mode) {
        //Заглушка, но можно реализовать логику здесь
    }

}
