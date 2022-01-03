#ifndef MIDITRACK_H
#define MIDITRACK_H

#include "MidiMessage.hpp"
#include <vector>


namespace mtherapp {


    class MidiTrack : public std::vector<MidiMessage> {
        public:
        std::uint32_t calculateHeader(bool skipSomeMessages=false);

        void pushChangeInstrument(std::uint8_t newInstrument, std::uint8_t channel,  std::uint32_t timeShift);
        void pushMetricsSignature(std::uint8_t numeration, std::uint8_t denumeration,
                                  std::uint32_t timeShift, std::uint8_t metr=24, std::uint8_t perQuat=8);

        void pushChangeBPM(std::uint16_t bpm, std::uint32_t timeShift);
        void pushChangeVolume(std::uint8_t newVolume, std::uint8_t channel);
        void pushChangePanoram(std::uint8_t newPanoram, std::uint8_t channel);
        void pushVibration(std::uint8_t channel, std::uint8_t depth, std::uint16_t step, std::uint8_t stepsCount=3);
        void pushSlideUp(std::uint8_t channel, std::uint8_t shift, std::uint16_t step, std::uint8_t stepsCount=3);
        void pushSlideDown(std::uint8_t channel, std::uint8_t shift, std::uint16_t step, std::uint8_t stepsCount=3);
        void pushTremolo(std::uint8_t channel,std::uint16_t offset);

        void pushFadeIn(std::uint16_t offset, std::uint8_t channel);
        void pushEvent47();
        void pushTrackName(std::string trackName);

        std::int16_t calculateRhythmDetail(std::uint8_t value, std::int16_t offset);

        [[deprecated]] void closeLetRings(std::uint8_t channel);
        [[deprecated]] void openLetRing(std::uint8_t stringN, std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel);
        [[deprecated]] void closeLetRing(std::uint8_t stringN, std::uint8_t channel);

        [[deprecated]] void finishIncomplete(short specialR);

        void pushNoteOn(std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel);
        void pushNoteOff(std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel);


        protected:
            char _chunkId[4]; //TODO as uint32_t = 'xxxx'
            std::uint32_t _trackSize;

        private:
            std::int32_t _accum;
            std::uint8_t _tunes[10];
            std::uint8_t _ringRay[10];

            double _timeLengthOnLoad; //Ms

       public:
            std::int32_t accumulate(std::int32_t addition) { _accum += addition; return _accum; }
            std::int32_t getAccum() const { return _accum; }
            void flushAccum() { _accum = 0; }

            std::uint32_t readFromFile(std::ifstream& f);
            std::uint32_t writeToFile(std::ofstream& f, bool skipSomeMessages=false) const;
    };

}

#endif
