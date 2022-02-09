#include "Track.hpp"

#include <iostream>
#include <QDebug>

bool trackLog = false;


using namespace aurals;



Track::Track() :  _pan(0), _drums(false), _status(0), _timeLoop(),
    _cursor(0),_cursorBeat(0),_stringCursor(0), _displayIndex(0),
    _lastSeen(0),_selectCursor(-1), _digitPress(-1)
{
    _midiInfo[3]=24;
    _selectionBarFirst=-1;
    _selectionBarLast=-1;
    _selectionBeatFirst=-1;
    _selectionBeatLast=-1;
}


Track& Track::operator=([[maybe_unused]]Track another) {
    //clone(another); //refactoring remove (Может быть нужно только для копирования?)
    return *this;
}


void Track::push_back(std::unique_ptr<Bar> val) {
    if (val){
        val->setParent(this);
        ChainContainer<Bar, Tab>::push_back(std::move(val));
    }
}


void Track::insertBefore(std::unique_ptr<Bar> val, int index){
    if (val){
        val->setParent(this);
        ChainContainer<Bar, Tab>::insertBefore(std::move(val),index);
    }
}


void Track::printToStream(std::ostream& stream) const {
    stream << "Outputing #" << size() << " Bars."<< std::endl;
    for (size_t ind = 0; ind < size(); ++ind)
            at(ind)->printToStream(stream);
}


void Track::connectAll()
{
    if (trackLog)
        qDebug() <<"Connection initiated for track "<<_name.c_str();
    connectBars();
    connectBeats();
    connectNotes();
    connectTimeLoop();
    if (trackLog)
        qDebug() <<"Connection finished for track "<<_name.c_str();
    return;
}



size_t Track::connectNotes() //for let ring
{
    Bar* curBar;
    Beat *curBeat;
    size_t index = 0;

    if (size() == 0)
       return 0;

    curBar = at(0).get();
    if (curBar->size())
    curBeat = curBar->at(0).get();
    else {
        while (curBar && curBar->size()==0)
            curBar = (Bar*)curBar->getNext();

        if (curBar==0)
            return 0;
       curBeat = curBar->at(0).get();
    }

    Note* ringRay[32] = {0};
    size_t indRay[32] = {0};
    size_t notesCount[32] = {0};
    size_t count = 0;

    while (index < _beatsAmount) {

       for (size_t noteI=0; noteI < curBeat->size(); ++noteI) {
           Note *curNote = curBeat->at(noteI).get();
           std::uint8_t stringN = curNote->getStringNumber();
           Note *prevNote = ringRay[stringN];
           size_t prevInd = indRay[stringN];
           if (prevNote) {
               prevNote->setNext(curNote);
               curNote->setPrev(prevNote);
           }
           std::uint8_t noteState = curNote->getState();

           if (noteState == 2)
           {
               if (prevNote) {
                   if ((index-prevInd)>1)
                       curNote->setState(3); //dead it
                   else {
                       std::uint8_t prevFret = prevNote->getFret();
                       if (trackLog)
                       qDebug() << "Prev found "<<prevNote->getStringNumber()<<
                              " "<<prevFret;
                       prevNote->signStateLeeged();

                       ABitArray prevEff = prevNote->getEffects();
                       curNote->addEffects(prevEff);
                       curNote->setFret(prevFret);
                   }
               }
               else
                   curNote->setState(3); //dead it Перепроверить что этот else не к другой ветке TODO
            }

           std::uint8_t newNoteState = curNote->getState();
           std::uint8_t nowFret = curNote->getFret();
           if (nowFret == 63)
               curNote->setState(3); //dead it

           if (trackLog)
               qDebug() <<stringN<<"ZFret "<<nowFret<<"; newState= "<<newNoteState<<"; oldS= "<<noteState;

           ringRay[stringN] = curNote;
           indRay[stringN] = index;
           ++count;
           ++notesCount[stringN];
       }

       ++index;
       curBeat = (Beat*)curBeat->getNext(); // like ++;
       if (curBeat == 0)
           break; //attention could miss the errors
    }

    if (trackLog)
        qDebug() << "Connect notes done "<<count<<"; S1="<<notesCount[1]<<
              "; S2="<<notesCount[2]<<"; S3="<<notesCount[3]<<"; S4="<<notesCount[4]<<
              "; S5="<<notesCount[5]<<"; S6="<<notesCount[6];

  return count;
}


