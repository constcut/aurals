#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <QAudioOutput>
#include <QAudioInput>
#include <memory>

#include "AudioReceiver.hpp"
#include "AudioSpeaker.hpp"
#include "midi/MidiRender.hpp"


namespace mtherapp {


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

        Q_INVOKABLE void loadFile(const QString filename);
        Q_INVOKABLE void saveFile(const QString filename) const;
        Q_INVOKABLE void loadWavFile(const QString filename);
        Q_INVOKABLE void saveWavFile(const QString filename) const;
        Q_INVOKABLE void loadOnlyWindow(const QString filename, const quint64 position, const quint64 window);
        Q_INVOKABLE void loadWindowPCM(const QByteArray window);

        Q_INVOKABLE int getSampleRate() const {
            return _commonFormat.sampleRate();
        }
        Q_INVOKABLE int getBitRate() const {
            return _commonFormat.sampleSize();
        }

        Q_INVOKABLE void setSampleRate(const int newSampleRate);
        Q_INVOKABLE void requestPermission() const;

        Q_INVOKABLE QStringList getRecords() const;
        Q_INVOKABLE void saveRecordTimstamp() const;
        Q_INVOKABLE void deleteRecord(const QString filename) const;
        Q_INVOKABLE void renameRecord(const QString filename, const QString newFilename) const;

        void requestStopRecord();
        void requestStopPlayback();
        void requestStopMidi();

        Q_INVOKABLE void startMidiPlayer();
        Q_INVOKABLE void stopMidiPlayer();

        Q_INVOKABLE void checkMidi();

        Q_INVOKABLE void openMidiFile(const QString filename);
        Q_INVOKABLE void saveMidiToWav(const QString filename) const;

        Q_INVOKABLE void changeMidiRenderVolume(const double db);
        Q_INVOKABLE void changeMidiSoundfont(const QString filename);

        Q_INVOKABLE QString getSoundfontFilename() const { return _soundfontFile; }
        Q_INVOKABLE double getMidiVolume() const { return _midiVolumeDb; }

    private:

        void initRecorder();
        void initPlayer();
        void initMidiPlayer();

        void loadSoundfont();

        std::unique_ptr<QAudioInput> _audioInput;
        std::unique_ptr<AudioReceiver> _audioReceiver;

        std::unique_ptr<QAudioOutput> _audioOutput;
        std::unique_ptr<AudioSpeaker> _audioPlayer;

        QByteArray _commonBufer;
        QAudioFormat _commonFormat;

        size_t _prevBufferSize;

        bool _isPlaying = false;
        bool _isRecording = false;

        std::unique_ptr<MidiRender> _midiRender;
        double _midiVolumeDb = -6.0;
        QString _soundfontFile = "piano.sf2";

        std::unique_ptr<QAudioOutput> _midiOutput;
        std::unique_ptr<AudioSpeaker> _midiPlayer;

        QByteArray _midiBufer;
        QAudioFormat _midiFormat;
    };

}


#endif // AUDIOHANDLER_H
