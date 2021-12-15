#include "midifile.h"

#include "log.hpp"
#include <QtEndian>

#include <cmath>
#include <math.h>


//full loging must be able to turn on\off

bool enableMidiLog = false;

unsigned int log2_local( unsigned int x )
{
  unsigned int ans = 0 ;
  while( x>>=1 ) ans++;
  return ans ;
}

//=====================VarInt===========================================

VariableInteger::VariableInteger(quint32 source)
{
    if (source == 0){
        append(0); return;
    }

    QList<quint8> byteParts;
    while (source)
    {
        quint8 nextByte = source % 128;
        byteParts<<nextByte;
        source /= 128;
    }

    for (int i = byteParts.size()-1; i >=0 ; --i)
        append(byteParts[i]);
}

quint32 VariableInteger::readFromFile(QFile &f)
{
    quint8 lastByte = 0;
    do
    {
        f.read((char*)&lastByte, 1);
        append(lastByte & 127);

    } while (lastByte & 128);

    if (enableMidiLog)
    {
        qDebug() << "VarInt read "<<getValue()<<" "<<size();

        for (auto i = 0; i < size(); ++i)
            qDebug() << "VarInt["<<i<<"] = "<<this->operator [](i);
    }

    return size();
}

quint32 VariableInteger::writeToFile(QFile &f)
{
  for (int i = 0; i < size(); ++i)
  {
      quint8 anotherByte = operator [](i);
      if (i != size()-1)
        anotherByte |= 128;

      f.write((const char*)&anotherByte,1);
  }

  if (enableMidiLog) {
  for (auto i = 0; i < size(); ++i)
      qDebug() << "VarInt["<<i<<"] = "<<this->operator [](i);

  qDebug() << "VarInt write "<<getValue()<<" "<<size();
  }

  return size();
}

quint32 VariableInteger::getValue()
{
 quint32 result = 0;
 int bytesToCollect = size() < 4 ? size() : 4;

 for (int i = 0; i < bytesToCollect; ++i)
 {
     result <<= 7;
     result += this->operator [](i);
 }

 return result;
}

//=======================Midi Message==========================================

quint8 MidiMessage::getEventType() {
    quint8 eventType = (byte0 & (0xf0))>>4; //name with enumeration byte blocks
    return eventType;
}

quint8 MidiMessage::getChannel() {
    qint8 midiChannel = byte0 & 0xf;//name with enumeration byte blocks
    return midiChannel;
}

bool MidiMessage::isMetaEvent() {
    return byte0 == 0xff;
}

MidiMessage::MidiMessage():byte0(0),p1(0),p2(0)
{
}

MidiMessage::MidiMessage(quint8 b0, quint8 b1, quint8 b2, quint32 timeShift):
    byte0(b0),p1(b1),p2(b2),timeStamp(timeShift)
{
}

quint32 MidiMessage::calculateSize(bool skip)
{
    quint32  messageSize = 0;

    if (skip == true)
        if (skipThat())
            return 0;

    messageSize += timeStamp.size();
    ++messageSize; //byte 0

    if (isMetaEvent() == false)
    {
        ++messageSize; //parameter 1
        quint8 eventType = getEventType();

        if ((eventType != 0xC) && (eventType != 0xD)&& (eventType != 0x2)&& (eventType != 0x3)&& (eventType != 0x4)&& (eventType != 0x5)&& (eventType != 0x6)&& (eventType != 0x0)) //MAKE ENUMERATION
           ++messageSize; //parameter 2
    }
    else
    {
        ++messageSize; //parameter 1 actually
        messageSize += metaLen.size();
        messageSize += metaBufer.size();
    }
    return messageSize;
}

///there is some debug or quick fix function found there also:
bool MidiMessage::skipThat()
{
    if (isMetaEvent()) {
            if (p1 == 47) //MAKE ENUMERATION
                return false;
            if (p1 == 81) //change tempo
                return false;
            if (p1 == 88)
                return false; //Time signature
            if (p1 == 3)
                return false; //track name

            //qDebug() << "SKIP META "<<p1;

            //all the meta event must be here to be playable in winamp at least

            return true;  //this gives play in winamp only
    }
    else  {
        quint8 eventType = getEventType();
        if ((eventType == 8) || (eventType == 9)) //MAKE ENUMERATION
            return false; //notes on and off


        //AT first skip events that useful to be played through MPC
        ///LATER WILL name them
        return true;
    }

    return true;
}

