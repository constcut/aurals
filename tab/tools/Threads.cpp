#include "Threads.hpp"

#include "tab/Tab.hpp"

#include <QDebug>


using namespace aurals;



void PlayAnimationThr::threadRun()
{
   /*
   int quarter = 60000/bpm;
   int firstTakt = (quarter/2)*21;
   int fullNote = 240000/bpm;
   int waitValue = (fullNote/denum)*num;
   */
   int indexWait = 0;

   _status = 0;

   //while ((*increment +1) < limit)
    for (size_t i = 0 ; i < _waitTimes.size(); ++i)
    {
        if (_pleaseStop) {
           _status = 1;
           break;
        }

        (*_cursorToInc1) = _waitIndexes[indexWait];
        int nowWait = _waitTimes[indexWait];

        if (_cursorToInc2)
        (*_cursorToInc2) = 0;

        if (_pleaseStop) {
            _status = 1;
            break;
        }

        if (_pleaseStop) {
           _status = 1;
           break;
        }

        callUpdate();

        if (_beatTimes.size() > i)
            for(size_t j = 0; j < _beatTimes[i].size(); ++j)
            {
                int beatWait = _beatTimes[i][j];

                nowWait-=beatWait;
                if (nowWait < 0)
                 beatWait += nowWait;

                if (_pleaseStop) {
                   _status = 1;
                   break;
                }

                if (beatWait > 0)
                    sleepThread(beatWait); //Conditional variables

                if (_pleaseStop) {
                   _status = 1;
                   break;
                }

                (*_cursorToInc2) = (*_cursorToInc2) +1;
                callUpdate();
            }
     //check for beats times - for cycle
     //a) sleep each beat
     //b) move beat cursor
     //c) update screen
     //d) decreace nowWait
     //e) escape nowWaite

     if (nowWait>0) {

         if (_pleaseStop) {
            _status = 1;
            break;
         }

         if (_cursorToInc2 != 0) {
            (*_cursorToInc2) = (*_cursorToInc2) - 1;
            callUpdate();
         }
         sleepThread(nowWait);
     }


     //(*increment) + 1; // ++
     //callUpdate(); //emit updateUI();
     ++indexWait;
   }

   _status = 1;
   if (_cursorToInc2 == 0)
       noticeFinished();
}


struct BpmWaitNode
{
    int waitTime;
    int newBpm;

    size_t bar;
    size_t beat;
};


