#include "miditrack.h"

#include <QDebug>

extern bool midiLog; //TODO review log groups, make atomics


void MidiTrack::printToStream(std::ostream &stream)
{
    stream << "Output MidiTrack.";
    stream << "Track Size = " << trackHeader.trackSize << std::endl;
    //size_t signalsAmount = size();
    //for (size_t i = 0; i < signalsAmount; ++i)
        //at(i)->printToStream(stream); //message printing diabled
}



bool MidiTrack::calculateHeader(bool skip)
{
    //this function responsable for calculations of data stored inside MidiTrack

    size_t calculatedSize = 0;
    for (size_t i =0; i < size(); ++i){
        calculatedSize += at(i)->calculateSize(skip);
    }
    if (midiLog)  qDebug() <<"Calculating track size : "<<calculatedSize;
    if (midiLog)  qDebug() <<"Previously stored : "<<trackHeader.trackSize;

    trackHeader.trackSize = calculatedSize;
    memcpy(trackHeader.chunkId,"MTrk",5);
    return true;
}


void MidiTrack::pushChangeInstrument(std::uint8_t newInstr, std::uint8_t channel, size_t timeShift)
{
    if (midiLog)  qDebug() << "Change instrument "<<newInstr<<" on CH "<<channel;
    auto instrumentChange = std::make_unique<MidiSignal>(0xC0|channel,newInstr,0,timeShift);
    this->push_back(std::move(instrumentChange));
}

void MidiTrack::pushMetrSignature(std::uint8_t num, std::uint8_t den,size_t timeShift=0, std::uint8_t metr, std::uint8_t perQuat)
{
    auto signatureEvent  = std::make_unique<MidiSignal>(0xff,88,0,timeShift);

    signatureEvent->_metaBufer.push_back(num);

    std::uint8_t transDur=0;
    switch (den)
    {
        case 1: transDur = 0; break;
        case 2: transDur = 1; break;
        case 4: transDur = 2; break;
        case 8: transDur = 3; break;
        case 16: transDur = 4; break;
        case 32: transDur = 5; break;
        case 64: transDur = 6; break;
    default:
        transDur=6;
    }

    signatureEvent->_metaBufer.push_back(transDur);
    signatureEvent->_metaBufer.push_back(metr);
    signatureEvent->_metaBufer.push_back(perQuat);

    std::uint8_t metaSize = 4;
    signatureEvent->_metaLen.push_back(metaSize);

    push_back(std::move(signatureEvent));
}

void MidiTrack::pushChangeBPM(int bpm, size_t timeShift)
{
    if (midiLog)  qDebug() << "We change midi temp to "<<bpm; //attention

    auto changeTempEvent = std::make_unique<MidiSignal>(0xff,81,0,timeShift);
    //changeTempEvent.byte0 = 0xff;
    //changeTempEvent.param1 = 81;
    size_t MCount = 60000000/bpm;

    std::uint8_t tempB1 = (MCount>>16)&0xff; //0x7
    std::uint8_t tempB2 = (MCount>>8)&0xff; //0xa1
    std::uint8_t tempB3 = MCount&0xff; //0x20

    changeTempEvent->_metaBufer.push_back(tempB1);
    changeTempEvent->_metaBufer.push_back(tempB2);
    changeTempEvent->_metaBufer.push_back(tempB3);

    std::uint8_t lenMeta = 3;
    changeTempEvent->_metaLen.push_back(lenMeta);

    //byte timeZero = 0;
    //changeTempEvent.param2 = 0;
    //changeTempEvent.time.
    this->push_back(std::move(changeTempEvent));
}


void MidiTrack::pushChangeVolume(std::uint8_t newVolume, std::uint8_t channel)
{
    auto volumeChange = std::make_unique<MidiSignal>();
    if (newVolume > 127)
         newVolume = 127;

    volumeChange->_byte0 = 0xB0 | channel;
    volumeChange->_param1 = 7; //volume change
    volumeChange->_param2 = newVolume;
    std::uint8_t timeZero = 0;
    volumeChange->timeStamp.push_back(timeZero);

    push_back(std::move(volumeChange));
}

void MidiTrack::pushChangePanoram(std::uint8_t newPanoram, std::uint8_t channel)
{
    auto panoramChange = std::make_unique<MidiSignal>();
    panoramChange->_byte0 = 0xB0 | channel;
    panoramChange->_param1 = 0xA; //change panoram
    panoramChange->_param2 = newPanoram;
    std::uint8_t timeZero = 0;
    panoramChange->timeStamp.push_back(timeZero);

    this->push_back(std::move(panoramChange));
}

