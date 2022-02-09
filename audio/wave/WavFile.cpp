/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "WavFile.hpp"

#include <cstring>

#include <qendian.h>
#include <QVector>
#include <QDebug>


using namespace aurals;


struct chunk {
    char        id[4];
    quint32     size;
};


struct RIFFHeader {
    chunk       descriptor;     // "RIFF"
    char        type[4];        // "WAVE"
};


struct WAVEHeader {
    chunk       descriptor;
    quint16     audioFormat;
    quint16     numChannels;
    quint32     sampleRate;
    quint32     byteRate;
    quint16     blockAlign;
    quint16     bitsPerSample;
};


struct DATAHeader {
    chunk       descriptor;
};


struct FACTHeader {
    chunk       descriptor;
    quint32 dwSampleLength; //maybe nice to look into specification and name after them everything
};


struct CombinedHeader {
    RIFFHeader  riff;
    WAVEHeader  wave;
};


WavFile::WavFile(QObject *parent)
    : QFile(parent)
    , _headerLength(0)
{
}


bool WavFile::open(const QString &fileName, OpenModeFlag openMode) {
    close();
    setFileName(fileName);
    if (openMode == QIODevice::ReadOnly)
        return QFile::open(openMode) && readHeader();
    else
        return QFile::open(openMode);
}


const QAudioFormat &WavFile::audioFormat() const {
    return _fileFormat;
}


qint64 WavFile::headerLength() const {
    return _headerLength;
}


bool WavFile::writeHeader(const quint32 sampleRate, const quint32 bitRate, const quint32 dataSize, bool stereo, bool floatFormat)
{
    CombinedHeader header;
    header.riff.descriptor.id[0] = 'R';
    header.riff.descriptor.id[1] = 'I';
    header.riff.descriptor.id[2] = 'F';
    header.riff.descriptor.id[3] = 'F';

    header.riff.type[0] = 'W';
    header.riff.type[1] = 'A';
    header.riff.type[2] = 'V';
    header.riff.type[3] = 'E';

    header.wave.descriptor.id[0]= 'f';
    header.wave.descriptor.id[1]= 'm';
    header.wave.descriptor.id[2]= 't';
    header.wave.descriptor.id[3]= ' ';

    header.wave.descriptor.size = 16;

    header.wave.audioFormat = floatFormat ? 3 : 1; //1-pcm 3-float wave
    header.wave.numChannels = stereo ? 2 : 1; //mono stereo
    header.wave.sampleRate = sampleRate;
    header.wave.bitsPerSample = bitRate; //16 //or 32 if float
    header.wave.byteRate = header.wave.numChannels*header.wave.sampleRate*header.wave.bitsPerSample/8;
    header.wave.blockAlign = header.wave.numChannels*header.wave.bitsPerSample/8;

    DATAHeader dataHeader;
    dataHeader.descriptor.id[0] = 'd';
    dataHeader.descriptor.id[1] = 'a';
    dataHeader.descriptor.id[2] = 't';
    dataHeader.descriptor.id[3] = 'a';

    dataHeader.descriptor.size = dataSize;

    header.riff.descriptor.size = dataHeader.descriptor.size + 36;
    if (floatFormat)
        header.riff.descriptor.size += 12;

    bool result = write(reinterpret_cast<char*>(&header),sizeof(CombinedHeader));

    if (floatFormat) {
        FACTHeader fact;
        fact.descriptor.id[0] = 'f';
        fact.descriptor.id[0] = 'a';
        fact.descriptor.id[0] = 'c';
        fact.descriptor.id[0] = 't';
        fact.descriptor.size = 4;
        fact.dwSampleLength = dataSize/(4);
        if (stereo)
            fact.dwSampleLength /= 2;
        result &= write(reinterpret_cast<char*>(&fact), sizeof(FACTHeader));
    }
    result &= write(reinterpret_cast<char*>(&dataHeader), sizeof(DATAHeader));
    return result;
}


bool WavFile::readHeader() {
    seek(0);
    CombinedHeader header;
    bool result = read(reinterpret_cast<char *>(&header), sizeof(CombinedHeader)) == sizeof(CombinedHeader);
    if (result) {
        if ((std::memcmp(&header.riff.descriptor.id, "RIFF", 4) == 0
            || std::memcmp(&header.riff.descriptor.id, "RIFX", 4) == 0)
            && std::memcmp(&header.riff.type, "WAVE", 4) == 0
            && std::memcmp(&header.wave.descriptor.id, "fmt ", 4) == 0
            && (header.wave.audioFormat == 1 || header.wave.audioFormat == 0)) {

            DATAHeader dataHeader;

            if (qFromLittleEndian<quint32>(header.wave.descriptor.size) > sizeof(WAVEHeader)) {
                quint16 extraFormatBytes;
                if (peek(reinterpret_cast<char*>(&extraFormatBytes), sizeof(quint16)) != sizeof(quint16))
                    return false;
                const qint64 throwAwayBytes = sizeof(quint16) + qFromLittleEndian<quint16>(extraFormatBytes);
                if (read(throwAwayBytes).size() != throwAwayBytes)
                    return false;
            }

            if (read(reinterpret_cast<char*>(&dataHeader), sizeof(DATAHeader)) != sizeof(DATAHeader))
                return false;

            while (std::memcmp(&dataHeader.descriptor.id, "data",4) != 0)
                if (read(reinterpret_cast<char*>(&dataHeader), sizeof(DATAHeader)) != sizeof(DATAHeader))
                    return false;

            if (std::memcmp(&header.riff.descriptor.id, "RIFF", 4) == 0)
                _fileFormat.setByteOrder(QAudioFormat::LittleEndian);
            else
                _fileFormat.setByteOrder(QAudioFormat::BigEndian);

            int bps = qFromLittleEndian<quint16>(header.wave.bitsPerSample);
            _fileFormat.setChannelCount(qFromLittleEndian<quint16>(header.wave.numChannels));
            _fileFormat.setCodec("audio/pcm");
            _fileFormat.setSampleRate(qFromLittleEndian<quint32>(header.wave.sampleRate));
            _fileFormat.setSampleSize(qFromLittleEndian<quint16>(header.wave.bitsPerSample));
            _fileFormat.setSampleType(bps == 8 ? QAudioFormat::UnSignedInt : QAudioFormat::SignedInt);
        }
        else
            result = false;
    }
    _headerLength = pos();
    return result;
}
