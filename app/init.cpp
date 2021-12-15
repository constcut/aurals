#include "init.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTextCodec>
#include <QFontDatabase>
#include <QDir>

#include "log.hpp"
#include "app/loghandler.h"
//#include "music/graphicmap.h"

#include <iostream>
#include <vector>

#include <chrono>
#include <numeric>
#include <algorithm>
#include <fstream>
#include <cstdlib>

#include "audio/audiospeaker.h"
#include <QAudioOutput>
#include <QAudioInput>

using namespace std;


int mainInit(int argc, char *argv[]) {

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    Q_INIT_RESOURCE(soundfonts);
    QFile::copy(":/soundfonts/instrument.sf2","instrument.sf2");

    qmlRegisterType<ConsoleLogQML>("mther.app",1,0,"ConsoleLog");
    //qmlRegisterType<PianoQML>("mther.app",1,0,"Piano");

    QGuiApplication app(argc, argv);
    Q_INIT_RESOURCE(fonts);
    qDebug() << "Current working path "<<QDir::currentPath();
    int fontId = QFontDatabase::addApplicationFont(":/fonts/prefont.ttf");
    QStringList famList = QFontDatabase::applicationFontFamilies(fontId) ;
    qDebug() << famList << " font families for id "<<fontId;
    if (famList.isEmpty() == false)
        app.setFont(QFont(famList[0], 11, QFont::Normal, false));
    else
        qWarning() << "Failed to load font";

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1251")); //Настройки //KOI8-R //ISO 8859-5 //UTF-8 //Windows-1251
    QQmlApplicationEngine engine;

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

   auto audioSpeaker = std::make_unique<AudioSpeaker>(format);
   auto audioOutput = std::make_unique<QAudioOutput>(QAudioDeviceInfo::defaultOutputDevice(), format, nullptr);

   QFile audioFile;
   QString defaultRecFile = "record.temp";
   audioFile.setFileName(defaultRecFile);

   if (audioFile.open(QIODevice::ReadOnly) == false)
       qDebug() << "Failed to open audio for output";

   QByteArray allBytes = audioFile.readAll();
   audioFile.close();

   audioSpeaker->setAudioBufer(allBytes);
   audioSpeaker->start();
   audioOutput->start(audioSpeaker.get());

    /*
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

    auto audioInfo  = std::make_unique<AudioInfo>(format, nullptr);
    //connect(audioInfo, SIGNAL(update()), SLOT(refreshDisplay()));
    auto audioInput = std::make_unique<QAudioInput>(QAudioDeviceInfo::defaultInputDevice(), format, nullptr);

    audioInfo->start();
    audioInput->start(audioInfo.get());*/


    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    int res = app.exec();

    audioSpeaker->stop();
    audioOutput->stop();

    /*
    audioInfo->stop();
    audioInput->stop();
    audioInfo->dump();*/

    return res;
}