void MidiTrack::pushVibration(std::uint8_t channel, std::uint8_t depth, short int step, std::uint8_t stepsCount) {
    std::uint8_t shiftDown = 64-depth;
    std::uint8_t shiftUp = 64+depth;
    std::uint8_t signalKey = 0xE0 + channel;

    for (size_t vibroInd=0; vibroInd <stepsCount; ++vibroInd) {
         auto mSignalVibOn = std::make_unique<MidiSignal>(signalKey,0,shiftDown,step);
         auto mSignalVibOff = std::make_unique<MidiSignal>(signalKey,0,shiftUp,step);
         push_back(std::move(mSignalVibOn));
         push_back(std::move(mSignalVibOff));
    }

    auto mSignalVibOn = std::make_unique<MidiSignal>(signalKey,0,64,0);
    this->push_back(std::move(mSignalVibOn));
}

void MidiTrack::pushSlideUp(std::uint8_t channel, std::uint8_t shift, short int step, std::uint8_t stepsCount)
{
    std::uint8_t pitchShift = 64;
    std::uint8_t signalKey = 0xE0 + channel;

    for (size_t slideInd=0; slideInd <stepsCount; ++slideInd)
    {
         auto mSignalSlideOn = std::make_unique<MidiSignal>(signalKey,0,pitchShift,step);
         this->push_back(std::move(mSignalSlideOn));
         pitchShift+=shift;
    }
    auto mSignalSlideOff = std::make_unique<MidiSignal>(signalKey,0,64,0);
    this->push_back(std::move(mSignalSlideOff));
}

void MidiTrack::pushSlideDown(std::uint8_t channel, std::uint8_t shift, short int step, std::uint8_t stepsCount)
{
    std::uint8_t pitchShift = 64;
    std::uint8_t signalKey = 0xE0 + channel;

    for (size_t slideInd=0; slideInd <stepsCount; ++slideInd)
    {
         auto mSignalSlideOn = std::make_unique<MidiSignal>(signalKey,0,pitchShift,step);
         this->push_back(std::move(mSignalSlideOn));
         pitchShift-=shift;
    }
    auto mSignalSlideOff = std::make_unique<MidiSignal>(signalKey,0,64,0);
    this->push_back(std::move(mSignalSlideOff));
}



void MidiTrack::pushTremolo(short int rOffset)
{
    short int slideStep = rOffset/40; //10 steps of 1/4

    std::uint8_t pitchShift = 64;
    for (size_t slideInd=0; slideInd <10; ++slideInd)
    {
         auto mSignalBend = std::make_unique<MidiSignal>(0xE1,0,pitchShift,slideStep);
         this->push_back(std::move(mSignalBend));
         pitchShift-=3;//calibrate
    }

    rOffset -= rOffset/4;
    //last point
    auto mSignalBendLast = std::make_unique<MidiSignal>(0xE1,0,pitchShift, rOffset);
    this->push_back(std::move(mSignalBendLast));
    auto mSignalBendClose = std::make_unique<MidiSignal>(0xE1,0,64,0);
    this->push_back(std::move(mSignalBendClose));
}

void MidiTrack::pushFadeIn(short int rOffset, std::uint8_t channel)
{

    std::uint8_t newVolume = 27;
    short int fadeInStep = rOffset/20;

    auto volumeChangeFirst = std::make_unique<MidiSignal>(0xB0 | channel,7,newVolume,0);
    this->push_back(std::move(volumeChangeFirst));

    for (size_t i = 0; i < 20; ++i)
    {
        newVolume += 5;
        auto volumeChange = std::make_unique<MidiSignal>(0xB0 | channel,7,newVolume,fadeInStep);
        this->push_back(std::move(volumeChange));

    }
}

void MidiTrack::pushEvent47()
{

#ifdef WIN32
    auto trickA = std::make_unique<MidiSignal>(0x90 , 64, 3 ,240);
    auto trickB = std::make_unique<MidiSignal>(0x80 , 64, 3 ,240);
    //push_back(std::move(trickA));
    //push_back(std::move(trickB));
#endif

    auto event47 = std::make_unique<MidiSignal>(0xff,47,0,0);
    std::uint8_t lenZero = 0;
    event47->_metaLen.push_back(lenZero);
    this->push_back(std::move(event47));
}

