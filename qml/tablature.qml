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

        }
        onRejected: {
            saveFileDialog.close()
        }
        nameFilters: [ "Wav file (*.wav)" ]
    }


    ColumnLayout {

        id: mainLayout
        y: 10
        x: 10
        spacing:  10

        function refreshTrack() { //TODO смесить куда-то
            var trackIdx = parseInt(trackCombo.currentText)
            trackView.setFromTab(tabView, trackIdx)
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
                    tabView.prepareAllThreads(0)//
                    var testId = parseInt(tabCombo.currentText) + 1
                    audio.openTabFile("tests/3." + testId + ".gp4")
                    //TODO generate new midi file (for changes)
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
