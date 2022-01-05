#include "MidiTrack.hpp"

#include <string_view>
#include <cmath>

#include "log.hpp"
#include "MidiUtils.hpp"

using namespace mtherapp;



constexpr std::uint32_t returnChunkId(std::string_view str = "MTrk") {
    std::uint32_t value = 0;
    for (const auto &s : str) {
        value <<= 8;
        auto charId = s;
        value |= charId;
    }
    return value;
}


std::uint32_t mtherapp::MidiTrack::calculateHeader(bool skipSomeMessages) {

    std::uint32_t calculatedSize = 0;

    for (std::size_t i = 0; i < size(); ++i)
        calculatedSize += operator[](i).calculateSize(skipSomeMessages);

    if (enableMidiLog)
        if (_trackSize != calculatedSize)
            qDebug() << "UPDATING track size: " << _trackSize << " to " << calculatedSize;

    _trackSize = calculatedSize;

    _chunkId[0] = 'M';
    _chunkId[1] = 'T';
    _chunkId[2] = 'r';
    _chunkId[3] = 'k';
    return calculatedSize;
}


void mtherapp::MidiTrack::pushChangeInstrument(std::uint8_t newInstrument, std::uint8_t channel, std::uint32_t timeShift) {
    push_back(MidiMessage(MidiMasks::PatchChangeMask | channel, newInstrument, 0, timeShift));
}


void mtherapp::MidiTrack::pushTrackName(std::string trackName) {
    MidiMessage nameTrack(MidiEvent::MetaEvent, MidiMetaTypes::TrackName);
    nameTrack.metaLen() = NBytesInt(trackName.size());
    //.toLocal8Bit();
    for (size_t i = 0; i < trackName.size(); ++i)
        nameTrack.metaBufer().push_back(trackName[i]);

    push_back(nameTrack);
}


void mtherapp::MidiTrack::pushMetricsSignature(std::uint8_t numeration, std::uint8_t denumeration,
                                     std::uint32_t timeShift, std::uint8_t metr, std::uint8_t perQuat) {
    MidiMessage metrics(MidiEvent::MetaEvent, MidiMetaTypes::ChangeTimeSignature, 0, timeShift);
    metrics.metaBufer().push_back(numeration);

    std::uint8_t translatedDuration = std::log2(denumeration);
    metrics.metaBufer().push_back(translatedDuration);
    metrics.metaBufer().push_back(metr);
    metrics.metaBufer().push_back(perQuat);
    metrics.metaLen() = NBytesInt(4); //size of 4 bytes upper

    push_back(metrics);
}


void mtherapp::MidiTrack::pushChangeBPM(std::uint16_t bpm, std::uint32_t timeShift) {
    MidiMessage changeTempo(MidiEvent::MetaEvent, MidiMetaTypes::ChangeTempo, 0, timeShift);
    std::uint32_t nanoCount = 60000000 / bpm; //6e7 = amount of nanoseconds
    changeTempo.metaBufer().push_back((nanoCount >> 16) & 0xff);
    changeTempo.metaBufer().push_back((nanoCount >> 8) & 0xff);
    changeTempo.metaBufer().push_back(nanoCount & 0xff);
    changeTempo.metaLen() = NBytesInt(3); //size upper

    push_back(changeTempo);
}


void mtherapp::MidiTrack::pushChangeVolume(std::uint8_t newVolume, std::uint8_t channel) {
    MidiMessage volumeChange(MidiMasks::ControlChangeMask | channel, 7, newVolume > 127 ? 127 : newVolume, 0);
    push_back(volumeChange);
}


void mtherapp::MidiTrack::pushChangePanoram(std::uint8_t newPanoram, std::uint8_t channel) {
    MidiMessage panoramChange(MidiMasks::ControlChangeMask | channel, 0xA, newPanoram, 0);
    push_back(panoramChange);
}


