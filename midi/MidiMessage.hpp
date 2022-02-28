
#ifndef MidiMessage_H
#define MidiMessage_H

#include "NBytesInt.hpp"
#include <list>
#include <vector>

//http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html
//https://www.usb.org/sites/default/files/midi10.pdf

namespace aurals {


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
        NoteOffMask = 0x80,
        NoteOnMask = 0x90,
        AftetouchMask = 0xA0,
        ControlChangeMask = 0xB0,
        PatchChangeMask = 0xC0,
        ChannelPessureMask = 0xD0,
        PitchWheelMask = 0xE0,

        ChannelMask = 0xF,
        EventTypeMask = 0xF0,
        DrumTrackMask = 0x9
    };


    enum MidiChange {
        ChangeVolume = 0x7,
        ChangePanoram = 0xA
    };


    class MidiMessage
    {
    public:

        uint8_t getEventType() const;
        uint8_t getChannel() const;
        bool isMetaEvent() const;

        MidiMessage();
        MidiMessage(const uint8_t b0, const uint8_t b1,
                    const uint8_t b2=0, const uint32_t timeShift=0);

        uint32_t calculateSize(const bool skipSomeMessages=false) const;

        bool canSkipThat() const;

        uint32_t readStream(std::ifstream& f);
        uint32_t writeStream(std::ofstream& f, const bool skipSomeMessages=false) const;

        std::string nameEvent(const uint8_t eventNumber) const;
        std::string nameController(const uint8_t controllerNumber) const;

        double getSecondsLength(const double bpm=120.0) const;

        const std::vector<uint8_t>& getMetaInfo() const { return _metaBufer; }

        uint8_t getParameter1() const { return _param1; }
        uint8_t getParameter2() const { return _param2; }

        uint8_t getTypeAndChannel() const { return _typeAndChannel; }

        void setAbsoluteTime(const double time) { _absoluteTime = time; }
        double absoluteTime() const { return _absoluteTime; }
        NBytesInt& metaLen() { return _metaLen; }
        std::vector<uint8_t>& metaBufer() { return _metaBufer; }

        const NBytesInt& timeStamp() const { return _timeStamp; }

    protected:
        uint8_t _typeAndChannel;
        uint8_t _param1, _param2;

        NBytesInt _timeStamp;

        double _absoluteTime;

        NBytesInt _metaLen;
        std::vector<uint8_t> _metaBufer;

        bool isNotSingleParamEvent(const uint8_t eventType) const;

    };

}

#endif
