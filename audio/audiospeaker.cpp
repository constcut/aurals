#include "audiospeaker.h"
#include "audio/fft.h"
#include "audio/waveanalys.h"

#include <QDebug>
#include <QFile>
#include <QtEndian>


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


AudioHandler::AudioHandler() {
    initRecorder();
    initPlayer();
    //TODO set params + reset
}



void AudioHandler::startRecord() {
    //TODO request permission
    audioReceiver->start();
    audioInput->start(audioReceiver.get());
}

void AudioHandler::stopRecord() {
    audioReceiver->stop();
    audioInput->stop();
    audioReceiver->dump();
}

void AudioHandler::startPlayback() {
    QFile audioFile;
    QString defaultRecFile = "record.temp";
    audioFile.setFileName(defaultRecFile);

    if (audioFile.open(QIODevice::ReadOnly) == false)
        qDebug() << "Failed to open audio for output";

    QByteArray allBytes = audioFile.readAll();
    audioFile.close();

    audioPlayer->setAudioBufer(allBytes); //TODO хранить в данном классе, общий для IO
    audioPlayer->start();
    audioOutput->start(audioPlayer.get());
}

void AudioHandler::stopPlayback() {
    audioPlayer->stop();
    audioOutput->stop();
}

void AudioHandler::initRecorder() {
    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    if (!info.isFormatSupported(format)) {
        qDebug() << "Default format not supported - trying to use nearest";
        format = info.nearestFormat(format);
    }

    audioReceiver  = std::make_unique<AudioReceiver>(format, nullptr);
    //connect(audioInfo, SIGNAL(update()), SLOT(refreshDisplay()));
    audioInput = std::make_unique<QAudioInput>(QAudioDeviceInfo::defaultInputDevice(), format, nullptr);
}

void AudioHandler::initPlayer() {
    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1); //TODO for pcm but will have issue with records
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    if (!info.isFormatSupported(format)) {
        qDebug() << "Default format not supported - trying to use nearest";
        format = info.nearestFormat(format);
    }

    audioPlayer = std::make_unique<AudioSpeaker>(format);
    audioOutput = std::make_unique<QAudioOutput>(QAudioDeviceInfo::defaultOutputDevice(), format, nullptr);

}


void AudioHandler::deleteDump() {
    QFile audioFile;
    QString defaultRecFile = "record.temp";
    audioFile.setFileName(defaultRecFile);
    audioFile.remove();

}
