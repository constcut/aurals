#include "audioreceiver.h"

#include "audiospeaker.h"
#include "audio/fft.h"
#include "audio/waveanalys.h"

#include <QDebug>
#include <QFile>

//const int BufferSize = 4096;

AudioReceiver::AudioReceiver(const QAudioFormat& format, QObject *parent, QByteArray& commonBufer)
    :   QIODevice(parent)
    ,   format(format)
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


    int border = 16000*30; //each 60 seconds in smallest

    if (bufer.size() > border*10) //format*bitrate*minute
    {
        //TODO stop if more then minute on audio format
    }

    int fullLen = bufer.size();
        qDebug() << "r " << fullLen;

    return len;
}


void AudioReceiver::dump() {
    ///QByteArray compress = qCompress(collector,7);
    QString defaultRecFile = QString("record.temp");
    QFile f; f.setFileName(defaultRecFile);
    ///int compressedSize = compress.size();
    if (f.open(QIODevice::Append))
    {
        f.write(bufer);
        f.flush();
        f.close();
    }
    else
        qDebug() << "Open file for raw record error;";

   // collector.clear();
}

/// AUDIO INPUT FINISHED
// NOW OUTPUT
