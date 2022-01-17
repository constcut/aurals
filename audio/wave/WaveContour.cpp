#include "WaveContour.hpp"

#include <QDebug>

#include "WavFile.hpp"
#include "audio/features/FeatureExtractor.hpp"
#include "audio/features/PeaksOperations.hpp"
#include "AudioUtils.hpp"


#include "libs/kiss/kiss_fftr.h"
#include "libs/fft/FFTurealfix.hpp"
#include <QImage>
#include <QVector3D>


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



const double scalar32[32] = {0,0.032258065,0.064516129,0.096774194,0.129032258,0.161290323,0.193548387,0.225806452,0.258064516,0.290322581,0.322580645,0.35483871,0.387096774,0.419354839,0.451612903,0.483870968,0.516129032,0.548387097,0.580645161,0.612903226,0.64516129,0.677419355,0.709677419,0.741935484,0.774193548,0.806451613,0.838709677,0.870967742,0.903225806,0.935483871,0.967741935,1};

const int redDawn32[32] = {0,0,1,1,1,1,2,2,2,3,3,3,4,4,4,4,5,5,63,125,186,248,255,255,255,255,255,255,255,255,255,255};
const int greenDawn32[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,98,181,255,255,255,255};
const int blueDawn32[32] = {0,11,23,35,46,58,69,81,93,104,116,128,139,151,162,174,185,197,211,225,239,254,193,124,56,1,7,12,23,100,178,255};


//TODO sepparate it somewhere
void WaveContour::STFT(QString filename) {


    std::vector<QColor> colorVec;

    for (size_t i = 0; i < 32; ++i)
        colorVec.push_back(QColor(redDawn32[i], greenDawn32[i], blueDawn32[i]));

    const size_t windowSize = 4096*2;
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

    float minA = std::numeric_limits<float>::max();
    float maxA = std::numeric_limits<float>::min();

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

            if (amplitude > maxA)
                maxA = amplitude;

            if (amplitude < minA)
                minA = amplitude;

            /*
            float c = 255 * (a);
            if (a < 0.2)
                c /= 4;
            QColor color(0, c , c / 4);*/
        }

        for (size_t i = 0; i < outKiss.size() / specCut; ++i) {

            int index = pow(2.7182818284590452354, i * hScale);
            float a = amp[index];

            a = (a - minA) / (maxA - minA);
            a *= 0.95;

            size_t j;
            for (j = 0; j < 32; ++j)
                if (scalar32[j] > a)
                    break;

            img.setPixel(count, windowSize/specCut - i, colorVec[j].rgb());
        }


        position += windowStep;
        ++count;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    qDebug() << "Spent " << durationMs / 1000.0
             << " on whole STFT "  << count << " steps";

    qDebug() << minA << " " << maxA;

    img.save(filename);
}
