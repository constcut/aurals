#include "Init.hpp"

#include <iostream> //TODO review do we really need them?
#include <vector>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <fstream>
#include <cstdlib>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTextCodec>
#include <QFontDatabase>
#include <QDir>

#include "log.hpp"
#include "app/LogHandler.hpp"

#include "audio/AudioHandler.hpp"
#include "audio/WaveShape.hpp"
#include "audio/Spectrograph.hpp"
#include "music/Tapper.h"
//#include "music/graphicmap.h"

#include "midi/MidiFile.hpp"


using namespace std;


int mainInit(int argc, char *argv[]) {

    { //TEST plain midi generation
        mtherapp::MidiFile m;
        mtherapp::MidiTrack track;
        //Is it needed?
        track.pushChangeBPM(120, 0);
        track.pushMetricsSignature(4, 4, 0);
        track.pushChangeInstrument(0, 0, 0);
        track.pushChangePanoram(128, 0);
        track.pushChangeVolume(128, 0);
        //calcRhythmDetail
        track.pushNoteOn(60, 100, 0);
        track.accumulate(960);
        track.pushNoteOff(60, 100, 0);
        track.accumulate(960);
        track.pushNoteOn(60, 100, 0);
        track.accumulate(960);
        track.pushNoteOff(60, 100, 0);
        track.pushEvent47();
        m.push_back(std::move(track));

        m.writeToFile("/home/punnalyse/check2.mid");
    }
    return 0;

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    qmlRegisterType<mtherapp::ConsoleLogQML>("mther.app",1,0,"ConsoleLogView");
    //qmlRegisterType<PianoQML>("mther.app",1,0,"Piano");

    QGuiApplication app(argc, argv);

    app.setApplicationName("mtherapp");
    app.setOrganizationName("accumerite");
    app.setOrganizationDomain("acc.u.merite");

    Q_INIT_RESOURCE(fonts);
    Q_INIT_RESOURCE(soundfonts);

    if (QFile::exists("instrument.sf2") == false)
        QFile::copy(":/sf/instrument.sf2", "instrument.sf2");
    if (QFile::exists("test1.mid") == false)
        QFile::copy(":/sf/test1.mid", "test1.mid");

    qDebug() << "Current working path "<<QDir::currentPath();
    int fontId = QFontDatabase::addApplicationFont(":/fonts/prefont.ttf");
    QStringList famList = QFontDatabase::applicationFontFamilies(fontId) ;
    qDebug() << famList << " font families for id "<<fontId;
    if (famList.isEmpty() == false)
        app.setFont(QFont(famList[0], 11, QFont::Normal, false));
    else
        qWarning() << "Failed to load font";

    qmlRegisterType<mtherapp::WaveshapeQML>("mther.app", 1, 0, "Waveshape");
    qmlRegisterType<mtherapp::SpectrographQML>("mther.app", 1, 0,"Spectrograph");
    qmlRegisterType<mtherapp::Tapper>("mther.app", 1, 0,"Tapper");

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1251")); //Настройки //KOI8-R //ISO 8859-5 //UTF-8 //Windows-1251
    QQmlApplicationEngine engine;
    mtherapp::AudioHandler audio;
    engine.rootContext()->setContextProperty("audio", &audio);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    QDir dir;
    if (dir.exists("records") == false)
        dir.mkdir("records");

    int res = 0;
    try {
        res = app.exec();
    }
    catch(std::exception& e) {
        qDebug() << "Catched exception " << e.what();
        res = -1;
    }

    return res;
}
