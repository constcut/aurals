#include "audiohandler.h"

#include <QFile>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QTimer>

#include "app/androidtools.h"
#include "wavfile.h"

#include "music/midirender.h"


AudioHandler::AudioHandler() {
    _commonFormat.setSampleRate(44100);
    _commonFormat.setChannelCount(1);
    _commonFormat.setSampleSize(16); //The only format old Qt accepts on android :(
    _commonFormat.setSampleType(QAudioFormat::SignedInt);
    _commonFormat.setByteOrder(QAudioFormat::LittleEndian);
    _commonFormat.setCodec("audio/pcm");
    initRecorder();
    initPlayer();
    initMidiPlayer();
}


void AudioHandler::startRecord() {
    if (_isPlaying || _isRecording)
        return;
    _isRecording = true;
    _prevBufferSize = _commonBufer.size();
    _audioReceiver->start();
    _audioInput->start(_audioReceiver.get());
}


void AudioHandler::stopRecord() {
    _audioReceiver->stop();
    _audioInput->stop();
    _isRecording = false;
}


void AudioHandler::startPlayback() {
    if (_isPlaying || _isRecording)
        return;
    _isPlaying = true;
    const double sampleRate = _commonFormat.sampleRate();
    const double bitRate = _commonFormat.sampleSize();
    const double bytesPerSample = bitRate / 8.0;
    const double msInSecond = 1000.0;
    const double ms = static_cast<double>(_commonBufer.size()) / (bytesPerSample * sampleRate / msInSecond);
    _audioPlayer->start();
    _audioOutput->start(_audioPlayer.get());
    QTimer::singleShot(ms, this, &AudioHandler::requestStopPlayback);
}


void AudioHandler::stopPlayback() {
    _audioPlayer->stop();
    _audioOutput->stop();
    _isPlaying = false;
}


void AudioHandler::initMidiPlayer() {
    _midiFormat.setSampleRate(44100);
    _midiFormat.setChannelCount(2);
    _midiFormat.setSampleSize(16);
    _midiFormat.setSampleType(QAudioFormat::SignedInt);
    _midiFormat.setByteOrder(QAudioFormat::LittleEndian);
    _midiFormat.setCodec("audio/pcm");
    _midiPlayer = std::make_unique<AudioSpeaker>(_midiFormat, this, _midiBufer);
    _midiOutput = std::make_unique<QAudioOutput>(QAudioDeviceInfo::defaultOutputDevice(), _midiFormat, nullptr);
}


void AudioHandler::initRecorder() {
    if (_isPlaying || _isRecording)
        return;
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    if (!info.isFormatSupported(_commonFormat)) {
        qDebug() << "Default format not supported - trying to use nearest";
        _commonFormat = info.nearestFormat(_commonFormat);
        qDebug() << _commonFormat.sampleRate() << " " << _commonFormat.sampleSize();
    }
    _audioReceiver  = std::make_unique<AudioReceiver>(_commonFormat, this, _commonBufer); //    //connect(audioInfo, SIGNAL(update()), SLOT(refreshDisplay()));
    _audioInput = std::make_unique<QAudioInput>(QAudioDeviceInfo::defaultInputDevice(), _commonFormat, nullptr);
}


void AudioHandler::initPlayer() {
    if (_isPlaying || _isRecording)
        return;
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    if (!info.isFormatSupported(_commonFormat)) {
        qDebug() << "Default format not supported - trying to use nearest";
        _commonFormat = info.nearestFormat(_commonFormat);
        qDebug() << _commonFormat.sampleRate() << " " << _commonFormat.sampleSize();
    }
    _audioPlayer = std::make_unique<AudioSpeaker>(_commonFormat, this, _commonBufer);
    _audioOutput = std::make_unique<QAudioOutput>(QAudioDeviceInfo::defaultOutputDevice(), _commonFormat, nullptr);
}


void AudioHandler::resetBufer() {
    if (_isPlaying || _isRecording)
        return;
    _commonBufer.clear();
    _prevBufferSize = 0;
}

void AudioHandler::loadOnlyWindow(QString filename, quint64 position, quint64 window) {
    WavFile wav;
    if ( wav.open(filename) == false)
       return;
    const quint64 afterHeaderPosition = wav.pos();
    wav.seek(afterHeaderPosition + position*2);
    _commonBufer = wav.read(window*2);
}

