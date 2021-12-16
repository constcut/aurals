#include "audioreceiver.h"

#include "audiospeaker.h"
#include "audio/fft.h"
#include "audio/waveanalys.h"

#include <QDebug>
#include <QFile>
#include <QTimer>

#include "audiohandler.h" //TODO interface

//const int BufferSize = 4096;

AudioReceiver::AudioReceiver(const QAudioFormat& format, QObject *parent, QByteArray& commonBufer)
    :   QIODevice(parent)
    ,   audioFormat(format)
    ,   bufer(commonBufer)
    ,   maxAmplitude(0)
    ,   level(0.0)

{
    switch (format.sampleSize()) {
    case 8:
        switch (format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            maxAmplitude = 255;
            break;
        case QAudioFormat::SignedInt:
            maxAmplitude = 127;
            break;
        default:
            break;
        }
        break;
    case 16:
        switch (format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            maxAmplitude = 65535;
            break;
        case QAudioFormat::SignedInt:
            maxAmplitude = 32767;
            break;
        default:
            break;
        }
        break;

    case 32:
        switch (format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            maxAmplitude = 0xffffffff;
            break;
        case QAudioFormat::SignedInt:
            maxAmplitude = 0x7fffffff;
            break;
        case QAudioFormat::Float:
            maxAmplitude = 0x7fffffff; // Kind of
        default:
            break;
        }
        break;

    default:
        break;
    }
}



void AudioReceiver::start(){
    open(QIODevice::WriteOnly);
    //int border = audioFormat.sampleRate() * (audioFormat.sampleSize() / 8) * 10 //TODO check is buffer already bit in handler
    AudioHandler* handler = dynamic_cast<AudioHandler*>(this->parent());
    QTimer::singleShot(10000, handler, &AudioHandler::requestStopRecord);
}


void AudioReceiver::stop() {
    close();
}


qint64 AudioReceiver::readData(char *data, qint64 maxlen)
{   Q_UNUSED(data)
    Q_UNUSED(maxlen)
    return 0;
}



qint64 AudioReceiver::writeData(const char *data, qint64 len)
{
    bufer += QByteArray(data,len);

    static int lastSize = 0;

    if (bufer.size() - lastSize > 4100) {

        lastSize = bufer.size();
        short *sourceData = (short*)bufer.data();

        int fullSize = bufer.size()/2;
        int minusLastFrame = fullSize-2049;

        FFT fft(2048);
        fft.transform(&sourceData[minusLastFrame]);
        fft.countAmplitude();
        fft.findPeaks(7.8125/2.0);
        std::vector<Peak> *peaks = &fft.getPeaks();

        LocalFreqTable localFreq;
        localFreq.addPeaks(peaks);
        localFreq.sortPeaks();
        localFreq.voteNewWay();

        std::vector<LocalFreqTable::LFTvote> *votes = localFreq.getRezultVotes();

        double freq = (*votes)[0].rFreq;
    }

    /*
    int border = audioFormat.sampleRate() * (audioFormat.sampleSize() / 8) * 10; // TODO each 60 seconds in smallest

    if (bufer.size() > border) {
        AudioHandler* handler = dynamic_cast<AudioHandler*>(this->parent());
        handler->requestStopRecord();
    }
    */

    return len;
}
