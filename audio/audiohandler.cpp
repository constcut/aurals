#include "audiohandler.h"

#include <QFile>
#include <QDebug>

AudioHandler::AudioHandler() {
    initRecorder();
    initPlayer();
    //TODO set params + reset
}



void AudioHandler::startRecord() {
    //TODO request permission
    audioReceiver->start();
    audioInput->start(audioReceiver.get());
}


void AudioHandler::stopRecord() {
    audioReceiver->stop();
    audioInput->stop();
    audioReceiver->dump();
}


void AudioHandler::startPlayback() {
    QFile audioFile;
    QString defaultRecFile = "record.temp";
    audioFile.setFileName(defaultRecFile);

    if (audioFile.open(QIODevice::ReadOnly) == false)
        qDebug() << "Failed to open audio for output";

    QByteArray allBytes = audioFile.readAll();
    audioFile.close();

    audioPlayer->setAudioBufer(allBytes); //TODO хранить в данном классе, общий для IO
    audioPlayer->start();
    audioOutput->start(audioPlayer.get());
}


void AudioHandler::stopPlayback() {
    audioPlayer->stop();
    audioOutput->stop();
}


void AudioHandler::initRecorder() {
    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    if (!info.isFormatSupported(format)) {
        qDebug() << "Default format not supported - trying to use nearest";
        format = info.nearestFormat(format);
    }

    audioReceiver  = std::make_unique<AudioReceiver>(format, this, commonBufer);
    //connect(audioInfo, SIGNAL(update()), SLOT(refreshDisplay()));
    audioInput = std::make_unique<QAudioInput>(QAudioDeviceInfo::defaultInputDevice(), format, nullptr);
}


void AudioHandler::initPlayer() {
    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1); //TODO for pcm but will have issue with records
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    if (!info.isFormatSupported(format)) {
        qDebug() << "Default format not supported - trying to use nearest";
        format = info.nearestFormat(format);
    }

    audioPlayer = std::make_unique<AudioSpeaker>(format, this, commonBufer);
    audioOutput = std::make_unique<QAudioOutput>(QAudioDeviceInfo::defaultOutputDevice(), format, nullptr);

}


void AudioHandler::deleteDump() {
    QFile audioFile;
    QString defaultRecFile = "record.temp";
    audioFile.setFileName(defaultRecFile);
    audioFile.remove();
}
