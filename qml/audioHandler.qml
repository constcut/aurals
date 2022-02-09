import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1

Item {
    id: audioHandlerItem

    Dialog {
        id: testAudioDialog
        width: testLayout.width + 10

        ColumnLayout {
            spacing:  10
            id: testLayout
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
            }
            RowLayout {
                MidiRenderSettings {
                    id: midiSettings
                }
            }
        }
    }

    Dialog {
        id: renameDialog
        standardButtons:  Dialog.NoButton
        RowLayout {
            spacing: 10
            TextField {
                id: filenameEdit
                width: 300
            }
            ToolButton {
                text: "Rename"
                onClicked: {
                    audio.renameRecord(filesModel.filename, filenameEdit.text)
                    audioHandlerItem.reload()
                    filesModel.filename = ""
                    renameDialog.close()
                }
            }
            ToolButton {
                text: "Cancel"
                onClicked: {
                    renameDialog.close()
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
                text: recording ? "Stop+save" : "Start rec"
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

            ToolButton {
                text: "Rename"
                onClicked: {
                    if (filesModel.filename === "")
                        return
                    renameDialog.open()
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
            ToolButton {
                text: "P"
                onClicked: {
                    testAudioDialog.visible = !testAudioDialog.visible
                }
            }
            ToolButton {
                text: "Wave"
                onClicked:  {
                    thatWindow.requestWaveshape("records/" + filenameEdit.text)
                }
            }
            ToolButton {
                text: "ACF"
                onClicked:  {
                    thatWindow.requestAFC("records/" + filenameEdit.text)
                }
            }
            ToolButton {
                text: "STFT"
                onClicked:  {
                    thatWindow.requestSTFT("records/" + filenameEdit.text)
                }
            }
            ToolButton {
                text: "CQT"
                onClicked:  {
                    thatWindow.requestCQT("records/" + filenameEdit.text)
                }
            }
            ToolButton {
                text: "Filters"
                onClicked:  {
                    thatWindow.requestFilters("records/" + filenameEdit.text)
                }
            }
            ToolButton {
                text: "Cepstrum"
                onClicked:  {
                    thatWindow.requestCepstrum("records/" + filenameEdit.text)
                }
                visible: false //Yet unknow how to use it
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
            nameFilters: [ "Wav file (*.wav)" ]
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
                text: "Open wave"
                onTriggered: {
                    thatWindow.requestWaveshape("records/" + filenameEdit.text)
                }
            }
            MenuItem {
                text: "Open ACF"
                onTriggered: {
                    thatWindow.requestAFC("records/" + filenameEdit.text)
                }
            }
            MenuItem {
                text: "Open STFT"
                onTriggered: {
                    thatWindow.requestSTFT("records/" + filenameEdit.text)
                }
            }
            MenuItem {
                text: "Open CQT"
                onTriggered: {
                    thatWindow.requestCQT("records/" + filenameEdit.text)
                }
            }
            MenuItem {
                text: "Open filters"
                onTriggered: {
                    thatWindow.requestFilters("records/" + filenameEdit.text)
                }
            }
            MenuItem {
                text: "Open cepstrum"
                onTriggered: {
                    thatWindow.requestCepstrum("records/" + filenameEdit.text)
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

    function keyboardEventSend(key, mode) {
        //Заглушка, но можно реализовать логику здесь
    }

}
