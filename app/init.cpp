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

using namespace std;

#include "libs/stft/STFT.h"
#include "libs/stft/WAV.h"

int test(){

  const int ch = 2;
  const int rate = 44100;
  const int frame = 512;
  const int shift = 128;
  int length;

  WAV input;
  WAV output(ch,rate);
  STFT process(ch,frame,shift);

  input.OpenFile("/home/punnalyse/Downloads/LibrosaCpp-main/samples/g.wav");
  output.NewFile("output.wav");

  short buf_in[ch*shift];
  double **data;
  short buf_out[ch*shift];

  data = new double*[ch];
  for(int i=0;i<ch;i++){
    data[i] = new double[frame+2];
    memset(data[i],0,sizeof(double)*(frame+2));
  }

  std::vector<std::vector<double>> sp;

  while(!input.IsEOF()){
    length = input.ReadUnit(buf_in,shift*ch);
    process.stft(buf_in,length,data);

    std::vector<double> loc;
    for (size_t i = 0; i < frame; ++i) {
        double r = data[0][i];
        double img = data[0][i + frame]; //frame
        double amp = sqrt(r*r + img*img);
        loc.push_back(amp);
    }
    sp.push_back(std::move(loc));


    process.istft(data,buf_out);
    output.Append(buf_out,shift*ch);
  }

  for(int i=0;i<ch;i++)
    delete[] data[i];
  delete[] data;

  std::string formatType = "P6";
  uint32_t width = sp.size();
  uint32_t height = sp[0].size();
  uint32_t maxColors = 255;

  std::vector<uint8_t> buffer;
  buffer.resize(width * height * 3);

  std::ofstream outfile("s_last.ppm", std::ofstream::binary);
  outfile << formatType << "\n" << width << " " << height << "\n" << maxColors << "\n";

  for (size_t i = 0; i < height; ++i)
  {
      for (size_t j = 0; j < width; ++j) {

          double samp = sp[j][i];
          uint32_t pixel = samp * 16581375;
          outfile.write(reinterpret_cast<char *>(&pixel), 3);

          /*
          size_t pos = (i * width + j) * 3;
          buffer[pos] = 0;
          buffer[pos + 1] = 254;
          buffer[pos + 2] = 0;*/
      }
  }




  return 0;
}


void testPPMIO() {
    std::ofstream outfile("check.ppm", std::ofstream::binary);
    if (outfile.fail()){
        std::cout << "Failed open PPM file for writing" << std::endl;
        return;
    }

    std::string formatType = "P6";
    uint32_t width = 100;
    uint32_t height = 100;
    uint32_t maxColors = 255;

    std::vector<uint8_t> buffer;
    buffer.resize(width * height * 3);

    for (size_t i = 0; i < height; ++i)
    {
        for (size_t j = 0; j < width; ++j) {
            size_t pos = (i * width + j) * 3;
            buffer[pos] = 0;
            buffer[pos + 1] = 254;
            buffer[pos + 2] = 0;
        }
    }

    outfile << formatType << "\n" << width << " " << height << "\n" << maxColors << "\n";
    outfile.write(reinterpret_cast<char *>(buffer.data()), width * height * 3);

    std::cout << "Test PPMIO finished" << std::endl;
}


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

    testPPMIO();
    test();

    return 0;

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1251")); //Настройки //KOI8-R //ISO 8859-5 //UTF-8 //Windows-1251
    QQmlApplicationEngine engine;

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    int res = app.exec();
    return res;
}
