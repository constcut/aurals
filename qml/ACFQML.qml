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

                onPressed: {
                    flick.pressedX = mouseX
                }
                onReleased: {
                    var diff =  flick.pressedX - mouseX
                    flick.contentX += diff
                    if (flick.contentX < 0)
                        flick.contentX = 0
                }
                onClicked:{
                    var minRmStep = waveShape.getMinRmsStep()
                    waveShape.setWindowPosition(mouseX * minRmStep/2.0)
                    //If we need to load some file: spectrum.loadSpectrum(item.filename,mouseX*minRmStep/2.0) //This version reloads file
                    spectrum.loadByteArray(waveShape.getPCM(mouseX * minRmStep / 2.0, spectrum.getSamplesAmount()));

                    var winRmsDb = waveShape.getWindowRmsDb()
                    var winYinF0 = waveShape.getWindowYinF0()
                    //TODO ACF
                }
                onDoubleClicked: {
                    var minRmStep = waveShape.getMinRmsStep()
                    //If we need to load some file: audio.loadOnlyWindow(item.filename, mouseX * minRmStep / 2.0, spectrum.getSamplesAmount())
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

    Dialog {
        id: settingsDialog
        ColumnLayout {
            spacing: 10
            id: dialogLayout

            RowLayout {
                spacing:  10
                id: upperLayout
                Text{
                    text: "Yin window size: "
                }
                ComboBox {
                    model: ["2048","4096","8192","16384"]
                    currentIndex: 3
                    onCurrentTextChanged: {

                    }
                }
            }
        }
    }

    Text {
        id: specInfo
        y : spectrum.y + spectrum.height + 10
        x : 25
        text: "Yin info"
    }

    Button {
        id: settingsButton
        y: specInfo.y
        x: parent.width - width - 10
        text: "Settings"
        onClicked:  {
            settingsDialog.visible = true;
        }
    }

    Spectrograph {
        y: waveShape.height + waveShape.y + 5
        width: parent.width
        height: parent.height / 6
        id:spectrum

        MouseArea {
            anchors.fill: parent
            function log10(val) {
              return Math.log(val) / Math.LN10;
            }
            onClicked: {
                spectrum.onPress(mouseX, mouseY, spectrum.width, spectrum.height)
                specInfo.text = spectrum.getFreq1().toFixed(2) + "-" + spectrum.getFreq2().toFixed(2) + " Hz"
                +  " lvl = " + (20*log10(spectrum.getValue())).toFixed(1) + " idx " + spectrum.getIndex()
                        + " val " + spectrum.getValue().toFixed(4)
            }
        }
        onSpectrumCalculated: {
            outputPeaksGroup()
        }
        function outputPeaksGroup() {
            peaksGroup.text = "";
            //Removed can be reused
        }
    }

}
