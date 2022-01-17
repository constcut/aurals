#include "WaveContour.hpp"

#include <QDebug>

#include "WavFile.hpp"
#include "audio/features/FeatureExtractor.hpp"
#include "audio/features/PeaksOperations.hpp"
#include "AudioUtils.hpp"


#include "libs/kiss/kiss_fftr.h"
#include <QImage>


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
        _yinLine.push_back(pitch.first);
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



//TODO sepparate it somewhere
void WaveContour::STFT(QString filename) {

    const size_t windowSize = 1024;
    const size_t windowStep = 64; //TODO 512 or 256 even


    kiss_fftr_cfg cfg = kiss_fftr_alloc( windowSize, 0, 0, 0 );
    std::vector<kiss_fft_cpx> outKiss(windowSize); //TODO std::complex

    size_t width = (_floatSamples.size() - windowSize) / windowStep;

    qDebug() << "Width " << width;
    QImage img(width + 1, (windowSize / 2) + 1, QImage::Format_RGB32);


    auto start = std::chrono::high_resolution_clock::now();

    size_t count = 0;
    size_t position = 0;
    while (position + windowSize < _floatSamples.size()) {
        const float* window = &_floatSamples[position];
        kiss_fftr( cfg , window , outKiss.data() );
        //TODO image

        for (size_t i = 0; i < outKiss.size() / 2; ++i) {
            const auto b = outKiss[i];
            const float magnitude = b.i * b.i + b.r * b.r;
            const float simpleG = 255 * magnitude;
            QColor color(0, simpleG, 0);
            img.setPixel(count, windowSize/2 - i, color.rgb());
        }

        position += windowStep;
        ++count;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    qDebug() << "Spent " << durationMs / 1000.0
             << " on whole STFT "  << count << " steps";

    img.save(filename);
}