size_t Track::connectBeats()
{
    Bar* curBar;
    Beat* curBeat;

    if (size() == 0)
       return 0;

    curBar = at(0).get();
    size_t trackLen = size();
    size_t fullCount=0;

    if (trackLog)
       qDebug() <<"ConnectingBeats";

    for (size_t barI = 0; barI < trackLen; ++barI)
    {
       curBar = at(barI).get();

       if (curBar->size())
           for (size_t beatI = 0; beatI < (curBar->size() - 1); ++beatI)
           {
               curBeat = curBar->at(beatI).get();
               Beat *nextBeat = curBar->at(beatI + 1).get();
               if ((nextBeat)&&(curBeat)) {
                   nextBeat->setPrev(curBeat);
                   curBeat->setNext(nextBeat);
                   ++fullCount;
               }
               else
                   qDebug() <<"Issue connecting beats";
           }

       if (barI + 1 != trackLen)
       { //not the lastBar
           Bar *nextBar = at(barI + 1).get();
           if (nextBar->size()==0)
               continue;
           curBeat = 0;

           if (curBar->size() == 0) {
               //Bar *prevBar = curBar;
           }
           else
               curBeat = curBar->at(curBar->size()-1).get();

           Beat *nextBeat = nextBar->at(0).get();
           if ((nextBeat)&&(curBeat))
           {
               nextBeat->setPrev(curBeat);
               curBeat->setNext(nextBeat);
               ++fullCount;
           }
           else
               qDebug() <<"Issue connecting beatz";
       }
    }

    if (trackLog)
        qDebug() << "Full count of the beats is "<<fullCount;
    _beatsAmount = fullCount;

    return fullCount;
}



size_t Track::connectBars()
{
    if (trackLog)
        qDebug() <<"ConnectingBars "<<(int)size() ;

    if (size() == 0)
        return 0;

    std::uint8_t currentNum = at(0)->getSignNum();
    std::uint8_t currentDen = at(0)->getSignDenum();

    size_t trackLen = size();

    for(size_t barsI=1; barsI < trackLen; ++barsI)
    {
        at(barsI)->setPrev(at(barsI-1).get());
        at(barsI-1)->setNext(at(barsI).get());
        std::uint8_t thatNum = at(barsI)->getSignNum();
        std::uint8_t thatDen = at(barsI)->getSignDenum();

        if (thatNum==0)
            at(barsI)->setSignNum(currentNum);
        else
            currentNum = thatNum;

        if (thatDen==0)
            at(barsI)->setSignDenum(currentDen);
        else
            currentDen = thatDen;
    }
    return trackLen;
}