///===========================================

quint32 MidiMessage::readFromFile(QFile &f)
{
    quint32 totalBytesRead = 0;

    totalBytesRead +=  timeStamp.readFromFile(f);
    f.read((char*)&byte0,1); ++totalBytesRead;
    f.read((char*)&p1,1); ++totalBytesRead;


    if (isMetaEvent())
    {
        totalBytesRead += metaLen.readFromFile(f);
        quint32 bytesInMetaBufer = metaLen.getValue();
        metaBufer.clear(); //to be sure we don't makeit grow

        for (quint32 i = 0; i < bytesInMetaBufer; ++i)
        {
            quint8 byteBufer;
            f.read((char*)&byteBufer,1);
            metaBufer.append(byteBufer); //maybe better make a vector and read whole block once
        }

        totalBytesRead += bytesInMetaBufer;

        if (enableMidiLog)
        qDebug() <<"Midi meta mes read "<<byte0<<p1<<metaLen.getValue() <<timeStamp.getValue()<<" total bytes "<<totalBytesRead <<" "<<f.pos();

        //SAME HERE PLEASE NAME SOME EVENTS
    }
    else
    {
        quint8 eventType = getEventType(); //ups - update conditions
        if ((eventType != 0xC) && (eventType != 0xD) && (eventType != 0x2)&& (eventType != 0x3)&& (eventType != 0x4)&& (eventType != 0x5)&& (eventType != 0x6) && (eventType != 0x0)) //MAKE ENUMERATION
        {
            f.read((char*)&p2,1);
            ++totalBytesRead;
        }
         if (enableMidiLog)
        qDebug() << "Midi message read "<< nameEvent(eventType) << " ( " << eventType << getChannel()<< " ) " << p1 << p2 <<" t: "<<timeStamp.getValue()<<" total bytes "<<totalBytesRead<<" "<<f.pos();

        if (eventType == 0xB)
        {
             if (enableMidiLog)
            qDebug() << "Controller name: " << nameController(p1);
            if (p1 == 100)
            {
                //qDebug() << "WATCH OUT";
            }
        }

    }


    if (totalBytesRead > calculateSize())
        qDebug() << "Error! overread "<<f.pos();

    return totalBytesRead;
}

 QString MidiMessage::nameEvent(qint8 eventNumber)
 {
     if (eventNumber == 0x8) return "Note off";
     if (eventNumber == 0x9) return "Note on";
     if (eventNumber == 0xA) return "Aftertouch";
     if (eventNumber == 0xB) return "Control change";
     if (eventNumber == 0xC) return "Program (patch) change";
     if (eventNumber == 0xD) return "Channel pressure";
     if (eventNumber == 0xE) return "Pitch Wheel";

     return "Unknown_EventType";
 }

 QString MidiMessage::nameController(quint8 controllerNumber)
 {
     struct controllesNames
     {
         quint8 index;
         char *name;
     };

     controllesNames names[] ={{0  ,"Bank Select"},
     {1  ,"Modulation Wheel (coarse)"},
     {2  ,"Breath controller (coarse)"},
     {4  ,"Foot Pedal (coarse)"},
     {5  ,"Portamento Time (coarse)"},
     {6  ,"Data Entry (coarse)"},
     {7  ,"Volume (coarse)"},
     {8  ,"Balance (coarse)"},
     {10 ,"Pan position (coarse)"},
     {11 ,"Expression (coarse)"},
     {12 ,"Effect Control 1 (coarse)"},
     {13 ,"Effect Control 2 (coarse)"},
     {16 ,"General Purpose Slider 1"},
     {17 ,"General Purpose Slider 2"},
     {18 ,"General Purpose Slider 3"},
     {19 ,"General Purpose Slider 4"},
     {32 ,"Bank Select (fine)"},
     {33 ,"Modulation Wheel (fine)"},
     {34 ,"Breath controller (fine)"},
     {36 ,"Foot Pedal (fine)"},
     {37 ,"Portamento Time (fine)"},
     {38 ,"Data Entry (fine)"},
     {39 ,"Volume (fine)"},
     {40 ,"Balance (fine)"},
     {42 ,"Pan position (fine)"},
     {43 ,"Expression (fine)"},
     {44 ,"Effect Control 1 (fine)"},
     {45 ,"Effect Control 2 (fine)"},
     {64 ,"Hold Pedal (on/off)"},
     {65 ,"Portamento (on/off)"},
     {66 ,"Sustenuto Pedal (on/off)"},
     {67 ,"Soft Pedal (on/off)"},
     {68 ,"Legato Pedal (on/off)"},
     {69 ,"Hold 2 Pedal (on/off)"},
     {70 ,"Sound Variation"},
     {71 ,"Sound Timbre"},
     {72 ,"Sound Release Time"},
     {73 ,"Sound Attack Time"},
     {74 ,"Sound Brightness"},
     {75 ,"Sound Control 6"},
     {76 ,"Sound Control 7"},
     {77 ,"Sound Control 8"},
     {78 ,"Sound Control 9"},
     {79 ,"Sound Control 10"},
     {80 ,"General Purpose Button 1 (on/off)"},
     {81 ,"General Purpose Button 2 (on/off)"},
     {82 ,"General Purpose Button 3 (on/off)"},
     {83 ,"General Purpose Button 4 (on/off)"},
     {91 ,"Effects Level"},
     {92 ,"Tremulo Level"},
     {93 ,"Chorus Level"},
     {94 ,"Celeste Level"},
     {95 ,"Phaser Level"},
     {96 ,"Data Button increment"},
     {97 ,"Data Button decrement"},
     {98 ,"Non-registered Parameter (coarse)"},
     {99 ,"Non-registered Parameter (fine)"},
     {100,"Registered Parameter (coarse)"},
     {101,"Registered Parameter (fine)"},
     {120,"All Sound Off"},
     {121,"All Controllers Off"},
     {122,"Local Keyboard (on/off)"},
     {123,"All Notes Off"},
     {124,"Omni Mode Off"},
     {125,"Omni Mode On"},
     {126,"Mono Operation"},
     {127,"Poly Operation"}};

     for (auto i = 0; i < (sizeof(names)/sizeof(controllesNames)); ++i)
     {
         if (names[i].index == controllerNumber)
             return names[i].name;
     }

     return "Unknown_ControllerName";
 }