void AudioHandler::loadWindowPCM(QByteArray window) {
    _commonBufer = window;
}


void AudioHandler::loadFile(QString filename) {
    if (_isPlaying || _isRecording)
        return;
    QFile audioFile;
    audioFile.setFileName(filename);
    if (audioFile.open(QIODevice::ReadOnly) == false)
        qDebug() << "Failed to open audio for output";
    _commonBufer = audioFile.readAll();
    audioFile.close();
}


void AudioHandler::saveFile(QString filename) const {
    QFile f;
    f.setFileName(filename);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(_commonBufer);
        f.flush();
        f.close();
    }
    else
        qDebug() << "Open file for raw record error;";
}


void AudioHandler::setSampleRate(int newSampleRate) { //?TODO resampling
    if (_isPlaying || _isRecording)
        return;
    _commonFormat.setSampleRate(newSampleRate);
    initRecorder();
    initPlayer();
}


void AudioHandler::requestStopRecord(){
    stopRecord();
}


void AudioHandler::requestStopPlayback() {
    stopPlayback();
}

void AudioHandler::requestStopMidi() {
    stopMidiPlayer();
}

void AudioHandler::requestPermission() const {
    ::requestAudioPermission();
}


QStringList AudioHandler::getRecords() const {
    QDir dir("records/");
    auto list = dir.entryList({"*"});
    return list;
}


void AudioHandler::saveRecordTimstamp() const {
    auto timestamp = QDateTime::currentDateTime().toString("yyyy-MM-ddTHH.mm.ss");
    QString filename = "records/" + timestamp + ".wav";
    saveWavFile(filename);
}


void AudioHandler::saveWavFile(QString filename) const {
    WavFile wav;
    wav.open(filename, QIODevice::WriteOnly);
    wav.writeHeader(_commonFormat.sampleRate(), _commonFormat.sampleSize(), _commonBufer.size(), _commonFormat.channelCount() == 2, false); //EH not float fuck stupid QT, not cute at all
    wav.write(_commonBufer);
}


void AudioHandler::loadWavFile(QString filename) {
    if (_isPlaying || _isRecording)
        return;
    WavFile wav;
    wav.open(filename);
    _commonBufer = wav.readAll();
}


void AudioHandler::deleteRecord(QString filename) const {
    QFile::remove("records/" + filename);
}


void AudioHandler::renameRecord(QString filename, QString newFilename) const {
    QFile::rename("records/" + filename, "records/" + newFilename);
}


void AudioHandler::startMidiPlayer() {
    /*if (_isPlaying || _isRecording)
        return;
    _isPlaying = true;*/
    const double sampleRate = _midiFormat.sampleRate();
    const double bytesPerSample = _midiFormat.sampleSize() / 8.0;
    const double msInSecond = 1000.0;
    const double channels = 2.0;
    const double ms = static_cast<double>(_midiBufer.size()) / (channels * bytesPerSample * sampleRate / msInSecond);
    _midiPlayer->start();
    _midiOutput->start(_midiPlayer.get());
    QTimer::singleShot(ms, this, &AudioHandler::requestStopMidi);
}


void AudioHandler::stopMidiPlayer() {
    _midiPlayer->stop();
    _midiOutput->stop();
    //_isPlaying = false;
}


void AudioHandler::checkMidi() {
    openMidiFile("test1.mid");
}


void AudioHandler::openMidiFile(QString filename) {
     static mtherapp::MidiRender render;
     static bool loaded = false;
     if (loaded == false) {
         render.openSoundFont("instrument.sf2"); //TODO configurable
         loaded = true;
     }
     _midiBufer = render.renderShort(filename);
}


void AudioHandler::saveMidiToWav(QString filename) {
    WavFile wav;
    wav.open(filename, QIODevice::WriteOnly);
    wav.writeHeader(_midiFormat.sampleRate(), _midiFormat.sampleSize(), _midiBufer.size(), _midiFormat.channelCount() == 2, false); //EH not float fuck stupid QT, not cute at all
    wav.write(_midiBufer);
}
