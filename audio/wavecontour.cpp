#include "wavecontour.h"
#include "wavfile.h"

#include "featureextractor.h"
#include "findpeaks.hpp"
#include "utils.h"

#include <QDebug>



WaveContour::WaveContour(QString filename) {
    if (loadWavFile(filename)==false)
        qDebug() << "Failed to load contour from file "<<filename;
}


void WaveContour::calculateF0() {
    _yinLine.clear();
    size_t yinSize = 2048; //TODO configurable
    size_t yinFrames = _floatSamples.size() / yinSize;
    for (size_t step = 0; step < yinFrames; ++step) {
        auto forLocalYin = _floatSamples.mid(yinSize * step, yinSize);
        if (forLocalYin.empty())
            break;
        auto pitch = calc_YinF0(forLocalYin.data(), forLocalYin.size(), _yinTheshold);
        _yinLine.append(pitch);
    }
}



bool WaveContour::loadWavFile(QString filename) {
    WavFile wav;
    wav.open(filename);
    QByteArray samplesBytes = wav.readAll();
    _floatSamples.clear();
    const char *ptr = samplesBytes.constData();
    for (int i = 0; i < samplesBytes.size()/2; ++i) {
        auto pcmSample2 = *reinterpret_cast<const qint16*>(ptr);
        const float realSample = pcmToReal(pcmSample2);
        _floatSamples.append(realSample);
        ptr += 2; //16 bit audio
    }
    calculateRms();
    return _floatSamples.empty() == false;
}


void WaveContour::calculateRms() {

    auto calcRms = [&](double coef, QVector<double>& container) {
      container.clear();
      double localStep = _rmsStep / coef;
      size_t rmsFrames = _floatSamples.size() / (localStep);
      for (size_t step = 0; step < rmsFrames; ++step) {
          auto forRmsLocal = _floatSamples.mid(localStep * step, _rmsStep);
          auto db = calc_dB(forRmsLocal.data(), forRmsLocal.size());
          container.append(db);
      }
    };

    calcRms(1.0, _rmsLine);
    calcRms(2.0, _halfRmsLine);
    calcRms(4.0, _quaterRmsLine);
    calcRms(8.0, _8RmsLine);

    std::vector<double> stdRms(_rmsLine.begin(), _rmsLine.end());
    _rmsHigh = peakIndexesInData(stdRms, _peakSensetivity);
    for (auto& s: stdRms)
        s *= -1;
    _rmsLow = peakIndexesInData(stdRms, _peakSensetivity);
}




