import QtQuick 2.15
import QtQml 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1
import aurals 1.0


Item {

    id: patternInputItem



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
                onClicked:   {
                    audio.stopMidiPlayer()

                }
            }
        }
    }


    ScrollView {
        y: mainLayout.y + mainLayout.height + 20
        width: parent.width
        height: parent.height - y

        Flickable {
            id: flick
            width: parent.width
            height: parent.height
            contentWidth: 3000
            contentHeight:  parent.height

            //Component with repeaters + MouseArea

            Repeater {
                model: 5

                RowLayout {
                    y: index * 60 //Заменить на значение высоты
                    spacing: 10
                    ComboBox {
                        model: [4, 6, 8, 3, 2, 1] //Basic
                        onCurrentTextChanged: {
                            pattern.setNumerator(parseInt(currentText))
                        }
                    }
                    ComboBox {
                        model: [4, 8, 16, 2, 1, 32]
                        onCurrentTextChanged: {
                            pattern.setDenomenator(parseInt(currentText))
                        }
                    }
                    Text {
                        text: ":"
                    }
                    ComboBox {
                        model: [4, 8, 16]
                        onCurrentTextChanged: {
                            pattern.setBrickSize(parseInt(currentText))
                        }
                    }

                    PatternLine {
                        id: pattern
                        width: 300
                        height: 50
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