quint32 MidiMessage::writeToFile(QFile &f, bool skip)
{
    quint32 totalBytesWritten = 0;

    if (skip && skipThat())
        return 0;

    totalBytesWritten += timeStamp.writeToFile(f);
    f.write((const char*)&byte0,1);
    f.write((const char*)&p1,1);
    totalBytesWritten += 2;

    //qDebug() << "Writing midi message "<<byte0<<" "<<p1<<" : position "<<f.pos();

    if (isMetaEvent())
    {
        //also maybe its nice to recalculate, yet we don't edit meta midi event,
        //but maybe once

         if (enableMidiLog)
        qDebug() <<"Midi meta mes write "<<byte0<<p1<<metaLen.getValue() <<timeStamp.getValue()<<" total bytes "<<totalBytesWritten <<" "<<f.pos();


        totalBytesWritten += metaLen.writeToFile(f);

        for (int i = 0; i < metaBufer.size(); ++i)
            f.write((const char*)&metaBufer[i],1); //if there was a vector can write a block


        totalBytesWritten += metaBufer.size();
    }
    else
    {
        quint8 eventType = getEventType();
        if ((eventType != 0xC) && (eventType != 0xD)&& (eventType != 0x2)&& (eventType != 0x3)&& (eventType != 0x4)&& (eventType != 0x5)&& (eventType != 0x6) && (eventType != 0x0))
        {
            f.write((const char*)&p2,1);
            ++totalBytesWritten;
        }

         if (enableMidiLog)
         {
            qDebug() << "Midi message write "<< nameEvent(eventType) << " ( " << eventType << getChannel()<< " ) " << p1 << p2 <<" t: "<<timeStamp.getValue()<<" total bytes "<<totalBytesWritten<<" "<<f.pos();

            if (eventType == 0xB)
                qDebug() << "Controller name: " << nameController(p1);
         }
    }


    if (enableMidiLog) {
    qDebug() << "Total bytes written in message "<<totalBytesWritten;

    if (totalBytesWritten > calculateSize())
        qDebug() << "Error! overwritten "<<f.pos();
    }

    return totalBytesWritten;
}

