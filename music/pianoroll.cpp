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

void PianoRoll::reset() {
    _mid = MidiFile();
    update();
}

void PianoRoll::paint(QPainter* painter) {

    if (_mid.empty())
        return;

    int minMidi = 128;
    int maxMidi = 0;
    for (const auto& message: _mid.at(_currentTrack)) {
        if (message.getEventType() == MidiEvent::NoteOn) {
            const auto midiNote = message.getParameter1();
            if (maxMidi < midiNote)
                maxMidi = midiNote;
            if (minMidi > midiNote)
                minMidi = midiNote;
        }
    }

    const int noteHeight = 5;
    painter->setPen(QColor("lightgray"));
    for (size_t i = minMidi; i < maxMidi; ++i) {
        painter->drawLine(0, i * noteHeight,
                          width(), i * noteHeight);
    }


    //notes
    qDebug() << "Total tracks " << _mid.size();
    qDebug() << "Messages on track " << _currentTrack << " "
             << _mid.at(_currentTrack).size();

    for (const auto& message: _mid.at(_currentTrack)) {

        //qDebug() << "__ " << message.absoluteTime();

        const auto pos =  message.absoluteTime() / 50.0;


        if (message.getEventType() == MidiEvent::NoteOn) {
            const auto midiNote = message.getParameter1();
            painter->setPen(QColor("red"));
            painter->drawRect(pos, noteHeight * midiNote, 2, noteHeight);
        }
        if (message.getEventType() == MidiEvent::NoteOff) {
            const auto midiNote = message.getParameter1();
            //todo colors
            painter->setPen(QColor("blue"));
            painter->drawRect(pos, noteHeight * midiNote, 2, noteHeight);
        }
    }

}
