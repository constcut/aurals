#include "audiohandler.h"

#include <QFile>
#include <QDebug>

#include "app/androidtools.h"


AudioHandler::AudioHandler() {

    commonFormat.setSampleRate(16000);
    commonFormat.setChannelCount(1);
    commonFormat.setSampleSize(16);
    commonFormat.setSampleType(QAudioFormat::SignedInt);
    commonFormat.setByteOrder(QAudioFormat::LittleEndian);
    commonFormat.setCodec("audio/pcm");

    initRecorder();
    initPlayer();
}


void AudioHandler::startRecord() {

    #ifdef Q_OS_ANDROID

    #endif

    audioReceiver->start();
    audioInput->start(audioReceiver.get());
}


void AudioHandler::stopRecord() {
    audioReceiver->stop();
    audioInput->stop();
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
}


void AudioHandler::loadFile(QString filename) {
    QFile audioFile;
    audioFile.setFileName(filename);

    if (audioFile.open(QIODevice::ReadOnly) == false)
        qDebug() << "Failed to open audio for output";

    commonBufer = audioFile.readAll();
    audioFile.close();
}


void AudioHandler::saveFile(QString filename) {
    QFile f;
    f.setFileName(filename);
    if (f.open(QIODevice::Append))
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

void AudioHandler::setBitRate(int newBitRate) {
    commonFormat.setSampleSize(newBitRate);
    initRecorder();
    initPlayer();
}


void AudioHandler::requestStopRecord(){
    //TODO more?
    stopRecord();
}


void AudioHandler::requestStopPlayback() {
    //TODO more?
    stopPlayback();
}


void AudioHandler::requestPermission() {
    ::requestPermission();
}