void mtherapp::MidiTrack::pushVibration(std::uint8_t channel, std::uint8_t depth, std::uint16_t step, std::uint8_t stepsCount) {
    std::uint8_t shiftDown = 64 - depth;
    std::uint8_t shiftUp = 64 + depth;
    std::uint8_t signalKey = MidiMasks::PitchWheelMask + channel;

    for (std::uint32_t i = 0; i < stepsCount; ++i) {
        push_back(MidiMessage(signalKey, 0, shiftDown, step));
        push_back(MidiMessage(signalKey, 0, shiftUp, step));
    }
    push_back(MidiMessage(signalKey, 0, 64, 0));
}


void mtherapp::MidiTrack::pushSlideUp(std::uint8_t channel, std::uint8_t shift, std::uint16_t step, std::uint8_t stepsCount) {
    std::uint8_t pitchShift = 64;
    std::uint8_t signalKey = MidiMasks::PitchWheelMask + channel;
    for (std::uint32_t i = 0; i < stepsCount; ++i) {
        push_back(MidiMessage(signalKey, 0, pitchShift, step));
        pitchShift += shift;
    }
    push_back(MidiMessage(signalKey, 0, 64, 0));
}


void mtherapp::MidiTrack::pushSlideDown(std::uint8_t channel, std::uint8_t shift, std::uint16_t step, std::uint8_t stepsCount) {
    std::uint8_t pitchShift = 64;
    std::uint8_t signalKey = MidiMasks::PitchWheelMask + channel;
    for (std::uint32_t i = 0; i < stepsCount; ++i) {
        push_back(MidiMessage(signalKey, 0, pitchShift, step));
        pitchShift -= shift;
    }
    push_back(MidiMessage(signalKey, 0, 64, 0));
}


void mtherapp::MidiTrack::pushTremolo(std::uint8_t channel, std::uint16_t offset) {
    std::uint16_t slideStep = offset / 40;
    std::uint8_t pitchShift = 64;
    for (int i = 0; i < 10; ++i) {
        push_back(MidiMessage(MidiMasks::PitchWheelMask | channel, 0, pitchShift, slideStep));
        pitchShift -= 3;
    }
    offset -= offset / 4;
    push_back(MidiMessage(MidiMasks::PitchWheelMask | channel, 0, pitchShift, offset));
    push_back(MidiMessage(MidiMasks::PitchWheelMask | channel, 0, 64, 0));
}


void mtherapp::MidiTrack::pushFadeIn(std::uint16_t offset, std::uint8_t channel) {
    std::uint8_t newVolume = 27;
    std::uint16_t fadeInStep = offset / 20;
    push_back(MidiMessage(MidiMasks::ControlChangeMask | channel, 7, newVolume, 0));
    for (int i = 0; i < 20; ++i) {
        newVolume += 5;
        push_back(MidiMessage(MidiMasks::ControlChangeMask | channel, 7, newVolume, fadeInStep));
    }
}

void mtherapp::MidiTrack::pushEvent47() { //Emergency event
    MidiMessage event47(MidiEvent::MetaEvent, MidiMetaTypes::KindOfFinish, 0, 0);
    event47.metaLen() = NBytesInt(0);
    push_back(event47);
}


std::int16_t mtherapp::MidiTrack::calculateRhythmDetail(std::uint8_t value, std::int16_t offset) {
    std::uint16_t resultOffset = 0;
    if (value == 3)
        resultOffset = (offset * 2) / 3;
    if (value == 5)
        resultOffset = (offset * 4) / 5;
    if (value == 6)
        resultOffset = (offset * 5) / 6;
    if (value == 7)
        resultOffset = (offset * 4) / 7;
    if (value == 9)
        resultOffset = (offset * 8) / 9;
    return resultOffset;
}


