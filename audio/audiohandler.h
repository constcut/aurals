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
    Q_INVOKABLE void saveFile(QString filename) const;
    Q_INVOKABLE void loadWavFile(QString filename);
    Q_INVOKABLE void saveWavFile(QString filename) const;


    Q_INVOKABLE int getSampleRate() const {
        return commonFormat.sampleRate();
    }
    Q_INVOKABLE int getBitRate() const {
        return commonFormat.sampleSize();
    }

    Q_INVOKABLE void setSampleRate(int newSampleRate);

    Q_INVOKABLE void requestPermission() const;

    //TODO возможно выделить отдельно
    Q_INVOKABLE QStringList getRecords() const;
    Q_INVOKABLE void saveRecordTimstamp();
    Q_INVOKABLE void deleteRecord(QString filename) const;
    Q_INVOKABLE void renameRecord(QString filename, QString newFilename) const;

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

    size_t prevPosition;

    bool isPlaying = false;
    bool isRecording = false;
};


#endif // AUDIOHANDLER_H
