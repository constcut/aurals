import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1

Item {
    id: audioHandlerItem


    Dialog {
        id: testAudioDialog
        ColumnLayout {
            spacing:  10
            RowLayout {
                spacing:  10
                id: upperLayout
                Text {
                    text: "Sample rate:"
                    visible: false
                }
                ComboBox {
                    model: ["22050", "44100", "48000"]
                    currentIndex: 1
                    onCurrentTextChanged: {
                        var sr = parseInt(currentText)
                        audio.setSampleRate(sr)
                    }
                }
                ToolButton {
                    text: "Start record"
                    onClicked: audio.startRecord()
                }
                ToolButton {
                    text: "Stop record"
                    onClicked: audio.stopRecord()
                }
                ToolButton {
                    text: "Reset"
                     onClicked: audio.resetBufer();
                }
                ToolButton {
                    text: "Start player"
                    onClicked: audio.startPlayback()
                }
                ToolButton {
                    text: "Stop player"
                    onClicked: audio.stopPlayback()
                }
                ToolButton {
                    text: "Save dump"
                    onClicked: audio.saveWavFile("rec.dump.wav")
                }
                ToolButton {
                    text: "Load dump"
                    onClicked: audio.loadWavFile("rec.dump.wav")
                }
                //ToolButton {
                //    text: "Request permission"
                //    onClicked: audio.requestPermission()
                //}
            }
            RowLayout {
                spacing:  10
                ToolButton {
                    text: "Check midi"
                    onClicked:  {
                        audio.checkMidi() //LOAD default file
                    }
                }
                ToolButton {
                    text: "Start midi"
                    onClicked:  {
                        audio.startMidiPlayer()
                    }
                }
                ToolButton {
                    text: "Stop midi"
                    onClicked:  {
                        audio.stopMidiPlayer()
                    }
                }
                ToolButton {
                    text: "Save midi dump"
                    onClicked: {
                        audio.saveMidiToWav("midi_.wav")
                    }
                }
                Text {
                    text: "Volume"
                }
                ComboBox {
                    model : ["24.0", "18.0", "12.0", "6.0", "0.0", "-6.0", "-12.0"]
                    currentIndex: 5
                    onCurrentTextChanged: {
                        var db = parseFloat(currentText)
                        audio.changeMidiRenderVolume(db)
                    }
                }
                ComboBox {
                    model: ["piano.sf2", "guitar.sf2", "el_guitar.sf2"]
                    currentIndex: 0
                    onCurrentTextChanged: {
                        audio.changeMidiSoundfont(currentText)
                    }
                }
            }
        }
    }

    ColumnLayout {
        spacing:  10
        y: 10
        x: 20

        RowLayout {
            spacing: 10
            ToolButton {
                property bool recording: false
                text: recording ? "Stop and save" : "Start record"
                onClicked: {
                    if (recording) {
                        audio.stopRecord()
                        audio.saveRecordTimstamp()
                        audioHandlerItem.reload()
                        recording = false
                    }
                    else {
                       audio.resetBufer()
                       audio.startRecord()
                       recording = true
                    }
                }
            }
            TextEdit {
                id: filenameEdit
                width: 250
            }
            ToolButton {
                text: "Rename"
                onClicked: {
                    if (filesModel.filename === "")
                        return
                    //Возможно стоит открывать диалог, в котором вводить новое имя, и одновременно иметь возможность отменить действие
                    audio.renameRecord(filesModel.filename, filenameEdit.text)
                    audioHandlerItem.reload()
                    filesModel.filename = ""
                }
            }
            ToolButton {
                text: "Remove"
                onClicked: {
                    if (filesModel.filename === "")
                        return
                    confirmDialog.visible = true
                }
            }
            ToolButton {
                text: "Show/Hide panel"
                onClicked: {
                    testAudioDialog.visible = !testAudioDialog.visible
                }
            }
            ToolButton {
                text: "Play"
                onClicked: {
                    audio.resetBufer()
                    audio.loadWavFile("records/" + filenameEdit.text)
                    audio.startPlayback()
                }
            }
            ToolButton {
                text: "Open file"
                onClicked:  {
                    fileDialog.visible = true
                }
            }
        }

        FileDialog {
            id: fileDialog
            title: "Please choose a wav file (16 bit, 44100 sr)"
            folder: shortcuts.home
            onAccepted: {
                thatWindow.requestWaveshape(fileDialog.fileUrls[0].substring(7))
                fileDialog.visible = false
            }
            onRejected: {
                fileDialog.visible = false
            }
        }

        MessageDialog {
            id: confirmDialog
            title: "Delete file"
            text: "Do you really want to delete selected file?"
            standardButtons: StandardButton.Yes | StandardButton.No
            onYes: {
                audio.deleteRecord(filesModel.filename)
                audioHandlerItem.reload()
                filesModel.filename = ""
            }
            visible: false
        }

        ListModel {
            id: filesModel
            property string filename: ""
        }

        Component {
            id: highlightBar
            Rectangle {
                id: highlightBarRect
                width: 200; height: 50
                color: "#FFFF88"
                y: filesList.currentItem == null ? 0 : filesList.currentItem.y
                Behavior on y { SpringAnimation { spring: 2; damping: 0.3 } }
            }
        }

        Component {
            id: fileDeligate
            Item {
                id: wrapper
                width: filesList.width
                height: 35
                Column {
                    Text {
                        text: name
                    }
                }
                states: State {
                    name: "Current"
                    when: wrapper.ListView.isCurrentItem
                    PropertyChanges { target: wrapper; x: 20 }
                }
                transitions: Transition {
                    NumberAnimation { properties: "x"; duration: 200 }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        wrapper.ListView.view.currentIndex = index
                        filesModel.filename = name
                        filenameEdit.text = name
                    }
                    onDoubleClicked: {
                        thatWindow.requestWaveshape("records/" + filenameEdit.text)
                    }
                    onPressAndHold: {
                        wrapper.ListView.view.currentIndex = index
                        filesModel.filename = name
                        filenameEdit.text = name
                        filesMenu.x = mouse.x
                        filesMenu.y = parent.y + mouse.y
                        filesMenu.open()
                    }
                }
            }
        }


        Menu {
            id: filesMenu
            MenuItem {
                text: "Play"
                onTriggered: {
                    audio.resetBufer()
                    audio.loadWavFile("records/" + filenameEdit.text)
                    audio.startPlayback()
                }
            }
            MenuItem {
                text: "Open"
                onTriggered: {
                    thatWindow.requestWaveshape("records/" + filenameEdit.text)
                }
            }
            MenuItem {
                text: "Delete"
                onTriggered: {
                    if (filesModel.filename === "")
                        return
                    confirmDialog.visible = true
                }
            }
        }


        Rectangle { //DELAYED: поиск по записям
            id: mainRect
            width: 600
            height: audioHandlerItem.height - y - 10
            ListView {
                id: filesList
                clip: true
                anchors.fill: parent
                model: filesModel
                Behavior on y { NumberAnimation{ duration: 200 } }
                onContentYChanged: {} //When implement search bar copy behavior
                delegate: fileDeligate
                highlight: highlightBar
                focus:  true
                ScrollBar.vertical: ScrollBar {}
            }
        }
    }

    property bool useHotFixFirstRecord: true

    function reload() {
        if (useHotFixFirstRecord)
            audio.startRecord() //Hotfix of first record, before records or playback
        var files = audio.getRecords();
        filesModel.clear()
        for (var i = 2; i < files.length; ++i)
            filesModel.append({"name": files[i]})
        if (useHotFixFirstRecord) {
            audio.stopRecord()
            audio.resetBufer()
        }
    }

    Component.onCompleted: {
        audioHandlerItem.reload()
    }

}
