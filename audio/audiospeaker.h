#ifndef AUDIOSPEAKER_H
#define AUDIOSPEAKER_H

#include <QIODevice>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioInput>

#include <memory>


class AudioReceiver : public QIODevice
{
    Q_OBJECT

protected:

public: //TODO
    QByteArray collector;

public:
    AudioReceiver(const QAudioFormat &format, QObject *parent);
    ~AudioReceiver();

    void start();
    void stop();

    qreal level() const { return m_level; }

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

    void dump();

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
    qint64 m_pos;
    QByteArray m_buffer;
};


class AudioHandler : public QObject
{
    Q_OBJECT

public:
    AudioHandler();

    Q_INVOKABLE void startRecord();
    Q_INVOKABLE void stopRecord();

    Q_INVOKABLE void startPlayback();
    Q_INVOKABLE void stopPlayback();

    Q_INVOKABLE void deleteDump();

private:

    void initRecorder();
    void initPlayer();

    std::unique_ptr<QAudioInput> audioInput;
    std::unique_ptr<AudioReceiver> audioReceiver;

    std::unique_ptr<QAudioOutput> audioOutput;
    std::unique_ptr<AudioSpeaker> audioPlayer;

};


#endif // AUDIOSPEAKER_H
