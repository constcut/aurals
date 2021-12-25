#include "audiohandler.h"

#include <QFile>
#include <QDebug>
#include <QDir>
#include <QDateTime>

#include "app/androidtools.h"
#include "wavfile.h"


AudioHandler::AudioHandler() {

    commonFormat.setSampleRate(22050);
    commonFormat.setChannelCount(1);
    commonFormat.setSampleSize(32);
    commonFormat.setSampleType(QAudioFormat::Float);
    commonFormat.setByteOrder(QAudioFormat::LittleEndian);
    commonFormat.setCodec("audio/pcm");

    initRecorder();
    initPlayer();
}


void AudioHandler::startRecord() {

    #ifdef Q_OS_ANDROID
    //TODO request? Why falls only on mine phone
    #endif
    prevPosition = commonBufer.size();

    audioReceiver->start();
    audioInput->start(audioReceiver.get());
}


void AudioHandler::stopRecord() {
    audioReceiver->stop();
    audioInput->stop();
    //TODO сохранить в wav, загрузить в DAW изучить, возможно вместо вырезания занулить
    if (prevPosition == 0)
        commonBufer.remove(0, 4*2205); //Щелчёк, однако дозаписывать в такую запись будет неудачным решением, вернее нужно будет сохранять позицию и вырезать из неё щелчёк так же
}


void AudioHandler::startPlayback() {
    audioPlayer->start();
    audioOutput->start(audioPlayer.get());
}


void AudioHandler::stopPlayback() {
    audioPlayer->stop();
    audioOutput->stop();
}


void AudioHandler::initRecorder() {
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    if (!info.isFormatSupported(commonFormat)) {
        qDebug() << "Default format not supported - trying to use nearest";
        commonFormat = info.nearestFormat(commonFormat);
    }
    audioReceiver  = std::make_unique<AudioReceiver>(commonFormat, this, commonBufer); //    //connect(audioInfo, SIGNAL(update()), SLOT(refreshDisplay()));
    audioInput = std::make_unique<QAudioInput>(QAudioDeviceInfo::defaultInputDevice(), commonFormat, nullptr);
}


void AudioHandler::initPlayer() {
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    if (!info.isFormatSupported(commonFormat)) {
        qDebug() << "Default format not supported - trying to use nearest";
        commonFormat = info.nearestFormat(commonFormat);
    }
    audioPlayer = std::make_unique<AudioSpeaker>(commonFormat, this, commonBufer);
    audioOutput = std::make_unique<QAudioOutput>(QAudioDeviceInfo::defaultOutputDevice(), commonFormat, nullptr);

}


void AudioHandler::resetBufer() {
    commonBufer.clear();
    prevPosition = 0;
}


void AudioHandler::loadFile(QString filename) {
    QFile audioFile;
    audioFile.setFileName(filename);

    if (audioFile.open(QIODevice::ReadOnly) == false)
        qDebug() << "Failed to open audio for output";

    commonBufer = audioFile.readAll();
    audioFile.close();
}


void AudioHandler::saveFile(QString filename) const {
    QFile f;
    f.setFileName(filename);
    if (f.open(QIODevice::WriteOnly))
    {
        f.write(commonBufer);
        f.flush();
        f.close();
    }
    else
        qDebug() << "Open file for raw record error;";
}



void AudioHandler::setSampleRate(int newSampleRate) {
    qDebug() << "New sample rate " << newSampleRate;
    commonFormat.setSampleRate(newSampleRate);
    initRecorder();
    initPlayer();
    //TODO если уже есть заполненный буфер, возможно его стоит преобразовать
}


void AudioHandler::requestStopRecord(){
    //TODO more?
    stopRecord();
}


void AudioHandler::requestStopPlayback() {
    //TODO more?
    stopPlayback();
}


void AudioHandler::requestPermission() const {
    ::requestPermission();
}


QStringList AudioHandler::getRecords() const {
    QDir dir("records/");
    auto list = dir.entryList({"*"});
    qDebug() << list;
    return list;
}


void AudioHandler::saveRecordTimstamp() {
    auto timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH_mm_ss");
    QString filename = "records/" + timestamp + "_" + QString::number(commonFormat.sampleRate()) + ".wav";
    saveWavFile(filename);
}

void AudioHandler::saveWavFile(QString filename) const {
    WavFile wav;
    wav.open(filename, QIODevice::WriteOnly);
    wav.writeHeader(commonFormat.sampleRate(), commonFormat.sampleSize(), commonBufer.size(), false, true);
    wav.write(commonBufer);
}

void AudioHandler::loadWavFile(QString filename) {
    WavFile wav;
    wav.open(filename);
    commonBufer = wav.readAll();
    commonFormat = wav.audioFormat();
    qDebug() << commonBufer.size() << " loaded bytes in bufer";
}


void AudioHandler::deleteRecord(QString filename) const {
    QFile::remove("records/" + filename);
}


void AudioHandler::renameRecord(QString filename, QString newFilename) const {
    QFile::rename("records/" + filename, "records/" + newFilename);
}
