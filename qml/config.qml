import QtQuick 2.15
import QtQuick.Controls 2.15
import aurals 1.0

TableView {

    y: 60
    x: 40
    height: parent.height - y
    width: parent.width - x

    columnSpacing: 1
    rowSpacing: 1
    clip: true

    model: ConfigTableModel {
        id: tableModel
    }

    delegate: Rectangle {
        implicitWidth: 300
        implicitHeight: 50

        TextField {
            visible: column == 1

            width: 300
            text: display
            onTextEdited: {
                if (text.length != 0)
                    tableModel.updateValue(row, text)
            }
        }

        Text {
            visible: column == 0
            text: display
        }
    }

    function keyboardEventSend(key, mode) {

    }
}
