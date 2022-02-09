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

                /*
                preventStealing: true
                onPressed: {
                    flick.pressedX = mouseX
                }
                onReleased: {
                    var diff =  flick.pressedX - mouseX
                    //console.log('Move diff: ', diff) //Use for moving note start\end
                }*/ //Kills flick
                onClicked:{
                    var minRmStep = waveShape.getMinRmsStep()
                    waveShape.setWindowPosition(mouseX * minRmStep/2.0)
                    //If we need to load some file: spectrum.loadSpectrum(item.filename,mouseX*minRmStep/2.0) //This version reloads file
                    //spectrum.loadByteArray(waveShape.getPCM(mouseX * minRmStep / 2.0, spectrum.getSamplesAmount()));
                    spectrum.loadFloatSamples(waveShape.getFloatSamples(mouseX * minRmStep / 2.0, spectrum.getSamplesAmount()))

                    var winRmsDb = waveShape.getWindowRmsDb()
                    var winYinF0 = waveShape.getWindowYinF0()

                    windowInfo.text = "Window RMS = " + winRmsDb.toFixed(4)
                    + "\nWin Pitch = " + winYinF0.toFixed(3)
                    + "\nMIDI# = " + waveShape.freqToMidi(winYinF0).toFixed(3)
                    + "\nSpecPitch= " + spectrum.getSpectrumF0().toFixed(3)
                    + "\nTime = " + ((mouseX * minRmStep / 2.0) / 44100.0).toFixed(4)
                    outputRmsGroup(mouseX)

                    windowInfo.text += "\n"
                    if (tooLoudRms)
                        windowInfo.text += "RMS clipped!  "
                    if (spectrum.clipped())
                        windowInfo.text += "Spectrum clipped! "
                    if (spectrum.gotGap() === false)
                        windowInfo.text += "Noisy spectrum! "

                    windowInfo.text += "Gaps: " + spectrum.gapLevel()

                    chromaGroup.text = ""
                    var chroma = spectrum.getChroma()
                    var maxIdx = spectrum.getChromaMaxIdx()

                    for (var i = 0; i < 12; ++i) {

                        if (i === maxIdx)
                             chromaGroup.text += "<b><font color='green'>"

                        chromaGroup.text += chroma[i].toFixed(2)

                        if (i === maxIdx)
                             chromaGroup.text += "</font></b>"

                        if (i != 11)
                            chromaGroup.text +=  ", "
                    }

                    chromaGroup.text += "<br>"

                    var list = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']
                    for (i = 0; i < 12; ++i) {
                        chromaGroup.text += list[i]
                        var spaces = 8  - list[i].length
                        chromaGroup.text += Array(spaces).fill('&nbsp;').join('')
                    }

                    chromaGroup.text += "<br><br>Octaves energy:<br>"
                    var octaveEnergy = spectrum.getOctaveEnergy()
                    for (i = 0; i < 12; ++i)
                        chromaGroup.text += octaveEnergy[i].toFixed(2) + " "

                    chromaGroup.text += "<br>"
                    for (i = 0; i < 12; ++i)
                        chromaGroup.text += i + Array(7).fill('&nbsp;').join('')
                }
                onDoubleClicked: {
                    var minRmStep = waveShape.getMinRmsStep()
                    //If we need to load some file: audio.loadOnlyWindow(item.filename, mouseX * minRmStep / 2.0, spectrum.getSamplesAmount())
                    audio.loadWindowPCM(waveShape.getPCM(mouseX * minRmStep / 2.0, spectrum.getSamplesAmount()))
                    audio.startPlayback()
                }

                property bool tooLoudRms: false

                function outputRmsGroup(mouseX) {
                    tooLoudRms = false
                    var min = 120.0
                    var max = -120.0

                    var rmsStep = waveShape.getRmsStep();
                    var rmsMinStep = waveShape.getMinRmsStep();
                    var rmsCoef = rmsStep / (rmsMinStep / 2.0)

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

                    var dbLevelBorder = parseInt(rmsLevel2.currentText)
                    var dbLevelBorderSmall = parseInt(rmsLevel1.currentText)

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
                        var barsCount = parseInt(currentText)
                        spectrum.changeBarsCount(barsCount)
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
                        var highFreq = parseInt(currentText)
                        spectrum.changeHighFreq(highFreq)
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
                        var peaksSense = parseFloat(currentText)
                        waveShape.setPeakSence(peaksSense)
                    }
                }
                Text {
                    text: "RMS step"
                }
                ComboBox {
                    model : ["125", "250", "500", "1000", "1500", "2000", "4000"]
                    currentIndex: 2
                    onCurrentTextChanged: {
                        var rmsStep = parseInt(currentText)
                        waveShape.setRmsStep(rmsStep)
                    }
                }
                ComboBox {

                    model: ["No window", "Hann", "Gaus", "Blackman", "Hamm"]
                    currentIndex: 1

                    onCurrentTextChanged: {
                        spectrum.setWindowFunction(currentIndex)
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
                    currentIndex: 3
                    onCurrentTextChanged: {
                        var threshold = parseFloat(currentText)
                        waveShape.setYinThreshold(threshold)
                    }
                }
                Text {
                    text: "Yin window size"
                }
                ComboBox {
                    model : [1536, 2048, 3072, 4096, 6144, 8192, 16384]
                    currentIndex: 1
                    onCurrentTextChanged: {
                        var yinWindow = parseInt(currentText)
                        waveShape.setYinWindowSize(yinWindow)
                    }
                }
            }
            RowLayout {
                Text {
                    text: "RMS db small"
                }
                ComboBox {
                    id: rmsLevel1
                    model: [2,3,4,5,6,7,8,9,10,11,12]
                    currentIndex: 4
                }
                Text {
                    text: "RMS db big"
                }
                ComboBox {
                    id: rmsLevel2
                    model: [6,7,8,9,10,11,12,13,14,15,16,17,18]
                    currentIndex: 3
                }
                CheckBox {
                    text: "Half cut fft"
                    checked: false
                    onCheckStateChanged: {
                        spectrum.changeHalfCut(checked)
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
        y: windowInfo.y
        x: parent.width / 4
        text: "<font color='red'>RMS group</font>"
    }

    Text {
        id: chromaGroup
        y: specInfo.y
        x: 2 * parent.width / 4 - 50
        text: "<font color='green'>Chroma group</font>"
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

    ToolButton {
        id: saveWavButton
        y: specInfo.y
        x: parent.width - width - 10 - settingsButton.width - 10
        text: "Save wave to file"
        onClicked: {
            saveFileDialog.source = "Wave"
            saveFileDialog.open()
        }
    }

    ToolButton {
        y: saveWavButton.y
        x: saveWavButton.x - width - 10
        text: "CWT"
        onClicked:  {
            waveShape.makeCWT()
        }
        visible: false
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


    ToolButton {
        y: f0Button.y
        x: parent.width - width - 10 - f0Button.width - 10
        text: "Save spectrum to file"
        onClicked: {
            saveFileDialog.source = "Spectrum"
            saveFileDialog.open()
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

    Button {
        id: showSTFT
        y: showNotesButton.y
        x: parent.width - width - 10 - showNotesButton.width - 10
        text: "Show STFT"
        onClicked:  {
            strechedImg.setImage(waveShape.makeSTFT(), 800);
            specDialog.mode = "STFT"
            specDialog.open()
        }
    }

    ToolButton {
        y: showSTFT.y
        x: showSTFT.x - width - 10
        text: "Const-Q"
        onClicked:  {
            strechedImg.setImage(waveShape.makeCQT(), 800);
            specDialog.mode = "CQT"
            specDialog.open()
        }
    }



    Dialog {
        id: specDialog

        width: 825
        height: 500

        property string mode: ""

        StretchImage {
            x: 0
            y: 0
            width: 800
            height: 400
            id: strechedImg
        }

        ToolButton {
            y: parent.height - height - 5
            x: parent.width - width - 10
            text: "Save to file"
            onClicked: {
                saveFileDialog.source = specDialog.mode
                saveFileDialog.open()
            }
        }
    }

    FileDialog {
        id: saveFileDialog
        title: "Save image to file"
        folder: shortcuts.home
        selectExisting: false
        selectMultiple: false
        defaultSuffix: "jpg"

        property string source: ""

        onAccepted: {
            var filename = saveFileDialog.fileUrls[0].substring(7)
            if (source == "STFT") {
                waveShape.saveSTFT(filename)
            }
            if (source == "Wave") {
                waveShape.saveImage(filename)
            }
            if (source == "Spectrum") {
                spectrum.saveImage(filename)
            }
            if (source == "CQT") {
                waveShape.saveCQT(filename)
            }
            saveFileDialog.visible = false
        }
        onRejected: {
            saveFileDialog.visible = false
        }
        nameFilters: [ "JPEG file (*.jpg)" , "All files(*)"]
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

        }
    }

    function keyboardEventSend(key, mode) {
        //Заглушка, но можно реализовать логику здесь
    }

}
