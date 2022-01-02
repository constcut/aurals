#include "wavecontour.h"
#include "wavfile.h"

#include "featureextractor.h"
#include "findpeaks.hpp"
#include "utils.h"

#include <array>
#include <cmath>

#include <QDataStream>
#include <QDebug>

#include "findpeaks.hpp"


WaveContour::WaveContour(QString filename) {
    if (loadWavFile(filename)==false)
        qDebug() << "Failed to load contour from file "<<filename;
}


void WaveContour::calculateF0() {
    _yinLine.clear();
    size_t yinSize = 2048;
    size_t yinFrames = _floatSamples.size() / yinSize;
    for (size_t step = 0; step < yinFrames; ++step) {
        auto forLocalYin = _floatSamples.mid(yinSize * step, yinSize);
        if (forLocalYin.empty())
            break;
        auto pitch = calc_YinF0(forLocalYin.data(), forLocalYin.size(), _yinTheshold);
        //qDebug() << step << " Yin tracking " << pitch;
        _yinLine.append(pitch);
    }
}



bool WaveContour::loadWavFile(QString filename) { //TODO sepparate into sub-functions
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
    createSubRms();

    return _floatSamples.empty() == false;
}


void WaveContour::calculateRms() {
    size_t rmsFrames = _floatSamples.size() / _rmsStep;
    _rmsLine.clear();
    for (size_t step = 0; step < rmsFrames; ++step) {
        auto forRmsLocal = _floatSamples.mid(_rmsStep*step, _rmsStep);
        auto db = calc_dB(forRmsLocal.data(), forRmsLocal.size());
        _rmsLine.append(db);
    }

    std::vector<double> stdRms(_rmsLine.begin(), _rmsLine.end());
    _rmsHigh = peakIndexesInData(stdRms, _peakSensetivity);
    for (auto& s: stdRms)
        s *= -1;
    _rmsLow = peakIndexesInData(stdRms, _peakSensetivity);
}


void WaveContour::createSubRms() {
    {
        _halfRmsLine.clear();
        double localStep = _rmsStep / 2.0;
        size_t rmsFrames = _floatSamples.size() / (localStep);
        for (size_t step = 0; step < rmsFrames; ++step) {
            auto forRmsLocal = _floatSamples.mid(localStep * step, _rmsStep);
            auto db = calc_dB(forRmsLocal.data(), forRmsLocal.size());
            _halfRmsLine.append(db);
        }
    }
    {
        _quaterRmsLine.clear();
        double localStep = _rmsStep / 4.0;
        size_t rmsFrames = _floatSamples.size() / (localStep);
        for (size_t step = 0; step < rmsFrames; ++step) {
            auto forRmsLocal = _floatSamples.mid(localStep * step, _rmsStep);
            auto db = calc_dB(forRmsLocal.data(), forRmsLocal.size());
            _quaterRmsLine.append(db);
        }
    }
    {
        _8RmsLine.clear();
        double localStep = _rmsStep / 8.0;
        size_t rmsFrames = _floatSamples.size() / (localStep);
        for (size_t step = 0; step < rmsFrames; ++step) {
            auto forRmsLocal = _floatSamples.mid(localStep * step, _rmsStep);
            auto db = calc_dB(forRmsLocal.data(), forRmsLocal.size());
            _8RmsLine.append(db);
        }
    }
}


double WaveContour::findBPM() {
    return 0.0;
}
