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
            return (128 - midiNote) * noteHeight;
        else {
           const int localNote = maxMidi - midiNote; //midiNote - minMidi;
           return localNote * noteHeight;
        }
    };

    //TODO всё что выше перенести наружу

    painter->setPen(QColor("lightgray"));
    for (int i = minMidi - 1; i < maxMidi + 1; ++i) {
        painter->drawLine(0, midiNoteToPosition(i),
                          width(), midiNoteToPosition(i));
    }

    std::vector<double> ray(128, -1.0); //TODO _notes {x,y,w,h} .hit(x,y) на подобии как BarView в TrackView

    auto paintNote = [&](auto pos, auto midiNote) {
        const int noteWidth = pos - ray[midiNote]; //TODO cover under lambda
        painter->fillRect(ray[midiNote], midiNoteToPosition(midiNote), noteWidth, noteHeight,
                          QBrush(QColor("green")));
        painter->setPen(QColor("lightgreen"));
        painter->drawRect(ray[midiNote], midiNoteToPosition(midiNote), noteWidth, noteHeight);
    };


    for (const auto& message: _mid.at(_currentTrack))
    {
        const auto pos = ( message.absoluteTime() / 50.0 ) * _xZoomCoef;
        const auto midiNote = message.getParameter1();

        if (message.getEventType() == MidiEvent::NoteOn)
        {
            if (ray[midiNote] != -1.0)
                paintNote(pos, midiNote);

            ray[midiNote] = pos;
        }
        if (message.getEventType() == MidiEvent::NoteOff)
        {
            if (ray[midiNote] != -1.0) {
                paintNote(pos, midiNote);
                ray[midiNote] = -1.0;
            }
        }
    }

}
