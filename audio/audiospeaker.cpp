#include "audiospeaker.h"
#include "audio/fft.h"
#include "audio/waveanalys.h"

#include <QDebug>
#include <QFile>
#include <QtEndian>


//const int BufferSize = 4096;

AudioInfo::AudioInfo(const QAudioFormat &format, QObject *parent)
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

AudioInfo::~AudioInfo()
{
}

void AudioInfo::start()
{ open(QIODevice::WriteOnly);}

void AudioInfo::stop()
{close();}

qint64 AudioInfo::readData(char *data, qint64 maxlen)
{   Q_UNUSED(data)
    Q_UNUSED(maxlen)

    //used for output data stream

    return 0;
}



qint64 AudioInfo::writeData(const char *data, qint64 len)
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


void AudioInfo::dump() {
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

AudioSpeaker::AudioSpeaker([[maybe_unused]]const QAudioFormat &format,
                     QObject *parent)
    :   QIODevice(parent)
    ,   m_pos(0)
{
}

AudioSpeaker::~AudioSpeaker()
{
}

void AudioSpeaker::start()
{
    open(QIODevice::ReadOnly);
}

void AudioSpeaker::stop()
{
    m_pos = 0;
    close();
}

void AudioSpeaker::generateData(const QAudioFormat &format, qint64 durationUs, int sampleRate)
{
    const int channelBytes = format.sampleSize() / 8;
    const int sampleBytes = format.channelCount() * channelBytes;

    qint64 length = (format.sampleRate() * format.channelCount() * (format.sampleSize() / 8))
                        * durationUs / 100000;

    Q_ASSERT(length % sampleBytes == 0);
    Q_UNUSED(sampleBytes); // suppress warning in release builds

    m_buffer.resize(length);
    unsigned char *ptr = reinterpret_cast<unsigned char *>(m_buffer.data());
    int sampleIndex = 0;

    while (length) {
        const qreal x = 0; //qSin(2 * M_PI * sampleRate * qreal(sampleIndex % format.sampleRate()) / format.sampleRate());
        for (int i=0; i<format.channelCount(); ++i) {
            if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::UnSignedInt) {
                const quint8 value = static_cast<quint8>((1.0 + x) / 2 * 255);
                *reinterpret_cast<quint8*>(ptr) = value;
            } else if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::SignedInt) {
                const qint8 value = static_cast<qint8>(x * 127);
                *reinterpret_cast<quint8*>(ptr) = value;
            } else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::UnSignedInt) {
                quint16 value = static_cast<quint16>((1.0 + x) / 2 * 65535);
                if (format.byteOrder() == QAudioFormat::LittleEndian)
                    qToLittleEndian<quint16>(value, ptr);
                else
                    qToBigEndian<quint16>(value, ptr);
            } else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::SignedInt) {
                qint16 value = static_cast<qint16>(x * 32767);
                if (format.byteOrder() == QAudioFormat::LittleEndian)
                    qToLittleEndian<qint16>(value, ptr);
                else
                    qToBigEndian<qint16>(value, ptr);
            }

            ptr += channelBytes;
            length -= channelBytes;
        }
        ++sampleIndex;
    }
}

qint64 AudioSpeaker::readData(char *data, qint64 len)
{
    qint64 total = 0;
    if (!m_buffer.isEmpty()) {
        while (len - total > 0) {
            const qint64 chunk = qMin((m_buffer.size() - m_pos), len - total);
            memcpy(data + total, m_buffer.constData() + m_pos, chunk);
            m_pos = (m_pos + chunk) % m_buffer.size();
            total += chunk;
        }
    }
    return total;
}

qint64 AudioSpeaker::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint64 AudioSpeaker::bytesAvailable() const
{
    return m_buffer.size() + QIODevice::bytesAvailable();
}


void AudioSpeaker::setAudioBufer(QByteArray &aStream)
{
    m_buffer.clear();
    m_buffer += aStream;
}
