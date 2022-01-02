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
                    windowInfo.text = "Window RMS = " + spectrum.getRMSNoWindow().toFixed(4)
                    + "\nPitch = " + spectrum.getPitch().toFixed(3)
                    + "\nMIDI# = " + spectrum.freqToMidi(spectrum.getPitch())
                    + "\nSpecPitch= " + spectrum.getSpectrumF0().toFixed(3)
                    + "\nTime = " + ((mouseX*125.0/2.0) / 44100.0).toFixed(4)
                    outputRmsGroup(mouseX)

                    windowInfo.text += "\n" //TODO rename component
                    if (tooLoudRms)
                        windowInfo.text += "RMS clipped!  "
                    if (spectrum.clipped())
                        windowInfo.text += "Spectrum clipped! "
                    if (spectrum.gotGap() === false)
                        windowInfo.text += "Noisy spectrum! "

                    windowInfo.text += "Gaps: " + spectrum.gapLevel()
                }
                onDoubleClicked: {
                    audio.loadOnlyWindow(item.filename, mouseX*125.0/2.0, parseInt(sizeComboBox.currentText))
                    audio.startPlayback() //TODO with repeat
                }

                property bool tooLoudRms: false

                function outputRmsGroup(mouseX) {
                    tooLoudRms = false
                    var min = 120.0
                    var max = -120.0

                    var rmsStep = waveShape.getRmsStep();
                    var rmsCoef = rmsStep / (125 / 2.0)

                    for (var i = 0; i < 50; i+=2) {
                        var rmsCheckValue = waveShape.getRMS((mouseX + i) / rmsCoef)
                        if (rmsCheckValue > max)
                            max = rmsCheckValue
                        if (rmsCheckValue < min)
                            min = rmsCheckValue
                    }

                    if (max > -1.0)
                        tooLoudRms = true
                    rmsGroup.text = "<b>{</b>";
                    var prev = waveShape.getRMS(mouseX/ rmsCoef)

                    var dbLevelBorder = 9 //TODO settable params
                    var dbLevelBorderSmall = 6

                    var totalSize = waveShape.getRmsSize();

                    for (i = 0; i < 25; ++i) {
                        if (i && i % 5 == 0)
                            rmsGroup.text += "<br> ";

                        var idx = mouseX / rmsCoef + i
                        if (idx >= totalSize)
                            break;

                        var rmsValue = waveShape.getRMS(mouseX / rmsCoef + i)
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
                    text: "Window size: "
                }
                ComboBox {
                    model: ["256", "1024","2048","4096","8192","16384"]
                    currentIndex: 3
                    onCurrentTextChanged: {
                        var windowWidth = parseInt(currentText)
                        waveShape.setWindowWidth(windowWidth)
                        if (spectrum != null) {
                            spectrum.setSamplesAmount(windowWidth)
                            spectrum.setFFTLimit((windowWidth))
                        }
                    }
                }
                Text{
                    text: "Spectrum bins: "
                }
                ComboBox {
                    model : [100, 200, 400, 600, 800, 1000]
                    currentIndex: 2
                    onCurrentTextChanged: {
                        spectrum.changeBarsCount(parseInt(currentText))
                    }
                }
                Text{
                    id: highFreqText
                    text: "High freq: "
                }
                ComboBox {
                    model: ["1000", "2000", "3000", "4000", "5000", "6000", "7000"]
                    currentIndex: 3
                    onCurrentTextChanged: {
                        spectrum.changeHighFreq(parseInt(currentText))
                    }
                }
            }
            RowLayout {
                spacing:  10
                Text {
                    text: "RMS Peak sence: "
                }
                ComboBox {
                    model : ["2.0", "3.0", "4.0", "5.0", "6.0", "7.0"]
                    currentIndex: 2
                    onCurrentTextChanged: {
                        waveShape.setPeakSence(parseFloat(currentText))
                    }
                }
                Text {
                    text: "RMS step"
                }
                ComboBox {
                    model : ["125", "250", "500", "1000", "1500", "2000", "4000"]
                    currentIndex: 2
                    onCurrentTextChanged: {
                        waveShape.setRmsStep(parseInt(currentText))
                    }
                }

            }
            RowLayout {
                spacing:  10
                Text {
                    text: "Yin threshold"
                }
                ComboBox {
                    model : ["0.5", "0.1", "0.15", "0.2", "0.25", "0.3", "0.35", "0.4", "0.45", "0.5"]
                    currentIndex: 2
                    onCurrentTextChanged: {
                        var threshold = parseFloat(currentText)
                        waveShape.setYinThreshold(threshold)
                        spectrum.setYinThreshold(threshold)
                    }
                }
                Text{
                    text: "Yin limit size: "
                }
                ComboBox {
                    model: ["2048", "4096", "6000", "8192"]
                    currentIndex: 1
                    onCurrentTextChanged: {
                        if (spectrum)
                            spectrum.setYinLimit(parseInt(currentText))
                    }
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
        id : windowInfo
        y : specInfo.y + specInfo.height + 5
        x: 25
        text: "rms yin info"
    }

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
            var bins = spectrum.getBinTable()
            var summs = spectrum.getBinCount()
            var checkCount = bins.length < 20 ? bins.length : 20

            for (var i = 0; i < checkCount; ++i) {
                var bin = bins[i]
                var sum = summs[i]
                if (i && i % 5 == 0)
                    peaksGroup.text += "<br>";
                peaksGroup.text += "<b>" + bin + "</b> : " + sum
                peaksGroup.text += " ; "
            }
        }
    }

}
