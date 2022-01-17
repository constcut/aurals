#include "WaveContour.hpp"

#include <QDebug>

#include "WavFile.hpp"
#include "audio/features/FeatureExtractor.hpp"
#include "audio/features/PeaksOperations.hpp"
#include "AudioUtils.hpp"


#include "libs/kiss/kiss_fftr.h"
#include "libs/fft/FFTurealfix.hpp"
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


inline float hannWindowFun(float t, size_t N) {
    return 0.5 * (1 - std::cos((2 * M_PI * t) / N)); //There was N-1 in Qt, but why?
}



//TODO sepparate it somewhere
void WaveContour::STFT(QString filename) {

    const size_t windowSize = 1024 * 4;
    const size_t windowStep = 64; //TODO 512 or 256 even


    kiss_fftr_cfg cfg = kiss_fftr_alloc( windowSize, 0, 0, 0 );


    FFTRealFixLen<12> fft_fix;

    size_t width = (_floatSamples.size() - windowSize) / windowStep;


    size_t specCut = 4;

    qDebug() << "Width " << width;
    QImage img(width + 1, (windowSize / specCut) + 1, QImage::Format_RGB32);


    auto start = std::chrono::high_resolution_clock::now();

    std::vector<float> windowFun(windowSize);
    for (size_t i = 0; i < windowSize; ++i)
        windowFun[i] = hannWindowFun(i, windowSize); //window[i]

    std::vector<float> windowed(windowSize);


    std::vector<kiss_fft_cpx> outKiss(windowSize); //TODO std::complex
    std::vector<float> outFix(windowSize);

    std::vector<float> amp(windowSize, 0.f);

    size_t height = windowSize / specCut;
    float hScale = log(height) / height;

    size_t count = 0;
    size_t position = 0;
    while (position + windowSize < _floatSamples.size()) {

        const float* winPos = &_floatSamples[position];

        for (size_t i = 0; i < windowSize; ++i)
            windowed[i] = winPos[i] * windowFun[i];

        kiss_fftr( cfg , windowed.data() , outKiss.data() );
        //TODO image
        //fft_fix.do_fft(outFix.data() , windowed.data());



        for (size_t i = 0; i < outKiss.size() / specCut; ++i) {

            const auto b = outKiss[i];
            const float magnitude = sqrt(b.i * b.i + b.r * b.r);
            const float amplitude = 0.15 * std::log(magnitude);
            amp[i] = amplitude;

            //const float simpleG = 255 * (amplitude); //log10(magnitude);
            //QColor color(0, simpleG, simpleG / 4.0); //.rgb
            //img.setPixel(count, windowSize/specCut - i, color.rgb());
        }

        for (size_t i = 0; i < outKiss.size() / specCut; ++i) {
            int index = pow(2.7182818284590452354, i * hScale);
            const float a = amp[index];
            const float simpleG = 255 * (a);
            QColor color(0, simpleG, simpleG / 4.0);
            img.setPixel(count, windowSize/specCut - i, color.rgb());
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
