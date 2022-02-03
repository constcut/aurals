import QtQuick 2.15
import QtQml 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1
import mther.app 1.0


Item {

    id: midiPlayerItem

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
        title: "Save pcm file" //TODO to midi + to wav
        folder: shortcuts.home
        selectExisting: false
        selectMultiple: false
        onAccepted: {
            var pcmName = saveFileDialog.fileUrls[0].substring(7)
            tabView.exportMidi("temp.mid", 0) //TODO shift
            audio.openMidiFile("temp.mid")
            audio.saveMidiToWav(pcmName)
        }
        onRejected: {
            saveFileDialog.close()
        }
        nameFilters: [ "Wav file (*.wav)" ]
    }


    FileDialog {
        id: saveMidiDialog
        title: "Save midi file" //TODO to midi + to wav
        folder: shortcuts.home
        selectExisting: false
        selectMultiple: false
        onAccepted: {
            var midiName = saveMidiDialog.fileUrls[0].substring(7)
            tabView.exportMidi(midiName, 0) //TODO shift
        }
        onRejected: {
            saveFileDialog.close()
        }
        nameFilters: [ "Midi file (*.mid)" ]
    }





    ColumnLayout {

        id: mainLayout
        y: 10
        x: 10
        spacing:  10

        function refreshTrack() { //TODO смесить куда-то
            var trackIdx = parseInt(trackCombo.currentText)
            trackView.setFromTab(tabView, trackIdx)

            var instrId = trackView.getInstrumet()
            var volume = trackView.getVolume()
            var panoram = trackView.getPanoram()
            var status = trackView.getStatus()

            instrumentCombo.currentIndex = instrId
            volumeSlider.value = volume
            panoramSlider.value = panoram - 8 //todo review!
            trackStatusCombo.currentIndex = status
        }

        RowLayout {
            spacing: 10

            ComboBox {
                id: tabCombo
                model: 70
                currentIndex: 0

                onCurrentTextChanged: {
                    var testId = parseInt(tabCombo.currentText) + 1
                    var tabName = "tests/3." + testId + ".gp4"
                    tabView.loadTab(tabName)
                    trackCombo.model = tabView.tracksCount()
                    mainLayout.refreshTrack()
                    tabPannel.open()
                }

            }

            ComboBox {
                id: trackCombo
                onCurrentTextChanged: {
                    mainLayout.refreshTrack()
                }
            }
            ToolButton {
                text: "Play"
                onClicked:  {
                    tabView.prepareAllThreads(0)//TODO shift?
                    tabView.exportMidi("temp.mid", 0) //TODO shift
                    audio.openMidiFile("temp.mid")
                    audio.startMidiPlayer()
                    tabView.launchAllThreads()
                }
            }
            ToolButton {
                text: "Stop"
                onClicked:  {
                    tabView.stopAllThreads()
                    audio.stopMidiPlayer()
                }
            }
            ToolButton {
                text: "TabView"
                onClicked: tabPannel.open()
            }
            ToolButton {
                text: "Open file"
                onClicked: openFileDialog.open()
            }
            ToolButton {
                text: "Save wav"
                onClicked: saveFileDialog.open()
            }
            ToolButton {
                text: "Save midi"
                onClicked: saveMidiDialog.open()
            }
        }

        RowLayout {
            spacing: 10

            ComboBox {
                id: instrumentCombo
                Layout.minimumWidth: implicitIndicatorWidth
                model : [ //TODO ui C++ item to get QList of QStings
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
        }
        RowLayout {
            spacing: 10

            Button {
                text: ":" //Future menu button
            }
            ComboBox {
                id: tabCommandCombo
                model : ["SetSignTillEnd",
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
                    "OpenTrack",
                    "NewTrack",
                    "DeleteTrack",
                    "PlayMidi",
                    "GenerateMidi",
                    "PauseMidi",
                    "AddMarker",
                    "OpenReprise",
                    "CloseReprise",
                    "GotoBar",
                    "Tune"]
            }
            Button {
                text: "tab"
                onClicked: {
                    tabView.passTabCommand(tabCommandCombo.currentIndex)
                }
            }
            ComboBox {
                id: trackCommandCombo
                model: [
                    "PlayFromStart",
                    "GotoStart",
                    "SetSignForSelected",
                    "SelectionExpandLeft",
                    "SelectionExpandRight",
                    "InsertBar",
                    "NextBar",
                    "PrevBar",
                    "NextPage",
                    "PrevPage",
                    "NextTrack",
                    "PrevTrack",
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
                    "PlayTrackMidi",
                    "SaveFile",
                    "SaveAsFromTrack",
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
                    "Undo"
                ]
            }
            Button {
                text: "track"
                onClicked: {
                    tabView.passTrackCommand(trackCommandCombo.currentIndex)
                }
            }
        }
    }





    Drawer {
        id: tabPannel
        width: parent.width
        height: Qt.platform.os == "android" ? 0.7 * parent.height  : 0.3 * parent.height
        edge: Qt.BottomEdge

        TabView {
            id: tabView
            x: 0
            y: 0
            width: parent.width
            height: parent.height

            MouseArea {
                anchors.fill: parent
                onClicked:
                    tabView.onclick(mouseX, mouseY) //TODO can crash
                onDoubleClicked:
                    tabView.onclick(mouseX, mouseY)
            }
        }
    }


    TrackView {

        y: mainLayout.y + mainLayout.height + 5
        id: trackView
        width: parent.width
        height: parent.height - y

        MouseArea {
            anchors.fill: parent
            onClicked:
                trackView.onclick(mouseX, mouseY)
            onDoubleClicked:
                trackView.ondblclick(mouseX, mouseY)
        }
    }


    function keyboardEventSend(key) {
        tabView.keyPress(key)
    }

}
