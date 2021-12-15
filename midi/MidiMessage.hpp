
#ifndef MIDIMESSAGE_H
#define MIDIMESSAGE_H

#include "NBytesInt.hpp"
#include <list>
#include <vector>



namespace mtherapp {

    class MidiMessage
    {
    public:

        std::uint8_t getEventType() ;
        std::uint8_t getChannel();
        bool isMetaEvent();

        MidiMessage();
        MidiMessage(std::uint8_t b0, std::uint8_t b1, std::uint8_t b2=0, std::uint32_t timeShift=0);

        std::uint32_t calculateSize(bool skipSomeMessages=false);

        bool canSkipThat(); ///TODO doublecheck

        std::uint32_t readFromFile(std::ifstream& f);
        std::uint32_t writeToFile(std::ofstream& f, bool skipSomeMessages=false); //?Todo review name skip

        std::string nameEvent(std::int8_t eventNumber);
        std::string nameController(std::uint8_t controllerNumber);

        double getSecondsLength(double bpm=120.0);

        const std::vector<std::uint8_t>& getMetaInfo() { return metaBufer; }

    protected:
        NBytesInt timeStamp;

        std::uint8_t byte0; //type + channel //TODO rename typeAndChannel
        std::uint8_t p1, p2; //parameters //TODO rename paramter1, parameter2

    public: //TODO cover with push functions

        double absoluteTime; //bad code detected

        NBytesInt metaLen;
        std::vector<std::uint8_t> metaBufer;
    };

}

#endif
