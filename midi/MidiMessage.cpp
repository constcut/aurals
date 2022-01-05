#include "MidiMessage.hpp"

#include "log.hpp"


using namespace mtherapp;

MidiMessage::MidiMessage() : _typeAndChannel(0), _param1(0), _param2(0){}

MidiMessage::MidiMessage(std::uint8_t b0, std::uint8_t b1, std::uint8_t b2, std::uint32_t timeShift)
: _typeAndChannel(b0), _param1(b1), _param2(b2), _timeStamp(timeShift) {}


double MidiMessage::getSecondsLength(double bpm) const {
    double seconds = static_cast<double>(_timeStamp.getValue()) * (120.0 / bpm) / 960.0; //TODO перепроверить
    return seconds;
}


std::uint8_t MidiMessage::getEventType() const {
    std::uint8_t eventType = (_typeAndChannel & (0xf0)) >> 4; //name with enumeration byte blocks
    return eventType;
}


std::uint8_t MidiMessage::getChannel() const {
    std::int8_t midiChannel = _typeAndChannel & 0xf; //name with enumeration byte blocks
    return midiChannel;
}


bool MidiMessage::isMetaEvent() const {
    return _typeAndChannel == 0xff;
}


std::uint32_t MidiMessage::calculateSize(bool skipSomeMessages) const {
    std::uint32_t messageSize = 0;

    if (skipSomeMessages == true)
        if (canSkipThat())
            return 0;

    messageSize += _timeStamp.size();
    ++messageSize; //byte 0

    if (isMetaEvent() == false) {
        ++messageSize; //parameter 1
        std::uint8_t eventType = getEventType();

        if ((eventType != 0xC) && (eventType != 0xD) && (eventType != 0x2) && (eventType != 0x3) && (eventType != 0x4) && (eventType != 0x5) && (eventType != 0x6) && (eventType != 0x0)) //MAKE ENUMERATION
            ++messageSize;                                                                                                                                                                //parameter 2
    }
    else {
        ++messageSize; //parameter 1 actually
        messageSize += _metaLen.size();
        messageSize += _metaBufer.size();
    }
    return messageSize;
}


bool MidiMessage::canSkipThat() const {
    if (isMetaEvent()) {
        if (_param1 == 47) //MAKE ENUMERATION
            return false;
        if (_param1 == 81) //change tempo
            return false;
        if (_param1 == 88)
            return false; //Time signature
        if (_param1 == 3)
            return false; //track name
        return true;
    }
    else {
        std::uint8_t eventType = getEventType();
        if ((eventType == 8) || (eventType == 9))
            return false;
        return true;
    }
    return true;
}


std::uint32_t MidiMessage::readFromFile(std::ifstream& f) {

    std::uint32_t totalBytesRead = 0;
    totalBytesRead += _timeStamp.readFromFile(f);
    f.read((char*)&_typeAndChannel, 1);
    ++totalBytesRead;
    f.read((char*)&_param1, 1);
    ++totalBytesRead;

    if (isMetaEvent()) {
        totalBytesRead += _metaLen.readFromFile(f);
        std::uint32_t bytesInMetaBufer = _metaLen.getValue();
        _metaBufer.clear();

        for (std::uint32_t i = 0; i < bytesInMetaBufer; ++i) {
            std::uint8_t byteBufer;
            f.read((char *)&byteBufer, 1);
            _metaBufer.push_back(byteBufer);
        }
        totalBytesRead += bytesInMetaBufer;
        if (enableMidiLog)
            qDebug() << "Midi meta mes read " << _typeAndChannel << _param1 << _metaLen.getValue() << _timeStamp.getValue() << " total bytes " << totalBytesRead << " " << f.tellg();
    }
    else {
        std::uint8_t eventType = getEventType();  //TODO ENUMERATION                                                                                                                                        
        if ((eventType != 0xC) && (eventType != 0xD) && (eventType != 0x2) && (eventType != 0x3) 
        && (eventType != 0x4) && (eventType != 0x5) && (eventType != 0x6) && (eventType != 0x0)) {
            f.read((char *)&_param2, 1);
            ++totalBytesRead;
        }
        if (enableMidiLog)
            qDebug() << "Midi message read " << nameEvent(eventType) << " ( " << eventType << getChannel() << " ) "
                   << (int)_param1 << " " << (int)_param2 << " t: " << _timeStamp.getValue() << " total bytes " << totalBytesRead << " " << f.tellg();
        if (eventType == 0xB) {
            if (enableMidiLog)
                qDebug() << "Controller name: " << nameController(_param1);
        }
    }

    if (totalBytesRead > calculateSize())
        qDebug() << "Error! overread " << f.tellg();
    return totalBytesRead;
}


std::string MidiMessage::nameEvent(std::int8_t eventNumber) const {
    switch (eventNumber) {
        case 0x8:
            return "Note off";
        case 0x9:
            return "Note on";
        case 0xA:
            return "Aftertouch";
        case 0xB:
            return "Control change";
        case 0xC:
            return "Program (patch) change";
        case 0xD:
            return "Channel pressure";
        case 0xE:
            return "Pitch Wheel";
    }
    return "Unknown_EventType";
}