void PlayAnimationThr::setupValues(Tab *tab, Track *track, size_t shiftTheCursor)
{
    size_t timeLoopLen = track->getTimeLoop().size();
    int localWait = 0;
    std::vector<BpmWaitNode> bpmChangeList;

    const auto& timeLine = tab->getTimeLine();
    for (size_t ind = 0 ; ind < timeLine.size(); ++ind) {

       if (timeLine[ind].type == 0) //TODO enumerate
           localWait += timeLine[ind].value;

       if (timeLine[ind].type == 1) {
           BpmWaitNode newNode;

           newNode.newBpm = timeLine[ind].value;
           newNode.waitTime = localWait;
           newNode.bar = timeLine[ind].bar;
           newNode.beat = timeLine[ind].beat;

           localWait = 0;
           bpmChangeList.push_back(newNode);
       }
    }

    size_t changeIndex = 0;

    setBPM(bpmChangeList[changeIndex].newBpm);
    _beatTimes.clear();
    _waitTimes.clear();
    _waitIndexes.clear();

    ++changeIndex;

    int toTheNextWait = bpmChangeList[changeIndex].waitTime;

    std::vector<int> barMoments;

    //TODO возможна запарка если к этому момент BPM менялся от изначального (зависит от поиска BPM в такте)


    for (size_t barI = shiftTheCursor; barI < timeLoopLen; ++barI)
    {
        Bar *bar = track->getTimeLoop().at(barI);
        barMoments.clear();


        for (size_t beatI = 0; beatI < bar->size(); ++beatI)
        {
            auto& beat = bar->at(beatI);

            std::uint8_t dur = beat->getDuration();
            std::uint8_t dot = beat->getDotted();
            std::uint8_t det = beat->getDurationDetail();


            int beatAbs = translaeDuration(dur);

            if (dot == 1) {
                beatAbs *= 3;
                beatAbs /= 2;
            }

            if (det)
                beatAbs = updateDurationWithDetail(det,beatAbs);

            toTheNextWait -= beatAbs;

            if (changeIndex < bpmChangeList.size() &&
                bpmChangeList[changeIndex].bar == barI &&
                bpmChangeList[changeIndex].beat == beatI) // toTheNextWait <= 1
            {
                const auto newBpm = bpmChangeList[changeIndex].newBpm;

                //qDebug() << "Changing bpm: " << newBpm << " " << toTheNextWait <<
                            //" next wait " << bpmChangeList[changeIndex + 1].waitTime;

                if (newBpm != 0)
                    _bpm =  newBpm;

                ++changeIndex;
                toTheNextWait = bpmChangeList[changeIndex].waitTime;
            }

            long int noteTime = 2400000 / _bpm;

            if (dot == 1) {
                noteTime *= 3;
                noteTime /= 2;
            }

            switch (dur)
            {
                case 1: noteTime /= 2; break;
                case 2: noteTime /= 4; break;
                case 3: noteTime /= 8; break;
                case 4: noteTime /= 16; break;
                case 5: noteTime /= 32; break;
                case 6: noteTime /= 64; break;
            }

            if (det)
                noteTime = updateDurationWithDetail(det, noteTime);

            noteTime /= 10;
            barMoments.push_back(noteTime);

            //qDebug() << barI << " got note time " << noteTime;

        }
        _beatTimes.push_back(barMoments);
        addNumDenum(bar->getSignNum(), bar->getSignDenum(), track->getTimeLoopIndexes()[barI]);
    }

    setLimit(track->getTimeLoop().size());
}

void PlayAnimationThr::addNumDenum(std::uint8_t nu, std::uint8_t de, size_t nextIndex)
{
    int fullNote = 240000/_bpm;
    int waitValue = (fullNote/de)*nu;
    _waitTimes.push_back(waitValue);
    _waitIndexes.push_back(nextIndex);

    //log << "Adding n="<<nu<<"; d="<<de<<"; ind="<<nextIndex;
}

double PlayAnimationThr::calculateSeconds()
{
    double totalSumm = 0;

    for (size_t i = 0; i < _waitTimes.size(); ++i)
        totalSumm += _waitTimes[i];

    int seconds = totalSumm/1000.0;
    return seconds;
}


void PlayAnimationThr::addBeatTimes(Bar* bar)
{
    Bar *curBar = (Bar*)bar;

    std::vector<int> barMoments;

    //int toNextChange =

    for (size_t i = 0; i < curBar->size(); ++i)
    {
        auto& beat = curBar->at(i);


        //CHANGE BPM
        if (beat->getEffects().getEffectAt(Effect::Changes))
        {
            {
                ChangesList *changes = beat->getChangesPtr();

                for (size_t indexChange=0; indexChange < changes->size(); ++indexChange)
                if (changes->at(indexChange).changeType==8)
                {
                    size_t newBPM = changes->at(indexChange).changeValue;

                    //CHANGING BPM from mix table!

                    _bpm = newBPM;
                    break;
                }
            }
        }
        //animation

        int noteTime = 2400000/_bpm; //full note

        std::uint8_t dur = beat->getDuration();
        std::uint8_t dot = beat->getDotted();
        std::uint8_t det = beat->getDurationDetail();

        if (dot == 1)
        {
            noteTime *= 3;
            noteTime /= 2;
        }

        switch (dur)
        {
            case 1: noteTime /= 2; break;
            case 2: noteTime /= 4; break;
            case 3: noteTime /= 8; break;
            case 4: noteTime /= 16; break;
            case 5: noteTime /= 32; break;
            case 6: noteTime /= 64; break;
        }

        noteTime = updateDurationWithDetail(det,noteTime);

        noteTime /= 10;

        barMoments.push_back(noteTime);
    }

    _beatTimes.push_back(barMoments);
}
