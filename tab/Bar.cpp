#include "Bar.hpp"

#include <iostream>
#include <QDebug>

using namespace aurals;

bool barLog = false;


int aurals::translateDenum(std::uint8_t den)
{
    switch (den)
    {
       case 1: return 4000;
       case 2: return 2000;
       case 4: return 1000;
       case 8: return 500;
       case 16: return 250;
       case 32: return 125;
    }

    return 0;
}


int aurals::translaeDuration(std::uint8_t dur)
{
    switch (dur)
    {
       case 0: return 4000;
       case 1: return 2000;
       case 2: return 1000;
       case 3: return 500;
       case 4: return 250;
       case 5: return 125;
    }

    return 0;
}


int aurals::updateDurationWithDetail(std::uint8_t detail, int base)
{
    int result = base;

    switch (detail)
    {
       case 3:
       case 6:
       case 12:
           result = base*2.0/3.0;
        break;

       case 5:
       case 10:
           result = base*4.0/5.0;
        break;

       case 7:
           result = base*4.0/7.0;
        break;

       case 9:
           result = base*8.0/9.0;
        break;

       case 11:
           result = base*8.0/11.0;
        break;

       case 13:
           result = base*8.0/13.0;
        break;
    }

    return result;
}



void Bar::clone(Bar *from) {
   flush();

   _signatureDenum = from->_signatureDenum;
   _signatureNum = from->_signatureNum;

   for (size_t i = 0; i < from->size(); ++i)
   {
       Beat *beat = from->at(i).get();
       auto newBeat = std::make_unique<Beat>();
       newBeat->clone(beat);
       push_back(std::move(newBeat));
   }
}

Bar& Bar::operator=(Bar *another) {
    clone(another);
    return *this;
}


size_t Bar::getCompleteIndex() const {
   return _completeIndex;
}


double Bar::getCompleteAbs() const {
    return _completeAbs;
}


//Сохранять кэш, который инвалидировать иногда
std::uint8_t Bar::getCompleteStatus()
{
    std::uint8_t completeStatus = 255; //calculation failed

    std::uint8_t thatNum = getSignNum();
    std::uint8_t thatDen = getSignDenum();

    int barAbs = translateDenum(thatDen)*thatNum;

    int usedAbs = 0;
    int lastAbs = 0;

    _completeIndex = 0;

    size_t barSize = size();

    for (size_t i = 0; i < barSize; ++i)
    {
       std::uint8_t beatDur = at(i)->getDuration();
       std::uint8_t durDetail = at(i)->getDurationDetail();
       std::uint8_t isDotted = at(i)->getDotted();

       int localAbs = translaeDuration(beatDur);

       if (durDetail)
           localAbs = updateDurationWithDetail(durDetail,localAbs);

       if (isDotted&1) {
           //first only one dot
           localAbs *= 3;
           localAbs /= 2;
       }

       usedAbs += localAbs;

       //qDebug() << "On "<<i<<" used = "<<usedAbs;

       if ((i+1) != barSize) //without case of last
           if (usedAbs == barAbs) {
               completeStatus=2; //exceed
               _completeIndex=i;
               _completeAbs=localAbs;
               return completeStatus;
           }

       if (usedAbs > barAbs) {
           completeStatus=2; //exceed
           _completeIndex=i;

           int exceedAbs = usedAbs - barAbs;

           if (barLog)
           qDebug() << "EXCEED abs "<<exceedAbs <<" localAbs "<<localAbs<<" Ind "<<_completeIndex;

           short newNoteAbs = localAbs - exceedAbs;

           if (barLog)
           qDebug() << "New note "<<newNoteAbs;

           if (newNoteAbs > 10)
               lastAbs = newNoteAbs;
           else
               --_completeIndex;

           _completeAbs = lastAbs;

           break;
       }

       lastAbs = localAbs;

    }


   if (usedAbs < barAbs)  {
       _completeIndex=0; //just for logs
       completeStatus=1; //incomplete
       lastAbs=barAbs-usedAbs;

       _completeAbs = lastAbs;

       if (lastAbs < 50)
           completeStatus = 0;
   }

   if (usedAbs == barAbs)
       completeStatus = 0;
   //qDebug() << "Bar abs len "<< barAbs<<"; used "<<usedAbs;
   //qDebug() <<"Complete status="<<completeStatus<<"; lastAbs="<<lastAbs<<"; cInd="<<completeIndex;

    return completeStatus;
}



std::pair<uint8_t, uint8_t> Bar::countUsedSigns() const
{
    size_t num = 0;
    size_t denum = 32;

    denum *= 3;//temp action for triplets (for n trumplets better multipleyer must)
    denum *= 2;

    for (size_t beatInd = 0; beatInd < size(); ++beatInd)
    {
        size_t duration = at(beatInd)->getDuration();
        size_t detail = at(beatInd)->getDurationDetail();
        size_t addition = 0;

        switch (duration) { //remember 8 is 8
            case 5: addition=1; break;
            case 4: addition=2; break;
            case 3: addition=4; break;
            case 2: addition=8; break;
            case 1: addition=16; break;
            case 0: addition=32; break;
            default:
                if (barLog)  qDebug()<<"Shit in duration "<<duration;
        }

        addition *= 2; //dots on 32
        addition *= 3; //triplets

        if (detail == 3) {
            addition *= 2;
            addition /= 3;
        }

        bool byteDote = at(beatInd)->getDotted();
        if (byteDote == 1)
            addition += addition/2;

        //recalculations for addition
         num += addition;
        if (barLog)  qDebug()<<"Addition is "<<addition<<"; det= "<<detail
             <<" dur= "<<duration<<" full "<<num<<"; dot "<<byteDote;
    }

    while ( (num % 2 == 0) && (denum % 2 == 0) ) {
        num /= 2;
        denum /= 2;
    }

    while ( (num % 3 == 0) && (denum % 3 == 0) ) {
        num /= 3;
        denum /= 3;
    }

    return {num, denum};
}


void Bar::printToStream(std::ostream &stream) const
{
    stream << "Outputing #"<<size()<<" Beats."<<std::endl;
    for (size_t ind = 0; ind < size(); ++ind)
            at(ind)->printToStream(stream);
}


void Bar::flush() {
    _signatureNum = _signatureDenum = 4; //standart bar
    _repeat = _repeatTimes = _altRepeat = 0;
    _markerColor = 0;
    _completeStatus = 0;
}


void Bar::push_back(std::unique_ptr<Beat> val) {
    if (val) {
        val->setParent(this);
        ChainContainer<Beat, Track>::push_back(std::move(val));
    }
}


void Bar::insertBefore(std::unique_ptr<Beat> val, int index) {
    if (val) {
        val->setParent(this);
        ChainContainer<Beat, Track>::insertBefore(std::move(val),index);
    }
}


void Bar::setRepeat(std::uint8_t rValue, std::uint8_t times) {
    if (rValue == 0)
        _repeat = 0;
    else
        _repeat |= rValue;
    if (times)
        _repeatTimes=times;
}
