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
                    acgraph.loadByteArray(waveShape.getPCM(mouseX * minRmStep / 2.0, 4096));
                    yinInfo.text = acgraph.getLastFreq()

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
                    model: ["2048","4096","8192","16384"]
                    currentIndex: 3
                    onCurrentTextChanged: {

                    }
                }
            }
        }
    }

    Text {
        id: yinInfo
        y : acgraph.y + acgraph.height + 30
        x : 25
        text: "Yin info"
    }

    Button {
        id: settingsButton
        y: yinInfo.y
        x: parent.width - width - 10
        text: "Settings"
        onClicked:  {
            settingsDialog.visible = true;
        }
    }


    ScrollView {
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

                id:acgraph

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        //TODO just position + freq
                    }
                }
            }

        }

    }

}
