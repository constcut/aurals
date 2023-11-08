import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1
import QtMultimedia 5.12

Item {
    id: searchItem

    Dialog {
        id: searchFilterDialog
        width: searchItem.width + 10

        RowLayout {
            spacing:  10
            Slider {
                id: durationSlider
                from: 0
                to: 60
                stepSize: 1
                value: 10
                ToolTip {
                    parent: durationSlider.handle
                    visible: durationSlider.hovered
                    text: "max duration: " + durationSlider.value.toFixed(2)
                }
                onValueChanged: {
                    searchItem.reload()
                }
            }
            TextField {
                id: wordSearch
                placeholderText: "Word to search"
            }
            ToolButton {
                onClicked: {
                    var files = audio.getReports();
                    filesModel.clear()
                    for (var i = 0 ; i < files.length; ++i) {
                        var report = audio.getSingleReport("json/" + files[i])
                        var word_found = false
                        for (var j = 0; j < report["events"].length; ++j) {
                            if (report["events"][j]["type"] === "word") {
                                if (wordSearch.text === report["events"][j]["word"]) {
                                    word_found = true
                                    break
                                }
                            }
                        }
                        if (word_found) {
                            filesModel.append({"name": files[i]})
                        }
                    }
                }
            }
        }
    }

    ColumnLayout {
        spacing:  10
        y: 10
        x: 20

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
                        //thatWindow.requestWaveshape("records/" + filenameEdit.text)
                    }
                    onPressAndHold: {
                        wrapper.ListView.view.currentIndex = index
                        filesModel.filename = name
                        //filenameEdit.text = name
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
                text: "Open"
                onTriggered: {
                    searchFilterDialog.open()
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
            height: searchItem.height - y - 10
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
        for (var i = 0 ; i < files.length; ++i) {
            var report = audio.getSingleReport("json/" + files[i])
            if (report["duration"] > durationSlider.value) {
                continue;
            }
            //TODO slider value in check
            filesModel.append({"name": files[i]})
        }
    }

    Component.onCompleted: {
        searchItem.reload()
        searchFilterDialog.open()
    }

    function keyboardEventSend(key, mode) {
        //Заглушка, но можно реализовать логику здесь
    }

}
