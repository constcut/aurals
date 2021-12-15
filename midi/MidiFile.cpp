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



mtherapp::MidiFile::MidiFile():bpm(120)
{
}


std::uint32_t mtherapp::MidiFile::calculateHeader(bool skipSomeMessages) {

    std::uint32_t totalBytesCalculated = 0;
    chunkId[0] = 'M';
    chunkId[1] = 'T';
    chunkId[2] = 'h';
    chunkId[3] = 'd';

    //std::uint32_t* example = chunkId; //TODO
    //qDebug() << "Example value for MThd in uint32: " << example;

    formatType = 1;
    timeDevisition = 480; //Explain
    chunkSize = 6;
    tracksCount = size();
    totalBytesCalculated += 8 + chunkSize;

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
    chunkSize = 0;
    formatType = 0;
    tracksCount = 0;
    timeDevisition = 0;

    f.read(&chunkId[0], 4);
    f.read((char*)&chunkSize, 4); // << fails :(((
    f.read((char*)&formatType, 2);
    f.read((char*)&tracksCount, 2);
    f .read((char*)&timeDevisition, 2);
    totalBytesRead += 14;

    chunkSize = swapEndian<std::uint32_t>(chunkSize);
    formatType = swapEndian<std::uint16_t>(formatType);
    tracksCount = swapEndian<std::uint16_t>(tracksCount);
    timeDevisition = swapEndian<std::uint16_t>(timeDevisition);


    //TODO short function based on constExpr
    if ((chunkId[0]!='M') || (chunkId[1]!='T') || (chunkId[2]!='h')
            || (chunkId[3]!='d')) 
    {
        if (enableMidiLog)
            qDebug() << "Midi header corrupted - error "
                 << chunkId[0] << chunkId[1] << chunkId[2] << chunkId[3];

        return totalBytesRead;
    }

    if (chunkSize != 6) {
        if (enableMidiLog)
            qDebug()<< "Issue chunk size != 6";
        return totalBytesRead;
    }

    if (enableMidiLog)
        qDebug() << "Reading midi file "<<chunkId[0]<<chunkId[1]<<chunkId[2]<<chunkId[3]<<
                " "<<chunkSize<<" "<<formatType<<" "<<tracksCount<<" "<<timeDevisition;

    for (auto i = 0; i < tracksCount; ++i) {
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

    std::uint32_t sizeInverted = swapEndian<std::uint32_t>(chunkSize);
    std::uint16_t formatInverted = swapEndian<std::uint16_t>(formatType);
    std::uint16_t tracksNInverted = swapEndian<std::uint16_t>(tracksCount);
    std::uint16_t timeDInverted = swapEndian<std::uint16_t>(timeDevisition);

    f << chunkId;
    f << sizeInverted;
    f << formatInverted;
    f << tracksNInverted;
    f << timeDInverted;
    totalBytesWritten += 14;

    if (enableMidiLog)
        qDebug() << "Writing midi file " << chunkId[0] << chunkId[1] << chunkId[2] << chunkId[3] <<
                " " << chunkSize << " " << formatType << " " << tracksCount << " " << timeDevisition;

    for (size_t i = 0; i < size(); ++i)
        totalBytesWritten += this->operator [](i).writeToFile(f, skipSomeMessages);

    return totalBytesWritten;
}
