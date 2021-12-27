import QtQuick 2.7
import mther.app 1.0
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
        height: parent.height/2

        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOn
        verticalScrollBarPolicy:  Qt.ScrollBarAlwaysOff

        Flickable
        {
            id: flick
            y: 5
            x: 0
            width: parent.width
            height: parent.height-20

            contentWidth: 3000
            contentHeight:  parent.height

            //QML2.ScrollBar.horizontal:  QML2.ScrollBar { }

            property int pressedX : 0
            MouseArea {
                //anchors.fill: parent

                x:0
                y:20

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
                    rmsYinIngo.text = "RMS = " + spectrum.getRMS()
                }
            }

            WavePosition
            {
                id:wavPos
                height: 20
                width: 1000 //actually must calculate it for each file
                y:  5

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
                height:  parent.height
                width: 1000

                y: wavPos.y  + wavPos.height

                Component.onCompleted: {
                    waveShape.loadFile(item.filename)
                    flick.contentWidth = waveShape.getPixelsLength() + 10
                    waveShape.width = flick.contentWidth
                    wavPos.width = waveShape.width
                    console.log("component competed waveshape ", item.filename)

                }
            }

        }
    }

    ComboBox
    {
        id: sizeComboBox

        y:  spectrum.y + spectrum.height + 10
        x: parent.width / 2

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
        y: waveShape.height + waveShape.y + 5
        width: parent.width
        height: parent.height / 6

        id:spectrum
        Component.onCompleted: {
            //spectrum.setSoundEngine(soundEngine)
            spectrum.changeBarsCount(200)
        }
        MouseArea {
            anchors.fill: parent
            function log10(val) {
              return Math.log(val) / Math.LN10;
            }
            onClicked: {
                spectrum.onPress(mouseX, mouseY, spectrum.width, spectrum.height)
                specInfo.text = spectrum.getFreq1() + "-" + spectrum.getFreq2() + " Hz"
                + " value " + spectrum.getValue() + " dB = " + 20 + log10(spectrum.getValue())
               // https://stackoverflow.com/questions/3019278/how-can-i-specify-the-base-for-math-log-in-javascript
            }
        }
    }
    Text {
        id: specInfo
        y : spectrum.y + spectrum.height + 10
        x : 25
        text: "Spectrum info"
    }
    Text {
        id : rmsYinIngo
        y : specInfo.y + specInfo.height + 5
        x: 25
        text: "rms yin info"
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
