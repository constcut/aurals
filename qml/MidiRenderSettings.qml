import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {

    height: midiSampleRate.height
    width: midiRenderLayout.width

    RowLayout {
        id: midiRenderLayout
        spacing:  10
        Text {
            text: "Volume"
        }
        ComboBox {
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
            id: midiSampleRate
            model: [8000, 11025, 16000, 22050, 44100, 48000]
            currentIndex: 4
            onCurrentTextChanged: {
                midiRenderLayout.updateSampleRates()
            }
        }
        Text {
            text: "Speed"
        }
        ComboBox {
            id: midiSpeedCoef
            model : [0.5, 1.0, 2.0, 4.0]
            currentIndex: 1
            onCurrentTextChanged: {
                midiRenderLayout.updateSampleRates()
            }
        }
        function updateSampleRates() {
            var coef = parseFloat(midiSpeedCoef.currentText)
            var playerSR = parseFloat(midiSampleRate.currentText)
            var renderSR = playerSR / coef
            audio.setMidiPlayerSampleRate(playerSR)
            audio.changeMidiSampleRate(renderSR)
        }
    }
}
