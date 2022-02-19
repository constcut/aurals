#include "PianoRoll.h"
#include <QPainter>
#include <QDebug>

using namespace aurals;

void PianoRoll::loadMidi(QString filename) {
    _mid.readFromFile(filename.toStdString());
    update();
}

 int PianoRoll::getContentWidth() {
    auto track = _mid.size();
    //calculate time moments
    return 100;
}

int PianoRoll::getContentHeight() {
    return 128;
}

void PianoRoll::paint(QPainter* painter) {

    const int noteHeight = 5;
    for (size_t i = 0; i < 128; ++i) {
        painter->drawLine(0, i * noteHeight,
                          width(), i * noteHeight);
    }

    if (_mid.empty())
        return;
    //notes
    qDebug() << "Total tracks " << _mid.size();
    qDebug() << "Messages on track 0 " << _mid.at(1).size();
    for (const auto& message: _mid.at(1)) {

        //qDebug() << "__ " << message.absoluteTime();

        const auto pos =  message.absoluteTime() / 250.0;


        if (message.getEventType() == MidiEvent::NoteOn) {
            const auto midiNote = message.getParameter1();
            painter->setPen(QColor("red"));
            painter->drawRect(pos, noteHeight * midiNote, 10, noteHeight);
        }
        if (message.getEventType() == MidiEvent::NoteOff) {
            const auto midiNote = message.getParameter1();
            //todo colors
            painter->setPen(QColor("blue"));
            painter->drawRect(pos, noteHeight * midiNote, 10, noteHeight);
        }
    }

}
