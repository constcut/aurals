#ifndef AUDIOSPEAKER_H
#define AUDIOSPEAKER_H

#include <QIODevice>
#include <QAudioFormat>


namespace aurals {

    class AudioSpeaker : public QIODevice
    {
        Q_OBJECT

    public:
        AudioSpeaker(const QAudioFormat &format, QObject *parent, QByteArray& commonBufer);
        ~AudioSpeaker() = default;

        void start();
        void stop();

        qint64 readData(char *data, const qint64 maxlen);
        qint64 writeData(const char *data,const qint64 len);
        qint64 bytesAvailable() const;

    private:

        qint64 _position;
        QAudioFormat _audioFormat;
        QByteArray& _buffer;
    };

}


#endif // AUDIOSPEAKER_H
