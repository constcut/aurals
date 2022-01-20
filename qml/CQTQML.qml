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

    property int waveHeight: height/3
    property int stftHeight: Qt.platform.os == "android" ? height/2 : 400

    ScrollView {
        width: parent.width
        height:  item.waveHeight + item.stftHeight + 20 //parent.height / 3 + 420

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
                }
                onDoubleClicked: {
                    var minRmStep = waveShape.getMinRmsStep()
                    audio.loadWindowPCM(waveShape.getPCM(mouseX * minRmStep / 2.0, 4096))
                    audio.startPlayback()
                }

            }

            Waveshape {
                id: waveShape
                height:  item.waveHeight
                width: 1000
                y: 5
                Component.onCompleted: {
                    waveShape.loadFile(item.filename)
                    flick.contentWidth = waveShape.getPixelsLength() + 10
                    waveShape.width = flick.contentWidth
                    strechedImg.setImage(waveShape.makeCQT(), waveShape.getPixelsLength() + 10);
                    strechedImg.width = flick.contentWidth
                }
            }

            StretchImage {
                y: waveShape.height + 5
                width: parent.width
                height: item.stftHeight

                id: strechedImg

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                            //Yet accurate only on desktop, very evil ariphmetic
                        var hScale = Math.log(600) / 600;
                        var inversedPos = strechedImg.height  - mouseY
                        inversedPos += 234 //12 bins shift TODO calculate
                        var freqInBin = 44100 / 8192

                        var minRmStep = waveShape.getMinRmsStep()
                        var binIdx = Math.pow(Math.exp(1), inversedPos * hScale)
                        stftInfo.text = "Freq=  " + freqInBin * binIdx
                                + "\nTime= " + ((mouseX * minRmStep / 2.0) / 44100.0).toFixed(4)
                                + "\nBin=" + binIdx
                    }
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
                    text: "FFT window size: "
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
        id: stftInfo
        y : settingsButton.y
        x : 25
        text: "CQT info"
    }

    Text {
        id: stftInfo2
        y : settingsButton.y
        x : parent.width/2
        text: "CQT info"
        visible: false
    }

    Button {
        id: settingsButton
        y: parent.height - height - 10
        x: parent.width - width - 10
        text: "Settings"
        onClicked:  {
            settingsDialog.visible = true;
        }
    }


}
