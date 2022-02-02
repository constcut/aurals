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
            visible:  Qt.platform.os == "android" ? false : true
        }
        ComboBox {
            model : ["24.0", "18.0", "12.0", "6.0", "0.0", "-6.0", "-12.0"]
            currentIndex: 5
            onCurrentTextChanged: {
                var db = parseFloat(currentText)
                audio.changeMidiRenderVolume(db)
            }
            width: 50
        }
        Text {
            text: "Soundfont"
            visible:  Qt.platform.os == "android" ? false : true
        }
        ComboBox {
            model: ["epiano.sf2", "guitar.sf2", "fullset.sf2", "eguitar.sf2", "nylon_guitar.sf2", "piano.sf2", "drums.sf2"]
            currentIndex: 2
            onCurrentTextChanged: {
                audio.changeMidiSoundfont(currentText)
            }
        }
        Text {
            text: "Sample Rate"
            visible:  Qt.platform.os == "android" ? false : true
        }
        ComboBox {
            id: midiSampleRate
            model: [8000, 11025, 16000, 22050, 44100, 48000]
            currentIndex: Qt.platform.os == "android" ? 0 : 4
            onCurrentTextChanged: {
                midiRenderLayout.updateSampleRates()
            }
        }
        Text {
            text: "Speed"
            visible:  Qt.platform.os == "android" ? false : true
        }
        ComboBox {
            id: midiSpeedCoef
            model : [0.5, 1.0, 2.0, 4.0]
            currentIndex: 1
            onCurrentTextChanged: {
                midiRenderLayout.updateSampleRates()
            }
            implicitWidth: 70
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
