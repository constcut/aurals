#include "audiospeaker.h"

#include <QDebug>
#include <QFile>
#include <QTimer>

#include "audiohandler.h"


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
    int ms = m_buffer.size() / (2 * 8); //16 bit 8k samplerate - k is ms
    AudioHandler* handler = dynamic_cast<AudioHandler*>(this->parent());
    QTimer::singleShot(ms, handler, &AudioHandler::stopPlayback); //TODO stop player on level of handler
    open(QIODevice::ReadOnly);
}

void AudioSpeaker::stop()
{
    m_pos = 0;
    close();
}


qint64 AudioSpeaker::readData(char *data, qint64 len)
{
    qDebug() << "Reading " << len;

    qint64 total = 0;
    if (!m_buffer.isEmpty()) {
        while (len - total > 0) {
            const qint64 chunk = qMin((m_buffer.size() - m_pos), len - total);
            memcpy(data + total, m_buffer.constData() + m_pos, chunk); //TODO std
            m_pos = (m_pos + chunk) % m_buffer.size();
            total += chunk;
        }
    }

    qDebug() << "Total " << total;
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


