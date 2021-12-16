#include "audioreceiver.h"

#include "audiospeaker.h"
#include "audio/fft.h"
#include "audio/waveanalys.h"

#include <QDebug>
#include <QFile>

//const int BufferSize = 4096;

AudioReceiver::AudioReceiver(const QAudioFormat &format, QObject *parent)
    :   QIODevice(parent)
    ,   m_format(format)
    ,   m_maxAmplitude(0)
    ,   m_level(0.0)

{
    switch (m_format.sampleSize()) {
    case 8:
        switch (m_format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 255;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 127;
            break;
        default:
            break;
        }
        break;
    case 16:
        switch (m_format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 65535;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 32767;
            break;
        default:
            break;
        }
        break;

    case 32:
        switch (m_format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 0xffffffff;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 0x7fffffff;
            break;
        case QAudioFormat::Float:
            m_maxAmplitude = 0x7fffffff; // Kind of
        default:
            break;
        }
        break;

    default:
        break;
    }
}

AudioReceiver::~AudioReceiver()
{
}

void AudioReceiver::start()
{ open(QIODevice::WriteOnly);}

void AudioReceiver::stop()
{close();}

qint64 AudioReceiver::readData(char *data, qint64 maxlen)
{   Q_UNUSED(data)
    Q_UNUSED(maxlen)

    //used for output data stream

    return 0;
}



qint64 AudioReceiver::writeData(const char *data, qint64 len)
{

    collector += QByteArray(data,len);

    static int lastSize = 0;

    if (collector.size() - lastSize > 4100) {

        lastSize = collector.size();
        short *sourceData = (short*)collector.data();

        int fullSize = collector.size()/2;
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

    if (collector.size() > border*10) //format*bitrate*minute
    {
        //TODO stop if more then minute

        ///QByteArray compress = qCompress(collector,7);
        QString defaultRecFile = QString("record.temp");
        QFile f; f.setFileName(defaultRecFile);
        ///int compressedSize = compress.size();
        if (f.open(QIODevice::Append))
        {
            f.write(collector);
            f.flush();
            f.close();
        }
        else
            qDebug() << "Open file for raw record error;";

        collector.clear();
    }

    int fullLen = collector.size();
    int cutLen = len;

    qDebug() << "Wroten audio data "<<cutLen<<"; in bufer "<<fullLen;
    return len;
}


void AudioReceiver::dump() {
    ///QByteArray compress = qCompress(collector,7);
    QString defaultRecFile = QString("record.temp");
    QFile f; f.setFileName(defaultRecFile);
    ///int compressedSize = compress.size();
    if (f.open(QIODevice::Append))
    {
        f.write(collector);
        f.flush();
        f.close();
    }
    else
        qDebug() << "Open file for raw record error;";

    collector.clear();
}

/// AUDIO INPUT FINISHED
// NOW OUTPUT
