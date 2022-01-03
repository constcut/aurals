#ifndef MIDISIGNAL_H
#define MIDISIGNAL_H

#include <list>
#include <vector>

#include <deque>
#include <fstream>



class NBytesInt : public std::deque<std::uint8_t> {

public:
    NBytesInt(){}
    NBytesInt(std::uint32_t source);

    std::uint32_t readStream(std::ifstream& f);
    std::uint32_t writeStream(std::ofstream& f);

    std::uint32_t getValue();
};



class MidiSignal
{
public:

    std::uint8_t getEventType() ;
    std::uint8_t getChannel();
    bool isMetaEvent();

    MidiSignal();
    MidiSignal(std::uint8_t b0, std::uint8_t b1, std::uint8_t b2=0, std::uint32_t timeShift=0);

    std::uint32_t calculateSize(bool skipSomeMessages=false);

    bool canSkipThat(); ///TODO doublecheck

    std::uint32_t readStream(std::ifstream& f);
    std::uint32_t writeStream(std::ofstream& f, bool skipSomeMessages=false); //?Todo review name skip

    std::string nameEvent(std::int8_t eventNumber);
    std::string nameController(std::uint8_t controllerNumber);

    double getSecondsLength(double bpm=120.0);

    const std::vector<std::uint8_t>& getMetaInfo() { return _metaBufer; }

public: //TODO :(
    NBytesInt timeStamp;

    std::uint8_t _byte0;
    std::uint8_t _param1, _param2;

    double _absoluteTime;

    NBytesInt _metaLen;
    std::vector<std::uint8_t> _metaBufer;
};

#endif // MIDISIGNAL_H
