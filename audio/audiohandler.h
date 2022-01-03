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

    Q_INVOKABLE void startMidiPlayer();
    Q_INVOKABLE void stopMidiPlayer();

    Q_INVOKABLE void loadFile(QString filename);
    Q_INVOKABLE void saveFile(QString filename) const;
    Q_INVOKABLE void loadWavFile(QString filename);
    Q_INVOKABLE void saveWavFile(QString filename) const;
    Q_INVOKABLE void loadOnlyWindow(QString filename, quint64 position, quint64 window);
    Q_INVOKABLE void loadWindowPCM(QByteArray window);

    Q_INVOKABLE int getSampleRate() const {
        return _commonFormat.sampleRate();
    }
    Q_INVOKABLE int getBitRate() const {
        return _commonFormat.sampleSize();
    }

    Q_INVOKABLE void setSampleRate(int newSampleRate);
    Q_INVOKABLE void requestPermission() const;

    Q_INVOKABLE QStringList getRecords() const;
    Q_INVOKABLE void saveRecordTimstamp() const;
    Q_INVOKABLE void deleteRecord(QString filename) const;
    Q_INVOKABLE void renameRecord(QString filename, QString newFilename) const;

    void requestStopRecord();
    void requestStopPlayback();

    Q_INVOKABLE void checkMidi();

private:

    void initRecorder();
    void initPlayer();
    void initMidiPlayer();

    std::unique_ptr<QAudioInput> _audioInput;
    std::unique_ptr<AudioReceiver> _audioReceiver;

    std::unique_ptr<QAudioOutput> _audioOutput;
    std::unique_ptr<AudioSpeaker> _audioPlayer;

    QByteArray _commonBufer;
    QAudioFormat _commonFormat;

    size_t _prevBufferSize;

    bool _isPlaying = false;
    bool _isRecording = false;

    std::unique_ptr<QAudioOutput> _midiOutput;
    std::unique_ptr<AudioSpeaker> _midiPlayer;

    QByteArray _midiBufer;
    QAudioFormat _midiFormat;
};


#endif // AUDIOHANDLER_H
