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
    ,   _audioFormat(format)
    ,   _bufer(commonBufer)
    ,   _maxAmplitude(0)
    ,   _level(0.0)

{
    switch (format.sampleSize()) {
    case 8:
        switch (format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            _maxAmplitude = 255;
            break;
        case QAudioFormat::SignedInt:
            _maxAmplitude = 127;
            break;
        default:
            break;
        }
        break;
    case 16:
        switch (format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            _maxAmplitude = 65535;
            break;
        case QAudioFormat::SignedInt:
            _maxAmplitude = 32767;
            break;
        default:
            break;
        }
        break;

    case 32:
        switch (format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            _maxAmplitude = 0xffffffff;
            break;
        case QAudioFormat::SignedInt:
            _maxAmplitude = 0x7fffffff;
            break;
        case QAudioFormat::Float:
            _maxAmplitude = 0x7fffffff; // Kind of
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
    AudioHandler* handler = dynamic_cast<AudioHandler*>(this->parent());
    QTimer::singleShot(_msToStopRecord, handler, &AudioHandler::requestStopRecord);
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
    _bufer += QByteArray(data,len);
    static int lastSize = 0;
    lastSize = _bufer.size();

    //TODO here maybe realtime FFT\YIN\RMS

    if (_bufer.size() - lastSize > 4100) {

        lastSize = _bufer.size();
        short *sourceData = (short*)_bufer.data();

        int fullSize = _bufer.size()/2;
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

    return len;
}
