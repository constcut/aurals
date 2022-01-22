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
    }
}