#include "Tapper.hpp"

#include <QDebug>

#include "midi/MidiFile.hpp"

using namespace aurals;
using namespace std;


void Tapper::pressed(const int idx) {
    _mouseEvents.push_back(MouseEvent{idx, true, std::chrono::high_resolution_clock::now()});
}


void Tapper::released(const int idx) {
    _mouseEvents.push_back(MouseEvent{idx, false, std::chrono::high_resolution_clock::now()});
}


void Tapper::reset() {
    _mouseEvents.clear();
    _tapEvents.clear();
}


void Tapper::saveClicksAsMidi(const QString filename) const {
    //Для начала мы рассмотрим самый простой случай, одной единственной ноты
    //Тогда нам не нужно делать анализ какая нота начала звучать, а какая закончила
    if (_mouseEvents.size() % 2 != 0)
        qDebug() << "ERROR! Not all tap events where closed!!!";

    std::vector<long> msIntervals;
    moment prevMoment;
    for (size_t i = 0; i < _mouseEvents.size() / 2; ++i) {
        long fromPrev = 0;
        auto startMoment = _mouseEvents[i * 2].time;
        if (i != 0)
            fromPrev = chrono::duration_cast<chrono::milliseconds>(startMoment - prevMoment).count();
        msIntervals.push_back(fromPrev);
        auto endMoment = _mouseEvents[i * 2 + 1].time;
        auto durationMs = chrono::duration_cast<chrono::milliseconds>(endMoment - startMoment).count();
        msIntervals.push_back(durationMs);
        prevMoment = endMoment;
    }

    MidiTrack track;
    track.pushChangeBPM(240, 0); //somehow 240 is realtime

    for (size_t i = 0; i < msIntervals.size(); ++i) {
        if (i % 2 == 0) {
            //Возможно стоит использовать коэфициент 500 преобразовывать в 480
            long fromPrevNote = msIntervals[i];
            qDebug() << "From prev note " << fromPrevNote;
            track.accumulate(fromPrevNote * 2);
            track.pushNoteOn(60, 100, 0);
        }
        else {
            long noteDuration = msIntervals[i];
            qDebug() << "Note duration " << noteDuration;
            track.accumulate(noteDuration * 2);
            track.pushNoteOff(60, 100, 0);
        }
    }

    track.pushEvent47();
    MidiFile midi;
    midi.push_back(track);
    midi.writeToFile(filename.toStdString());
}


void Tapper::tapped(const int idx) {
    _tapEvents.push_back(TapEvent{idx, std::chrono::high_resolution_clock::now()});
}


void Tapper::saveTapsAsMidi(const QString filename) const {

    MidiTrack track;
    track.pushChangeBPM(240, 0); //somehow 240 is almost! realtime
    track.pushChangeInstrument(0, 9);
    const double coef = 1.0 / _speedCoef;

    moment prevMoment;
    int prevNote = 0;
    for (size_t i = 0; i < _tapEvents.size(); ++i) {
        long fromPrevTap = 0;

        moment currentMoment = _tapEvents[i].time;
        int currentNote = _tapEvents[i].idx;

        if (i != 0)
            fromPrevTap = chrono::duration_cast<chrono::milliseconds>(currentMoment - prevMoment).count();

        double accurateValue = 2.0 * fromPrevTap * (480.0 / 500.0) * coef;
        if (i != 0) {
            track.accumulate(accurateValue);
            track.pushNoteOff(prevNote, 100, 9);
        }

        track.pushNoteOn(currentNote, 100, 9);
        prevMoment = currentMoment;
        prevNote = currentNote;
    }
    track.accumulate(480 * coef);
    track.pushNoteOff(prevNote, 100, 9);
    track.accumulate(2000 * coef);
    track.pushNoteOff(60, 100, 9); //Pause in the end:_)
    track.pushEvent47();

    MidiFile midi;
    midi.push_back(track);
    midi.writeToFile(filename.toStdString());
}
