#ifndef AUDIOSPEAKER_H
#define AUDIOSPEAKER_H

#include <QIODevice>
#include <QAudioFormat>


class AudioInfo : public QIODevice
{
    Q_OBJECT

protected:

public: //temp operations
    QByteArray collector;

public:
    AudioInfo(const QAudioFormat &format, QObject *parent);
    ~AudioInfo();

    void start();
    void stop();

    qreal level() const { return m_level; }

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
    const QAudioFormat m_format;
    quint32 m_maxAmplitude;
    qreal m_level; // 0.0 <= m_level <= 1.0

signals:
    void update();
};


class AudioSpeaker : public QIODevice
{
    Q_OBJECT

public:
    AudioSpeaker(const QAudioFormat &format, QObject *parent=0);
    ~AudioSpeaker();

    void start();
    void stop();

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;

    void setAudioBufer(QByteArray &aStream);

private:
    void generateData(const QAudioFormat &format, qint64 durationUs, int sampleRate);

private:
    qint64 m_pos;
    QByteArray m_buffer;
};



#endif // AUDIOSPEAKER_H
