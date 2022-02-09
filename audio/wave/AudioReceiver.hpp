#ifndef AUDIORECEIVER_H
#define AUDIORECEIVER_H

#include <QIODevice>
#include <QAudioFormat>

namespace aurals {

    class AudioReceiver : public QIODevice
    {
        Q_OBJECT

    public:
        AudioReceiver(const QAudioFormat& format, QObject *parent, QByteArray& commonBufer);
        ~AudioReceiver() = default;

        void start();
        void stop();

        qreal normLevel() const { return _level; }

        qint64 readData(char *data, const qint64 maxlen);
        qint64 writeData(const char *data, const qint64 len);


    private:

        const QAudioFormat _audioFormat;
        QByteArray& _bufer;
        quint32 _maxAmplitude;
        qreal _level; // 0.0 <= m_level <= 1.0

        const quint32 msToStopRecord = 30000;

    signals:
        void update();
    };

}

#endif // AUDIORECEIVER_H
