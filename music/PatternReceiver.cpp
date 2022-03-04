#include "PatternReceiver.hpp"

#include <QPainter>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

#include <cmath>
#include <numeric>

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
    }
}


std::vector<int> PatternReceiver::calculateRepeatTimes()
{
    std::vector<double> fullSizes;
    double maxSize = 0.0;

    int lesserCommon = 1;

    for (auto& line: _lines)
    {
        int full = 32 * line->getNum() / line->getDenom();
        if (full > maxSize)
            maxSize = full;

        lesserCommon = std::lcm(lesserCommon, full);
        fullSizes.push_back(full);
    }

    std::vector<int> repeatTimesLine;
    for (auto singleSize: fullSizes)
        repeatTimesLine.push_back(lesserCommon / singleSize);

    return repeatTimesLine;
}


MappedMidiSignals PatternReceiver::calculateMappedSignals()
{
    std::vector<int> repeatTimesLine = calculateRepeatTimes();
    MappedMidiSignals midiMap;
    size_t lineIdx = 0;

    for (auto& line: _lines)
    {
        const auto& bricks = line->getBricks();
        const int brickSize = line->getBrickSize();
        const uint8_t midiNote = line->getMidiNote();
        const double barSize = static_cast<double>(line->getNum()) / line->getDenom();
        const int polyRepeats = repeatTimesLine[lineIdx];

        for (int t = 0; t < polyRepeats * _repeatTimes; ++t) //Возможно здесь удастся сделать умножение на _repeatTimes
        {
            const unsigned long timesShift = t * 480 * 4 * barSize;

            bool borderReached = false;
            for (size_t i = 0; i < bricks.size(); ++i)
            {
                unsigned long timeSize = 480 * 4 / brickSize;
                unsigned long start = timesShift + timeSize * i;
                unsigned long finish = timesShift + timeSize * (i + 1);

                if (bricks[i].on == true)
                {
                    if (midiMap.count(start) == 0)
                        midiMap[start] = {};
                    if (midiMap.count(finish) == 0)
                        midiMap[finish] = {};

                    midiMap[start].push_back({true, midiNote});
                    midiMap[finish].push_back({false, midiNote});
                }

                double totalSeconds = finish * (120.0 / _bpm) / 960.0;
                if (totalSeconds > _secondsLimit) {
                    borderReached = true;
                    break;
                }
            }
            if (borderReached)
                break;
        }
        ++lineIdx;
    }

    return midiMap;
}



double PatternReceiver::totalSeconds()
{
    if (_lines.empty())
        return 0.0;

    auto reapeatTimes = calculateRepeatTimes();
    const int polyRepeats = reapeatTimes[0];
    auto line = _lines[0];
    const int brickSize = line->getBrickSize();
    const double barSize = static_cast<double>(line->getNum()) / line->getDenom();
    const unsigned long timesShift = (polyRepeats * _repeatTimes - 1) * 480 * 4 * barSize;
    unsigned long timeSize = 480 * 4 / brickSize;
    unsigned long finish = timesShift + timeSize * line->getBricks().size();
    double totalSeconds = finish * (120.0 / _bpm) / 960.0;

    return totalSeconds;
}


void PatternReceiver::generateMidi(QString filename)
{
    MidiTrack track;

    track.pushChangeBPM(_bpm, 0);
    track.pushChangeInstrument(0, 9);

    MappedMidiSignals midiMap = calculateMappedSignals();

    unsigned long prevTime = 0;
    for (const auto& events: midiMap) //Общий код с piano roll - refactoring
    {
        unsigned long currentTime = events.first;
        for (auto& event: events.second)
        {
            if (prevTime != currentTime) { //Можно считать TotalTime тут
                track.accumulate(currentTime - prevTime);
                prevTime = currentTime;
            }

            if (event.on)
                track.pushNoteOn(event.note, 127, 9);
            else
                track.pushNoteOff(event.note, 127, 9);
        }
    }

    track.pushEvent47();

    MidiFile midi;
    midi.push_back(track);
    midi.writeToFile(filename.toStdString());
}


void PatternReceiver::storeState() {
    _states.clear();

    for (auto line: _lines)
        _states.push_back(line->getState());
}


void PatternReceiver::saveToFile(QString filename)
{
    QJsonArray arr;
    for (auto line: _lines)
        arr.push_back(line->getState().serialize());

    QJsonObject root;
    root["lines"] = arr;

    QJsonDocument doc(root);
    auto bytes = doc.toJson();

    QFile output(filename);
    output.open(QIODevice::WriteOnly);
    output.write(bytes);
}


void PatternReceiver::loadState()
{
    for (size_t i = 0; i < _states.size(); ++i)
        if (i < _lines.size()) {
            _lines[i]->setState(_states[i]);
            _lines[i]->update();
        }
}


void PatternReceiver::loadFromFile(QString filename)
{
    //Step 1 load state
    //Step 2 set model repeater
    //Step 3 load state

    QFile input(filename);
    input.open(QIODevice::ReadOnly);
    auto bytes = input.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    auto mainObject = doc.object();
    auto arr = mainObject["lines"].toArray();

    _states.clear();
    for (auto line: arr) {
        auto lineObj = line.toObject();
        PatternLineState state;
        state.deserialize(lineObj);
        _states.push_back(state);
    }
}
