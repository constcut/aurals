import QtQuick 2.0
import mther.app 1.0

Item {
    Piano {

        y: 100
        width: parent.width
        height: 200
        x:50

        id:piano

        property int lastPress : 0

        MouseArea {
            anchors.fill: parent
            onPressed: {
                piano.lastPress = piano.getAssociation(mouseX,mouseY)
                console.log("Graphical association pr " + piano.lastPress)
            }
            onReleased: {
            }
        }
    }

    /*
    Connections {

        target: midiThread
        id: midiThreadConnection
        property int counter : 0
        onMessageReceived: {
            if (messageType == 144) {
                if (p2 == 0)
                    piano.unSelectByAssociation(p1) //note off
                else
                    piano.selectByAssociation(p1)  //note on
            }
        }
    }*/
}
