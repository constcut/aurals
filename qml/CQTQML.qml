import QtQuick 2.15
import aurals 1.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1

Item {
    id: item

    property string filename: "last.wav"

    function reloadFile() {
        waveShape.loadFile(item.filename)
    }

    property int waveHeight: height/4
    property int cqtHeight: Qt.platform.os == "android" ? height/2 : 480

    ScrollView {
        width: parent.width
        height:  item.waveHeight + item.cqtHeight + 20 //parent.height / 3 + 420

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
                height: item.cqtHeight

                id: strechedImg

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        var bin = 60.0 * 8.0  - mouseY - 1
                        var freq = 58.0891 * Math.pow(2, bin / 60.0)
                        var minRmStep = waveShape.getMinRmsStep()
                        ctqInfo.text = "Freq= " + freq + "  Time= " + ((mouseX * minRmStep / 2.0) / 44100.0).toFixed(4)
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
        id: ctqInfo
        y : settingsButton.y
        x : 25
        text: "CQT info"
    }

    Text {
        id: ctqInfo2
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

    function keyboardEventSend(key, mode) {
        //Заглушка, но можно реализовать логику здесь
    }

}