//CALC helpers
short int MidiTrack::calcRhythmDetail(std::uint8_t RDValue, short int rhythmOffset)
{
    short int rOffset = rhythmOffset;
    if (RDValue == 3) //truplet
    {
        rOffset *= 2;
        rOffset /= 3;
    }
    if (RDValue == 5) //five-plet
    {
        rOffset *= 4;
        rOffset /= 5;
    }
    if (RDValue == 6) //five-plet
    {
        rOffset *= 5;
        rOffset /= 6;
    }
    if (RDValue == 7) //-plet
    {
        rOffset *= 4;
        rOffset /= 7;
    }
    if (RDValue == 9) //-plet
    {
        rOffset *= 8;
        rOffset /= 9;
    }

    //FEW MISSING

    //MAYBE NPLET posibility??
    //attention for other connections here
    //miss single double and tripple dot here
    return rOffset;
}

std::uint8_t MidiTrack::calcMidiPanoramGP(std::uint8_t pan)
{
    if (midiLog)  qDebug() << "Panoram value = "<<pan;
    std::uint8_t midiPanoram = pan*8;
    if (midiPanoram>=128) midiPanoram=128;
    return midiPanoram;
}
std::uint8_t MidiTrack::calcMidiVolumeGP(std::uint8_t vol)
{
    if (midiLog)  qDebug() <<"Volume is "<<vol;
    std::uint8_t midiVolume = vol*8;
    if (midiVolume >= 128) midiVolume=128;
    return midiVolume;
}
std::uint8_t MidiTrack::calcPalmMuteVelocy(std::uint8_t vel)
{
    std::uint8_t outputVelocy = vel;
    std::uint8_t decreaceVelocy = vel/5;
    outputVelocy -= decreaceVelocy;
    return outputVelocy;
}
std::uint8_t MidiTrack::calcLeggatoVelocy(std::uint8_t vel)
{
    return calcPalmMuteVelocy(vel);//for possible difference
}
//HELPERS END

/*
void MidiTrack::add(MidiSignal &val)
{
    ChainContainer::add(val);
  //  std::cout<<std::endl;
  //  val.printToStream(std::cout);
   // if (midiLog)  qDebug() <<"lo";
}
*/



//new fun
void MidiTrack::closeLetRings(std::uint8_t channel)
{
    for (size_t i = 0; i < 10; ++i)
    {
        if (_ringRay[i] != 255)
        {
            closeLetRing(i,channel);
        }
    }
}

void MidiTrack::closeLetRing(std::uint8_t stringN, std::uint8_t channel)
{
    if (stringN > 8)
    {
        qDebug() <<"String issue "<<stringN;
        return;
    }

    std::uint8_t ringNote = _ringRay[stringN];
    _ringRay[stringN]=255;

    std::uint8_t ringVelocy=80;

    if (ringNote != 255)
    pushNoteOff(ringNote,ringVelocy,channel);

}

void MidiTrack::openLetRing(std::uint8_t stringN, std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel)
{
    if (stringN > 8)
    {
        qDebug() <<"String issue "<<stringN;
        return;
    }

    if (_ringRay[stringN]!=255)
    {
        closeLetRing(stringN,channel);
    }
    _ringRay[stringN]=midiNote;

    pushNoteOn(midiNote,velocity,channel);
}

void MidiTrack::finishIncomplete(short specialR)
{
    //constant refact
     short int rhyBase = 120;

    short int power2 = 2<<(3);
    int preRValue = rhyBase*power2/4;

    preRValue *= specialR;
    preRValue /= 1000;

    accumulate(preRValue);
}

void MidiTrack::pushNoteOn(std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel)
{
    auto noteOn = std::make_unique<MidiSignal>(0x90 | channel, midiNote, velocity,_accum);
    takeAccum();
    push_back(std::move(noteOn));
}

void MidiTrack::pushNoteOff(std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel)
{
    auto noteOn = std::make_unique<MidiSignal>(0x80 | channel, midiNote, velocity,_accum);
    takeAccum();
    push_back(std::move(noteOn));
}

bool MidiTrack::checkForLeegFails()
{
}

void MidiTrack::startLeeg(std::uint8_t stringN, std::uint8_t channel)
{
}

void MidiTrack::stopLeeg(std::uint8_t stringN, std::uint8_t channel)
{
}