//=====================MidiTrack===================================

quint32 MidiTrack::calculateHeader(bool skip)
{
    quint32 calculatedSize = 0;

    for (quint32 i = 0; i < size(); ++i)
        calculatedSize += operator [](i).calculateSize(skip);

    if (enableMidiLog)
    if (trackSize != calculatedSize)
        qDebug () << "UPDATING track size: "<<trackSize<<" to "<<calculatedSize;

    trackSize = calculatedSize;

    memcpy(chunkId,"MTrk",4);
    return calculatedSize;
}


void MidiTrack::pushChangeInstrument(quint8 newInstrument, quint8 channel,  quint32 timeShift){
    append (MidiMessage(0xC0 | channel, newInstrument, 0, timeShift));
}

void MidiTrack::pushTrackName(QString trackName)
{
   MidiMessage nameTrack(0xff,3);
   nameTrack.metaLen = VariableInteger(trackName.size());

   QByteArray nameBytes = trackName.toLocal8Bit();

   for (auto i = 0; i < trackName.size(); ++i)
       nameTrack.metaBufer.append(nameBytes[i]);

    append(nameTrack);
}

void MidiTrack::pushMetricsSignature(quint8 numeration, quint8 denumeration,
                          quint32 timeShift, quint8 metr, quint8 perQuat)
{
    MidiMessage metrics(0xff,88,0,timeShift);
    metrics.metaBufer.append(numeration);

    qint8 translatedDuration = log2_local(denumeration); //there might be some issue on wrong data

    metrics.metaBufer.append(translatedDuration);
    metrics.metaBufer.append(metr);
    metrics.metaBufer.append(perQuat);
    metrics.metaLen = VariableInteger(4); //size of 4 bytes upper

    append(metrics);
}

void MidiTrack::pushChangeBPM(quint16 bpm, quint32 timeShift)
{
  MidiMessage changeTempo(0xff,81,0,timeShift);

   quint32 nanoCount = 60000000/bpm; //6e7 = amount of nanoseconds

   changeTempo.metaBufer.append((nanoCount>>16)&0xff);
   changeTempo.metaBufer.append((nanoCount>>8)&0xff);
   changeTempo.metaBufer.append(nanoCount&0xff);

   changeTempo.metaLen = VariableInteger(3);//size upper

   append(changeTempo);
}

void MidiTrack::pushChangeVolume(quint8 newVolume, quint8 channel){
    MidiMessage volumeChange(0xB0 | channel, 7, newVolume > 127 ? 127 : newVolume,0);
    append(volumeChange);
}

void MidiTrack::pushChangePanoram(quint8 newPanoram, quint8 channel){
    MidiMessage panoramChange(0xB0 | channel, 0xA, newPanoram, 0);
    append(panoramChange);
}

void MidiTrack::pushVibration(quint8 channel, quint8 depth, quint16 step, quint8 stepsCount)
{
    quint8 shiftDown = 64-depth;
    quint8 shiftUp = 64+depth;
    quint8 signalKey = 0xE0 + channel;

    for (quint32 i = 0; i < stepsCount; ++i)
    {
        append(MidiMessage(signalKey,0,shiftDown,step));
        append(MidiMessage(signalKey,0,shiftUp,step));
    }
    append(MidiMessage(signalKey,0,64,0));
}

void MidiTrack::pushSlideUp(quint8 channel, quint8 shift, quint16 step, quint8 stepsCount)
{
    quint8 pitchShift = 64;
    quint8 signalKey = 0xE0 + channel;
    for (quint32 i = 0; i < stepsCount; ++i)
    {
        append(MidiMessage(signalKey,0,pitchShift,step));
        pitchShift += shift;
    }
    append(MidiMessage(signalKey,0,64,0));
}

void MidiTrack::pushSlideDown(quint8 channel, quint8 shift, quint16 step, quint8 stepsCount)
{
    quint8 pitchShift = 64;
    quint8 signalKey = 0xE0 + channel;
    for (quint32 i = 0; i < stepsCount; ++i)
    {
        append(MidiMessage(signalKey,0,pitchShift,step));
        pitchShift -= shift;
    }
    append(MidiMessage(signalKey,0,64,0));
}

