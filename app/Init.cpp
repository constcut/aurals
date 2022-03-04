#include "Init.hpp"

//#include <QGuiApplication>
#include <QApplication>


#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTextCodec>
#include <QFontDatabase>
#include <QDir>

#include <QTemporaryDir>
#include <QStandardPaths>
#include <QDateTime>


#include <signal.h>

#include "app/log.hpp"
#include "app/LogHandler.hpp"


//TODO find nicer way to register components - maybe move all registration in sepparated header
#include "audio/wave/AudioHandler.hpp"
#include "audio/wave/WaveShape.hpp"
#include "audio/spectrum/Spectrograph.hpp"
#include "audio/spectrum/Cepstrumgraph.hpp"

#include "music/PianoRoll.hpp"
#include "music/PatternLine.hpp"
#include "music/PatternReceiver.hpp"

#include "audio/features/ACFgraph.hpp"
#include "app/StretchImage.hpp"

#include "music/Tapper.hpp"
//#include "music/graphicmap.h"
#include "midi/MidiFile.hpp"

#include "app/Config.hpp"

#include "app/Tests.hpp"
#include "app/Regression.hpp"

#include "tab/ui/TabViews.hpp"

#include "tab/tools/Commands.hpp"


using namespace std;
using namespace aurals;


void checkMidiRegression() {

    QList<QString> filesToCheck {"test1.mid", "test2.mid", "test3.mid",
                                 "test4.mid", "test5.mid", "test6.mid", "hotfix.mid"};

    for (auto& filename: filesToCheck) {
        aurals::MidiFile m;
        m.readFromFile(filename.toStdString());

        auto resavedFilename = "re_" + filename;
        m.writeToFile(resavedFilename.toStdString());

        QFile original, resaved;
        original.setFileName(filename);
        original.open(QIODevice::ReadOnly);

        resaved.setFileName(resavedFilename);
        resaved.open(QIODevice::ReadOnly);

        auto origialBytes = original.readAll();
        auto resavedBytes = resaved.readAll();

        if (origialBytes.size() == 0)
            qDebug() << "ERROR! couldn't read original midi file";

        if (origialBytes != resavedBytes)
            qDebug() << "ERROR! Midi regression detected!";
    }
}


void copySoundfontsAndTests() {
    //Delayed: разделить на 2 разных списка, складывать в разные папки
    //Для возможности добавления собственный sf2

    QList<QString> filesToCopy {"epiano.sf2", "guitar.sf2", "fullset.sf2", "nylon_guitar.sf2",
                                "piano.sf2", "drums.sf2", "eguitar.sf2", "test1.mid",
                                "test2.mid", "test3.mid", "test4.mid", "test5.mid", "test6.mid", "hotfix.mid"};

    for (auto& file: filesToCopy)
        if (QFile::exists(file) == false)
            QFile::copy(":/sf/" + file, file);
}


void posix_death_signal(int signum)
{
    qDebug() << "Crash happend signal:"<<signum;
    signal(signum, SIG_DFL);


    std::string logName = Config::getInst().testsLocation + std::string("log.txt");


    QFile logF;
    logF.setFileName(logName.c_str());
    logF.open(QIODevice::ReadOnly);

    QByteArray logWhole = logF.readAll();
    QByteArray logCompressed = qCompress(logWhole,9);

    qDebug() << "Compressed log size "<<logCompressed.size();

    QFile crashLog;


    QDateTime timeDate(QDateTime::currentDateTime());
    QString st = timeDate.toString(Qt::ISODate);
    st.replace(":","-");
    std::string time = st.toStdString();
    //time = time.substr(time.find("T")+1);

    std::cout << std::endl << time.c_str() << std::endl;

    QString baseLocation = Config::getInst().testsLocation.c_str();
    QString crashName = baseLocation + QString("crashs.glog");
    crashLog.setFileName(crashName);

    crashLog.open(QIODevice::Append);

    crashLog.write(time.c_str());
    crashLog.write(logCompressed);
    crashLog.close();

    if (CONF_PARAM("addRootCrashes")=="1")
    {
        QFile crashTextLog;
        QString textCrashFn;

#ifdef __ANDROID_API__
        textCrashFn = "/sdcard/glog.C";
#else
        textCrashFn = "glog.C";
#endif
        QDateTime timeDate(QDateTime::currentDateTime());
        QString st = timeDate.toString(Qt::ISODate);
        st.replace(":","-");

        textCrashFn+=st;
        textCrashFn+=QString(".txt");

        crashTextLog.setFileName(textCrashFn);
        crashTextLog.open(QIODevice::WriteOnly);
        crashTextLog.write(logWhole);
        crashTextLog.close();


    }

    exit(3);
}


int sayType(QByteArray &file)
{
    if (file[0]=='G')
    {
        if (file[1]=='A')
            return 1; //incomplete check
    }
    else if (file[0]=='p')
    {
        if (file[1]=='t' && file[2]=='a' && file[3]=='b')
            return 2;
    }
    else if (file[1]=='F')
    {
        if (file[2]=='I' && file[3]=='C')
        {
            char v = file[21];
            if (v==51)
                return 3;
            if (v==52)
                return 4;
            if (v==53)
                return 5;
        }
    }

    return -1;
}


void setLibPath() {
#ifdef WIN32
    QStringList libPath = QCoreApplication::libraryPaths();

    libPath.append(".");
    libPath.append("platforms");
    libPath.append("imageformats");

    QCoreApplication::setLibraryPaths(libPath);
#endif
}


