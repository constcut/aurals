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
    _noteCursor = -1;
    _notes.clear();
    _xZoomCoef = 1.0;
    update();
}


void PianoRoll::onMoveVertical(int newY) {

    if (_noteCursor < 0)
        return;

    _notes[_noteCursor].y = newY;
    update();
}


Q_INVOKABLE void PianoRoll::ondblclick(int x, int y)
{
    for (size_t i = 0; i < _notes.size(); ++i)
        if (x >= _notes[i].x && y >= _notes[i].y)
        {
            int xDiff = x - _notes[i].x;
            int yDiff = y - _notes[i].y;

            if (xDiff <= _notes[i].w &&
                yDiff <= _notes[i].h)
            {
                _noteCursor = i;
                update();
                break;
            }
        }
}


void PianoRoll::paint(QPainter* painter) {

    if (_mid.empty())
        return;

    int minMidi = 128; //Вынести на рефакторинге
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
           const int localNote = maxMidi - midiNote;
           return localNote * noteHeight;
        }
    };


    if (_notes.empty()) {

        std::vector<double> ray(128, -1.0);

        auto addNote = [&](auto pos, auto midiNote) {
            const int noteWidth = pos - ray[midiNote];
            _notes.push_back({static_cast<int>(ray[midiNote]), midiNoteToPosition(midiNote), noteWidth, noteHeight, midiNote});
        };


        for (const auto& message: _mid.at(_currentTrack))
        {
            const auto pos = ( message.absoluteTime() / 50.0 ) * _xZoomCoef;
            const auto midiNote = message.getParameter1();

            if (message.getEventType() == MidiEvent::NoteOn)
            {
                if (ray[midiNote] != -1.0)
                    addNote(pos, midiNote);

                ray[midiNote] = pos;
            }
            if (message.getEventType() == MidiEvent::NoteOff)
            {
                if (ray[midiNote] != -1.0) {
                    addNote(pos, midiNote);
                    ray[midiNote] = -1.0;
                }
            }
        }
    }

    //Позже внимательней изучить случаи перерисовки, всё что выше можно перенести из отрисовки в загрузку

    painter->setPen(QColor("lightgray"));
    for (int i = minMidi - 1; i < maxMidi + 1; ++i)
        painter->drawLine(0, midiNoteToPosition(i), width(), midiNoteToPosition(i));


    int i = 0;
    for (const auto& note: _notes) {

        QString fillColor = "green";
        if (i == _noteCursor)
            fillColor = "blue";

        painter->fillRect(note.x, note.y, note.w, note.h, fillColor);

        painter->setPen(QColor("lightgreen"));
        painter->drawRect(note.x, note.y, note.w, note.h);
        ++i;
    }

}
