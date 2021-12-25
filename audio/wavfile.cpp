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

#include <qendian.h>
#include <QVector>
#include <QDebug>
#include "wavfile.h"


struct chunk
{
    char        id[4];
    quint32     size;
};


struct RIFFHeader
{
    chunk       descriptor;     // "RIFF"
    char        type[4];        // "WAVE"
};


struct WAVEHeader
{
    chunk       descriptor;
    quint16     audioFormat;
    quint16     numChannels;
    quint32     sampleRate;
    quint32     byteRate;
    quint16     blockAlign;
    quint16     bitsPerSample;
};


struct DATAHeader
{
    chunk       descriptor;
};


struct FACTHeader
{
    chunk       descriptor;
    quint32 dwSampleLength; //maybe nice to look into specification and name after them everything
};


struct CombinedHeader
{
    RIFFHeader  riff;
    WAVEHeader  wave;
};


WavFile::WavFile(QObject *parent)
    : QFile(parent)
    , m_headerLength(0)
{

}


bool WavFile::open(const QString &fileName, OpenModeFlag openMode) {
    close();
    setFileName(fileName);
    if (openMode == QIODevice::ReadOnly)
        return QFile::open(openMode) && readHeader();
    else
    {
        return QFile::open(openMode);
    }
}


const QAudioFormat &WavFile::audioFormat() const
{
    return m_fileFormat;
}


qint64 WavFile::headerLength() const
{
    return m_headerLength;
}


void WavFile::writeHeader(unsigned int sampleRate, unsigned int bitRate, unsigned long dataSize, bool stereo, bool floatFormat)
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
    header.wave.numChannels = stereo ? 2 : 1;
    header.wave.sampleRate = sampleRate; //or other
    header.wave.bitsPerSample = bitRate; //16 //or 32 if float
    header.wave.byteRate = header.wave.numChannels*header.wave.sampleRate*header.wave.bitsPerSample/8;
    header.wave.blockAlign = header.wave.numChannels*header.wave.bitsPerSample/8;
    //IN FILE ITS HERE: //header.wave.bitsPerSample = 16; //or 24

    DATAHeader dataHeader;
    dataHeader.descriptor.id[0] = 'd';
    dataHeader.descriptor.id[1] = 'a';
    dataHeader.descriptor.id[2] = 't';
    dataHeader.descriptor.id[3] = 'a';

    dataHeader.descriptor.size = dataSize;
    //ch2 size == NumSamples * NumChannels * BitsPerSample/8

    header.riff.descriptor.size = dataHeader.descriptor.size + 36;
    if (floatFormat)
        header.riff.descriptor.size += 12;
    /// 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)

    bool result = write(reinterpret_cast<char*>(&header),sizeof(CombinedHeader));

    ///IF THERE IS A FLOAT FORMAT THEN fact 4 dwSampleLength	4	Number of samples (per channel)
    if (floatFormat) {
        FACTHeader fact;
        fact.descriptor.id[0] = 'f';
        fact.descriptor.id[0] = 'a';
        fact.descriptor.id[0] = 'c';
        fact.descriptor.id[0] = 't';
        fact.descriptor.size = 4; //next bytes
        fact.dwSampleLength = dataSize/(4); //4 bytes per float

        if (stereo)
            fact.dwSampleLength /= 2;

        result &= write(reinterpret_cast<char*>(&fact),sizeof(FACTHeader));

    }

    result &= write(reinterpret_cast<char*>(&dataHeader),sizeof(DATAHeader));

}

bool WavFile::readHeader() //this function looks too complicated rewrite in our own manner
{
    seek(0);
    CombinedHeader header;
    bool result = read(reinterpret_cast<char *>(&header), sizeof(CombinedHeader)) == sizeof(CombinedHeader);
    if (result) {
        if ((memcmp(&header.riff.descriptor.id, "RIFF", 4) == 0
            || memcmp(&header.riff.descriptor.id, "RIFX", 4) == 0)
            && memcmp(&header.riff.type, "WAVE", 4) == 0
            && memcmp(&header.wave.descriptor.id, "fmt ", 4) == 0
            && (header.wave.audioFormat == 1 || header.wave.audioFormat == 0)) {

            // Read off remaining header information
            DATAHeader dataHeader;

            if (qFromLittleEndian<quint32>(header.wave.descriptor.size) > sizeof(WAVEHeader)) {
                // Extended data available
                quint16 extraFormatBytes;
                if (peek((char*)&extraFormatBytes, sizeof(quint16)) != sizeof(quint16))
                    return false;
                const qint64 throwAwayBytes = sizeof(quint16) + qFromLittleEndian<quint16>(extraFormatBytes);
                if (read(throwAwayBytes).size() != throwAwayBytes)
                    return false;
            }

            if (read((char*)&dataHeader, sizeof(DATAHeader)) != sizeof(DATAHeader))
                return false;

            while (memcmp(&dataHeader.descriptor.id,"data",4) != 0)
            {
                if (read((char*)&dataHeader, sizeof(DATAHeader)) != sizeof(DATAHeader))
                    return false;
            }

            // Establish format
            if (memcmp(&header.riff.descriptor.id, "RIFF", 4) == 0)
                m_fileFormat.setByteOrder(QAudioFormat::LittleEndian);
            else
                m_fileFormat.setByteOrder(QAudioFormat::BigEndian);

            int bps = qFromLittleEndian<quint16>(header.wave.bitsPerSample);
            m_fileFormat.setChannelCount(qFromLittleEndian<quint16>(header.wave.numChannels));
            m_fileFormat.setCodec("audio/pcm");
            m_fileFormat.setSampleRate(qFromLittleEndian<quint32>(header.wave.sampleRate));
            m_fileFormat.setSampleSize(qFromLittleEndian<quint16>(header.wave.bitsPerSample));
            m_fileFormat.setSampleType(bps == 8 ? QAudioFormat::UnSignedInt : QAudioFormat::SignedInt);
        } else {
            result = false;
        }
    }
    m_headerLength = pos();
    return result;
}