void MidiTrack::pushTremolo(quint8 channel,quint16 offset)
{
    quint16 slideStep = offset/40;

    quint8 pitchShift = 64;
    for (int i = 0; i < 10; ++i)
    {
        append(MidiMessage(0xE0 | channel, 0, pitchShift, slideStep));
        pitchShift -= 3;
    }

    offset -= offset/4;

    append(MidiMessage(0xE0 | channel, 0, pitchShift, offset));
    append(MidiMessage(0xE0 | channel, 0, 64, 0));
}

void MidiTrack::pushFadeIn(quint16 offset, quint8 channel)
{
    quint8 newVolume = 27;
    quint16 fadeInStep = offset/20;

    append(MidiMessage(0xB0 | channel, 7, newVolume, 0));

    for (int i = 0; i < 20; ++i)
    {
        newVolume += 5;
        append(MidiMessage(0xB0 | channel, 7, newVolume, fadeInStep));
    }
}

void MidiTrack::pushEvent47()
{
    MidiMessage event47(0xff,47,0,0);
    event47.metaLen = VariableInteger(0);
    append(event47);
}

qint16 MidiTrack::calculateRhythmDetail(quint8 value, qint16 offset) //maybe nice to have double equalent to calculate more proper?
{
    quint16 resultOffset = 0;

    if (value == 3) resultOffset = (offset*2)/3;
    if (value == 5) resultOffset = (offset*4)/5;
    if (value == 6) resultOffset = (offset*5)/6;
    if (value == 7) resultOffset = (offset*4)/7;
    if (value == 9) resultOffset = (offset*8)/9;

    return resultOffset;
}

quint32 MidiTrack::readFromFile(QFile &f)
{
    f.read(chunkId,4); //CHECK ITS RIGHT
    f.read((char*)&trackSize,4);  //when there is too big - its sign of know error

    if ((chunkId[0]!='M') || (chunkId[1]!='T')
            || (chunkId[2]!='r') || (chunkId[3]!='k'))
    {
       if (enableMidiLog)
        qDebug() << "Error: Header of track corrupted "
                 <<chunkId[0]<<chunkId[1]<<chunkId[2]<<chunkId[3];
        return 8;
    }

    trackSize = qToBigEndian(trackSize);

     if (enableMidiLog)
        qDebug() << "Reading midi track "<<chunkId[0]<<chunkId[1]<<chunkId[2]<<chunkId[3]<<
             trackSize;

    double totalTime = 0.0;
    int beatsPerMinute = 120; //default value

    quint32 bytesRead = 0;
    while (bytesRead < trackSize)
    {
         MidiMessage midiMessage;
         bytesRead += midiMessage.readFromFile(f);

         //WE NEED LOOK FOR BPM CHANGE HERE AND UPDATE variable
         ///beatsPerMinute = midiMessage.getBPM(); //helper function to unpack bpm rate
         //
         ///AND IN FUTURE NEED TO MAKE A PSEUDO MIDI TRACK mixed from all other
         /// containing ptrs to MidiMessages, and so this is the way to set all the absolute time
         /// well together so bpm change on one track will afect another

         totalTime += midiMessage.getSecondsLength(beatsPerMinute)*1000.0; //to ms
         midiMessage.absoluteTime = totalTime;

         append(midiMessage);
    }

    timeLengthOnLoad = totalTime;

    if (bytesRead > trackSize)
    {
        if (enableMidiLog)
            qDebug() << "ERROR readen more bytes then needed "<<bytesRead<< trackSize;

        //Recovery system on issues
        if (enableMidiLog)
        {
            if (f.seek(f.pos() - (bytesRead-trackSize)))
            qDebug() <<"Rolled back to continue read file as possible";
            else
            qDebug() << "Failed to roll back";
        }

    }

    return bytesRead + 8;
}

