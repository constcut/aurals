﻿#include "WaveContour.hpp"

#include <QDebug>
#include <QImage>

#include "WavFile.hpp"
#include "audio/features/FeatureExtractor.hpp"
#include "audio/features/PeaksOperations.hpp"
#include "audio/wave/AudioUtils.hpp"
#include "audio/features/WindowFunction.hpp"

#include "libs/kiss/kiss_fftr.h"
#include "libs/cqt/ConstantQ.h"



using namespace aural_sight;


WaveContour::WaveContour(QString filename) {
    if (loadWavFile(filename)==false)
        qDebug() << "Failed to load contour from file "<<filename;
}


void WaveContour::calculateF0() {
    _yinLine.clear();
    size_t yinFrames = _floatSamples.size() / _yinWindowSize;
    for (size_t step = 0; step < yinFrames; ++step) {
        auto forLocalYin = std::vector<float>(_floatSamples.begin() + _yinWindowSize * step,
                                _floatSamples.begin() + _yinWindowSize * step + _yinWindowSize);
        if (forLocalYin.empty())
            break;
        auto pitch = calc_YinF0(forLocalYin.data(), forLocalYin.size(), _yinTheshold);
        _yinLine.push_back(pitch);
    }
}



bool WaveContour::loadWavFile(const QString filename) {
    WavFile wav;
    wav.open(filename);
    QByteArray samplesBytes = wav.readAll();
    _floatSamples.clear();
    const char *ptr = samplesBytes.constData();
    for (int i = 0; i < samplesBytes.size()/2; ++i) {
        auto pcmSample2 = *reinterpret_cast<const qint16*>(ptr);
        const float realSample = pcmToReal(pcmSample2);
        _floatSamples.push_back(realSample);
        ptr += 2; //16 bit audio
    }
    calculateRms();
    _audioFormat = wav.audioFormat();
    return _floatSamples.empty() == false;
}


void WaveContour::calculateRms() {

    auto calcRms = [&](double coef, std::vector<double>& container) {
      container.clear();
      double localStep = _rmsStep / coef;
      size_t rmsFrames = _floatSamples.size() / (localStep);
      for (size_t step = 0; step < rmsFrames; ++step) {
          auto forRmsLocal = std::vector<float>(_floatSamples.begin() + localStep * step,
                                _floatSamples.begin() + localStep * step + _rmsStep);
          auto db = calc_dB(forRmsLocal.data(), forRmsLocal.size());
          container.push_back(db);
      }
    };

    calcRms(1.0, _rmsLine);
    calcRms(2.0, _halfRmsLine);
    calcRms(4.0, _quaterRmsLine);
    calcRms(8.0, _8RmsLine);

    std::vector<double> stdRms(_rmsLine.begin(), _rmsLine.end());
    _rmsHigh = aural_sight::peakIndexes<double>(stdRms, _peakSensetivity);
    for (auto& s: stdRms)
        s *= -1;
    _rmsLow = aural_sight::peakIndexes<double>(stdRms, _peakSensetivity);
}



QByteArray WaveContour::getPCM(const quint64 position, const quint64 samples) {
    std::vector<quint16> intSamples(samples, 0); //Научить спекрограф получасть сразу float сэмплы
    for (size_t i = position; i < position + samples; ++i) {
       if (i >= _floatSamples.size())
           break;
       intSamples[i - position] = realToPcm(_floatSamples[i]);
    }
    const int bytesInSample = 2;
    QByteArray analyseData = QByteArray(reinterpret_cast<const char*>(intSamples.data()), intSamples.size() * bytesInSample);
    return analyseData;
}


QByteArray WaveContour::getFloatSamples(const quint64 position, const quint64 samples) {
    QByteArray analyseData = QByteArray(reinterpret_cast<const char*>(&_floatSamples[position]),
                                        samples * sizeof(float));
    return analyseData;
}



void WaveContour::STFTtoFile(QString filename) {
    makeSTFT().save(filename);
}



QRgb colorFromAmp(float amp) {
    int gColor = 256 * amp;
    int rColor = 0;
    int bColor = 0;

    if (gColor > 80)
        gColor += 64;
    if (gColor > 255)
        gColor = 255;

    if (gColor > 150)
        rColor = gColor / 3;
    if (gColor > 100)
        bColor = gColor / 2;

    QColor color(rColor,gColor,bColor);
    return color.rgb();
}


QImage WaveContour::makeSTFT() {
    const size_t windowSize = 4096*2;
    const size_t windowStep = 256;
    size_t width = (_floatSamples.size() - windowSize) / windowStep + 1;

    size_t height = 600;
    float hScale = log(height) / height;
    size_t spaceOf12Bins = 234; //TODO calculate
    size_t realHeight = 600 - spaceOf12Bins + 34;

    QImage img(width, realHeight, QImage::Format_RGB32);
    img.fill(QColor("black"));

    std::vector<float> window(windowSize);
    for (size_t i = 0; i < windowSize; ++i)
        window[i] = hannWindow(i, windowSize);

    std::vector<float> windowedSamples(windowSize);
    std::vector<kiss_fft_cpx> fftOutput(windowSize); //TODO std::complex
    std::vector<float> amplitudes(windowSize, 0.f);

    kiss_fftr_cfg cfg = kiss_fftr_alloc( windowSize, 0, 0, 0 );

    size_t count = 0;
    size_t position = 0;
    while (position + windowSize < _floatSamples.size()) {

        const float* winPos = &_floatSamples[position];
        for (size_t i = 0; i < windowSize; ++i)
            windowedSamples[i] = winPos[i] * window[i];

        kiss_fftr( cfg , windowedSamples.data() , fftOutput.data() );

        for (size_t i = 0; i < fftOutput.size() / 2; ++i) { // /2 because we don't search for all of the bins
            const auto b = fftOutput[i];
            const float magnitude = sqrt(b.i * b.i + b.r * b.r);
            float amplitude = 0.15 * std::log(magnitude);
            amplitude = std::max(0.f, amplitude);
            amplitude = std::min(1.f, amplitude);
            amplitudes[i] = amplitude;
        }

        for (size_t i = 0; i < height; ++i) {
            int index = pow(M_E, i * hScale);
            if (index < 12)
                continue;
            img.setPixel(count, realHeight - 1 - i + spaceOf12Bins, colorFromAmp(amplitudes[index]));
        }

        position += windowStep;
        ++count;
    }
    return img;
}



void WaveContour::makeCQT() {
    CQParameters params(44100, 100, 14700, 60);
    ConstantQ cq(params);

    auto start = std::chrono::high_resolution_clock::now();
    auto cqt = cq.process(_floatSamples);
    auto end = std::chrono::high_resolution_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    qDebug() << "Const-Q took " << durationMs / 1000.0 << "ms";

}
