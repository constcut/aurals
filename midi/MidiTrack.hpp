#ifndef MIDITRACK_H
#define MIDITRACK_H

#include "MidiMessage.hpp"

#include <vector>
#include <fstream>

namespace mtherapp {


    class MidiTrack : public std::vector<MidiMessage> {
        public:
        uint32_t calculateHeader(bool skipSomeMessages=false);

        void pushChangeInstrument(uint8_t newInstrument, uint8_t channel,  uint32_t timeShift);
        void pushMetricsSignature(uint8_t numeration, uint8_t denumeration,
                                  uint32_t timeShift, uint8_t metr=24, uint8_t perQuat=8);

        void pushChangeBPM(uint16_t bpm, uint32_t timeShift);
        void pushChangeVolume(uint8_t newVolume, uint8_t channel);
        void pushChangePanoram(uint8_t newPanoram, uint8_t channel);
        void pushVibration(uint8_t channel, uint8_t depth, uint16_t step, uint8_t stepsCount=3);
        void pushSlideUp(uint8_t channel, uint8_t shift, uint16_t step, uint8_t stepsCount=3);
        void pushSlideDown(uint8_t channel, uint8_t shift, uint16_t step, uint8_t stepsCount=3);
        void pushTremolo(uint8_t channel,uint16_t offset);

        void pushFadeIn(uint16_t offset, uint8_t channel);
        void pushEvent47();
        void pushTrackName(std::string trackName);

        int16_t calculateRhythmDetail(uint8_t value, int16_t offset);

        void closeLetRings(uint8_t channel);//Those functions used for generation from tablature, in case we would bring tablatures there
        void openLetRing(uint8_t stringN, uint8_t midiNote, uint8_t velocity, uint8_t channel);
        void closeLetRing(uint8_t stringN, uint8_t channel);
        void finishIncomplete(short specialR);

        void pushNoteOn(uint8_t midiNote, uint8_t velocity, uint8_t channel);
        void pushNoteOff(uint8_t midiNote, uint8_t velocity, uint8_t channel);


        protected:
            char _chunkId[4]; //DELAYED: as uint32_t = 'xxxx'?
            uint32_t _trackSize;

        private:
            int32_t _accum = 0;
            uint8_t _tunes[10];
            uint8_t _ringRay[10];

            double _timeLengthOnLoad; //Ms

       public:
            int32_t accumulate(int32_t addition) { _accum += addition; return _accum; }
            int32_t getAccum() const { return _accum; }
            void flushAccum() { _accum = 0; }

            uint32_t readFromFile(std::ifstream& f);
            uint32_t writeToFile(std::ofstream& f, bool skipSomeMessages=false) const;
    };

}

#endif
