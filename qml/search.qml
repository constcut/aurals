import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1
import QtMultimedia 5.12

Item {
    id: audioHandlerItem //TODO rename + delete all exceeds

    ColumnLayout {
        spacing:  10
        y: 10
        x: 20

        Audio { //TODO check android, linux, windows
            id: playMusic
            source: "file:///Users/constcut/dev/builds/aurals/build-aurals-Desktop_Qt_5_15_2_clang_64bit-Debug/aurals.app/Contents/MacOS/records/2023-11-05T14.01.51.wav"
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
                        //filenameEdit.text = name
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
                }
            }
            MenuItem {
                text: "None"
                onTriggered: {
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
        var files = audio.getReports();
        filesModel.clear()
        for (var i = 0 ; i < files.length; ++i)
            filesModel.append({"name": files[i]})
        console.log(files)
        var first = audio.getSingleReport("json/" + files[0])
        console.log(first["duration"], "report from JS")
    }

    Component.onCompleted: {
        audioHandlerItem.reload()
    }

    function keyboardEventSend(key, mode) {
        //Заглушка, но можно реализовать логику здесь
    }

}
