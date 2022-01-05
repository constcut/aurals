
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

        std::uint8_t getParameter1() const { return _param1; }
        std::uint8_t getParameter2() const { return _param2; }

        std::uint8_t getTypeAndChannel() const { return _typeAndChannel; }

    protected:
        std::uint8_t _typeAndChannel;
        std::uint8_t _param1, _param2;

        NBytesInt _timeStamp;

        double _absoluteTime;

        NBytesInt _metaLen;
        std::vector<std::uint8_t> _metaBufer;

    public:

        void setAbsoluteTime(double time) { _absoluteTime = time; }
        double absoluteTime() const { return _absoluteTime; }
        NBytesInt& metaLen() { return _metaLen; }
        std::vector<std::uint8_t>& metaBufer() { return _metaBufer; }

    };

}

#endif
