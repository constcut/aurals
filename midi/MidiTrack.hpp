#ifndef MIDITRACK_H
#define MIDITRACK_H

#include "MidiMessage.hpp"

#include <vector>
#include <fstream>


namespace aural_sight {


    class MidiTrack : public std::vector<MidiMessage> {
        public:
        uint32_t calculateHeader(bool skipSomeMessages=false);

        void pushChangeInstrument(const uint8_t newInstrument, const uint8_t channel, const uint32_t timeShift=0);
        void pushMetricsSignature(const uint8_t numeration, const uint8_t denumeration,
                                  const uint32_t timeShift, const uint8_t metr=24, const uint8_t perQuat=8);

        void pushChangeBPM(const uint16_t bpm, const uint32_t timeShift);
        void pushChangeVolume(const uint8_t newVolume, const uint8_t channel);
        void pushChangePanoram(const uint8_t newPanoram, const uint8_t channel);
        void pushVibration(const uint8_t channel, const uint8_t depth, const uint16_t step, const uint8_t stepsCount=3);
        void pushSlideUp(const uint8_t channel, const uint8_t shift, const uint16_t step, const uint8_t stepsCount=8);
        void pushSlideDown(const uint8_t channel, const uint8_t shift, const uint16_t step, const uint8_t stepsCount=8);
        void pushTremolo(const uint8_t channel, uint16_t offset);

        void pushFadeIn(const uint16_t offset, const uint8_t channel);
        void pushEvent47();
        void pushTrackName(const std::string trackName);

        int16_t calculateRhythmDetail(const uint8_t value, const int16_t offset) const;
        std::uint8_t calcMidiPanoramFromTab(std::uint8_t pan);
        std::uint8_t calcMidiVolumeFromTab(std::uint8_t vol);
        std::uint8_t calcPalmMuteVelocy(std::uint8_t vel);
        std::uint8_t calcLeggatoVelocy(std::uint8_t vel);


        void closeLetRings(const uint8_t channel);//Those functions used for generation from tablature, in case we would bring tablatures there
        void openLetRing(const uint8_t stringN, const uint8_t midiNote, const uint8_t velocity, const uint8_t channel);
        void closeLetRing(const uint8_t stringN, const uint8_t channel);
        void finishIncomplete(short specialR);

        void pushNoteOn(const uint8_t midiNote, const uint8_t velocity, const uint8_t channel);
        void pushNoteOff(const uint8_t midiNote, const uint8_t velocity, const uint8_t channel);



        void setTunes(std::uint8_t *from) {
            for (std::uint8_t i=0; i < 10; ++i){
                _tunes[i]=from[i];
                _ringRay[i]=255;
            }
        }

        uint8_t* getTunes() {
            return _tunes;
        }


        protected:
            char _chunkId[4]; //DELAYED: as uint32_t = 'xxxx'?
            uint32_t _trackSize;

        private:
            int32_t _accum = 0;
            uint8_t _tunes[10];
            uint8_t _ringRay[10];

            double _timeLengthOnLoad; //Ms

       public:


            void setAccum(int32_t newAcc) { _accum = newAcc; }
            int32_t accumulate(const int32_t addition) { _accum += addition; return _accum; }
            int32_t getAccum() const { return _accum; }
            void flushAccum() { _accum = 0; }

            uint32_t readFromFile(std::ifstream& f);
            uint32_t writeToFile(std::ofstream& f, bool skipSomeMessages=false) const;
    };

}

#endif
