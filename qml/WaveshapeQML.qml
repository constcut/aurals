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
        console.log("reload file of waveshape qml called ", filename)
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
                    if (flick.contentX >wavPos.width)
                        flick.contentX = wavPos.width
                }
                onClicked:{
                    waveShape.setWindowPosition(mouseX*125.0/2.0)
                    spectrum.loadSpectrum(item.filename,mouseX*125.0/2.0)
                    rmsYinIngo.text = "Window RMS = " + spectrum.getRMSNoWindow().toFixed(4)
                    + "\nPitch = " + spectrum.getPitch().toFixed(4) + "\nMIDI# = " + spectrum.freqToMidi(spectrum.getPitch())
                    + "\nTime = " + ((mouseX*125.0/2.0) / 44100.0).toFixed(4)
                    + "\nTotal peaks = " + spectrum.peaksCount()
                    outputRmsGroup(mouseX)
                }



                function outputRmsGroup(mouseX) {
                    var min = 120.0
                    var max = -120.0
                    for (var i = 0; i < 50; i+=2) {
                        var rmsCheckValue = waveShape.getRMS((mouseX + i) / 2.0)

                        if (rmsCheckValue > max)
                            max = rmsCheckValue
                        if (rmsCheckValue < min)
                            min = rmsCheckValue
                    }

                    rmsGroup.text = "<b>{</b>";

                    var prev = waveShape.getRMS(mouseX/ 2.0)

                    var dbLevelBorder = 9 //TODO settable params
                    var dbLevelBorderSmall = 6

                    for (i = 0; i < 25; ++i) {
                        if (i && i % 5 == 0)
                            rmsGroup.text += "<br> ";

                        var rmsValue = waveShape.getRMS(mouseX / 2.0 + i)


                        if (rmsValue === min)
                            rmsGroup.text += "<font color='blue'>";
                        else if (rmsValue === max)
                            rmsGroup.text += "<font color='red'>";

                        var diff = Math.abs(prev - rmsValue)
                        prev = rmsValue

                        if (diff > dbLevelBorderSmall)
                            rmsGroup.text += "<b>"
                        if (diff > dbLevelBorder)
                            rmsGroup.text += "<u>"

                        rmsGroup.text += rmsValue.toFixed(2)

                        if (diff > dbLevelBorder)
                            rmsGroup.text += "</u>"
                        if (diff > dbLevelBorderSmall)
                            rmsGroup.text += "</b>"

                        if (rmsValue === min || rmsValue === max)
                            rmsGroup.text += "</font>"

                        rmsGroup.text += " ; "
                    }

                    rmsGroup.text += "<b>}</b>"
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
                    //console.log("component competed waveshape ", item.filename)
                }
            }

        }
    }

    Dialog {
        id: settingsDialog
        RowLayout {
            spacing:  10
            id: upperLayout
            Text{
                y:  spectrum.y + spectrum.height + 10
                x: parent.width / 3
                id: specLabel
                text: "Big Window size: "
            }

            ComboBox
            {
                id: sizeComboBox

                y:  spectrum.y + spectrum.height + 10
                x: specLabel.x + specLabel.width + 10
                model: ["125", "1024","2048","4096","8192","16384"]

                currentIndex: 3

                onCurrentTextChanged: {
                    //console.log("Selected " + sizeComboBox.currentIndex + sizeComboBox.currentText )

                    var windowWidth = parseInt(currentText)

                    waveShape.setWindowWidth(windowWidth)
                    if (spectrum != null)
                        spectrum.setSamplesAmount(windowWidth)
                }
            }

            Text{
                y:  spectrum.y + spectrum.height + 10
                x: sizeComboBox.x + sizeComboBox.width + 10
                id: yinLabel
                text: "Yin limit size: "
            }
            ComboBox {
                id: yinLimitCombo

                y: spectrum.y + spectrum.height + 10
                x: yinLabel.x + yinLabel.width + 10
                currentIndex: 1
                model: ["3000", "4096", "6000"] //TODO cuctom size

                onCurrentTextChanged: {
                    if (spectrum)
                        spectrum.setYinLimit(parseInt(currentText))
                }
            }
            Text{
                y:  spectrum.y + spectrum.height + 10
                x: yinLimitCombo.x + yinLimitCombo.width + 10
                id: windowLimitCombo
                text: "Window limit: "
            }

            ComboBox {
                id: windowCutCombo
                y: spectrum.y + spectrum.height + 10
                x: windowLimitCombo.x + windowLimitCombo.width + 10
                currentIndex: 4
                model: ["256","512","1024", "2048", "4096", "8192"] //TODO cuctom size

                onCurrentIndexChanged: {
                    if (spectrum)
                        spectrum.setFFTLimit(parseInt(currentText))
                }
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
    }//TODO different parameters for yin to be set!

    Text {
        id: rmsGroup
        y: specInfo.y
        x: parent.width / 4
        text: "<font color='red'>RMS group</font>"
    }

    Text {
        id: peaksGroup
        y: specInfo.y
        x: 2 * parent.width / 4 + 30
        text: "<font color='green'>Peaks group</font>"
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

    Button {
        id: f0Button
        y: settingsButton.y + settingsButton.height + 5
        x: parent.width - width - 10
        text: "F0"
        onClicked:  {
            waveShape.calculateF0()
        }
    }

    Button {
        id: showNotesButton
        y: f0Button.y + f0Button.height + 5
        x: parent.width - width - 10
        text: "Show notes"
        onClicked:  {
            waveShape.showNotes()
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
                +  " lvl = " + 20*log10(spectrum.getValue()).toFixed(5) //" value " + spectrum.getValue() +
               // https://stackoverflow.com/questions/3019278/how-can-i-specify-the-base-for-math-log-in-javascript
            }
        }
        onSpectrumCalculated: {
            outputPeaksGroup()
        }
        function outputPeaksGroup() {
            peaksGroup.text = "{ ";

            var freqs = spectrum.getFreqPeaks();
            var amps = spectrum.getAmpPeaks();
            var max = -120;

            for (var i = 0; i < amps.length; ++i) {
                if (amps[i] > max)
                    max = amps[i]
            }

            var checkFreqsCount = freqs.length > 49 ? 49 : freqs.length

            for (i = 0; i < checkFreqsCount; ++i) {
                var freq = freqs[i]
                var amp = amps[i]
                var ratio = max / amp;

                if (i && i % 7 == 0)
                    peaksGroup.text += "<br>  ";

                if (ratio > 0.5)
                    peaksGroup.text += "<b>"
                if (ratio > 0.75)
                    peaksGroup.text += "<u>"

                if (i && freq < 100)
                    peaksGroup.text += "_"

                peaksGroup.text += freq.toFixed(0)

                if (ratio > 0.75)
                    peaksGroup.text += "</u>"
                if (ratio > 0.5)
                    peaksGroup.text += "</b>"

                peaksGroup.text += " ; "
            }

            peaksGroup.text += " }";
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
