#ifndef AUDIOSPEAKER_H
#define AUDIOSPEAKER_H

#include <QIODevice>
#include <QAudioFormat>



class AudioSpeaker : public QIODevice
{
    Q_OBJECT

public:
    AudioSpeaker(const QAudioFormat &format, QObject *parent, QByteArray& commonBufer);
    ~AudioSpeaker();

    void start();
    void stop();

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;

    void setAudioBufer(QByteArray &aStream);

private:
    qint64 position;
    QAudioFormat audioFormat;
    QByteArray buffer;
};




#endif // AUDIOSPEAKER_H
