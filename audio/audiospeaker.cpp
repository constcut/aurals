#include "audiospeaker.h"

#include <QDebug>
#include <QFile>
#include <QTimer>

#include "audiohandler.h"


AudioSpeaker::AudioSpeaker(const QAudioFormat &format, QObject *parent, QByteArray& commonBufer)
    :   QIODevice(parent)
    ,   position(0)
    ,   audioFormat(format)
    ,   buffer(commonBufer)
{
}


AudioSpeaker::~AudioSpeaker()
{
}


void AudioSpeaker::start()
{
    qDebug() << "Pressed start " << buffer.size();
    int ms = buffer.size() / (2 * 8); //16 bit 8k samplerate - k is ms
    AudioHandler* handler = dynamic_cast<AudioHandler*>(this->parent());
    QTimer::singleShot(ms, handler, &AudioHandler::stopPlayback); //TODO stop player on level of handler
    open(QIODevice::ReadOnly);
}


void AudioSpeaker::stop()
{
    position = 0;
    close();
}


qint64 AudioSpeaker::readData(char *data, qint64 len)
{
    qDebug() << "R " << len;
    qint64 total = 0;
    if (!buffer.isEmpty()) {
        while (len - total > 0) {
            const qint64 chunk = qMin((buffer.size() - position), len - total);
            memcpy(data + total, buffer.constData() + position, chunk); //TODO std
            position = (position + chunk) % buffer.size();
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
    return buffer.size() + QIODevice::bytesAvailable();
}




