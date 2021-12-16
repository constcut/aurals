#ifndef AUDIORECEIVER_H
#define AUDIORECEIVER_H

#include <QIODevice>
#include <QAudioFormat>



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


#endif // AUDIORECEIVER_H
