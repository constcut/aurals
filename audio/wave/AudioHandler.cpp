#include "AudioHandler.hpp"

#include <QFile>
#include <QDebug>
#include <QDir>
#include <QDateTime>

#include "app/AndroidTools.hpp"
#include "WavFile.hpp"

//#include "midi/MidiRender.hpp"
#include "tab/Tab.hpp"
#include "tab/tools/TabLoader.hpp"
#include "tab/tools/MidiExport.hpp"

#include "midi/MidiEngine.hpp"


using namespace aurals;


AudioHandler::AudioHandler() {
    initAudioHandler();
}


void AudioHandler::initAudioHandler() {
    _commonFormat.setSampleRate(44100);
    _commonFormat.setChannelCount(1);
    _commonFormat.setSampleSize(16); //The only format old Qt accepts on android :(
    _commonFormat.setSampleType(QAudioFormat::SignedInt);
    _commonFormat.setByteOrder(QAudioFormat::LittleEndian);
    _commonFormat.setCodec("audio/pcm");

    _midiFormat.setSampleRate(8000); //Its default in render
    _midiFormat.setChannelCount(2);
    _midiFormat.setSampleSize(16);
    _midiFormat.setSampleType(QAudioFormat::SignedInt);
    _midiFormat.setByteOrder(QAudioFormat::LittleEndian);
    _midiFormat.setCodec("audio/pcm");

    initRecorder();
    initPlayer();
    initMidiPlayer();
    _audioStopRequestTimer.setSingleShot(true);
    QObject::connect(&_audioStopRequestTimer, &QTimer::timeout, this, &AudioHandler::requestStopPlayback);
    _midiStopRequestTimer.setSingleShot(true);
    QObject::connect(&_midiStopRequestTimer, &QTimer::timeout, this, &AudioHandler::requestStopMidi);
}




void AudioHandler::startRecord() {
    if (_isPlaying || _isRecording)
        return;
    _isRecording = true;
    _prevBufferSize = _commonBufer.size();
    _audioReceiver->start();
    _audioInput->start(_audioReceiver.get());
}


void AudioHandler::startMidiAndRecording() {
    if (_isPlaying || _isRecording)
        return;
    _isRecording = true;
    _prevBufferSize = _commonBufer.size();
    _audioReceiver->start();

    const double sampleRate = _midiFormat.sampleRate();
    const double bytesPerSample = _midiFormat.sampleSize() / 8.0;
    const double msInSecond = 1000.0;
    const double channels = 2.0;
    const double ms = static_cast<double>(_midiBufer.size()) / (channels * bytesPerSample * sampleRate / msInSecond);
    _midiPlayer->start();

    //Вероятно потребуется общее устройство, и регистрация времени каждого семпла в момент получения
    _audioInput->start(_audioReceiver.get());
    _midiOutput->start(_midiPlayer.get());

    _midiStopRequestTimer.start(ms);
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
    _audioStopRequestTimer.start(ms);
    _audioOutput->start(_audioPlayer.get());
}


void AudioHandler::stopPlayback() {
    _audioPlayer->stop();
    _audioOutput->stop();
    _isPlaying = false;
    _audioStopRequestTimer.stop();
}


void AudioHandler::initMidiPlayer() {
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

void AudioHandler::loadOnlyWindow(const QString filename, const quint64 position, const quint64 window) {
    WavFile wav;
    if ( wav.open(filename) == false)
       return;
    const quint64 afterHeaderPosition = wav.pos();
    wav.seek(afterHeaderPosition + position*2);
    _commonBufer = wav.read(window*2);
}

void AudioHandler::loadWindowPCM(const QByteArray window) {
    _commonBufer = window;
}


void AudioHandler::loadFile(const QString filename) {
    if (_isPlaying || _isRecording)
        return;
    QFile audioFile;
    audioFile.setFileName(filename);
    if (audioFile.open(QIODevice::ReadOnly) == false)
        qDebug() << "Failed to open audio for output";
    _commonBufer = audioFile.readAll();
    audioFile.close();
}


void AudioHandler::mixRecordAndMidi()
{
    qint16* record = reinterpret_cast<qint16*>(_commonBufer.data());
    const qint16* midi = reinterpret_cast<const qint16*>(_midiBufer.constData());

    //Must be measured 150+++ ms is so much - maybe Qt wouldn't work fine here
    int pseudoRoundtrip = -7000; //Only for 44100 only on some device.. just hotfix attempt
    //But maybe issue is in midi pattern playing

    for (int i = 0; i < _commonBufer.size() / 2; ++i) {
        if (i >= -pseudoRoundtrip)
            record[i] += midi[i*2 + pseudoRoundtrip * 2] / 2
                    + midi[i*2 + 1 + pseudoRoundtrip * 2] / 2;
    }
    //Doesn't work so simple, we have offset
}


void AudioHandler::saveFile(const QString filename) const {
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


void AudioHandler::setSampleRate(const int newSampleRate) { //DELAYED: resampling
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


void AudioHandler::saveWavFile(const QString filename) const {
    WavFile wav;
    wav.open(filename, QIODevice::WriteOnly);
    wav.writeHeader(_commonFormat.sampleRate(), _commonFormat.sampleSize(), _commonBufer.size(), _commonFormat.channelCount() == 2, false); //EH not float fuck stupid QT, not cute at all
    wav.write(_commonBufer);
}


void AudioHandler::loadWavFile(const QString filename) {
    if (_isPlaying || _isRecording)
        return;
    WavFile wav;
    wav.open(filename);
    _commonBufer = wav.readAll();
}


void AudioHandler::deleteRecord(const QString filename) const {
    QFile::remove("records/" + filename);
}


void AudioHandler::renameRecord(const QString filename, const QString newFilename) const {
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
    _midiStopRequestTimer.start(ms);
}


void AudioHandler::stopMidiPlayer() {
    _midiPlayer->stop();
    _midiOutput->stop();
    //_isPlaying = false;
    _midiStopRequestTimer.stop();
}


void AudioHandler::openMidiFile(const QString filename) {
    _render.openSoundFont();
    _midiBufer = _render.renderShort(filename);
}



void AudioHandler::openMidiFileEngine(const QString filename) {
    MidiEngine::getInst().closeFile();
    MidiEngine::getInst().openFile(filename);
}

void AudioHandler::startMidiFileEngine() {
    MidiEngine::getInst().startFile();
}

void AudioHandler::stopMidiFileEngine() {
    MidiEngine::getInst().stopFile();
}

void AudioHandler::openTabFile(const QString filename) {
    GTabLoader loader;
    loader.open(filename.toStdString());
    auto midi = exportMidi(loader.getTab().get(), 0);
    midi->writeToFile("tab.mid");
    _render.openSoundFont();
    _midiBufer = _render.renderShort("tab.mid");
}


void AudioHandler::saveMidiToWav(const QString filename) const {
    WavFile wav;
    wav.open(filename, QIODevice::WriteOnly);
    wav.writeHeader(_midiFormat.sampleRate(), _midiFormat.sampleSize(), _midiBufer.size(), _midiFormat.channelCount() == 2, false); //EH not float fuck stupid QT, not cute at all
    wav.write(_midiBufer);
}
