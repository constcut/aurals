#ifndef MIDIFILE_H
#define MIDIFILE_H

#include <vector>
#include <fstream>
#include <string_view>

#include "MidiTrack.hpp"


namespace aurals {

    class MidiFile : public std::vector<MidiTrack> {

        public:
            MidiFile() = default;
            MidiFile(std::string_view filename);

            uint32_t calculateHeader(bool skipSomeMessages=false);

            uint32_t readFromFile(std::string_view filename);
            uint32_t readStream(std::ifstream& file);

            uint32_t writeToFile(std::string_view filename, bool skipSomeMessages=false);
            uint32_t writeStream(std::ofstream& file, bool skipSomeMessages=false);

            uint16_t getBPM() const { return _bpm; }
            void setBPM(const uint16_t newBPM) { _bpm = newBPM; }

        protected:
            uint16_t _bpm = 120;

            char _chunkId[4];
            uint32_t _chunkSize;
            uint16_t _formatType;
            uint16_t _tracksCount;
            uint16_t _timeDevisition;
    };

}

#endif // MIDIFILE_H
