#include "PianoRoll.h"
#include <QPainter>
#include <QDebug>

using namespace aurals;


void PianoRoll::loadMidi(QString filename) {
    _mid.readFromFile(filename.toStdString());
    update();
}


int PianoRoll::getContentWidth() {
    int maxPixels = 0;

    //Или делать это локально по 1 треку? т.к. длина будет варироваться
    for (const auto& mTrack: _mid)
        if (mTrack.empty() == false)
        {
           double maxX = mTrack.at(mTrack.size() - 1).absoluteTime()  / 50.0;
           if (maxPixels < maxX)
               maxPixels = maxX;
        }

    return maxPixels;
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

    const int noteHeight = [&](){
      if (_fillHeight == false)
          return 5;
      else {
          const int dist = maxMidi - minMidi;
          return static_cast<int>(height() / dist);
      }
    }();

    auto midiNoteToPosition = [&](int midiNote) {
        if (_fillHeight == false)
            return midiNote * noteHeight;
        else {
           const int localNote = midiNote - minMidi;
           return localNote * noteHeight;
        }
        return 0;
    };


    painter->setPen(QColor("lightgray"));
    for (int i = minMidi; i < maxMidi; ++i) {
        painter->drawLine(0, midiNoteToPosition(i),
                          width(), midiNoteToPosition(i));
    }



    for (const auto& message: _mid.at(_currentTrack))
    {
        const auto pos = ( message.absoluteTime() / 50.0 ) * _xZoomCoef;

        //TODO open ray with positions double values for opened once notes

        if (message.getEventType() == MidiEvent::NoteOn) {
            const auto midiNote = message.getParameter1();
            painter->setPen(QColor("red"));
            painter->drawRect(pos, midiNoteToPosition(midiNote), 2, noteHeight);

            //1. If no record on ray - then add it
            //2. If record - then paint object + create it with x,y, w,h - to catch clicking

        }
        if (message.getEventType() == MidiEvent::NoteOff) {
            const auto midiNote = message.getParameter1();
            painter->setPen(QColor("blue"));
            painter->drawRect(pos, midiNoteToPosition(midiNote), 2, noteHeight);

            //1. If no record - just ignore
            //2. If record - paint + create object + clean the ray
        }
    }

}
