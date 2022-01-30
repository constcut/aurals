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
        title: "Please choose a midi file"
        folder: shortcuts.home
        selectMultiple: false
        onAccepted: {
            audio.openMidiFile(openFileDialog.fileUrls[0].substring(7))
            audio.startMidiPlayer()
            openFileDialog.visible = false
        }
        onRejected: {
            openFileDialog.visible = false
        }
        nameFilters: [ "Midi file (*.mid *.midi)" ]
    }



    FileDialog {
        id: saveFileDialog
        title: "Save midi file to wav"
        folder: shortcuts.home
        selectExisting: false
        selectMultiple: false
        onAccepted: {
            audio.saveMidiToWav(saveFileDialog.fileUrls[0].substring(7))
            saveFileDialog.visible = false
        }
        onRejected: {
            saveFileDialog.visible = false
        }
        nameFilters: [ "Wav file (*.wav)" ]
    }


    ColumnLayout {

        id: mainLayout
        y: 30
        x: 10

        spacing:  10

        RowLayout {
            spacing: 10
            MidiRenderSettings {
            }
        }
        RowLayout {
            spacing: 10
            ComboBox {
                id: testFilesCombo
                model: ["test1.mid", "test2.mid", "test3.mid", "test4.mid", "test5.mid", "test6.mid"]
            }
            ToolButton {
                text: "Load and play"
                onClicked:  {
                    audio.openMidiFile(testFilesCombo.currentText)
                    audio.startMidiPlayer()
                }
            }
            ToolButton {
                text: "Stop"
                onClicked:  audio.stopMidiPlayer()
            }
            ToolButton {
                text: "Load test file"
                onClicked: audio.loadMidiFile(testFilesCombo.currentText)
            }
            ToolButton {
                text: "Play loaded file"
                onClicked: audio.startMidiPlayer()
            }
            ToolButton {
                text: "Load and play from file"
                onClicked: openFileDialog.visible = true
            }
            ToolButton {
                text: "Export pcm"
                onClicked: saveFileDialog.visible = true
            }
        }
        RowLayout {
            spacing: 10

            ComboBox {
                id: tabCombo
                model: 70


            }
            ToolButton {
                text: "Load and play"
                onClicked:  {
                    var testId = parseInt(tabCombo.currentText) + 1
                    audio.openTabFile("tests/3." + testId + ".gp4")
                    audio.startMidiPlayer()
                }
            }
            ToolButton {
                text: "Load view"
                onClicked: {
                    var testId = parseInt(tabCombo.currentText) + 1
                    var tabName = "tests/3." + testId + ".gp4"
                    console.log(tabName, "loading tab")
                    tabView.loadTab(tabName)
                    trackCombo.model = tabView.tracksCount()
                }
            }
            ToolButton {
                text: "Open track view"
                onClicked: {
                    var trackIdx = parseInt(trackCombo.currentText)
                    trackView.setFromTab(tabView, trackIdx)
                    trackDialog.open()
                }
            }
            ComboBox {
                id: trackCombo
            }
            ToolButton {
                text: "animation"
                onClicked: {
                    tabView.prepareAllThreads(0)//TODO on TabLoad
                    tabView.launchAllThreads()
                }
            }
            ToolButton {
                text: "stop animation"
                onClicked:  {
                    tabView.stopAllThreads()
                }
            }
            ToolButton {
                text: "Full play"
                onClicked:  {
                    tabView.prepareAllThreads(0)//
                    var testId = parseInt(tabCombo.currentText) + 1
                    audio.openTabFile("tests/3." + testId + ".gp4")
                    audio.startMidiPlayer()
                    tabView.launchAllThreads()
                }
            }
        }
    }

    Component.onCompleted:  {
        //Autoload tab for a while
        var testId = parseInt(tabCombo.currentText) + 1
        var tabName = "tests/3." + testId + ".gp4"
        console.log(tabName, "loading tab")
        tabView.loadTab(tabName)
        trackCombo.model = tabView.tracksCount()
    }

    TabView {

        id: tabView

        x: 0
        y: mainLayout.y + mainLayout.height
        width: parent.width
        height: parent.height - 10 - y

        MouseArea {
            anchors.fill: parent
            onClicked:
                tabView.onclick(mouseX, mouseY)
            onDoubleClicked:
                tabView.onclick(mouseX, mouseY)
        }
    }

    function keyboardEventSend(key) {
        tabView.keyPress(key)
    }

    Dialog {
        id: trackDialog
        width: 700
        height: 400

        ToolButton {
            text: "animation"
            onClicked: {
                tabView.prepareAllThreads(0)//
                var testId = parseInt(tabCombo.currentText) + 1
                audio.openTabFile("tests/3." + testId + ".gp4")
                audio.startMidiPlayer()
                tabView.launchAllThreads()
            }
        }

        ToolButton {
            text: "fullplay"
            x: parent.width - width - 10
            onClicked:  {
                trackView.prepareThread(0)//TODO on connection
                var testId = parseInt(tabCombo.currentText) + 1
                audio.openTabFile("tests/3." + testId + ".gp4")
                audio.startMidiPlayer()
                trackView.launchThread()
            }
        }

        TrackView {

            y: 30
            id: trackView
            width: parent.width
            height: parent.height

            MouseArea {
                anchors.fill: parent
                onClicked:
                    trackView.onclick(mouseX, mouseY)
                onDoubleClicked:
                    trackView.ondblclick(mouseX, mouseY)
            }
        }
    }

}
