#ifndef WAVEANALYS_H
#define WAVEANALYS_H

#include "fft.h"
#include <QSharedPointer>

struct AmplitudeValue
{
    int stepsFromLocalMin; //==0 is minimum, else - steps before current maximum there was minimum
    int stepsToNextMax;    //==0 is maximus, else - steps after  current minimum there was maximum

    long peakSize;
    long localOffset;
};

struct RythmValue
{
    AmplitudeValue attack; //point a (MAX)
    AmplitudeValue attenuation; //point b (MIN)

    AmplitudeValue attackBegining; //point z (MIN)

    AmplitudeValue sustainBegining; //point s0
    AmplitudeValue sustainEndining; //point s1
};


struct scaledWaveElement
{
    short upper;
    short upperLocal;
    short lower;
    short lowerLocal;
    int energy;
};


struct soundFragment
{
    long start;
    int amount;

    soundFragment(long startFrom):amount(1),start(startFrom)
    {}

    void addOne()
    {++amount;}
};

class sausageSeq
{
    int lastWasSound;

public:

    std::vector<soundFragment> sounds;
    std::vector<soundFragment> pauseList;

    sausageSeq()
    {
       lastWasSound = -1;
    }

    void pushToSound([[maybe_unused]]long pst, int frameNumber)
    {
        if ((lastWasSound==-1)||(lastWasSound == 0))
        {
            //also start new
            soundFragment sf = soundFragment(frameNumber);
            sounds.push_back(sf);
        }

        if (lastWasSound == 1) //it was sound
            sounds[sounds.size()-1].addOne();

         lastWasSound = 1;
    }

    void pushToSilent([[maybe_unused]]long pst, int frameNumber)
    {
        if ((lastWasSound==-1)||(lastWasSound == 1))
        {
            soundFragment sf = soundFragment(frameNumber);
            pauseList.push_back(sf);
        }

        if (lastWasSound == 0) //it was silent
            pauseList[pauseList.size()-1].addOne();

        lastWasSound = 0;
    }

};



class AmplitudeAnalys
{
public: // WHILE TEMP PAINTING DEBBUGING
    std::vector<scaledWaveElement> scaledWave;
    std::vector<AmplitudeValue> rythmicMaxSequence;
    std::vector<AmplitudeValue> rythmicMinSequence;
    std::vector<AmplitudeValue> rythmicSplit;

    std::vector<RythmValue> rezultSequence;

    bool isAnalysDone;

    long middleLine;

    sausageSeq ss;

    std::vector<long> candiPeaks;
    std::vector<long> candiStops;

public:

    AmplitudeAnalys();


    ///old functions
    void startAnalys(short *samples, long monoLength);
    void calculateNotes();
    bool isDone();

    //new fun
    void calculateSilence(short *samples, long monoLength);
    void divideAgglutinateSounds();
    void calculatePeaks();

    void acceptPeaks(); //-editeble + ADSR now
    void editPeaks();//1

    /*
    void findAndDeletePeakMin();
    void findAndDeletePeakMax();
    void itherePeak();
    */ //ed part


    //ADSR searchers
    AmplitudeValue searchAttackStart(AmplitudeValue a);
    bool searchSustains(RythmValue &rv);


    //common
    std::vector<RythmValue> *accessRythmSequence();

    void cleanUseless();
    void fullClean();

private:

};

class LocalFreqTable
{
public:
    struct LFTvote
    {
       static FreqTable ft;

       double freq;
       double value;
       void addValue(double v2)
       {value += v2;}

       double rFreq;
       void takeReal()
       {
           rFreq = ft.getNearestScaled(freq);
       }
    };

private:
    std::vector<Peak> tableElements;
    std::vector<unsigned short> harmUnion;
    std::vector<LFTvote> votes;

 public:
    void addPeak(Peak &p);
    void addPeaks(std::vector<Peak> *peaks);

    void voteNow();
    void voteNewWay();
    void recountHarmonicalUnion();

    std::vector<LFTvote> *getRezultVotes();
    std::vector<Peak> *getPeaks();

    void sortPeaks();
private:
    void voteFor(double freq, double voteValue);
    void voteForNew(double freq, double voteValue);
};


class HarmonicAnalys
{
public:
    static FreqTable freqTab;

    AmplitudeAnalys *ampPtr;

    std::vector< QSharedPointer<FFT> > analyses;
    std::vector< QSharedPointer<LocalFreqTable> > votes;

    int knockOne;
public:
    HarmonicAnalys();
    HarmonicAnalys(AmplitudeAnalys *amp);

    bool connectAmp(AmplitudeAnalys *amp);

    //taking notes one by one from amp, and checking therie stats
    void startAnalys(short int *zeroPoint);
    void startTable(short int *zeroPoint);



};

#endif // WAVEANALYS_H
