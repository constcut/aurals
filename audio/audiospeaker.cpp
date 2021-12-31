#include "audiospeaker.h"

#include <QDebug>
#include <QFile>
#include <QTimer>

#include <cstring>

#include "audiohandler.h"


AudioSpeaker::AudioSpeaker(const QAudioFormat &format, QObject *parent, QByteArray& commonBufer)
    :   QIODevice(parent)
    ,   _position(0)
    ,   _audioFormat(format)
    ,   _buffer(commonBufer)
{
}


void AudioSpeaker::start() {
    open(QIODevice::ReadOnly);
}


void AudioSpeaker::stop() {
    _position = 0;
    close();
}


qint64 AudioSpeaker::readData(char *data, const qint64 len)
{
    qint64 total = 0;
    if (!_buffer.isEmpty()) {
        while (len - total > 0) {
            const qint64 chunk = qMin((_buffer.size() - _position), len - total);
            std::memcpy(data + total, _buffer.constData() + _position, chunk);
            _position = (_position + chunk) % _buffer.size();
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


qint64 AudioSpeaker::bytesAvailable() const {
    return _buffer.size() + QIODevice::bytesAvailable();
}


