#ifndef AUDIORECEIVER_H
#define AUDIORECEIVER_H

#include <QIODevice>
#include <QAudioFormat>



class AudioReceiver : public QIODevice
{
    Q_OBJECT

public:
    AudioReceiver(const QAudioFormat& format, QObject *parent, QByteArray& commonBufer);
    ~AudioReceiver() = default;

    void start();
    void stop();

    qreal normLevel() const { return level; }

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);


private:
    const QAudioFormat audioFormat;
    QByteArray& bufer;
    quint32 maxAmplitude;
    qreal level; // 0.0 <= m_level <= 1.0

signals:
    void update();
};


#endif // AUDIORECEIVER_H
