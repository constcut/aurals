#include "Init.hpp"

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


void checkMidiRegression() {
    mtherapp::MidiFile m;
    m.readFromFile("test1.mid");
    m.writeToFile("test1_regression.mid");

    QFile original, resaved;
    original.setFileName("test1.mid");
    original.open(QIODevice::ReadOnly);

    resaved.setFileName("test1_regression.mid");
    resaved.open(QIODevice::ReadOnly);

    auto origialBytes = original.readAll();
    auto resavedBytes = resaved.readAll();

    if (origialBytes.size() == 0)
        qDebug() << "ERROR! couldn't read original midi file";

    if (origialBytes != resavedBytes)
        qDebug() << "ERROR! Midi regression detected!";
}


void copySoundfontsAndTests() {
    if (QFile::exists("piano.sf2") == false)
        QFile::copy(":/sf/piano.sf2", "piano.sf2");
    if (QFile::exists("guitar.sf2") == false)
        QFile::copy(":/sf/guitar.sf2", "guitar.sf2");
    if (QFile::exists("el_guitar.sf2") == false)
        QFile::copy(":/sf/el_guitar.sf2", "el_guitar.sf2");
    if (QFile::exists("test1.mid") == false)
        QFile::copy(":/sf/test1.mid", "test1.mid");
}


int mainInit(int argc, char *argv[]) {

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    qmlRegisterType<mtherapp::ConsoleLogQML>("mther.app",1,0,"ConsoleLogView");
    //qmlRegisterType<PianoQML>("mther.app",1,0,"Piano");

    QGuiApplication app(argc, argv);

    app.setApplicationName("mtherapp");
    app.setOrganizationName("accumerite");
    app.setOrganizationDomain("acc.u.merite");

    Q_INIT_RESOURCE(fonts);
    Q_INIT_RESOURCE(soundfonts);

    copySoundfontsAndTests();
    checkMidiRegression();

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