quint32 MidiTrack::writeToFile(QFile &f, bool skip)
{
    quint32 totalBytesWritten = 0;

    f.write(chunkId,4);
    quint32 sizeInverted = qToBigEndian(trackSize);
    f.write((char *)&sizeInverted,4);
    totalBytesWritten += 8;

    if (enableMidiLog)
        qDebug() << "Writing midi track "<<chunkId[0]<<chunkId[1]<<chunkId[2]<<chunkId[3]<<
                trackSize;

    for (auto i = 0; i < size(); ++i)
        totalBytesWritten += this->operator [](i).writeToFile(f,skip);

    return totalBytesWritten;
}

//=========================MidiFile===============================

MidiFile::MidiFile():bpm(120)
{
}

quint32 MidiFile::calculateHeader(bool skip)
{
    quint32 totalBytesCalculated = 0;

    memcpy(chunkId,"MThd",4);
    formatType = 1; //look over 0 and are there other formats? //0 is 1 track
    timeDevisition = 480; //also look please
    chunkSize = 6;
    tracksCount = size();

    totalBytesCalculated += 8 + chunkSize; //header total size

    for (int i = 0; i < size(); ++i)
        totalBytesCalculated += this->operator [](i).calculateHeader();

    return totalBytesCalculated;
}

MidiFile::MidiFile(QString filename)
{
    readFromFile(filename);
}

quint32 MidiFile::readFromFile(QString filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    readFromFile(file);
}

quint32 MidiFile::readFromFile(QFile &f)
{
    quint32 totalBytesRead = 0;

    chunkSize = 0;
    formatType = 0;
    tracksCount = 0;
    timeDevisition = 0;

    f.read((char*)&chunkId,4); //Must check is value inside right
    f.read((char*)&chunkSize,4); //should check is it == 6 - if not notify and read the space
    f.read((char*)&formatType,2);  //Must check is it 0 or 1 if else we don't know it
    f.read((char*)&tracksCount,2);
    f.read((char*)&timeDevisition,2);
    totalBytesRead += 14;

    chunkSize = qToBigEndian(chunkSize);
    formatType = qToBigEndian(formatType);
    tracksCount = qToBigEndian(tracksCount);
    timeDevisition = qToBigEndian(timeDevisition);

    if ((chunkId[0]!='M') || (chunkId[1]!='T') || (chunkId[2]!='h')
            ||(chunkId[3]!='d'))
    {
        if (enableMidiLog)
            qDebug() << "Midi header corrupted - error "
                 <<chunkId[0]<<chunkId[1]<<chunkId[2]<<chunkId[3];
        return totalBytesRead;
    }

    if (chunkSize != 6)
    {
        if (enableMidiLog)
            qDebug()<< "Issue chunk size != 6";
        return totalBytesRead;
    }

    if (enableMidiLog)
        qDebug() << "Reading midi file "<<chunkId[0]<<chunkId[1]<<chunkId[2]<<chunkId[3]<<
                " "<<chunkSize<<" "<<formatType<<" "<<tracksCount<<" "<<timeDevisition;

    for (auto i = 0; i < tracksCount; ++i) //something about future codding style I hope
    {
        MidiTrack track;
        totalBytesRead += track.readFromFile(f);
        append(track);
    }

    return totalBytesRead;
}

quint32 MidiFile::writeToFile(QString filename, bool skip)
{
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    return writeToFile(file,skip);
}

quint32 MidiFile::writeToFile(QFile &f, bool skip)
{
    quint32 totalBytesWritten=0;

    calculateHeader(skip);

    quint32 sizeInverted = qToBigEndian(chunkSize);
    quint16 formatInverted = qToBigEndian(formatType);
    quint16 tracksNInverted = qToBigEndian(tracksCount);
    quint16 timeDInverted = qToBigEndian(timeDevisition);

    f.write((const char*)&chunkId,4);
    f.write((const char*)&sizeInverted,4);
    f.write((const char*)&formatInverted,2);
    f.write((const char*)&tracksNInverted,2);
    f.write((const char*)&timeDInverted,2);
    totalBytesWritten += 14;

    if (enableMidiLog)
        qDebug() << "Writing midi file "<<chunkId[0]<<chunkId[1]<<chunkId[2]<<chunkId[3]<<
                " "<<chunkSize<<" "<<formatType<<" "<<tracksCount<<" "<<timeDevisition;

    for (auto i = 0; i < size(); ++i)
        totalBytesWritten += this->operator [](i).writeToFile(f, skip);

    return totalBytesWritten;
}
