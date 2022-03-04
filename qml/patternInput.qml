import QtQuick 2.15
import QtQml 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1
import aurals 1.0


Item {

    id: patternInputItem

    Dialog {
        id: dialog

        width: 150
        height: 150

        PatternReceiver {
            width: 110
            height: 110
            id: receiver
        }
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
            ToolButton {
                text: "Circle"
                onClicked: {
                    dialog.open()
                }
            }
            ComboBox {
                implicitWidth: 80
                model: [60, 120, 180, 240]
                currentIndex: 1
                onCurrentTextChanged: {
                    receiver.setBPM(parseInt(currentText))
                }
            }
            ComboBox {
                implicitWidth: 70
                model: [2, 3, 4, 1, 8, 10, 20, 50]
                onCurrentTextChanged: {
                    receiver.setRepeatTimes(parseInt(currentText))
                }
            }

            ComboBox {
                implicitWidth: 80
                model: [60, 120, 300, 60000]
                onCurrentTextChanged: {
                    receiver.setSecondsLimit(parseInt(currentText))
                }
            }

            ToolButton {
                text: ">"
                onClicked: {
                    totalTimeText.text = receiver.totalSeconds()
                    receiver.generateMidi("pattern.mid")
                    audio.openMidiFile("pattern.mid")
                    audio.saveMidiToWav("pattern.wav")
                    audio.startMidiPlayer() //TODO some issue in player eats few notes sometimes
                }
            }
            ToolButton {
                text: "Stop"
                onClicked:   {
                    audio.stopMidiPlayer()

                }
            }
            Text {
                id: totalTimeText
                text: ""
            }
            ToolButton {
                text: "+"
                onClicked: {
                    receiver.storeState()
                    receiver.reset()
                    patternRepeater.model = patternRepeater.model + 1
                    receiver.loadState()

                    flick.contentHeight = patternRepeater.model * 50 + 10

                    for (var i = 0; i < patternRepeater.model; ++i)
                        patternRepeater.itemAt(i).updateCombos()
                }
            }
            ToolButton {
                text: "-"
                onClicked: {
                    receiver.storeState()
                    receiver.reset()
                    if (patternRepeater.model)
                        patternRepeater.model = patternRepeater.model - 1
                    receiver.loadState()

                    flick.contentHeight = patternRepeater.model * 50 + 10

                    for (var i = 0; i < patternRepeater.model; ++i)
                        patternRepeater.itemAt(i).updateCombos()
                }
            }
            ToolButton {
                text: "S"
                onClicked: {
                    receiver.saveToFile("pattern.json")
                }
            }
            ToolButton {
                text: "L"
                onClicked: {
                    receiver.loadFromFile("pattern.json")
                    patternRepeater.model = receiver.getStoreSize()
                    receiver.loadState()

                    for (var i = 0; i < patternRepeater.model; ++i)
                        patternRepeater.itemAt(i).updateCombos()
                }
            }
            ToolButton {
                text: "Rec"
                onClicked:  {
                    receiver.generateMidi("pattern.mid")
                    audio.openMidiFile("pattern.mid")
                    audio.resetBufer()
                    audio.startMidiAndRecording()
                }
            }
            ToolButton {
                text: "Stop rec"
                onClicked: {
                    audio.stopRecord()
                    audio.stopMidiPlayer()
                    audio.mixRecordAndMidi()
                    audio.saveWavFile("mix.wav")
                }
            }
        }
    }


    ScrollView {
        y: mainLayout.y + mainLayout.height + 20
        width: parent.width
        height: parent.height - y

        clip: true

        Flickable {
            id: flick
            width: parent.width
            height: parent.height
            contentWidth: 3000
            contentHeight: patternRepeater.model * 50 + 10

            boundsBehavior: Flickable.StopAtBounds
            boundsMovement: Flickable.StopAtBounds

            Repeater {
                id: patternRepeater
                model: 5

                RowLayout {
                    y: index * 50 //Заменить на значение высоты
                    spacing: 10

                    id: patternRow

                    function updateCombos() {
                        var num = pattern.getNum()
                        numCombo.currentIndex = num - 1

                        var denom = pattern.getDenom()
                        denomCombo.currentIndex = Math.log2(denom)

                        var brickSize = pattern.getBrickSize()
                        brickSizeCombo.currentIndex = Math.log2(brickSize)

                        var midiNote = pattern.getMidiNote()
                        midiNoteCombo.currentIndex = midiNote
                    }

                    ComboBox {

                        id: numCombo

                        implicitWidth: 70
                        model: [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17] //Basic
                        currentIndex: 3

                        onCurrentTextChanged: {
                            pattern.setNumerator(parseInt(currentText))
                            var fullWidth = pattern.fullWidth()
                            pattern.width = fullWidth + 10
                            if (flick.contentWidth < fullWidth)
                                flick.contentWidth = fullWidth
                            //Умное обновление при уменьшение, хранить весь массив длин линий, и выбирать максимальный
                        }
                    }
                    ComboBox {

                        id: denomCombo

                        implicitWidth: 70
                        model: [1, 2, 4, 8, 16, 32]
                        currentIndex: 2

                        onCurrentTextChanged: {
                            pattern.setDenomenator(parseInt(currentText))
                            var fullWidth = pattern.fullWidth()
                            pattern.width = fullWidth + 10
                            if (flick.contentWidth < fullWidth)
                                flick.contentWidth = fullWidth
                        }
                    }
                    Text {
                        text: ":"
                    }
                    ComboBox {

                        id: brickSizeCombo

                        implicitWidth: 70
                        model: [1, 2, 4, 8, 16, 32]
                        currentIndex: 2

                        onCurrentTextChanged: {
                            pattern.setBrickSize(parseInt(currentText))
                            var fullWidth = pattern.fullWidth()
                            pattern.width = fullWidth + 10 //SOMETHING broken
                            if (flick.contentWidth < fullWidth)
                                flick.contentWidth = fullWidth
                        }
                    }
                    ComboBox {

                        id: midiNoteCombo

                        implicitWidth: 70
                        property var notes: [36, 40, 57, 48, 46, 35, 35, 35]
                        model: 61
                        currentIndex: index < 8 ? notes[index] : 35
                        onCurrentTextChanged: {
                            pattern.setMidiNote(parseInt(currentText))
                        }
                    }

                    PatternLine {
                        id: pattern
                        width: 3000
                        height: 50

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                pattern.onClick(mouseX, mouseY)
                            }
                        }

                        Component.onCompleted: {
                            receiver.connectLine(pattern)
                        }
                    }
                }
            }
        }
    }


    Component.onCompleted: {
    }

    function keyboardEventSend(key, mode) {
        //Заглушка, но можно реализовать логику здесь
    }

}