std::string MidiMessage::nameController(std::uint8_t controllerNumber) const
{
    struct controllesNames {
        std::uint8_t index;
        std::string name;
    };

    controllesNames names[] = {{0, "Bank Select"},
                               {1, "Modulation Wheel (coarse)"},
                               {2, "Breath controller (coarse)"},
                               {4, "Foot Pedal (coarse)"},
                               {5, "Portamento Time (coarse)"},
                               {6, "Data Entry (coarse)"},
                               {7, "Volume (coarse)"},
                               {8, "Balance (coarse)"},
                               {10, "Pan position (coarse)"},
                               {11, "Expression (coarse)"},
                               {12, "Effect Control 1 (coarse)"},
                               {13, "Effect Control 2 (coarse)"},
                               {16, "General Purpose Slider 1"},
                               {17, "General Purpose Slider 2"},
                               {18, "General Purpose Slider 3"},
                               {19, "General Purpose Slider 4"},
                               {32, "Bank Select (fine)"},
                               {33, "Modulation Wheel (fine)"},
                               {34, "Breath controller (fine)"},
                               {36, "Foot Pedal (fine)"},
                               {37, "Portamento Time (fine)"},
                               {38, "Data Entry (fine)"},
                               {39, "Volume (fine)"},
                               {40, "Balance (fine)"},
                               {42, "Pan position (fine)"},
                               {43, "Expression (fine)"},
                               {44, "Effect Control 1 (fine)"},
                               {45, "Effect Control 2 (fine)"},
                               {64, "Hold Pedal (on/off)"},
                               {65, "Portamento (on/off)"},
                               {66, "Sustenuto Pedal (on/off)"},
                               {67, "Soft Pedal (on/off)"},
                               {68, "Legato Pedal (on/off)"},
                               {69, "Hold 2 Pedal (on/off)"},
                               {70, "Sound Variation"},
                               {71, "Sound Timbre"},
                               {72, "Sound Release Time"},
                               {73, "Sound Attack Time"},
                               {74, "Sound Brightness"},
                               {75, "Sound Control 6"},
                               {76, "Sound Control 7"},
                               {77, "Sound Control 8"},
                               {78, "Sound Control 9"},
                               {79, "Sound Control 10"},
                               {80, "General Purpose Button 1 (on/off)"},
                               {81, "General Purpose Button 2 (on/off)"},
                               {82, "General Purpose Button 3 (on/off)"},
                               {83, "General Purpose Button 4 (on/off)"},
                               {91, "Effects Level"},
                               {92, "Tremulo Level"},
                               {93, "Chorus Level"},
                               {94, "Celeste Level"},
                               {95, "Phaser Level"},
                               {96, "Data Button increment"},
                               {97, "Data Button decrement"},
                               {98, "Non-registered Parameter (coarse)"},
                               {99, "Non-registered Parameter (fine)"},
                               {100, "Registered Parameter (coarse)"},
                               {101, "Registered Parameter (fine)"},
                               {120, "All Sound Off"},
                               {121, "All Controllers Off"},
                               {122, "Local Keyboard (on/off)"},
                               {123, "All Notes Off"},
                               {124, "Omni Mode Off"},
                               {125, "Omni Mode On"},
                               {126, "Mono Operation"},
                               {127, "Poly Operation"}};

    //Exaluate as contexpr
    for (size_t i = 0; i < (sizeof(names) / sizeof(controllesNames)); ++i) {
        if (names[i].index == controllerNumber)
            return names[i].name;
    }

    return "Unknown_ControllerName";
}

std::uint32_t MidiMessage::writeToFile(std::ofstream& f, bool skipSomeMessages) const {

    std::uint32_t totalBytesWritten = 0;
    if (skipSomeMessages && canSkipThat())
        return 0;

    totalBytesWritten += _timeStamp.writeToFile(f);
    //f << _typeAndChannel;
    //f << _param1; //TODO review on gtab works fine :(
    f.write((const char*)&_typeAndChannel, 1);
    f.write((const char*)&_param1, 1);
    totalBytesWritten += 2;

    if (isMetaEvent()) {
        if (enableMidiLog)
            qDebug() << "Midi meta mes write " << _typeAndChannel << _param1
                << _metaLen.getValue() << _timeStamp.getValue()
                << " total bytes " << totalBytesWritten << " " << f.tellp();

        totalBytesWritten += _metaLen.writeToFile(f);

        f.write(reinterpret_cast<const char*>(_metaBufer.data()), _metaBufer.size());
        totalBytesWritten += _metaBufer.size();
    }
    else {
        std::uint8_t eventType = getEventType();
        if ((eventType != 0xC) && (eventType != 0xD) && (eventType != 0x2) && (eventType != 0x3) 
            && (eventType != 0x4) && (eventType != 0x5) && (eventType != 0x6) && (eventType != 0x0)) {

            f << _param2;
            ++totalBytesWritten;
        }
        if (enableMidiLog) {
            qDebug() << "Midi message write " << nameEvent(eventType) << " ( " << eventType << getChannel() << " ) " << _param1 << _param2
                    << " t: " << _timeStamp.getValue() << " total bytes " << totalBytesWritten << " " << f.tellp();
            if (eventType == 0xB)
                qDebug() << "Controller name: " << nameController(_param1);
        }
    }

    if (enableMidiLog) {
        qDebug() << "Total bytes written in message " << totalBytesWritten;
        if (totalBytesWritten > calculateSize())
            qDebug() << "Error! overwritten " << f.tellp();
    }

    return totalBytesWritten;
}
