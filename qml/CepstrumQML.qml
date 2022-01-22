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


    Text {
        id: yinInfo
        y : saveButton.y
        x : 25
        text: "Cepstrum info"
    }

    Text {
        id: yinInfo2
        y : saveButton.y
        x : parent.width/3
        text: "Cepstrum info"
    }


    Button {
        id: saveButton
        y: parent.height - height - 30
        x: parent.width - width - 10
        text: "Save image"
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
            cepstrum.setWindowFunction(currentIndex)
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
        id: cepstrumScroll
        width: parent.width
        height: parent.height / 4
        y: waveShape.height + waveShape.y + 5

        Flickable {
            id: flick2
            y: 5
            x: 0
            width: parent.width
            height: parent.height - 20
            contentWidth: 2048
            contentHeight:  parent.height

            ACgraph {
                y: 5
                width: parent.width
                height: parent.height
                id: cepstrum

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        cepstrum.setCursor(mouseX)
                    }
                }
            }
        }
    }

}