//Возможно стоит упростить логику, или даже избавиться от альтернативных концовок
size_t Track::connectTimeLoop()
{
    if (size() == 0)
       return 0;

    _timeLoop.clear();
    _timeLoopIndexStore.clear();

    size_t lastIndex = size();
    size_t curIndex = 0;

    Bar* curBar = at(0).get();
    Bar* lastBeginRepeat = curBar;
    Bar* beginRepeat = 0;
    Bar* endRepeat = 0;

    size_t beginIndex = 0;
    size_t endIndex = 0;

    Bar* tailEnd = 0;
    Bar* tailBegin = 0;
    size_t tailEndIndex = 0;
    size_t tailBeginIndex = 0;

    if (trackLog)
       qDebug() << "Start connecting time-loop ";

    while (curIndex < lastIndex)  {

       if (curBar==0) //minifix attention
           break;

       if (curBar->getRepeat() & 1) {

           if (beginRepeat)
               while (beginRepeat != curBar)
               {
                   _timeLoop.push_back(beginRepeat);
                   _timeLoopIndexStore.push_back(beginIndex);
                   ++beginIndex;
                   beginRepeat = (Bar*)beginRepeat->getNext();
               }

           beginRepeat = curBar;
           beginIndex = curIndex;


           if (curBar->getRepeat() & 2)  //one bar reprize TODO refact optimizae that code, its plain like below
           {
               endIndex = curIndex;
               endRepeat = curBar;

               pushReprise(beginRepeat, endRepeat, nullptr,
                           nullptr, nullptr, beginIndex, endIndex);

               beginRepeat = endRepeat = 0;
               curBar = (Bar*)curBar->getNext();
               ++curIndex;
               continue;
           }
       }

       if (curBar->getRepeat() & 2)
       {


           endRepeat = curBar;
           endIndex = curIndex;

           //then search for alternative tail
           curBar = (Bar*)curBar->getNext();
           ++curIndex;

           if (curBar)
           {
               if (curBar->getAltRepeat() != 0) {
                  tailBegin = curBar;
                  //qDebug() <<"Tail begin set to "<<(int)tailBegin;
                  tailEnd = curBar;
                  tailBeginIndex = curIndex;
               }

               if ((curBar->getRepeat() & 1) == 0)
               {
                   while( curBar->getAltRepeat() != 0 ) //there is alternative
                   {
                       tailEnd = curBar;
                       tailEndIndex = curIndex;
                       curBar = (Bar*)curBar->getNext();
                       ++curIndex;
                       if (curIndex>=lastIndex)
                           break;
                   }
               }
               //else //alt is a start of a ne w repeat
           }

           if (beginRepeat == 0)
               beginRepeat = lastBeginRepeat;

           Bar* preTail = 0;
           size_t preTailIndex = 0;

           int indX=0;
           for (Bar* barI = endRepeat; barI != beginRepeat; barI= dynamic_cast<Bar*>(barI->getPrev()), ++indX)
               if (barI->getAltRepeat() != 0) {
                   preTail = barI;
                   preTailIndex = endIndex-indX;
               }

           pushReprise(beginRepeat,endRepeat,
                       preTail,tailBegin,tailEnd, beginIndex, endIndex,
                       preTailIndex,tailBeginIndex,tailEndIndex);

           lastBeginRepeat = beginRepeat;
           beginRepeat = endRepeat = 0;
           tailBegin = tailEnd = 0;
           preTail = 0;
       }

       if (curBar)
       {

           if (curBar->getRepeat() & 1) {
               beginRepeat = curBar;
               beginIndex = curIndex;
           }

           if (curIndex < lastIndex)
           {
               if (beginRepeat == 0) {
                   _timeLoop.push_back(curBar);
                   _timeLoopIndexStore.push_back(curIndex);
               }
               else if (curBar->getRepeat() == 3) {

                   pushReprise(curBar, curBar, nullptr,
                               nullptr, nullptr, curIndex, curIndex);

                   endIndex = curIndex;
                   beginRepeat = endRepeat = 0;
               }
           }
           curBar = (Bar*)curBar->getNext();
           ++curIndex;
       }
    }

    if (trackLog)
        qDebug() << "TIME LOOP size is "<<(int)_timeLoop.size();

    return _timeLoop.size();
}


//REFACT - cover under Track operations
typedef std::map<std::uint8_t, ChainedBars> AltRay;
typedef std::map<std::uint8_t, std::vector<int> > AltRayInd;

void createAltRay(AltRay &altRay, AltRayInd &altRayInd, Bar *a, Bar *b, size_t indA, size_t indB)
{

    std::uint8_t currentAlt = 0;

    size_t localInd = 0;
    for (Bar *barI = a; barI != b; barI = dynamic_cast<Bar*>(barI->getNext()), ++localInd)
    {
       if (trackLog)
        qDebug() << "ALTRAY Bar ptr "<<barI;
       if (barI->getAltRepeat() != 0)
           currentAlt = barI->getAltRepeat();

       for (std::uint8_t i=0; i < 8; ++i) {
           std::uint8_t altMaskI = currentAlt & (1 << i);
           if (altMaskI) {
               altRay[i].push_back(barI);
               altRayInd[i].push_back(localInd+indA);
           }
       }
    }

    currentAlt = b->getAltRepeat();
    for (std::uint8_t i = 0; i < 8; ++i) {
        std::uint8_t altMaskI = currentAlt & (1 << i);
        if (altMaskI) {
               altRay[i].push_back(b);
               altRayInd[i].push_back(indB);
        }
    }

}


