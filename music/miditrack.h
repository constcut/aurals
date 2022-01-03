#ifndef MIDITRACK_H
#define MIDITRACK_H

#include <vector>
#include <memory>

#include "midisignal.h"



class MidiTrack : public std::vector<std::unique_ptr<MidiSignal>> {

public:

    struct midiTrackHeader {
        char chunkId[5];
        size_t trackSize;
    };

    midiTrackHeader trackHeader; //think about it!

    MidiTrack() = default;
    virtual ~MidiTrack() = default;

    bool calculateHeader(bool skip=false);
    void printToStream(std::ostream &stream);

    void pushChangeInstrument(std::uint8_t newInstr, std::uint8_t channel, size_t timeShift=0);
    void pushChangeBPM(int bpm, size_t timeShift=0); //same way others
    void pushChangeVolume(std::uint8_t newVolume, std::uint8_t channel);
    void pushChangePanoram(std::uint8_t newPanoram, std::uint8_t channel);

    void pushMetrSignature(std::uint8_t num, std::uint8_t den, size_t timeShift, std::uint8_t metr=24, std::uint8_t perQuat=8);

    void pushVibration(std::uint8_t channel, std::uint8_t depth, short int step, std::uint8_t stepsCount=3);
    void pushSlideUp(std::uint8_t channel, std::uint8_t shift, short int step, std::uint8_t stepsCount=8);
    void pushSlideDown(std::uint8_t channel, std::uint8_t shift, short int step, std::uint8_t stepsCount=8);

    void pushTremolo(short int rOffset); //yet deadcoded one
    void pushFadeIn(short int rOffset, std::uint8_t channel);
    void pushEvent47();


    short int calcRhythmDetail(std::uint8_t RDValue, short int rhythmOffset);
    std::uint8_t calcMidiPanoramGP(std::uint8_t pan); //GUITAR P OPTION
    std::uint8_t calcMidiVolumeGP(std::uint8_t vol); //GUITAR P OPTION
    std::uint8_t calcPalmMuteVelocy(std::uint8_t vel);
    std::uint8_t calcLeggatoVelocy(std::uint8_t vel);

public:

    std::uint8_t _tunes[10];
    std::uint8_t _ringRay[10];

    void closeLetRings(std::uint8_t channel);		//-
    void openLetRing(std::uint8_t stringN, std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel); //-
    void closeLetRing(std::uint8_t stringN, std::uint8_t channel);  //-

    void finishIncomplete(short specialR); //-

    void pushNoteOn(std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel); //acummulated already inside
    void pushNoteOff(std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel); //-

    void startLeeg(std::uint8_t stringN, std::uint8_t channel);
    void stopLeeg(std::uint8_t stringN, std::uint8_t channel);

    bool checkForLeegFails();

public: //TODO

    int _accum;

    int accumulate(int value) { _accum += value; return _accum; }
    int getAccum() const { return _accum; }
    void takeAccum() { _accum=0; }

    void setTunes(std::uint8_t *from) {
        for (std::uint8_t i=0; i < 10; ++i){
            _tunes[i]=from[i];
            _ringRay[i]=255;
        }
    }
};


#endif // MIDITRACK_H
