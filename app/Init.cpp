#include "Init.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTextCodec>
#include <QFontDatabase>
#include <QDir>

#include <QTemporaryDir>
#include <QStandardPaths>
#include <QDateTime>


#include <signal.h>

#include "log.hpp"
#include "app/LogHandler.hpp"

#include "audio/wave/AudioHandler.hpp"
#include "audio/wave/WaveShape.hpp"
#include "audio/spectrum/Spectrograph.hpp"
#include "audio/spectrum/Cepstrumgraph.hpp"

#include "audio/features/ACFgraph.hpp"
#include "app/StretchImage.hpp"

#include "music/Tapper.h"
//#include "music/graphicmap.h"
#include "midi/MidiFile.hpp"


//FFT bench
#include "libs/fft/FFTRealFixed.hpp"

#include "libs/kiss/kiss_fftr.h"

#include <vector>
#include <chrono>
#include <iostream>

#include "app/Config.hpp"




using namespace std;
using namespace aural_sight;


void checkMidiRegression() {

    QList<QString> filesToCheck {"test1.mid", "test2.mid", "test3.mid",
                                 "test4.mid", "test5.mid", "test6.mid"};

    for (auto& filename: filesToCheck) {
        aural_sight::MidiFile m;
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
                                "test2.mid", "test3.mid", "test4.mid", "test5.mid", "test6.mid"};

    for (auto& file: filesToCopy)
        if (QFile::exists(file) == false)
            QFile::copy(":/sf/" + file, file);
}




void benchmarkFFT() {

    const int bits = 12;
    const int size = 2 << (bits-1);

    FFTRealFixLen<bits> fftUnf;

    std::vector<float> testVector(size, 0);
    std::vector<float> output(size, 0);


    for (auto& sample: testVector)
        sample = (rand() % 30000) / 30000.0f;

    auto bench = [&](auto& fft, auto name) {
        auto start = std::chrono::high_resolution_clock::now();
        fft.do_fft(&output[0], &testVector[0]);
        auto end = std::chrono::high_resolution_clock::now();
        auto durationMs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        //std::cout << name << " spent " << durationMs << std::endl;
        return durationMs;
    };


    std::vector<kiss_fft_cpx> outKiss(size);

    kiss_fftr_cfg cfg = kiss_fftr_alloc( size, 0, 0, 0 );
    kiss_fftr( cfg , testVector.data() , outKiss.data() );

    auto benchKiss = [&]() {
        auto start = std::chrono::high_resolution_clock::now();
        kiss_fftr( cfg , testVector.data() , outKiss.data() );
        auto end = std::chrono::high_resolution_clock::now();
        auto durationMs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        return durationMs;
    };

    qDebug() << "Sizes " << size << " and " << fftUnf.FFT_LEN;

    //exit(0); */

    unsigned long unfCount = 0;

    unsigned long kissCount = 0;

    for (size_t i = 0; i < 10000; ++i) {

        for (auto& sample: testVector)
            sample = (rand() % 30000) / 30000.0f;
        if (i % 2 == 0) {
            unfCount += bench(fftUnf, "UNf");
            kissCount += benchKiss();
        }
        else {
            kissCount += benchKiss();
            unfCount += bench(fftUnf, "UNf");
        }

    }

    qDebug() << "Total un fixed: " << unfCount / 1000.0;
    qDebug() << "Total kiss: " << kissCount / 1000.0;

}




