#include "Tab.hpp"

#include <algorithm>
#include <map>
#include <iostream>
#include <QDebug>



bool tabLog = false;

using namespace aurals;


void Tab::connectTracks(){
    for (size_t i = 0; i < size(); ++i)
    at(i)->connectAll();
    createTimeLine();
}


void Tab::postLoading() {
    for (size_t i = 0; i < size(); ++i) {
        size_t port = at(i)->getMidiInfo(0);
        size_t chan = at(i)->getMidiInfo(1);
        size_t ind = (chan-1) + (port-1)*16;
        if (ind < 70) {
            int instr = _midiChannels[ind].instrument;
            std::uint8_t pan = _midiChannels[ind].balance;
            std::uint8_t vol = _midiChannels[ind].volume;
            Track *t = at(i).get();
            t->setInstrument(instr);
            t->setPan(pan);
            t->setVolume(vol);
        }
     }
}


Tab &Tab::operator=([[maybe_unused]]Tab another) {
    //Скорее всего просто убить на рефакторинге / refactoring
    return *this;
}


void Tab::printToStream(std::ostream &stream) const {
    stream << "Outputing #" << size() << " Tracks." << std::endl;
    for (size_t ind = 0; ind < size(); ++ind)
            at(ind)->printToStream(stream);
}



std::uint8_t Tab::getBPMStatusOnBar(size_t barN) const {
    for (size_t i = 0 ; i < size(); ++i) {
        auto& bar = at(i)->at(barN);
        if (bar->size())
            if (bar->at(0)->getChangesPtr()->size())
                ;//bar->getV(0)->changes.getV(0)->changeType;
    }
    return 0;
}


int Tab::getBpmOnBar([[maybe_unused]] size_t barN) const {
    [[maybe_unused]] int bpmTrace = _bpmTemp;
    for (size_t i = 0; i < _timeLine.size(); ++i){
        //TODO + выше, использовать в рассчёте анимации не с первого такта
    }
    return 0;
}


void Tab::createTimeLine(size_t shiftTheCursor)
{
    _timeLine.clear();
    TimeLineKnot initKnot(1, getBPM());
    _timeLine.push_back(initKnot);
    int lastNumDen = 0;

    size_t barsAmount = at(0)->getTimeLoop().size();
    for (size_t barsI = shiftTheCursor; barsI < barsAmount; ++barsI)
    {
        std::vector<BpmChangeKnot> timeChanges;
        for (size_t tracksI = 0; tracksI < size(); ++tracksI)
        {
            short int localAccumulate = 0;
            const auto& timeLoop = at(tracksI)->getTimeLoop();
            if (timeLoop.size() <= barsI) //TODO по идее просто по индексу - всё что не 0 пропускаем
                continue;

            Bar* currentBar = timeLoop[barsI];

            for (size_t beatI = 0; beatI < currentBar->size(); ++beatI)
            {
                if (currentBar->at(beatI)->getEffects().getEffectAt(Effect::Changes)) //changes
                {
                    ChangesList *changes = currentBar->at(beatI)->getChangesPtr();
                    for (size_t indexChange = 0; indexChange != changes->size(); ++indexChange)
                        if (changes->at(indexChange).changeType == 8) {
                            size_t newBPM = changes->at(indexChange).changeValue;
                            BpmChangeKnot newChangeBpm(newBPM, localAccumulate);
                            newChangeBpm.setIdx(barsI, beatI);
                            timeChanges.push_back(newChangeBpm);
                            break;
                        }
                }

                std::uint8_t beatDur = currentBar->at(beatI)->getDuration();
                std::uint8_t durDetail = currentBar->at(beatI)->getDurationDetail();
                std::uint8_t isDotted = currentBar->at(beatI)->getDotted();

                int localAbs = translaeDuration(beatDur);

                if (durDetail)
                    localAbs = updateDurationWithDetail(durDetail,localAbs);

                if (isDotted & 1) {
                    localAbs *= 3;
                    localAbs /= 2;
                }
                localAccumulate += localAbs;
            }
        }

        std::uint8_t thatNum = at(0)->getTimeLoop().at(barsI)->getSignNum();
        std::uint8_t thatDen = at(0)->getTimeLoop().at(barsI)->getSignDenum();

        int packedMeter = 0;
        packedMeter = (thatNum << 8) + thatDen;

        if (packedMeter != lastNumDen) {
            TimeLineKnot changeNumDen(2, packedMeter);
            _timeLine.push_back(changeNumDen);
            lastNumDen = packedMeter;
        }

        int barAbs = translateDenum(thatDen) * thatNum;

        if (timeChanges.empty()) {
            TimeLineKnot noChangeBar(0, barAbs);
            _timeLine.push_back(noChangeBar);
        }
        else
        {
             std::sort(timeChanges.begin(),timeChanges.end());
             short int lastChange = -1;
             for (size_t i = 0; i < timeChanges.size(); ++i)
                if (timeChanges[i].time != lastChange) {
                    int currentChange = timeChanges[i].time - lastChange;
                    lastChange = timeChanges[i].time;
                    TimeLineKnot timeWait(0, currentChange);
                    TimeLineKnot bpmChange(1, timeChanges[i].bpm);
                    bpmChange.setIdx(timeChanges[i].bar, timeChanges[i].beat);
                    _timeLine.push_back(timeWait);
                    _timeLine.push_back(bpmChange);
                }

             if (lastChange < barAbs) {
                 TimeLineKnot timeWait(0, barAbs - lastChange);
                 _timeLine.push_back(timeWait);
             }
        }
        timeChanges.clear();
    }
}






