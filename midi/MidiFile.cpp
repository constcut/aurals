#include "MidiFile.hpp"

#include <QDebug>

#include "MidiUtils.hpp"

using namespace aurals;

bool midiLog = false;


uint32_t MidiFile::calculateHeader(bool skipSomeMessages) {

    uint32_t totalBytesCalculated = 0;
    _chunkId[0] = 'M';
    _chunkId[1] = 'T';
    _chunkId[2] = 'h';
    _chunkId[3] = 'd';

    _formatType = 1;
    _timeDevisition = 480; //Explain
    _chunkSize = 6;
    _tracksCount = size();
    totalBytesCalculated += 8 + _chunkSize;

    for (size_t i = 0; i < size(); ++i)
        totalBytesCalculated += this->operator [](i).calculateHeader(skipSomeMessages);

    return totalBytesCalculated;
}

MidiFile::MidiFile(std::string_view filename) {
    readFromFile(filename);
}

uint32_t MidiFile::readFromFile(std::string_view filename) {
    std::string strFileName {filename};
    std::ifstream file {strFileName, std::ios::binary};
    if (file.is_open())
        return readStream(file);
    return 0;
}

uint32_t MidiFile::readStream(std::ifstream& f) {

    uint32_t totalBytesRead = 0;
    _chunkSize = 0;
    _formatType = 0;
    _tracksCount = 0;
    _timeDevisition = 0;

    f.read(&_chunkId[0], 4);
    f.read(reinterpret_cast<char*>(&_chunkSize), 4); // << fails :(((
    f.read(reinterpret_cast<char*>(&_formatType), 2);
    f.read(reinterpret_cast<char*>(&_tracksCount), 2);
    f .read(reinterpret_cast<char*>(&_timeDevisition), 2);
    totalBytesRead += 14;

    _chunkSize = swapEndian<uint32_t>(_chunkSize);
    _formatType = swapEndian<uint16_t>(_formatType);
    _tracksCount = swapEndian<uint16_t>(_tracksCount);
    _timeDevisition = swapEndian<uint16_t>(_timeDevisition);


    if ((_chunkId[0]!='M') || (_chunkId[1]!='T')
     || (_chunkId[2]!='h') || (_chunkId[3]!='d')) {

        if (midiLog)
            qDebug() << "Midi header corrupted - error "
                 << _chunkId[0] << _chunkId[1] << _chunkId[2] << _chunkId[3];
        return totalBytesRead;
    }

    if (_chunkSize != 6) {
        if (midiLog)
            qDebug()<< "Issue chunk size != 6";
        return totalBytesRead;
    }

    if (midiLog)
        qDebug() << "Reading midi file "<<_chunkId[0]<<_chunkId[1]<<_chunkId[2]<<_chunkId[3]<<
                " "<<_chunkSize<<" "<<_formatType<<" "<<_tracksCount<<" "<<_timeDevisition;

    for (auto i = 0; i < _tracksCount; ++i) {
        MidiTrack track;
        totalBytesRead += track.readFromFile(f);
        push_back(track);
    }

    return totalBytesRead;
}

uint32_t MidiFile::writeToFile(std::string_view filename, bool skipSomeMessages) {
    std::string strFilename(filename);
    std::ofstream file(strFilename, std::ios::binary);
    return writeStream(file, skipSomeMessages);
}



uint32_t MidiFile::writeStream(std::ofstream& f, bool skipSomeMessages) {

    uint32_t totalBytesWritten=0;
    calculateHeader(skipSomeMessages);

    uint32_t sizeInverted = swapEndian<uint32_t>(_chunkSize);
    uint16_t formatInverted = swapEndian<uint16_t>(_formatType);
    uint16_t tracksNInverted = swapEndian<uint16_t>(_tracksCount);
    uint16_t timeDInverted = swapEndian<uint16_t>(_timeDevisition);

    f.write(_chunkId, 4);
    f.write(reinterpret_cast<const char*>(&sizeInverted), 4);
    f.write(reinterpret_cast<const char*>(&formatInverted), 2);
    f.write(reinterpret_cast<const char*>(&tracksNInverted), 2);
    f.write(reinterpret_cast<const char*>(&timeDInverted), 2);

    totalBytesWritten += 14;

    if (midiLog)
        qDebug() << "Writing midi file " << _chunkId[0] << _chunkId[1] << _chunkId[2] << _chunkId[3] <<
                " " << _chunkSize << " " << _formatType << " " << _tracksCount << " " << _timeDevisition;

    for (size_t i = 0; i < size(); ++i)
        totalBytesWritten += this->operator [](i).writeToFile(f, skipSomeMessages);

    return totalBytesWritten;
}
