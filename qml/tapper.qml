import QtQuick 2.15
import QtQml 2.15
import QtQuick.Controls 2.15
import mther.app 1.0

Item {
    id: tapperItem

    Tapper {
        id: tapper
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
                var midiNote = 60
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
                var midiNote = 72
                if (secondMidiNoteField.text != "")
                    midiNote = parseInt(secondMidiNoteField.text)
                tapper.tapped(midiNote)
            }
        }
    }

    ToolButton {
        id: resetButton
        text: "Reset"
        y: 5
        x: leftTap.width + 100
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
        id: saveTapButton
        text: "Save taps"
        y: 5
        x: resetButton.x + resetButton.width + 10
        onClicked: {
            tapper.saveTapsAsMidi("tapper.mid")
        }
    }
    ToolButton {
        id: playButton
        text: "Play"
        y: 5
        x: saveTapButton.x + saveTapButton.width + 10
        onClicked: {
            tapper.saveTapsAsMidi("tapper.mid") //tapper.mid
            audio.openMidiFile("tapper.mid")
            audio.startMidiPlayer()
        }
    }
    ToolButton {
        y: 5
        x: playButton.x + playButton.width + 10
        text: "Save taps audio"
        onClicked: {
            tapper.saveTapsAsMidi("tapper.mid")
            audio.openMidiFile("tapper.mid")
            audio.saveMidiToWav("tapper.wav")
        }
    }

}
