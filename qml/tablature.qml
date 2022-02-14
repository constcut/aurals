import QtQuick 2.15
import QtQml 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1
import aurals 1.0


Item {

    id: tablatureItem

    property bool useScrollArea: false

    FileDialog {
        id: openFileDialog
        title: "Please choose a tab file"
        folder: shortcuts.home
        selectMultiple: false
        onAccepted: {
            var tabName = openFileDialog.fileUrls[0].substring(7)
            tabView.loadTab(tabName)
            trackCombo.model = tabView.tracksCount()
            mainLayout.refreshTrack()
            tabPannel.open()
        }
        onRejected: {
            openFileDialog.visible = false
        }
        nameFilters: [ "Tab files (*)" ]
    }



    FileDialog {
        id: saveFileDialog
        title: "Save pcm file"
        folder: shortcuts.home
        selectExisting: false
        selectMultiple: false
        onAccepted: {
            var pcmName = saveFileDialog.fileUrls[0].substring(7)
            tabView.exportMidi("temp.mid", 0)
            audio.openMidiFile("temp.mid")

            if (Qt.platform.os !== "android")
                audio.saveMidiToWav(pcmName)
            else {
                var correctPath = decodeURIComponent(saveFileDialog.fileUrls[0])
                audio.saveMidiToWav(correctPath)
            }
        }
        onRejected: {
            saveFileDialog.close()
        }
        nameFilters: [ "Wav file (*.wav)" ]
    }


    FileDialog {
        id: saveMidiDialog
        title: "Save midi file" //Сделать позже единый диалог сохранения и вызов внешних функций в зависимости от его строкового названия property string type: value
        folder: shortcuts.home
        selectExisting: false
        selectMultiple: false
        onAccepted: {
            var midiName = saveMidiDialog.fileUrls[0].substring(7)

            if (Qt.platform.os !== "android")
                tabView.exportMidi(midiName, 0)
            else {
                var correctPath = decodeURIComponent(saveFileDialog.fileUrls[0])
                console.log("Midi save path", correctPath)
                tabView.exportMidi(correctPath, 0)
                //https://www.weiy.city/2021/12/file-io-between-qfile-and-std-fstream/ Need to wrap ofstream
            }

        }
        onRejected: {
            saveFileDialog.close()
        }
        nameFilters: [ "Midi file (*.mid)" ]
    }


    Menu {
        id: tabMenu
        MenuItem {
            text: "New"
            onTriggered:  {
                tabView.createNewTab()
                mainLayout.refreshTrack()
            }
        }
        MenuItem {
            text: "New track"
            onTriggered: {
                tabView.passTabCommand(Tab.NewTrack)
                trackCombo.model = tabView.tracksCount()
                tabPannel.open()
            }
        }

        MenuItem {
            text: "TabView"
            onTriggered: tabPannel.open()
        }
        MenuItem {
            text: "Open file"
            onTriggered: openFileDialog.open()
        }
        MenuItem {
            text: "Save as"
            onTriggered: tabView.passTabCommand(Tab.SaveAs) //TODO normal save (only for gmy files, if other then first save as)
        }
        MenuItem {
            text: "Export wav"
            onTriggered: saveFileDialog.open()
        }
        MenuItem {
            text: "Export midi"
            onTriggered: saveMidiDialog.open()
        }
        MenuItem {
            text: "Use scroll area"
            onTriggered: tablatureItem.useScrollArea = !tablatureItem.useScrollArea
        }
    }


    Drawer { //Moved to help fast ediitig pannel options
        id: tabPannel
        width: parent.width
        height: Qt.platform.os == "android" ? 0.7 * parent.height  : 0.3 * parent.height
        edge: Qt.BottomEdge


        RowLayout {
            spacing: 10
            id: trackLayout

            ComboBox {
                id: trackCombo
                onCurrentTextChanged: {
                    mainLayout.refreshTrack()
                }
            }

            //Actions: "BPM",
            //"NewTrack",
            //"DeleteTrack",
            //"AddMarker",
            //"OpenReprise",
            //"CloseReprise",
            //"GotoBar",
            //"Tune"
            //"SetSignTillEnd"

            //TODO other settings : drums, name
            ComboBox {
                id: instrumentCombo
                Layout.minimumWidth: implicitIndicatorWidth
                model : [
                    "Acoustic Grand Piano",
                    "Bright Acoustic Piano",
                    "Electric Grand Piano",
                    "Honky-tonk Piano",
                    "Rhodes Piano",
                    "Chorused Piano",
                    "Harpsichord",
                    "Clavinet",
                    "Celesta",
                    "Glockenspiel",
                    "Music Box",
                    "Vibraphone",
                    "Marimba",
                    "Xylophone",
                    "Tubular Bells",
                    "Dulcimer",
                    "Hammond Organ",
                    "Percussive Organ",
                    "Rock Organ",
                    "Church Organ",
                    "Reed Organ",
                    "Accodion",
                    "Hrmonica",
                    "Tango Accodion",
                    "Acoustic Guitar (nylon)",
                    "Acoustic Guitar (steel)",
                    "Electric Guitar (jazz)",
                    "Electric Guitar (clean)",
                    "Electric Guitar (muted)",
                    "Overdriven Guitar",
                    "Distortion Guitar",
                    "Guitar Harmonics",
                    "Acoustic Bass",
                    "Electric Bass (finger)",
                    "Electric Bass (pick)",
                    "Fretless Bass",
                    "Slap Bass 1",
                    "Slap Bass 2",
                    "Synth Bass 1",
                    "Synth Bass 2",
                    "Violin",
                    "Viola",
                    "Cello",
                    "Contrabass",
                    "Tremolo Strings",
                    "Pizzicato Strings",
                    "Orchestral Harp",
                    "Timpani",
                    "String Ensemble 1",
                    "String Ensemble 2",
                    "SynthStrings 1",
                    "SynthStrings 2",
                    "Choir Aahs",
                    "Voice Oohs",
                    "Synth Voice",
                    "Orchetra Hit",
                    "Trumpet",
                    "Trombone",
                    "Tuba",
                    "Muted Trumpet",
                    "French Horn",
                    "Brass Section",
                    "Synth Brass 1",
                    "Synth Brass 2",
                    "Soprano Sax",
                    "Alto Sax",
                    "Tenor Sax",
                    "Baritone Sax",
                    "Oboe",
                    "English Horn",
                    "Bassoon",
                    "Clarinet",
                    "Piccolo",
                    "Flute",
                    "Recorder",
                    "Pan Flute",
                    "Bottle Blow",
                    "Shakuhachi",
                    "Wistle",
                    "Ocarina",
                    "Lead 1 (square)",
                    "Lead 2 (sawtooth)",
                    "Lead 3 (caliope lead)",
                    "Lead 4 (chiff lead)",
                    "Lead 5 (charang)",
                    "Lead 6 (voice)",
                    "Lead 7 (hiths)",
                    "Lead 8 (bass + lead)",
                    "Pad 1 (new age)",
                    "Pad 2 (warm)",
                    "Pad 3 (polysynth)",
                    "Pad 4 (choir)",
                    "Pad 5 (bowed)",
                    "Pad 6 (metalic)",
                    "Pad 7 (halo)",
                    "Pad 8 (sweep)",
                    "FX 1 (rain)",
                    "FX 2 (soundrack)",
                    "FX 3 (crystl)",
                    "FX 4 (atmosphere)",
                    "FX 5 (brightness)",
                    "FX 6 (goblins)",
                    "FX 7 (echoes)",
                    "FX 8 (sci-fi)",
                    "Sitar",
                    "Banjo",
                    "Shamisen",
                    "Koto",
                    "Kalimba",
                    "Bigpipe",
                    "Fiddle",
                    "Shanai",
                    "Tinkle Bell",
                    "Agogo",
                    "Steel Drums",
                    "Woodblock",
                    "Taiko Drum",
                    "Melodic Tom",
                    "Synth Drum",
                    "Reverce Cymbal",
                    "Guitar Fret Noise",
                    "Breath Noise",
                    "Seashore",
                    "Bird Tweet",
                    "Telephone ring",
                    "Helicopter",
                    "Applause",
                    "Gunshot"]

                onCurrentTextChanged:  {
                    trackView.setInstrument(currentIndex)
                }
            }

            Slider {
                id: volumeSlider
                from: 0
                to: 16
                stepSize: 1
                ToolTip {
                    parent: volumeSlider.handle
                    visible: volumeSlider.hovered
                    text: volumeSlider.value.toFixed(2)
                }

                onValueChanged: {
                    trackView.setVolume(value)
                }
            }
            Slider {
                id: panoramSlider
                from: -8
                to: +8
                stepSize: 1
                ToolTip {
                    parent: panoramSlider.handle
                    visible: panoramSlider.hovered
                    text: panoramSlider.value.toFixed(2)
                }
                onValueChanged: {
                    trackView.setPanoram(value + 8)
                }
            }
            ComboBox {
                id: trackStatusCombo
                model: ["normal", "mute", "solo"]
                onCurrentTextChanged: {
                    trackView.setStatus(currentIndex)
                }
            }
        } //Row layout */


        TabView {
            id: tabView
            y: trackLayout.height
            width: parent.width
            height: parent.height + 100
        }
        // Возможно разбить на 2 компонента, 1 заголовок треков, другой серия тактов, и второй поместить в Scroll Area
    }



    ColumnLayout {

        id: mainLayout
        y: 10
        x: 10
        spacing:  10

        function refreshTrack() {

            var trackIdx = parseInt(trackCombo.currentText)
            trackView.setFromTab(tabView, trackIdx)
            tabView.setTrackIdx(trackIdx)

            if (Qt.platform.os != "android" && useScrollArea) {
                var realHeight = trackView.getFullPixelHeight();
                flick.contentHeight = realHeight
                //But then we missing autoscroll
            }

            instrumentCombo.currentIndex = trackView.getInstrumet()
            volumeSlider.value = trackView.getVolume()
            panoramSlider.value = trackView.getPanoram() - 8
            trackStatusCombo.currentIndex = trackView.getStatus() //TODO enum
        }

        RowLayout {
            spacing: 10

            ToolButton {
                text: "__Menu__"
                onClicked: tabMenu.open()
            }

            Text {
                text: "Test:"
            }

            ComboBox {
                id: tabCombo
                model: 70
                currentIndex: 0

                implicitWidth: 80

                onCurrentTextChanged: {
                    var testId = parseInt(tabCombo.currentText) + 1
                    var tabName = "tests/3." + testId + ".gp4"
                    tabView.loadTab(tabName)
                    trackCombo.model = tabView.tracksCount()
                    mainLayout.refreshTrack()
                    tabPannel.open()
                }
            }

            ToolButton {
                icon.source: "qrc:/icons/play.png"
                onClicked: {
                    var currentPosition = tabView.getTimeLineBar()
                    tabView.prepareAllThreads(currentPosition)
                    tabView.exportMidi("temp.mid", currentPosition)

                    if (aconfig.param("midi.config") === "on") {
                        audio.openMidiFile("temp.mid")
                        audio.startMidiPlayer()
                    }
                    else {
                        console.log("Alt config playing midi")
                        audio.openMidiFileEngine("temp.mid")
                        audio.startMidiFileEngine()
                    }
                    tabView.launchAllThreads()
                }
            }
            ToolButton {
                icon.source: "qrc:/icons/x.png"
                onClicked:  {
                    tabView.stopAllThreads()
                    audio.stopMidiPlayer()
                    audio.stopMidiFileEngine()
                }
            }

            ComboBox {
                id: trackCommandCombo
                currentIndex: 52
                model: [
                    "GotoStart",
                    "SetSignForSelected",
                    "SelectionExpandLeft",
                    "SelectionExpandRight",
                    "InsertBar",
                    "NextBar",
                    "PrevBar",
                    "NextPage",
                    "PrevPage",
                    "StringDown",
                    "StringUp",
                    "PrevBeat",
                    "NextBeat",
                    "SetPause",
                    "DeleteBar",
                    "DeleteSelectedBars",
                    "DeleteSelectedBeats",
                    "DeleteNote",
                    "IncDuration",
                    "DecDuration",
                    "NewBar",
                    "SetDot",
                    "SetTriole",
                    "Leeg",
                    "Dead",
                    "Vibrato",
                    "Slide",
                    "Hammer",
                    "LetRing",
                    "PalmMute",
                    "Harmonics",
                    "TremoloPickings",
                    "Trill",
                    "Stokatto",
                    "FadeIn",
                    "Accent",
                    "HeaveAccent",
                    "Bend",
                    "Chord",
                    "Text",
                    "Changes",
                    "UpStroke",
                    "DownStroke",
                    "SetBarSign",
                    "Cut",
                    "Copy",
                    "CopyBeat",
                    "CopyBars",
                    "Paste",
                    "Undo",
                    "PrevLine",
                    "NextLine",


                    "SetSignTillEnd",
                    "SaveAs",
                    "Mute",
                    "Solo",
                    "MoveLeft",
                    "MoveRight",
                    "MoveUp",
                    "MoveDown",
                    "Drums",
                    "Instument",
                    "Panoram",
                    "Volume",
                    "Name",
                    "BPM",
                    "NewTrack",
                    "DeleteTrack",
                    "PauseMidi",
                    "AddMarker",
                    "OpenReprise",
                    "CloseReprise",
                    "GotoBar",
                    "Tune"
                ]
                //currentIndex: 20
            }
            ToolButton {
                text: "!"
                width: 30
                onClicked: {

                    if (trackCommandCombo.currentIndex < 52)
                        tabView.passTrackCommand(trackCommandCombo.currentIndex)
                    else {
                        var commandIdx = trackCommandCombo.currentIndex - 52
                        tabView.passTabCommand((commandIdx))
                    }
                }
            }

            ToolButton {
                text: "+"
                width: 30

                PropertyAnimation {
                    id: showAnimation
                    target: editPannel
                    property: "y"
                    to: tablatureItem.height - tablatureItem.buttonSize * 3 - tablatureItem.buttonSpacing * 2
                }

                PropertyAnimation {
                    id: hideAnimation
                    target: editPannel
                    property: "y"
                    to: tablatureItem.height
                }

                property bool shown: false

                onClicked: {
                    if (shown) {
                        hideAnimation.start()
                        shown = false
                    }
                    else {
                        showAnimation.start()
                        shown = true
                    }
                }
            }

            ComboBox {
                id: tab2Combo
                model: 39
                currentIndex: 0

                implicitWidth: 80

                onCurrentTextChanged: {
                    var testId = parseInt(tab2Combo.currentText) + 1
                    var tabName = "tests/2." + testId + ".gp4"
                    tabView.loadTab(tabName)
                    trackCombo.model = tabView.tracksCount()
                    mainLayout.refreshTrack()
                    tabPannel.open()
                }

                visible: Qt.platform.os != "android"
            }

            ToolButton {
                text: "<"
                onClicked: tabView.passTrackCommand(Tab.PrevPage)
            }
            ToolButton {
                text: ">"
                onClicked: tabView.passTrackCommand(Tab.NextPage)
            }
        }
    }


    ScrollView {
        y: mainLayout.y + mainLayout.height + 5
        id: trackViewScroll
        width: parent.width
        height: parent.height - y - editPannel.height

        clip: true

        Flickable {
            id: flick

            boundsBehavior : Flickable.StopAtBounds
            boundsMovement : Flickable.StopAtBounds

            width: parent.width
            height: parent.height
            contentWidth: parent.width
            contentHeight:  parent.height


            TrackView {

                id: trackView
                width: parent.width
                height: parent.height

                property int lastPress: 0

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        trackView.onclick(mouseX, mouseY)
                        trackView.focus = true
                    }
                    onDoubleClicked: {
                        trackView.dropSelection()
                        trackView.onclick(mouseX, mouseY)
                        trackView.focus = true
                    }
                    onPressAndHold: {
                        trackView.onSelection(mouseX, mouseY)
                        trackView.focus = true
                    }

                }
            }

        }

    }




    property int buttonSize: 40
    property int buttonSpacing: 5


    Rectangle {
        visible: true
        id: editPannel

        border.color: "darkgray"

        x: 0
        y: parent.height //- tablatureItem.buttonSize * 3 - tablatureItem.buttonSpacing * 2
        width: parent.width
        height: parent.height - y

        RowLayout {
            x: parent.width - width

            Repeater { //Мы можем так же добавить параметрические значения, и просто в первой панели их вставлять после,
                model: 12

                ToolButton {
                    property int digitIdx: index
                    icon.source: iconName(digitIdx)

                    function iconName(idx) {

                        if (idx === 11)
                            return ""

                        if (idx === 10)
                            return "qrc:/icons/x.png"

                        if (idx < 10)
                            return "qrc:/icons/" + idx + ".png"
                    }

                    onClicked: {
                        if (digitIdx < 10)
                            tabView.keyPress(digitIdx + 48, 0) //ascii digit

                        if (digitIdx == 10)
                            tabView.passTrackCommand(Tab.Dead)
                    }

                    palette.button: digitIdx !== 11 ? "lightgray" : "transparent"
                    palette.mid: digitIdx !== 11  ? "lightgray" : "transparent"
                }
            } //Repeater
        } //RowLayout - digits


        RowLayout {
            x: parent.width - width
            y: tablatureItem.buttonSize + tablatureItem.buttonSpacing

            Repeater { //(map pictures to Tab.command (js function))

                id: secondPanelLine

                property var images: ["vib", "leeg", "", "qp", "qm", "", "del", "", "prevBar", "^", "nextBar", ""]
                property var commands: [Tab.Vibrato, Tab.Leeg, -1, Tab.IncDuration, Tab.DecDuration,
                    -1, Tab.DeleteNote, -1, Tab.PrevLine, Tab.StringDown, Tab.NextLine, -1] //replace by imageToCommand()

                model: images.length

                ToolButton {
                    property int idx: index
                    icon.source: secondPanelLine.images[index] !== "" ? "qrc:/icons/" + secondPanelLine.images[index] + ".png" : ""
                    onClicked: tabView.passTrackCommand(secondPanelLine.commands[idx])

                    //Way to make spaces
                    palette.button: secondPanelLine.images[index] !== "" ? "lightgray" : "transparent"
                    palette.mid: secondPanelLine.images[index] !== "" ? "lightgray" : "transparent"
                }

            } //Repeater
        } //RowLayout - arrows

        RowLayout {
            x: parent.width - width
            y: tablatureItem.buttonSize * 2 + tablatureItem.buttonSpacing * 2

            Repeater {

                id: thirdPanelLine

                property var images: ["pm", "lr", "", "newBar", "p", "", "undo", "", "prev", "V", "next", ""]
                property var commands: [Tab.PalmMute, Tab.LetRing, -1, Tab.NewBar, Tab.SetPause,
                                        -1, Tab.Undo, -1, Tab.PrevBeat, Tab.StringUp, Tab.NextBeat, -1]

                model: images.length

                ToolButton {
                    property int idx: index
                    icon.source:  thirdPanelLine.images[index] !== "" ? "qrc:/icons/" + thirdPanelLine.images[index] + ".png" : ""
                    onClicked: tabView.passTrackCommand(thirdPanelLine.commands[idx])

                    palette.button: thirdPanelLine.images[index] !== "" ? "lightgray" : "transparent"
                    palette.mid: thirdPanelLine.images[index] !== "" ? "lightgray" : "transparent"
                }
            } //Repeater
        } //RowLayout - arrows



    } //edit panel


    function keyboardEventSend(key, mode) {
        tabView.keyPress(key, mode)
    }

}
