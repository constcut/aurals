#ifndef MIDIFILE_H
#define MIDIFILE_H

#include <vector>
#include <fstream>
#include <string_view>

#include "MidiTrack.hpp"


namespace mtherapp {

    class MidiFile : public std::vector<MidiTrack> {

        public:
            MidiFile();
            MidiFile(std::string_view filename);

            std::uint32_t calculateHeader(bool skipSomeMessages=false);

            std::uint32_t readFromFile(std::string_view filename);
            std::uint32_t readFromFile(std::ifstream& file);

            std::uint32_t writeToFile(std::string_view filename, bool skipSomeMessages=false);
            std::uint32_t writeToFile(std::ofstream& file, bool skipSomeMessages=false);

        protected:
            std::uint16_t bpm;

            char chunkId[4];
            std::uint32_t chunkSize;
            std::uint16_t formatType;
            std::uint16_t tracksCount;
            std::uint16_t timeDevisition;
    };

}

#endif // MIDIFILE_H