std::uint32_t mtherapp::MidiTrack::readFromFile(std::ifstream& f)
{
    f.read(_chunkId, 4);
    f.read((char *)&_trackSize, 4);

    if ((_chunkId[0] != 'M') || (_chunkId[1] != 'T') || (_chunkId[2] != 'r') || (_chunkId[3] != 'k'))
    {
        if (enableMidiLog)
            qDebug() << "Error: Header of track corrupted "
                  << _chunkId[0] << _chunkId[1] << _chunkId[2] << _chunkId[3];
        return 8;
    }

    _trackSize = swapEndian<std::uint32_t>(_trackSize);

    if (enableMidiLog)
        qDebug() << "Reading midi track " << _chunkId[0] << _chunkId[1] << _chunkId[2] << _chunkId[3] << _trackSize;

    double totalTime = 0.0;
    int beatsPerMinute = 120; //default value

    std::uint32_t bytesRead = 0;
    while (bytesRead < _trackSize) {

        MidiMessage midiMessage;
        bytesRead += midiMessage.readFromFile(f);
        totalTime += midiMessage.getSecondsLength(beatsPerMinute) * 1000.0; //to ms
        midiMessage.setAbsoluteTime(totalTime);
        push_back(midiMessage);
    }

    _timeLengthOnLoad = totalTime;

    if (bytesRead > _trackSize) {
        if (enableMidiLog)
            qDebug() << "Critical ERROR readen more bytes then needed " << bytesRead << _trackSize;
    }

    return bytesRead + 8;
}


std::uint32_t mtherapp::MidiTrack::writeToFile(std::ofstream& f, bool skipSomeMessages) const {
    std::uint32_t totalBytesWritten = 0;
    //f << _chunkId;
    f.write(_chunkId, 4);

    std::uint32_t sizeInverted = swapEndian<std::uint32_t>(_trackSize);
    //f << sizeInverted;
    f.write((const char*)&sizeInverted, 4);
    totalBytesWritten += 8;

    if (enableMidiLog)
        qDebug() << "Writing midi track " << _chunkId[0] << _chunkId[1] << _chunkId[2] << _chunkId[3] << _trackSize;

    for (size_t i = 0; i < size(); ++i)
        totalBytesWritten += this->operator[](i).writeToFile(f, skipSomeMessages);

    return totalBytesWritten;
}


void mtherapp::MidiTrack::closeLetRings(std::uint8_t channel) {
    for (size_t i = 0; i < 10; ++i)
        if (_ringRay[i] != 255)
            closeLetRing(i, channel);
}


void mtherapp::MidiTrack::closeLetRing(std::uint8_t stringN, std::uint8_t channel) {
    if (stringN > 8){
        qDebug() <<"String issue " << stringN;
        return;
    }

    std::uint8_t ringNote = _ringRay[stringN];
    _ringRay[stringN]=255;
    std::uint8_t ringVelocy=80;
    if (ringNote != 255)
        pushNoteOff(ringNote,ringVelocy,channel);
}


void mtherapp::MidiTrack::openLetRing(std::uint8_t stringN, std::uint8_t midiNote,
                                      std::uint8_t velocity, std::uint8_t channel) {
    if (stringN > 8){
        qDebug() <<"String issue "<<stringN;
        return;
    }

    if (_ringRay[stringN]!=255)
        closeLetRing(stringN,channel);
    _ringRay[stringN]=midiNote;
    pushNoteOn(midiNote,velocity,channel);
}


void mtherapp::MidiTrack::finishIncomplete(short specialR) {
    short int rhyBase = 120;
    short int power2 = 2<<(3);
    int preRValue = rhyBase*power2/4;
    preRValue *= specialR;
    preRValue /= 1000;
    accumulate(preRValue);
}


void mtherapp::MidiTrack::pushNoteOn(std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel) {
    MidiMessage msg(MidiMasks::NoteOnMask | channel, midiNote, velocity,_accum);
    flushAccum();
    push_back(msg);
}


void mtherapp::MidiTrack::pushNoteOff(std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel) {
    MidiMessage msg(MidiMasks::NoteOffMask | channel, midiNote, velocity,_accum);
    flushAccum();
    push_back(msg);
}
