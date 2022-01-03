
#ifndef MIDIMESSAGE_H
#define MIDIMESSAGE_H

#include "NBytesInt.hpp"
#include <list>
#include <vector>



namespace mtherapp {

    class MidiMessage
    {
    public:

        std::uint8_t getEventType() const;
        std::uint8_t getChannel() const;
        bool isMetaEvent() const;

        MidiMessage();
        MidiMessage(std::uint8_t b0, std::uint8_t b1, std::uint8_t b2=0, std::uint32_t timeShift=0);

        std::uint32_t calculateSize(bool skipSomeMessages=false) const;

        bool canSkipThat() const;

        std::uint32_t readFromFile(std::ifstream& f);
        std::uint32_t writeToFile(std::ofstream& f, bool skipSomeMessages=false) const;

        std::string nameEvent(std::int8_t eventNumber) const;
        std::string nameController(std::uint8_t controllerNumber) const;

        double getSecondsLength(double bpm=120.0) const;

        const std::vector<std::uint8_t>& getMetaInfo() const { return _metaBufer; }

    protected:
        NBytesInt _timeStamp;

        std::uint8_t _byte0; //type + channel //TODO rename typeAndChannel
        std::uint8_t _p1, _p2; //parameters //TODO rename paramter1, parameter2

    public: //TODO cover with push functions

        double _absoluteTime; //bad code detected

        NBytesInt _metaLen;
        std::vector<std::uint8_t> _metaBufer;
    };

}

#endif
