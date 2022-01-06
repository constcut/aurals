#include "NBytesInt.hpp"

#include <vector>

#include "log.hpp"


using namespace mtherapp;

NBytesInt::NBytesInt(uint32_t source) {
    if (source == 0) {
        push_back(0);
        return;
    }

    std::vector<uint8_t> byteParts;
    while (source) {
        uint8_t nextByte = source % 128;
        byteParts.push_back(nextByte);
        source /= 128;
    }

    for (int i = static_cast<int>(byteParts.size()) - 1; i >= 0 ; --i)
        push_back(byteParts[i]);
}

uint32_t mtherapp::NBytesInt::readFromFile(std::ifstream& f) {
    uint8_t lastByte = 0;
    do {
        f.read(reinterpret_cast<char*>(&lastByte), 1);
        push_back(lastByte & 127);
    } while (lastByte & 128);

    if (enableMidiLog) {
        qDebug() << "VarInt read "<<getValue()<<" "<<size();

        for (size_t i = 0; i < size(); ++i)
            qDebug() << "VarInt["<<i<<"] = "<<this->at(i);
    }
    return size();
}

uint32_t mtherapp::NBytesInt::writeToFile(std::ofstream& f) const {
    for (size_t i = 0; i < size(); ++i) {
        uint8_t anotherByte = operator [](i);
        if (i != size()-1)
            anotherByte |= 128;
        f << anotherByte;
    }
    if (enableMidiLog) {
        for (size_t i = 0; i < size(); ++i)
            qDebug() << "VarInt[" << i << "] = " << this->operator [](i);
        qDebug() << "VarInt write " << getValue() << " " << size();
    }
    return size();
}

uint32_t mtherapp::NBytesInt::getValue() const {
    uint32_t value = 0;
    int bytesToCollect = size() < 4 ? size() : 4;

    for (int i = 0; i < bytesToCollect; ++i) {
        value <<= 7;
        value += this->operator [](i);
    }
    return value;
}
