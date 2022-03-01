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
                        implicitWidth: 70
                        model: [4, 6, 8, 3, 2, 1, 5, 9, 10, 7, 16, 32] //Basic
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
                        implicitWidth: 70
                        model: [4, 8, 16, 2, 1, 32]
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
                        implicitWidth: 70
                        model: [4, 8, 16, 32, 2, 1]
                        onCurrentTextChanged: {
                            pattern.setBrickSize(parseInt(currentText))
                            var fullWidth = pattern.fullWidth()
                            pattern.width = fullWidth + 10
                            if (flick.contentWidth < fullWidth)
                                flick.contentWidth = fullWidth
                        }
                    }

                    PatternLine {
                        id: pattern
                        width: 3000
                        height: 50

                        MouseArea {
                            anchors.fill: parent
                            onDoubleClicked: {
                                pattern.onClick(mouseX, mouseY)
                            }
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