void posix_death_signal(int signum)
{
    std::cerr << "Crash happend signal:"<<signum;
    signal(signum, SIG_DFL);


    std::string logName = AConfig::getInst().testsLocation + std::string("log.txt");


    QFile logF;
    logF.setFileName(logName.c_str());
    logF.open(QIODevice::ReadOnly);

    QByteArray logWhole = logF.readAll();
    QByteArray logCompressed = qCompress(logWhole,9);

    std::cerr << "Compressed log size "<<logCompressed.size();

    QFile crashLog;


    QDateTime timeDate(QDateTime::currentDateTime());
    QString st = timeDate.toString(Qt::ISODate);
    st.replace(":","-");
    std::string time = st.toStdString();
    //time = time.substr(time.find("T")+1);

    std::cout << std::endl << time.c_str() << std::endl;

    QString baseLocation = AConfig::getInst().testsLocation.c_str();
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

QString getUserID() {
    return "TODO";
}


int sayType(QByteArray &file)
{
    if (file[0]=='G')
    {
        if (file[1]=='A') return 1; //incomplete check
    }
    else if (file[0]=='p')
    {

        if (file[1]=='t')
            if (file[2]=='a')
               if (file[3]=='b') return 2;
    }
    else if (file[1]=='F')
    {
        if (file[2]=='I')
            if (file[3]=='C')
            {
                char v = file[21];
                //char subV = file[23];
                if (v==51) return 3;
                if (v==52) return 4;
                if (v==53) return 5;
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
    std::string currentPath = AConfig::getInst().testsLocation;
    AConfig& configuration = AConfig::getInst();
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




void copyResourcesIntoTempDir() { //TODO во временную копировать только тесты
    static QTemporaryDir tempDir;
    setTestLocation(tempDir.path().toStdString() + "/");
    QDir dir;
    QString regressionDir = tempDir.path() + "/regression/";
    dir.mkdir(regressionDir);
    QString regressionCheckDir = tempDir.path() + "/regression_check/";
    dir.mkdir(regressionCheckDir);
    QString allOutDir = tempDir.path() + "/all_out/";
    dir.mkdir(allOutDir);

    QString resourseSf = ":/sf/fullset.sf2";
    QString copySf = tempDir.path() + "/fullset.sf2";
    QFile::copy(resourseSf, copySf);

    std::unordered_map<size_t, size_t> groupLength = {
        {1, 12},
        {2, 39},
        {3, 71},
        {4, 109}
    };
    for (size_t groupIdx = 1; groupIdx <= 4; ++groupIdx) {
        size_t from = 1;
        size_t to = groupLength[groupIdx] - 1;
        for (size_t fileIndx = from; fileIndx <= to; ++fileIndx) {
            std::string testName = std::to_string(groupIdx) + "." + std::to_string(fileIndx);
            QString resourse = QString(":/own_tests/") + testName.c_str() + ".gp4";
            QString copy = tempDir.path() + "/" + testName.c_str() + ".gp4";
            QFile::copy(resourse, copy);

            if (groupIdx <= 2) {
                QString resourse1 = QString(":/regression/") + testName.c_str() + ".mid";
                QString resourse2 = QString(":/regression/") + testName.c_str() + ".gmy";
                QString copy1 = regressionDir + testName.c_str() + ".mid";
                QString copy2 = regressionDir + testName.c_str() + ".gmy";
                QFile::copy(resourse1, copy1);
                QFile::copy(resourse2, copy2);
            }
        }
    }
}





int mainInit(int argc, char *argv[]) {

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    qmlRegisterType<aural_sight::ConsoleLogQML>("mther.app",1,0,"ConsoleLogView");
    //qmlRegisterType<PianoQML>("mther.app",1,0,"Piano");

    QGuiApplication app(argc, argv);

    app.setApplicationName("aural_sight");
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

    qmlRegisterType<aural_sight::WaveshapeQML>("mther.app", 1, 0, "Waveshape");
    qmlRegisterType<aural_sight::SpectrographQML>("mther.app", 1, 0,"Spectrograph");
    qmlRegisterType<aural_sight::Tapper>("mther.app", 1, 0,"Tapper");
    qmlRegisterType<aural_sight::ACGraphQML>("mther.app", 1, 0,"ACgraph");
    qmlRegisterType<aural_sight::StretchImageQML>("mther.app", 1, 0,"StretchImage");
    qmlRegisterType<aural_sight::CepstrumgraphQML>("mther.app", 1, 0,"Cepstrumgraph");

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1251")); //Настройки //KOI8-R //ISO 8859-5 //UTF-8 //Windows-1251
    QQmlApplicationEngine engine;
    aural_sight::AudioHandler audio;
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
