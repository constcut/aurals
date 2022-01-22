import QtQuick 2.15
import mther.app 1.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1

Item {
    id: item

    property string filename: "last.wav"

    function reloadFile() {
        waveShape.loadFile(item.filename)
    }

    ScrollView {
        width: parent.width
        height: parent.height / 3

        Flickable {
            id: flick
            y: 5
            x: 0
            width: parent.width
            height: parent.height-20
            contentWidth: 3000
            contentHeight:  parent.height
            property int pressedX : 0
            MouseArea {
                x:0
                y:20
                width: parent.width
                height: 250


                onClicked:{
                    var minRmStep = waveShape.getMinRmsStep()
                    waveShape.setWindowPosition(mouseX * minRmStep/2.0)
                    spectrum.loadFloatSamples(waveShape.getFloatSamples(mouseX * minRmStep / 2.0, spectrum.getSamplesAmount()))
                    spectrum2.loadFloatSamples(waveShape.getFloatSamples(mouseX * minRmStep / 2.0, spectrum.getSamplesAmount()))
                }
                onDoubleClicked: {
                    var minRmStep = waveShape.getMinRmsStep()
                    audio.loadWindowPCM(waveShape.getPCM(mouseX * minRmStep / 2.0, spectrum.getSamplesAmount()))
                    audio.startPlayback()
                }
            }

            Waveshape {
                id: waveShape
                height:  parent.height
                width: 1000
                y: 5
                Component.onCompleted: {
                    waveShape.loadFile(item.filename)
                    flick.contentWidth = waveShape.getPixelsLength() + 10
                    waveShape.width = flick.contentWidth
                }
            }

        }
    }


    Spectrograph {
        y: waveShape.height + waveShape.y + 5
        width: parent.width
        height: parent.height / 6
        id:spectrum
        onSpectrumCalculated: {
        }
    }

    Spectrograph {
        y: spectrum.height + spectrum.y + 5
        width: parent.width
        height: parent.height / 6
        id: spectrum2
        MouseArea {
            anchors.fill: parent
            function log10(val) {
              return Math.log(val) / Math.LN10;
            }
            onClicked: {
                spectrum.onPress(mouseX, mouseY, spectrum.width, spectrum.height)
                spectrum2.onPress(mouseX, mouseY, spectrum2.width, spectrum2.height)
                specInfo.text = spectrum2.getFreq1().toFixed(2) + "-" + spectrum2.getFreq2().toFixed(2) + " Hz"
                +  " lvl = " + (20*log10(spectrum2.getValue())).toFixed(1) + " idx " + spectrum2.getIndex()
                        + " val " + spectrum2.getValue().toFixed(4)
            }
        }
    }

    Text {
        id: specInfo
        y : spectrum2.y + spectrum2.height + 10
        x : 25
        text: "Spectrum info"
    }

    ComboBox {

        y: specInfo.y
        x: parent.width / 3

        model: ["No window", "Hann", "Gaus", "Blackman", "Hamm"]
        currentIndex: 0

        onCurrentTextChanged: {
            spectrum.setWindowFunction(currentIndex)
            spectrum2.setWindowFunction(currentIndex)
        }
    }

    ComboBox {

        id: filterType

        y: specInfo.y
        x: parent.width / 2

        model: ["Low pass", "High pass"]
        currentIndex: 0

        onCurrentTextChanged: {
            spectrum2.setFilter(filterType.currentIndex,
                                parseInt(filterFreq.currentText))
        }
    }

    ComboBox {
        id: filterFreq

        y: specInfo.y
        x: parent.width / 2 + filterType.width + 10

        model : [250, 500, 1000, 2000]
        currentIndex: 2

        onCurrentTextChanged: {
            spectrum2.setFilter(filterType.currentIndex,
                                parseInt(filterFreq.currentText))
        }
    }

}
