import QtQuick 2.15
import aurals 1.0

TableView {

    y: 100
    height: parent.height - y
    width: parent.width

    columnSpacing: 1
    rowSpacing: 1
    clip: true

    model: ConfigTableModel {}

    delegate: Rectangle {
        implicitWidth: 100
        implicitHeight: 50
        Text {
            text: display
        }
    }
}
