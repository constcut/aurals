#include "PatternReceiver.h"

#include <QPainter>
#include <QDebug>

#include "midi/MidiFile.hpp"

using namespace aurals;


void PatternReceiver::paint(QPainter* painter) {

    painter->drawEllipse(0, 0, 100, 100);
    //Концентрические круги
    //Если анимация включена то рассчёт положения на них
}

void PatternReceiver::connectLine(QObject* line) {
    auto patternLine = dynamic_cast<PatternLine*>(line);
    if (patternLine != nullptr) {
        _lines.push_back(patternLine);
        qDebug() << "Connecting pattern line " << patternLine;
    }
}


void PatternReceiver::generateMidi(QString filename) {
    MidiTrack track;

    track.pushChangeBPM(120, 0); //TODO
    track.pushChangeInstrument(0, 9);

    struct MiniMidi { //TODO общий для PianoRoll
        bool on;
        uint8_t note;
    };

    std::map<unsigned long, std::vector<MiniMidi>> midiMap;


    //FIRST TRY ONLY SINGLE LINE
    {
        //TODO build map like in piano roll
        //For each line sepparated
        auto currentLine = _lines[0];

        //1. Добавить обработку единственной линии, получение размеров и череды кирпичиков

        //2. Добавить обработку всех линий - рассчёт разных размеров

        //3. Добавить предварительный рассчёт полиритмии
            //:Для каждой дорожки прокрутить столько циклов, сколько требуется

    }
    track.pushEvent47();

    MidiFile midi;
    midi.push_back(track);
    midi.writeToFile(filename.toStdString());
}
