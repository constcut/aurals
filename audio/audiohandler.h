#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <QAudioOutput>
#include <QAudioInput>
#include <memory>

#include "audioreceiver.h"
#include "audiospeaker.h"


class AudioHandler : public QObject
{
    Q_OBJECT

public:
    AudioHandler();

    Q_INVOKABLE void startRecord();
    Q_INVOKABLE void stopRecord();

    Q_INVOKABLE void startPlayback();
    Q_INVOKABLE void stopPlayback();

    Q_INVOKABLE void resetBufer();

    Q_INVOKABLE void loadFile(QString filename);
    Q_INVOKABLE void saveFile(QString filename);

    Q_INVOKABLE int getSampleRate() {
        return commonFormat.sampleRate();
    }
    Q_INVOKABLE int getBitRate() {
        return commonFormat.sampleSize();
    }

    Q_INVOKABLE void setSampleRate(int newSampleRate);
    Q_INVOKABLE void setBitRate(int newBitRate);

    Q_INVOKABLE void requestPermission();

    void requestStopRecord();
    void requestStopPlayback();

private:

    void initRecorder();
    void initPlayer();

    std::unique_ptr<QAudioInput> audioInput;
    std::unique_ptr<AudioReceiver> audioReceiver;

    std::unique_ptr<QAudioOutput> audioOutput;
    std::unique_ptr<AudioSpeaker> audioPlayer;

    QByteArray commonBufer;
    QAudioFormat commonFormat;

};


#endif // AUDIOHANDLER_H
