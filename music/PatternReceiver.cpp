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


    for (auto& currentLine: _lines)
    {
        const auto& bricks = currentLine->getBricks();

        for (size_t i = 0; i < bricks.size(); ++i)
        {

            if (bricks[i].on == true) {
                unsigned long start = 240 * (i + 2); //TODO this is bad, some issue playing
                unsigned long finish = 240 * (i + 3); //Maybe issue in very short file

                if (midiMap.count(start) == 0)
                    midiMap[start] = {};
                if (midiMap.count(finish) == 0)
                    midiMap[finish] = {};

                midiMap[start].push_back({true, 36});
                midiMap[finish].push_back({false, 36});

            }
        }

        //1. Добавить обработку единственной линии, получение размеров и череды кирпичиков

        //2. Добавить обработку всех линий - рассчёт разных размеров

        //3. Добавить предварительный рассчёт полиритмии
            //:Для каждой дорожки прокрутить столько циклов, сколько требуется

    }



    unsigned long prevTime = 0;
    for (const auto& events: midiMap)
    {
        unsigned long currentTime = events.first; //Rewrite structures binding

        for (auto& event: events.second) {

            if (prevTime != currentTime) {
                track.accumulate(currentTime - prevTime);
                prevTime = currentTime;
                //Можно считать TotalTime тут
            }

            if (event.on)
                track.pushNoteOn(36, 127, 9);
            else
                track.pushNoteOff(36, 127, 9);
        }

    }

    track.accumulate(240); //Hot fix for midi generation swallowed note
    track.pushNoteOff(36, 127, 9);
    track.pushEvent47();

    MidiFile midi;
    midi.push_back(track);
    midi.writeToFile(filename.toStdString());
}