void Track::pushReprise(Bar *beginRepeat, Bar *endRepeat,
                 Bar *preTail, Bar *tailBegin, Bar *tailEnd, size_t beginIndex, size_t endIndex,
                        size_t preTailIndex, size_t tailBeginIndex, size_t tailEndIndex)
{

    AltRay altRay; //Maybew drop this hell it works really bad:)
    AltRayInd altRayInd;

    if (preTail == nullptr)
    { //no alt ending in begin-end
        if (tailBegin == nullptr)
        { //no alt at all

            for (std::uint8_t i = 0; i < endRepeat->getRepeatTimes(); ++i)
            {
                size_t localIndex = 0;
                for (Bar *barI = beginRepeat; barI != endRepeat; barI=(Bar*)barI->getNext())
                {
                    _timeLoop.push_back(barI);
                    _timeLoopIndexStore.push_back(beginIndex + localIndex);
                    ++localIndex;
                }
                _timeLoop.push_back(endRepeat);
                _timeLoopIndexStore.push_back(endIndex);
            }
        }
        else
        {
           createAltRay(altRay, altRayInd, tailBegin,tailEnd,tailBeginIndex,tailEndIndex);
           //there is a tail after
           for (std::uint8_t i = 0; i < endRepeat->getRepeatTimes(); ++i)
           {
               size_t localIndex = 0;
               for (Bar *barI=beginRepeat; barI != endRepeat; barI=(Bar*)barI->getNext()) {
                   _timeLoop.push_back(barI);
                   _timeLoopIndexStore.push_back(beginIndex+localIndex);
                   ++localIndex;
               }

               if (altRay.find(i) != altRay.end())
               { //add alt ray from a tail
                   ChainedBars *thatEnd = &altRay[i];
                   std::vector<int> thatRayInd = altRayInd[i];

                   if ((thatEnd->at(0)->getRepeat() & 1)==0)
                   { //don't repeat alts that begin
                       for (size_t j = 0; j < thatEnd->size(); ++j) {
                           _timeLoop.push_back(thatEnd->at(j));
                           _timeLoopIndexStore.push_back(thatRayInd[j]);
                       }
                   }
               }
               else  {
                   _timeLoop.push_back(endRepeat);
                   _timeLoopIndexStore.push_back(endIndex);
               }
           }
        }
    }
    else
    {
       if (tailBegin == 0)
           createAltRay(altRay, altRayInd, preTail,endRepeat,preTailIndex,endIndex);
       else
           createAltRay(altRay, altRayInd, preTail,tailEnd,preTailIndex,tailEndIndex);

       for (std::uint8_t i = 0; i < endRepeat->getRepeatTimes(); ++i)
       {
           size_t localIndex = 0;
           for (Bar *barI=beginRepeat; barI != preTail; barI=(Bar*)barI->getNext()) {
               _timeLoop.push_back(barI);
               _timeLoopIndexStore.push_back(beginIndex+localIndex);
               ++localIndex;
           }

           if (altRay.find(i) != altRay.end()) { //add alt ray from a tail
               ChainedBars *thatEnd = &altRay[i];
               std::vector<int> thatRayInd = altRayInd[i];

               for (size_t j = 0; j < thatEnd->size(); ++j) {
                   _timeLoop.push_back(thatEnd->at(j));
                   _timeLoopIndexStore.push_back(thatRayInd[j]);
               }
           }
           else  {
               _timeLoop.push_back(endRepeat);
               _timeLoopIndexStore.push_back(endIndex);
               //POSSIBLE THERE would be ISSUE - hard test it
               if (trackLog)
               qDebug() << "Tail begin=0, pre tail=1?; attention";
           }
       }
    }
}



