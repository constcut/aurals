#include "Tapper.h"

#include <QDebug>

#include "midi/MidiFile.hpp"

using namespace mtherapp;
using namespace std;


void Tapper::pressed(int idx) {
    _events.push_back(TapEvent{idx, true, std::chrono::steady_clock::now()});
}


void Tapper::released(int idx) {
    _events.push_back(TapEvent{idx, false, std::chrono::steady_clock::now()});
}


void Tapper::reset() {
    _events.clear();
}


void Tapper::saveAsMidi(QString filename) {
    //Для начала мы рассмотрим самый простой случай, одной единственной ноты
    //Тогда нам не нужно делать анализ какая нота начала звучать, а какая закончила
    if (_events.size() % 2 != 0)
        qDebug() << "ERROR! Not all tap events where closed!!!";

    std::vector<long> msIntervals;
    moment prevMoment;
    for (size_t i = 0; i < _events.size() / 2; ++i) {
        long fromPrev = 0;
        auto startMoment = _events[i * 2].time;
        if (i != 0)
            fromPrev = chrono::duration_cast<chrono::microseconds>(prevMoment - startMoment).count();
        msIntervals.push_back(fromPrev);
        auto endMoment = _events[i * 2 + 1].time;
        auto durationMs = chrono::duration_cast<chrono::microseconds>(endMoment - startMoment).count();
        msIntervals.push_back(durationMs);
        prevMoment = endMoment;
    }

    MidiTrack track;

    track.pushChangeInstrument(0, 0, 0);
    track.pushChangePanoram(64, 0);
    track.pushChangeVolume(64, 0);

    for (size_t i = 0; i < msIntervals.size(); ++i) {
        if (i % 2 == 0) {
            //In 120 bpm 960 = 500 ms, so for now just stupid * 2
            long fromPrevNote = msIntervals[i]; //TODO wrong
            fromPrevNote = 500;

            qDebug() << "From prev note " << fromPrevNote;

            track.accumulate(fromPrevNote * 2);
            track.pushNoteOn(60, 100, 0);
        }
        else {
            long noteDuration = msIntervals[i];
            noteDuration = 960;
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
