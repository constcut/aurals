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
        }
        onRejected: {
            openFileDialog.visible = false
        }
        nameFilters: [ "Tab files (*)" ]
    }



    FileDialog {
        id: saveFileDialog
        title: "Save tab file to midi" //TODO to midi + to wav
        folder: shortcuts.home
        selectExisting: false
        selectMultiple: false
        onAccepted: {
        }
        onRejected: {
        }
        nameFilters: [ "Midi file (*.wav)" ]
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
                    console.log(tabName, "loading tab")
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
            ToolButton {
                text: "TabView"
                onClicked: tabPannel.open()
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
