
#ifndef MIDIMESSAGE_H
#define MIDIMESSAGE_H

#include "NBytesInt.hpp"
#include <list>
#include <vector>

//http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html
//https://www.usb.org/sites/default/files/midi10.pdf

namespace mtherapp {

    enum MidiEvent {
        ReservedExtentions = 0x0,
        CableReserved = 0x1,
        SystemCommonMessage2Bytes = 0x2,
        SystemCommonMessage3Bytes = 0x3,
        SysExStartOrContinue = 0x4,
        SystemCommonMessage1Byte = 0x5,
        SysExEnds2Bytes = 0x6,
        SysExEnds3Bytes = 0x7,
        NoteOff = 0x8,
        NoteOn = 0x9,
        Aftertouch = 0xA,
        ControlChange = 0xB,
        PatchChange = 0xC,
        ChannelPressure = 0xD,
        PitchWheel = 0xE,
        MetaEvent = 0xFF
    };

    enum MidiMetaTypes {
        TrackName = 0x3,
        KindOfFinish = 0x2f, //:)
        ChangeTempo = 0x51,
        ChangeTimeSignature = 0x58
    };

    enum MidiMasks {
        ChannelMask = 0xF,
        EventTypeMask = 0xF0,
    };

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

        void setAbsoluteTime(double time) { _absoluteTime = time; }
        double absoluteTime() const { return _absoluteTime; }
        NBytesInt& metaLen() { return _metaLen; }
        std::vector<std::uint8_t>& metaBufer() { return _metaBufer; }

    protected:
        std::uint8_t _typeAndChannel;
        std::uint8_t _param1, _param2;

        NBytesInt _timeStamp;

        double _absoluteTime;

        NBytesInt _metaLen;
        std::vector<std::uint8_t> _metaBufer;

        bool isNotSingleParamEvent(std::uint8_t eventType) const;

    };

}

#endif
