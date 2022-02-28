#include "PianoRoll.hpp"
#include <QPainter>
#include <QDebug>

using namespace aurals;


void PianoRoll::loadMidi(QString filename)
{
    _mid.readFromFile(filename.toStdString());

    if (_mid.empty() == false)
    {
        for (auto& event: _mid[0]) //Если не найдено - искать в других
            if (event.isMetaEvent() && event.getParameter1() == MidiMetaTypes::ChangeTempo) {
                auto& buf = event.metaBufer();
                uint32_t nanoCount = (buf[0] << 16) + (buf[1] << 8) + buf[2];

                _bpm = 60000000 / nanoCount;
            }
    }


    update();
}



void PianoRoll::setCurrentTrack(int newIdx)
{
    if (_notes.empty() == false) {
        auto track = makeCurrentTrack();
        _mid[_currentTrack] = track;
    }

    _currentTrack = newIdx;
    _notes.clear();
    _noteCursor = -1;

    for (const auto& event: _mid[_currentTrack])
        if (event.getEventType() == MidiEvent::PatchChange) {
            _currentInstrument = event.getParameter1();
            break;
        }

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
    _bpm = 120;
    _xZoomCoef = 1.0;
    update();
}


void PianoRoll::onMoveVertical(int newY) {

    if (_noteCursor < 0)
        return;

    newY /= noteHeight();
    newY *= noteHeight();

    _notes[_noteCursor].midiNote = positionToMidiNote(newY);
    _notes[_noteCursor].y = newY;
    update();
}


void PianoRoll::onMoveHorizontal(int newX) {
    if (_noteCursor < 0)
        return;

    _notes[_noteCursor].x = newX; //TODO сетка
    update();
}


Q_INVOKABLE void PianoRoll::onDoubleClick(int x, int y)
{
    bool found = false;

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
                found = true;
                break;
            }
        }

    if (found == false) {
        y /= noteHeight();
        y *= noteHeight();
        uint8_t mid = positionToMidiNote(y);
        _notes.push_back({x, y, static_cast<int>(5 * _xZoomCoef),
                          noteHeight(), mid});
        update();
    }
}


void PianoRoll::onPressAndHold(int x, int y)
{
    if (_noteCursor < 0)
        return;

    bool needRemove = false;

    for (size_t i = 0; i < _notes.size(); ++i)
        if (x >= _notes[i].x && y >= _notes[i].y)
        {
            int xDiff = x - _notes[i].x;
            int yDiff = y - _notes[i].y;

            if (xDiff <= _notes[i].w &&
                yDiff <= _notes[i].h)
            {
                if (static_cast<int>(i) == _noteCursor)
                needRemove = true;
                break;
            }
        }

    if (needRemove) {
        _notes.erase(_notes.begin() + _noteCursor);
        _noteCursor = -1;
        update();
    }
}


void PianoRoll::findMinMaxMidi() {

    if (_mid.empty())
        return;

    _minMidi = 128; //Вынести на рефакторинге + всё что ниже
    _maxMidi = 0;

    for (const auto& message: _mid.at(_currentTrack)) {
        if (message.getEventType() == MidiEvent::NoteOn) {
            const auto midiNote = message.getParameter1();
            if (_maxMidi < midiNote)
                _maxMidi = midiNote;
            if (_minMidi > midiNote)
                _minMidi = midiNote;
        }
    }

}


int PianoRoll::noteHeight()
{
    if (_fillHeight == false)
        return 5;

    const int dist = _maxMidi - _minMidi;
    return static_cast<int>(height() / dist);
}


int PianoRoll::midiNoteToPosition(int midiNote) {
    if (_fillHeight == false)
        return (127 - midiNote) * noteHeight();

   const int localNote = _maxMidi - midiNote;
   return localNote * noteHeight();
}


int PianoRoll::positionToMidiNote(int pos) {

    const int temp = pos / noteHeight();

    if (_fillHeight == false)
       return 127 - temp;

    return _maxMidi - temp;
}


void PianoRoll::fillNotes()
{

    std::vector<double> ray(128, -1.0);

    auto addNote = [&](auto pos, auto midiNote) {
        const int noteWidth = pos - ray[midiNote];
        _notes.push_back({static_cast<int>(ray[midiNote]),
               midiNoteToPosition(midiNote), noteWidth, noteHeight(), midiNote});
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


void PianoRoll::paint(QPainter* painter) {

    if (_notes.empty()) {
        findMinMaxMidi();
        fillNotes();
    }

    painter->setPen(QColor("lightgray"));
    for (int i = _minMidi - 1; i < _maxMidi + 1; ++i)
        painter->drawLine(0, midiNoteToPosition(i),
                          width(), midiNoteToPosition(i));


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


MidiTrack PianoRoll::makeCurrentTrack()
{

    struct MiniMidi {
        uint8_t typeAndChannel;
        //uint8_t channel;
        //Skipped velocity yet
        uint8_t param; //midi note
    };

    std::map<unsigned long, std::vector<MiniMidi>> midiMap;

    uint8_t trackId = _currentTrack;
    if (trackId)
        --trackId; //Tab symmetry generation

    MidiTrack newTrack;
    //newTrack.pushChangeBPM(_bpm, 0);
    newTrack.pushChangeInstrument(_currentInstrument, trackId);

    for (const auto& note: _notes)
    {
        unsigned long start = note.x * 50 / _xZoomCoef;
        unsigned long finish = note.w * 50 / _xZoomCoef + start;

        start *= 480;
        start /= 500; //Realtime to local

        finish *= 480;
        finish /= 500;

        if (midiMap.count(start) == 0)
            midiMap[start] = {};

        if (midiMap.count(finish) == 0)
            midiMap[finish] = {};

        midiMap[start].push_back({static_cast<uint8_t>(0x90), note.midiNote});
        midiMap[finish].push_back({static_cast<uint8_t>(0x80), note.midiNote});

        //y = midi note (yet can use inner field later update)
    }

    double totalTime = 0.0;

    unsigned long prevTime = 0;
    for (const auto& events: midiMap)
    {
        unsigned long currentTime = events.first; //Rewrite structures binding

        for (auto& event: events.second) {

            if (prevTime != currentTime) {
                newTrack.accumulate(currentTime - prevTime);
                prevTime = currentTime;
                //Можно считать TotalTime тут
            }

            if (event.typeAndChannel == 0x90) {
                newTrack.pushNoteOn(event.param, 127, trackId);
                totalTime += newTrack.back().getSecondsLength(_bpm);
                newTrack.back().setAbsoluteTime(totalTime * 1000);
            }

            if (event.typeAndChannel == 0x80) {
                newTrack.pushNoteOff(event.param, 127, trackId);
                totalTime += newTrack.back().getSecondsLength(_bpm);
                newTrack.back().setAbsoluteTime(totalTime * 1000);
            }
        }

    }

    newTrack.pushEvent47();
    return newTrack;
}


void PianoRoll::saveAs(QString filename) {
    MidiFile m = _mid;

    bool wereEmpty = false;
    if (_notes.empty()) { //Yet only for fast debug
        findMinMaxMidi();
        fillNotes();
        wereEmpty = true;
    }

    m[_currentTrack] = makeCurrentTrack();
    m.writeToFile(filename.toStdString());

    if (wereEmpty)
        _notes.clear();
}
