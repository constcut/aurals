#include "log.hpp"
#include "MidiFile.hpp"

#include <fstream>
#include <limits.h>

#include "log.hpp"

using namespace mtherapp;


template <typename T> //TODO moveout
T swapEndian(T u) {
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");
    union {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;
    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];
    return dest.u;
}



mtherapp::MidiFile::MidiFile():_bpm(120)
{
}


std::uint32_t mtherapp::MidiFile::calculateHeader(bool skipSomeMessages) {

    std::uint32_t totalBytesCalculated = 0;
    _chunkId[0] = 'M';
    _chunkId[1] = 'T';
    _chunkId[2] = 'h';
    _chunkId[3] = 'd';

    //std::uint32_t* example = chunkId; //TODO
    //qDebug() << "Example value for MThd in uint32: " << example;

    _formatType = 1;
    _timeDevisition = 480; //Explain
    _chunkSize = 6;
    _tracksCount = size();
    totalBytesCalculated += 8 + _chunkSize;

    for (size_t i = 0; i < size(); ++i)
        totalBytesCalculated += this->operator [](i).calculateHeader(skipSomeMessages);
        
    return totalBytesCalculated;
}

mtherapp::MidiFile::MidiFile(std::string_view filename) {
    readFromFile(filename);
}

std::uint32_t mtherapp::MidiFile::readFromFile(std::string_view filename) {
    std::string strFileName {filename};
    std::ifstream file {strFileName, std::ios::binary};
    if (file.is_open())
        return readFromFile(file);
    return 0;
}

std::uint32_t mtherapp::MidiFile::readFromFile(std::ifstream& f) {

    std::uint32_t totalBytesRead = 0;
    _chunkSize = 0;
    _formatType = 0;
    _tracksCount = 0;
    _timeDevisition = 0;

    f.read(&_chunkId[0], 4);
    f.read((char*)&_chunkSize, 4); // << fails :(((
    f.read((char*)&_formatType, 2);
    f.read((char*)&_tracksCount, 2);
    f .read((char*)&_timeDevisition, 2);
    totalBytesRead += 14;

    _chunkSize = swapEndian<std::uint32_t>(_chunkSize);
    _formatType = swapEndian<std::uint16_t>(_formatType);
    _tracksCount = swapEndian<std::uint16_t>(_tracksCount);
    _timeDevisition = swapEndian<std::uint16_t>(_timeDevisition);


    //TODO short function based on constExpr
    if ((_chunkId[0]!='M') || (_chunkId[1]!='T') || (_chunkId[2]!='h')
            || (_chunkId[3]!='d'))
    {
        if (enableMidiLog)
            qDebug() << "Midi header corrupted - error "
                 << _chunkId[0] << _chunkId[1] << _chunkId[2] << _chunkId[3];

        return totalBytesRead;
    }

    if (_chunkSize != 6) {
        if (enableMidiLog)
            qDebug()<< "Issue chunk size != 6";
        return totalBytesRead;
    }

    if (enableMidiLog)
        qDebug() << "Reading midi file "<<_chunkId[0]<<_chunkId[1]<<_chunkId[2]<<_chunkId[3]<<
                " "<<_chunkSize<<" "<<_formatType<<" "<<_tracksCount<<" "<<_timeDevisition;

    for (auto i = 0; i < _tracksCount; ++i) {
        MidiTrack track;
        totalBytesRead += track.readFromFile(f);
        push_back(track);
    }

    return totalBytesRead;
}

std::uint32_t mtherapp::MidiFile::writeToFile(std::string_view filename, bool skipSomeMessages) {
    std::string strFilename(filename);
    std::ofstream file(strFilename, std::ios::binary);
    return writeToFile(file, skipSomeMessages);
}



std::uint32_t mtherapp::MidiFile::writeToFile(std::ofstream& f, bool skipSomeMessages) {

    std::uint32_t totalBytesWritten=0;
    calculateHeader(skipSomeMessages);

    std::uint32_t sizeInverted = swapEndian<std::uint32_t>(_chunkSize);
    std::uint16_t formatInverted = swapEndian<std::uint16_t>(_formatType);
    std::uint16_t tracksNInverted = swapEndian<std::uint16_t>(_tracksCount);
    std::uint16_t timeDInverted = swapEndian<std::uint16_t>(_timeDevisition);

    f.write(_chunkId, 4);
    f.write((const char*)&sizeInverted, 4);
    f.write((const char*)&formatInverted, 2);
    f.write((const char*)&tracksNInverted, 2);
    f.write((const char*)&timeDInverted, 2);

    /*f << _chunkId; //This isn't works fine :(
    f << sizeInverted;
    f << formatInverted;
    f << tracksNInverted;
    f << timeDInverted;*/
    totalBytesWritten += 14;

    if (enableMidiLog)
        qDebug() << "Writing midi file " << _chunkId[0] << _chunkId[1] << _chunkId[2] << _chunkId[3] <<
                " " << _chunkSize << " " << _formatType << " " << _tracksCount << " " << _timeDevisition;

    for (size_t i = 0; i < size(); ++i)
        totalBytesWritten += this->operator [](i).writeToFile(f, skipSomeMessages);

    return totalBytesWritten;
}
