#include "init.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "log.hpp"
#include <QFontDatabase>
#include <QDir>

#include "app/loghandler.h"

#include "music/graphicmap.h"
#include "music/midiengine.h"
#include "music/midirender.h"
#include "music/midifile.h"

#include <QAudioOutput>

#include <QTextCodec>

MidiSaver midiSaver;
MidiRender render;
MidiEngine midEng; //ps can make a slot, to make possible play midi notes from keyboar also here
//MidiSpeaker midiSpeaker;


void saveRawAudio(QByteArray& ba) {
    QString defaultRecFile = QString("/home/punnalyse/.local/share/applications/wavOutput.graw");
    QFile f;
    f.setFileName(defaultRecFile);
    ///int compressedSize = compress.size(); //TODO compress
    if (f.open(QIODevice::WriteOnly)) {
        qDebug() << "Collector size was "<<ba.size();
        f.write(ba);
        f.flush();
    }
    else
        qDebug() << "Open file for raw record error;";
}


int mainInit(int argc, char *argv[]) {

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    Q_INIT_RESOURCE(soundfonts);
    QFile::copy(":/soundfonts/instrument.sf2","instrument.sf2");

    qmlRegisterType<ConsoleLogQML>("mther.app",1,0,"ConsoleLog");
    qmlRegisterType<PianoQML>("mther.app",1,0,"Piano");

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

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1251")); //KOI8-R //ISO 8859-5 //UTF-8 //Windows-1251
    QQmlApplicationEngine engine;
    //engine.rootContext()->setContextProperty("wavGen", &midiSpeaker.wavGen);

    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");

    QAudioOutput audioOutput(QAudioDeviceInfo::defaultOutputDevice(), format,0);


    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    int res = app.exec();
    return res;
}
