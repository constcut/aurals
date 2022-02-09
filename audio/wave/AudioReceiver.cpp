#include "AudioReceiver.hpp"

#include <QTimer>

#include "AudioHandler.hpp"

using namespace aurals;

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
    QTimer::singleShot(msToStopRecord, handler, &AudioHandler::requestStopRecord);
}


void AudioReceiver::stop() {
    close();
}


qint64 AudioReceiver::readData(char *data, const qint64 maxlen)
{   Q_UNUSED(data)
    Q_UNUSED(maxlen)
    return 0;
}


qint64 AudioReceiver::writeData(const char *data, const qint64 len) {
    _bufer += QByteArray(data,len);
    return len;
}
