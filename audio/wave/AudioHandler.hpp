#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <memory>

#include <QAudioOutput>
#include <QAudioInput>
#include <QTimer>

#include "AudioReceiver.hpp"
#include "AudioSpeaker.hpp"

#include "midi/MidiRender.hpp"

namespace aurals {


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

        Q_INVOKABLE void mixRecordAndMidi();
        Q_INVOKABLE void startMidiAndRecording();

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
        Q_INVOKABLE void setMidiPlayerSampleRate(const double sr) {
            _midiFormat.setSampleRate(sr);
            initMidiPlayer();
        }
        Q_INVOKABLE double getMidiPlayerSampleRate() const { return _midiFormat.sampleRate(); }

        Q_INVOKABLE void openMidiFile(const QString filename);
        Q_INVOKABLE void openTabFile(const QString filename);
        Q_INVOKABLE void saveMidiToWav(const QString filename) const;

        Q_INVOKABLE void openMidiFileEngine(const QString filename);
        Q_INVOKABLE void startMidiFileEngine();
        Q_INVOKABLE void stopMidiFileEngine();

        Q_INVOKABLE void changeMidiRenderVolume(const double db) { _render.setVolumeDb(db); }
        Q_INVOKABLE void changeMidiSoundfont(const QString filename) { _render.setSoundFont(filename); }
        Q_INVOKABLE void changeMidiSampleRate(const double sr) { _render.setSampleRate(sr);}

        Q_INVOKABLE QString getSoundfontFilename() const { return _render.getSoundFont(); }
        Q_INVOKABLE double getMidiVolume() const { return _render.getVolumeDb(); }
        Q_INVOKABLE double getMidiSampleRate() const { return _render.getSampleRate(); }

    private:

        void initAudioHandler();

        void initRecorder();
        void initPlayer();
        void initMidiPlayer();

        std::unique_ptr<QAudioInput> _audioInput;
        std::unique_ptr<AudioReceiver> _audioReceiver;

        std::unique_ptr<QAudioOutput> _audioOutput;
        std::unique_ptr<AudioSpeaker> _audioPlayer;
        QTimer _audioStopRequestTimer;

        QByteArray _commonBufer;
        QAudioFormat _commonFormat;

        size_t _prevBufferSize;

        bool _isPlaying = false;
        bool _isRecording = false;

        std::unique_ptr<QAudioOutput> _midiOutput;
        std::unique_ptr<AudioSpeaker> _midiPlayer;
        QTimer _midiStopRequestTimer;

        QByteArray _midiBufer;
        QAudioFormat _midiFormat;

        MidiRender _render;
    };

}


#endif // AUDIOHANDLER_H
