#include "Tapper.h"

#include <QDebug>

#include "midi/MidiFile.hpp"

using namespace mtherapp;


void Tapper::pressed(int idx) {
    qDebug() << "Tap pressed " << idx;
    _events.push_back(TapEvent{idx, true, std::chrono::steady_clock::now()});
}


void Tapper::released(int idx) {
    qDebug() << "Tap released " << idx;
    _events.push_back(TapEvent{idx, false, std::chrono::steady_clock::now()});
}


void Tapper::reset() {
    qDebug() << "Tap reset";
    _events.clear();
}


void Tapper::saveAsMidi(QString filename) {
    //TODO
}
