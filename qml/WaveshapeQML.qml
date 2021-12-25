import QtQuick 2.7
import wavesplat.music.wave 1.0
import QtQuick.Controls 1.4

Item {
    id: item

    property string filename: "last.wav"

    function reloadFile() {
        waveShape.loadFile(item.filename)
        console.log("reload file of waveshape qml called ", filename)
    }

    ScrollView
    {
        //Flickable
        width: parent.width
        height: 300

        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOn
        verticalScrollBarPolicy:  Qt.ScrollBarAlwaysOff

        Flickable
        {
            id: flick
            y: 50
            x: 0
            width: parent.width
            height: parent.height-20

            contentWidth: 3000
            contentHeight: 300

            //QML2.ScrollBar.horizontal:  QML2.ScrollBar { }

            property int pressedX : 0
            MouseArea {
                //anchors.fill: parent

                x:0
                y:70

                width: parent.width
                height: 250

                onPressed: {
                    flick.pressedX = mouseX
                }
                onReleased:
                {
                    var diff =  flick.pressedX - mouseX
                    flick.contentX += diff
                    if (flick.contentX < 0)
                        flick.contentX = 0
                    if (flick.contentX >wavPos.width)
                        flick.contentX = wavPos.width
                }

                onClicked:
                {
                    waveShape.setWindowPosition(mouseX*125.0/2.0)
                    spectrum.loadSpectrum(item.filename,mouseX*125.0/2.0)
                }
            }

            WavePosition
            {
                id:wavPos
                height: 20
                width: 3000 //actually must calculate it for each file
                y:  50

                property int time : 0
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        flick.contentX = 0
                        //soundEngine.loadFile(item.filename) //TODO
                        //soundEngine.startPlayback()
                    }
                }

            } //try to make transparent and put on the waveshape

            Waveshape
            {
                id: waveShape
                height: 200
                width: 3000

                y: 50  + wavPos.height

                Component.onCompleted: {
                    waveShape.loadFile(item.filename)
                    console.log("component competed waveshape ", item.filename)
                }
            }

        }
    }

    ComboBox
    {
        id: sizeComboBox

        y:  waveShape.height + waveShape.y + 35
        x: 1600 + 50

        model: ["1024","2048","4096","8192","16384"]

        currentIndex: 2

        onCurrentIndexChanged: {
            console.log("Selected " + sizeComboBox.currentIndex + sizeComboBox.currentText )

            var windowWidth = 4096

            if (sizeComboBox.currentIndex == 0) windowWidth = 1024
            if (sizeComboBox.currentIndex == 1) windowWidth = 2048
            if (sizeComboBox.currentIndex == 2) windowWidth = 4096
            if (sizeComboBox.currentIndex == 3) windowWidth = 8192
            if (sizeComboBox.currentIndex == 4) windowWidth = 16384

            waveShape.setWindowWidth(windowWidth)
            if (spectrum != null)
                spectrum.setSamplesAmount(windowWidth)
        }
    }

    Spectrograph
    {
        y: waveShape.height + waveShape.y + 35
        width: 1600
        height: 200

        id:spectrum
        Component.onCompleted: {
            //spectrum.setSoundEngine(soundEngine)
            spectrum.changeBarsCount(200)
        }
    }

    /*
    Connections
    {
        target: soundEngine

        id: positionConnection

        property int counter : 0
        onPlayPositionChanged:
        {
            //console.log("Play change position " + position);

            wavPos.changePosition(position)
            positionConnection.counter += 1
            //console.log(positionConnection.counter)
            if (positionConnection.counter >= 10)
            {
                flick.contentX += 100;
                positionConnection.counter -= 10
            }
        }
    }*/
}
