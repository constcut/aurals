#include "Init.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTextCodec>
#include <QFontDatabase>
#include <QDir>

#include "log.hpp"
#include "app/LogHandler.hpp"

#include "audio/wave/AudioHandler.hpp"
#include "audio/wave/WaveShape.hpp"
#include "audio/spectrum/Spectrograph.hpp"

#include "audio/features/ACFgraph.hpp"
#include "app/StretchImage.hpp"

#include "music/Tapper.h"
//#include "music/graphicmap.h"
#include "midi/MidiFile.hpp"


//FFT bench
#include "libs/fft/FFTunreal.hpp"
#include "libs/fft/FFTurealfix.hpp"

#include "libs/kiss/kiss_fftr.h"

#include <vector>
#include <chrono>
#include <iostream>



using namespace std;


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

    FFTReal<float> fftUn(size);

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

    unsigned long unCount = 0;
    unsigned long unfCount = 0;

    unsigned long kissCount = 0;

    for (size_t i = 0; i < 10000; ++i) {

        for (auto& sample: testVector)
            sample = (rand() % 30000) / 30000.0f;

        //unCount += bench(fftUn, "UN");

        if (i % 2 == 0) {
            unfCount += bench(fftUnf, "UNf");
            kissCount += benchKiss();
        }
        else {
            kissCount += benchKiss();
            unfCount += bench(fftUnf, "UNf");
        }

    }

    //qDebug() << "Total un: " << unCount / 1000.0;
    qDebug() << "Total un fixed: " << unfCount / 1000.0;
    qDebug() << "Total kiss: " << kissCount / 1000.0;

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
    qmlRegisterType<StretchImageQML>("mther.app", 1, 0,"StretchImage");

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
