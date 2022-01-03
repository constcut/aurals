#include "midisignal.h"

#include <QDebug>
bool enableMidiLog = false;
#define log qDebug


NBytesInt::NBytesInt(std::uint32_t source) {
    if (source == 0) {
        push_back(0);
        return;
    }

    std::vector<std::uint8_t> byteParts; //TODO reserve
    while (source) {
        std::uint8_t nextByte = source % 128;
        byteParts.push_back(nextByte);
        source /= 128;
    }

    for (int i = byteParts.size()-1; i >=0 ; --i)
        push_back(byteParts[i]); ///TODO algo
}

std::uint32_t NBytesInt::readStream(std::ifstream& f) {
    std::uint8_t lastByte = 0;
    do {
        f.read((char*)&lastByte, 1);
        push_back(lastByte & 127);
    } while (lastByte & 128);

    if (enableMidiLog) {
        log() << "VarInt read "<<getValue()<<" "<<size();

        for (size_t i = 0; i < size(); ++i)
            log() << "VarInt["<<i<<"] = "<<this->at(i);
    }
    return size();
}

std::uint32_t NBytesInt::writeStream(std::ofstream &f) {
    for (size_t i = 0; i < size(); ++i) {
        std::uint8_t anotherByte = operator [](i);
        if (i != size()-1)
            anotherByte |= 128;
        f.write((char*)&anotherByte, 1);
    }
    if (enableMidiLog) {
        for (size_t i = 0; i < size(); ++i)
            log() << "VarInt[" << i << "] = " << this->operator [](i);
        log() << "VarInt write " << getValue() << " " << size();
    }
    return size();
}

std::uint32_t NBytesInt::getValue() {
    std::uint32_t value = 0;
    size_t bytesToCollect = size() < 4 ? size() : 4;

    for (size_t i = 0; i < bytesToCollect; ++i) {
        value <<= 7;
        value += this->operator [](i);
    }
    return value;
}



MidiSignal::MidiSignal() : _byte0(0), _param1(0), _param2(0){}

MidiSignal::MidiSignal(std::uint8_t b0, std::uint8_t b1, std::uint8_t b2, std::uint32_t timeShift)
:  timeStamp(timeShift), _byte0(b0), _param1(b1), _param2(b2) {}


double MidiSignal::getSecondsLength(double bpm) {
    double seconds = (double)(timeStamp.getValue())/960.0;
    return seconds;
}


std::uint8_t MidiSignal::getEventType() {
    std::uint8_t eventType = (_byte0 & (0xf0)) >> 4; //name with enumeration byte blocks
    return eventType;
}


std::uint8_t MidiSignal::getChannel() {
    std::int8_t midiChannel = _byte0 & 0xf; //name with enumeration byte blocks
    return midiChannel;
}


bool MidiSignal::isMetaEvent() {
    return _byte0 == 0xff;
}


std::uint32_t MidiSignal::calculateSize(bool skipSomeMessages) {
    std::uint32_t messageSize = 0;

    if (skipSomeMessages == true)
        if (canSkipThat())
            return 0;

    messageSize += timeStamp.size();
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


bool MidiSignal::canSkipThat() {
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


std::uint32_t MidiSignal::readStream(std::ifstream& f) {

    std::uint32_t totalBytesRead = 0;
    totalBytesRead += timeStamp.readStream(f);
    f.read((char*)&_byte0, 1);
    ++totalBytesRead;
    f.read((char*)&_param1, 1);
    ++totalBytesRead;

    if (isMetaEvent()) { //TODO refact : metaProcess & usualProcess
        totalBytesRead += _metaLen.readStream(f);
        std::uint32_t bytesInMetaBufer = _metaLen.getValue();
        _metaBufer.clear(); //to be sure we don't makeit grow

        for (std::uint32_t i = 0; i < bytesInMetaBufer; ++i) {
            std::uint8_t byteBufer;
            f.read((char *)&byteBufer, 1);
            _metaBufer.push_back(byteBufer); //maybe better make a vector and read whole block once
        }
        totalBytesRead += bytesInMetaBufer;
        if (enableMidiLog)
            log() << "Midi meta mes read " << _byte0 << _param1 << _metaLen.getValue() << timeStamp.getValue() << " total bytes " << totalBytesRead << " " << f.tellg();
    }
    else {
        std::uint8_t eventType = getEventType();  //TODO ENUMERATION
        if ((eventType != 0xC) && (eventType != 0xD) && (eventType != 0x2) && (eventType != 0x3)
        && (eventType != 0x4) && (eventType != 0x5) && (eventType != 0x6) && (eventType != 0x0)) {
            f.read((char *)&_param2, 1);
            ++totalBytesRead;
        }
        if (enableMidiLog)
            log() << "Midi message read " << nameEvent(eventType).c_str() << " ( " << eventType << getChannel() << " ) "
                   << (int)_param1 << " " << (int)_param2 << " t: " << timeStamp.getValue() << " total bytes " << totalBytesRead << " " << f.tellg();
        if (eventType == 0xB) {
            if (enableMidiLog)
                log() << "Controller name: " << nameController(_param1).c_str();
        }
    }

    if (totalBytesRead > calculateSize())
        log() << "Error! overread " << f.tellg();
    return totalBytesRead;
}


std::string MidiSignal::nameEvent(std::int8_t eventNumber) {

    if (eventNumber == 0x8) //TODO switch?
        return "Note off";
    if (eventNumber == 0x9)
        return "Note on";
    if (eventNumber == 0xA)
        return "Aftertouch";
    if (eventNumber == 0xB)
        return "Control change";
    if (eventNumber == 0xC)
        return "Program (patch) change";
    if (eventNumber == 0xD)
        return "Channel pressure";
    if (eventNumber == 0xE)
        return "Pitch Wheel";

    return "Unknown_EventType";
}


std::string MidiSignal::nameController(std::uint8_t controllerNumber)
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
                               {94, "Celes			te Level"},
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

std::uint32_t MidiSignal::writeStream(std::ofstream& f, bool skipSomeMessages) {

    std::uint32_t totalBytesWritten = 0;
    if (skipSomeMessages && canSkipThat())
        return 0;

    totalBytesWritten += timeStamp.writeStream(f);
    f << _byte0;
    f << _param1;
    totalBytesWritten += 2;

    if (isMetaEvent()) {
        if (enableMidiLog)
            log() << "Midi meta mes write " << _byte0 << _param1
                << _metaLen.getValue() << timeStamp.getValue()
                << " total bytes " << totalBytesWritten << " " << f.tellp();

        totalBytesWritten += _metaLen.writeStream(f);

        //TODO compare
        //for (size_t i = 0; i < metaBufer.size(); ++i)
        //    f.write((const char *)&metaBufer[i], 1);
        f.write(reinterpret_cast<char*>(_metaBufer.data()), _metaBufer.size());
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
            log() << "Midi message write " << nameEvent(eventType).c_str() << " ( " << eventType << getChannel() << " ) " << _param1 << _param2
                    << " t: " << timeStamp.getValue() << " total bytes " << totalBytesWritten << " " << f.tellp();
            if (eventType == 0xB)
                log() << "Controller name: " << nameController(_param1).c_str();
        }
    }

    if (enableMidiLog) {
        log() << "Total bytes written in message " << totalBytesWritten;
        if (totalBytesWritten > calculateSize())
            log() << "Error! overwritten " << f.tellp();
    }
    return totalBytesWritten;
}
