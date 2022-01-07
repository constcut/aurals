import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {

    height: volumeComboBox.height

    RowLayout {
        spacing:  10
        Text {
            text: "Volume"
        }
        ComboBox {
            id: volumeComboBox
            model : ["24.0", "18.0", "12.0", "6.0", "0.0", "-6.0", "-12.0"]
            currentIndex: 5
            onCurrentTextChanged: {
                var db = parseFloat(currentText)
                audio.changeMidiRenderVolume(db)
            }
        }
        Text {
            text: "Soundfont"
        }
        ComboBox { //TODO list loader
            model: ["epiano.sf2", "guitar.sf2", "fullset.sf2", "eguitar.sf2", "nylon_guitar.sf2", "piano.sf2", "drums.sf2"]
            currentIndex: 0
            onCurrentTextChanged: {
                audio.changeMidiSoundfont(currentText)
            }
        }
        Text {
            text: "Sample Rate"
        }
        ComboBox {
            model: [8000, 11025, 16000, 22050, 44100, 48000]
            currentIndex: 4
            onCurrentTextChanged: {
                var sr = parseFloat(currentText)
                audio.changeMidiSampleRate(sr)
            }
        }
    }
}
