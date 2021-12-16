#include "audiohandler.h"

#include <QFile>
#include <QDebug>

AudioHandler::AudioHandler() {

    commonFormat.setSampleRate(8000);
    commonFormat.setChannelCount(1);
    commonFormat.setSampleSize(16);
    commonFormat.setSampleType(QAudioFormat::SignedInt);
    commonFormat.setByteOrder(QAudioFormat::LittleEndian);
    commonFormat.setCodec("audio/pcm");

    initRecorder();
    initPlayer();
}



void AudioHandler::startRecord() {
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


void AudioHandler::deleteDump() {
    QFile audioFile;
    QString defaultRecFile = "record.temp";
    audioFile.setFileName(defaultRecFile);
    audioFile.remove();
}

/*
void AudioHandler::loadFile(QString filename) {
    QFile audioFile;
    QString defaultRecFile = "record.temp";
    audioFile.setFileName(defaultRecFile);

    if (audioFile.open(QIODevice::ReadOnly) == false)
        qDebug() << "Failed to open audio for output";

    QByteArray allBytes = audioFile.readAll();
    audioFile.close();

    audioPlayer->setAudioBufer(allBytes); //TODO хранить в данном классе, общий для IO
}*/