void setTestsPath(QGuiApplication& a) {
    std::string currentPath;
    QString qPath = a.applicationDirPath();
    std::string appPath = qPath.toStdString();
    appPath += std::string("/tests/");
    QString pathStd = QStandardPaths::writableLocation(QStandardPaths::QStandardPaths::ApplicationsLocation); //AppDataLocation);
    currentPath =  pathStd.toStdString(); //"/sdcard/p";
    QDir dir;
    QString dirPath = currentPath.c_str();
    dir.mkdir(dirPath);
    currentPath += std::string("/");
    //SET PATH
    setTestLocation(currentPath);
}

void loadConfig() {
    std::string currentPath = Config::getInst().testsLocation;
    Config& configuration = Config::getInst();
    configuration.checkConfig();
    std::string confFileName = currentPath + "g.config";
    if (QFile::exists(confFileName.c_str())) {
        std::ifstream confFile(confFileName);
        if (confFile.is_open())
            configuration.load(confFile);
    }
    configuration.printValues();
}


void setPosixSignals() {
    signal(SIGSEGV, posix_death_signal);
    signal(SIGILL, posix_death_signal);
    signal(SIGFPE, posix_death_signal);
}




void copyResourcesIntoTempDir(bool forceRewrite=false) {

    QString testDir = "tests";
    setTestLocation(testDir.toStdString() + "/");

    QDir dir;
    QString regressionDir = testDir + "/regression/";
    dir.mkdir(regressionDir);
    QString regressionCheckDir = testDir + "/regression_check/";
    dir.mkdir(regressionCheckDir);
    QString allOutDir = testDir + "/all_out/";
    dir.mkdir(allOutDir);

    std::unordered_map<size_t, size_t> groupLength = {
        {1, 12},
        {2, 40},
        {3, 71},
        {4, 109}
    };

    for (size_t groupIdx = 1; groupIdx <= 4; ++groupIdx) {

        size_t from = 1;
        size_t to = groupLength[groupIdx] - 1;

        for (size_t fileIndx = from; fileIndx <= to; ++fileIndx) {

            std::string testName = std::to_string(groupIdx) + "." + std::to_string(fileIndx);
            QString resourse = QString(":/own_tests/") + testName.c_str() + ".gp4";
            QString copy = testDir + "/" + testName.c_str() + ".gp4";

            if (QFile::exists(copy) == false)
                QFile::copy(resourse, copy);

            if (groupIdx <= 3) {
                QString resourse1 = QString(":/regression/") + testName.c_str() + ".mid";
                QString resourse2 = QString(":/regression/") + testName.c_str() + ".gmy";
                QString copy1 = regressionDir + testName.c_str() + ".mid";
                QString copy2 = regressionDir + testName.c_str() + ".gmy";

                if (QFile::exists(copy1) == false || forceRewrite)
                    QFile::copy(resourse1, copy1);
                if (QFile::exists(copy2) == false || forceRewrite)
                    QFile::copy(resourse2, copy2);
            }
        }
    }
}





int mainInit(int argc, char *argv[]) {

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    qmlRegisterType<aurals::ConsoleLogQML>("aurals",1,0,"ConsoleLogView");
    //qmlRegisterType<PianoQML>("aurals",1,0,"Piano");

    QApplication app(argc, argv);

    app.setApplicationName("aurals");
    app.setOrganizationName("accumerite");
    app.setOrganizationDomain("acc.u.merite");

    Q_INIT_RESOURCE(fonts);
    Q_INIT_RESOURCE(soundfonts);
    Q_INIT_RESOURCE(tests);

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

    Config::getInst().checkConfig();

    qmlRegisterType<aurals::ConfigTableModel>("aurals", 1, 0, "ConfigTableModel");

    qmlRegisterType<aurals::WaveshapeQML>("aurals", 1, 0, "Waveshape");
    qmlRegisterType<aurals::SpectrographQML>("aurals", 1, 0,"Spectrograph");
    qmlRegisterType<aurals::Tapper>("aurals", 1, 0,"Tapper");
    qmlRegisterType<aurals::ACGraphQML>("aurals", 1, 0,"ACgraph");
    qmlRegisterType<aurals::StretchImageQML>("aurals", 1, 0,"StretchImage");
    qmlRegisterType<aurals::CepstrumgraphQML>("aurals", 1, 0,"Cepstrumgraph");

    qmlRegisterType<aurals::TabView>("aurals", 1, 0, "TabView");
    qmlRegisterType<aurals::TrackView>("aurals", 1, 0, "TrackView");
    //qmlRegisterType<aurals::BarView>("aurals", 1, 0, "BarView");
    qmlRegisterType<aurals::PianoRoll>("aurals", 1, 0, "PianoRoll");
    qmlRegisterType<aurals::PatternLine>("aurals", 1, 0, "PatternLine");
    qmlRegisterType<aurals::PatternReceiver>("aurals", 1, 0, "PatternReceiver");

    qmlRegisterUncreatableMetaObject(aurals::staticMetaObject,
        "aurals", 1, 0, "Tab", "Error: object creation for enum not supported");

    QDir dir;
    if (dir.exists("records") == false) {
        if (dir.mkdir("records") == false)
            qDebug() << "Failed to create records directory";
    }
    if (dir.exists("tests") == false) {
        if (dir.mkdir("tests") == false)
            qDebug() << "Failed to create tests directory";
    }

    copyResourcesIntoTempDir(true); //true if we need update regression tests
    //runRegressionTests();
    //checkMidiIORegression();

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1251")); //Настройки //KOI8-R //ISO 8859-5 //UTF-8 //Windows-1251
    QQmlApplicationEngine engine;
    aurals::AudioHandler audio;
    aurals::ConfigQML config;
    engine.rootContext()->setContextProperty("audio", &audio);
    engine.rootContext()->setContextProperty("aconfig", &config);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;


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
