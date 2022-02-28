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
        y: mainLayout.y + mainLayout.height
        width: parent.width
        height: parent.height - y

        Flickable {
            id: flick
            width: parent.width
            height: parent.height
            contentWidth: 3000
            contentHeight:  parent.height

            //Component with repeaters + MouseArea
        }
    }


    Component.onCompleted: {
    }

    function keyboardEventSend(key, mode) {
        //Заглушка, но можно реализовать логику здесь
    }

}
