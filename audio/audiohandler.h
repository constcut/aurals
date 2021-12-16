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

    Q_INVOKABLE void deleteDump();

private:

    void initRecorder();
    void initPlayer();

    std::unique_ptr<QAudioInput> audioInput;
    std::unique_ptr<AudioReceiver> audioReceiver;

    std::unique_ptr<QAudioOutput> audioOutput;
    std::unique_ptr<AudioSpeaker> audioPlayer;

};


#endif // AUDIOHANDLER_H
