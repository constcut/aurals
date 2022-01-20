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
                    //acgraph.loadByteArray(waveShape.getPCM(mouseX * minRmStep / 2.0, 4096));
                    acgraph.loadFloatSamples(waveShape.getFloatSamples(mouseX * minRmStep / 2.0, 4096))
                    yinInfo.text = acgraph.getLastFreq()
                            + "\nTime = " + ((mouseX * minRmStep / 2.0) / 44100.0).toFixed(4)
                            //+ "\nSpecPitch= " + spectrum.getSpectrumF0().toFixed(3)
                    //spectrum.loadFloatSamples(acgraph.getACF())

                }
                onDoubleClicked: {
                    var minRmStep = waveShape.getMinRmsStep()
                    audio.loadWindowPCM(waveShape.getPCM(mouseX * minRmStep / 2.0, 4096))
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
                    model: ["1024","2048","4096"]
                    currentIndex: 2
                    onCurrentTextChanged: {
                        var newWindowSize = parseInt(currentText)
                        acgraph.changeWindowSize(newWindowSize)
                        flick2.contentWidth = newWindowSize / 2
                        acgraph.width = newWindowSize / 2
                        waveShape.setWindowWidth(newWindowSize)
                    }
                }
            }
        }
    }

    Text {
        id: yinInfo
        y : settingsButton.y
        x : 25
        text: "Yin info"
    }

    Text {
        id: yinInfo2
        y : settingsButton.y
        x : parent.width/3
        text: "Yin info"
    }


    Button {
        id: settingsButton
        y: parent.height - height - 30
        x: parent.width - width - 10
        text: "Settings"
        onClicked:  {
            settingsDialog.visible = true;
        }
    }

    Button {
        id: saveButton
        y: settingsButton.y
        x: parent.width - width - 10 - settingsButton.width - 10
        text: "Save ACF image"
        onClicked:  {
            saveFileDialog.visible = true;
        }
    }

    ComboBox {
        y: saveButton.y - height - 10
        x: saveButton.x
        model: ["No window", "Hann", "Gaus", "Blackman", "Hamm"]
        currentIndex: 0
        onCurrentTextChanged: {
            acgraph.setWindowFunction(currentIndex)
        }
    }


    FileDialog {
        id: saveFileDialog
        title: "Save image to file"
        folder: shortcuts.home
        selectExisting: false
        selectMultiple: false

        onAccepted: {
            var filename = saveFileDialog.fileUrls[0].substring(7)
            acgraph.saveImage(filename)
            saveFileDialog.visible = false
        }
        onRejected: {
            saveFileDialog.visible = false
        }
        nameFilters: [ "JPEG file (*.jpg)" ]
    }


    ScrollView {
        id: acfScroll
        width: parent.width
        height: parent.height / 4
        y: waveShape.height + waveShape.y + 5

        Flickable {
            id: flick2
            y: 5// waveShape.height + waveShape.y + 5
            x: 0
            width: parent.width
            height: parent.height - 20
            contentWidth: 2048
            contentHeight:  parent.height

            ACgraph {
                y: 5// waveShape.height + waveShape.y + 5
                width: parent.width
                height: parent.height

                id: acgraph

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        //TODO just position + freq
                        yinInfo2.text = "pos= "+ mouseX + " freq = " +  44100.0 / mouseX
                        acgraph.setCursor(mouseX)
                    }
                }
            }
        }
    }

    /* //ACF spectrum experiment
    Spectrograph {
        y: acfScroll.height + acfScroll.y + 5
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
        }
    }*/

}
