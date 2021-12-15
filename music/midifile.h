#ifndef MIDIFILE_H
#define MIDIFILE_H

#include <QList>
#include <QFile>



class VariableInteger : public QList<quint8>
{
public:
    VariableInteger(){}
    VariableInteger(quint32 source);

    quint32 readFromFile(QFile &f);
    quint32 writeToFile(QFile &f);

    quint32 getValue();
};

//=====MIDI message=================================

class MidiMessage
{
public:

    quint8 getEventType() ;
    quint8 getChannel();
    bool isMetaEvent();

    MidiMessage();
    MidiMessage(quint8 b0, quint8 b1, quint8 b2=0, quint32 timeShift=0);

    quint32 calculateSize(bool skip=false);

    bool skipThat();///there is some debug or quick fix function found there also:

    quint32 readFromFile(QFile &f);
    quint32 writeToFile(QFile &f, bool skip=false);

    QString nameEvent(qint8 eventNumber);
    QString nameController(quint8 controllerNumber);

    //GREAT IDEA TO MAKE BPM WITH DOTS
    double getSecondsLength(double bpm=120.0)
    { //yet only 120 and on default 480 time division -modify later
        double seconds = (double)(timeStamp.getValue())/960.0;
        return seconds;
    }

public:                       //maybe protected is a better case - speeding up right now
    VariableInteger timeStamp;

    quint8 byte0; //type + channel
    quint8 p1,p2; //parameters

    double absoluteTime;

    //On case its a Meta message:
    VariableInteger metaLen;
    QList<quint8> metaBufer;
};


//===========================Midi Track=================

class MidiTrack : public QList<MidiMessage>
{
    public:
    quint32 calculateHeader(bool skip=false);

    void pushChangeInstrument(quint8 newInstrument, quint8 channel,  quint32 timeShift);
    void pushMetricsSignature(quint8 numeration, quint8 denumeration,
                              quint32 timeShift, quint8 metr=24, quint8 perQuat=8);

    void pushChangeBPM(quint16 bpm, quint32 timeShift);
    void pushChangeVolume(quint8 newVolume, quint8 channel);
    void pushChangePanoram(quint8 newPanoram, quint8 channel);
    void pushVibration(quint8 channel, quint8 depth, quint16 step, quint8 stepsCount=3);
    void pushSlideUp(quint8 channel, quint8 shift, quint16 step, quint8 stepsCount=3);
    void pushSlideDown(quint8 channel, quint8 shift, quint16 step, quint8 stepsCount=3);
    void pushTremolo(quint8 channel,quint16 offset);

    void pushFadeIn(quint16 offset, quint8 channel);

    void pushEvent47();

    void pushTrackName(QString trackName);

    qint16 calculateRhythmDetail(quint8 value, qint16 offset); //maybe nice to have double equalent to calculate more proper?
    //Helpers for tab loading: calcMidiPanoramGP, calcMidiVolumeGP, calcPalmMuteVelocy, calcLeggatoVelocy

    ///Helpers for let rings closeLetRings, openLetRing, closeLetRing,
    /// finishIncomplete, startLeeg, stopLeeg, checkForLeegFails

    protected:
        //Header: Mtrk + track size
        char chunkId[4];
        quint32 trackSize;

        //Helpers - might go private
        qint32 accum;
        quint8 tunes[10]; //legacy
        quint8 ringRay[10];     //stayed for tablature generation needs

        double timeLengthOnLoad; //miliseconds of track
        //yet it appaers only on load

   public:
        qint32 accumulate(qint32 addition) { accum += addition; return accum; }
        qint32 getAccum() { return accum; }
        void flushAccum() { accum = 0; }

    public:

        quint32 readFromFile(QFile &f);
        quint32 writeToFile(QFile &f, bool skip=false);
};

//=========================MidiFile===========================

class MidiFile : public QList<MidiTrack>
{
public:
    MidiFile();
    MidiFile(QString filename);

    quint32 calculateHeader(bool skip=false);

    quint32 readFromFile(QString filename);
    quint32 readFromFile(QFile &f);

    quint32 writeToFile(QString filename, bool skip=false);
    quint32 writeToFile(QFile &f, bool skip=false);

    //somthing called 'no metrics test' used with that (skip) calculateHeader
protected:
    quint16 bpm;

    //header
    char chunkId[4];
    quint32 chunkSize;
    quint16 formatType;
    quint16 tracksCount;
    quint16 timeDevisition;
};

#endif // MIDIFILE_H
