﻿#include "WaveContour.hpp"

#include <QDebug>
#include <QImage>

#include <complex>

#include "WavFile.hpp"
#include "audio/features/FeatureExtractor.hpp"
#include "audio/features/PeaksOperations.hpp"
#include "audio/wave/AudioUtils.hpp"
#include "audio/features/WindowFunction.hpp"

#include "libs/kiss/kiss_fftr.h"
#include "libs/cqt/CQSpectrogram.h"



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


void WaveContour::STFTtoFile(QString filename) const {
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


QImage WaveContour::makeSTFT() const {
    const size_t windowSize = 4096*2;
    const size_t windowStep = 256;
    size_t width = (_floatSamples.size() ) / windowStep + 1; //- windowSize

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
    std::vector<std::complex<float>> fftOutput(windowSize);
    std::vector<float> amplitudes(windowSize, 0.f);

    kiss_fftr_cfg cfg = kiss_fftr_alloc( windowSize, 0, 0, 0 );

    size_t count = windowSize / windowStep;
    size_t position = 0;
    while (position + windowSize < _floatSamples.size()) {

        const float* winPos = &_floatSamples[position];
        for (size_t i = 0; i < windowSize; ++i)
            windowedSamples[i] = winPos[i] * window[i];

        kiss_fftr( cfg , windowedSamples.data() , reinterpret_cast<kiss_fft_cpx*>(fftOutput.data()) );

        for (size_t i = 0; i < fftOutput.size() / 2; ++i) { // /2 because we don't search for all of the bins
            const auto b = fftOutput[i];
            const float magnitude = std::sqrt(std::norm(b));
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


void WaveContour::CQTtoFile(QString filename) const {
    makeCQT().save(filename);
}


QImage WaveContour::makeCQT() const {
    CQParameters params(44100, 60, 14700, 60);
    CQSpectrogram cq(params, CQSpectrogram::Interpolation::InterpolateLinear);

    size_t latency = cq.getLatency();
    std::vector<std::vector<float>> spectrogram;

    const float* fbuf = _floatSamples.data();
    size_t inframe = 0;

    while (inframe < _floatSamples.size())
    {
        size_t count = 0;
        if (_floatSamples.size() - inframe < 1024)
            count = _floatSamples.size() - inframe;
        else
            count = 1024;

        auto cQ = cq.process(fbuf, count);

        if (cQ.empty() == false && inframe > latency)
            spectrogram.insert(spectrogram.end(), cQ.begin(), cQ.end());

        inframe += count;
        fbuf += 1024;
    }

    auto r = cq.getRemainingOutput();
    spectrogram.insert(spectrogram.end(), r.begin(), r.end());

    size_t fullLen = _floatSamples.size() / 64;
    spectrogram.erase(spectrogram.begin() + fullLen, spectrogram.end());

    QImage img = QImage(spectrogram.size(), spectrogram[0].size(), QImage::Format_RGB32);

    for (size_t i = 0; i < spectrogram.size(); ++i) {
        for (size_t j = 0; j < spectrogram[i].size(); ++j) {
            const float mag = spectrogram[i][j];
            const float norm = (mag / 12.f) * 255; //TODO normalization + colormap
            QColor c(0, norm, 0);
            img.setPixel(i, j, c.rgb());
        }
    }

    return img;
}



QImage WaveContour::makeCWT() const {

    qDebug() << "Wavelets to good to be legal!";
    qDebug() << "If you want to turn them include files into project.";
    return QImage();

    //#include "libs/wavelet/wavelib.h"

    //+add libs/wavelet/* to project

    //Another reference on full scaled wavelets:

    /*
    size_t length = 44100 * 2;

    std::vector<double> samples(length, 0.0);
    std::vector<double> outSamples(length, 0.0);

    for (size_t i = 0; i < length; ++i)
        samples[i] = _floatSamples[44100 + i];


    char wave[] = "morlet";// Set Morlet wavelet. Other options "paul" and "dog"
    char type[] = "pow";

    int N = length;
    double param = 6.0;
    int subscale = 16;
    double dt = 1.0 / 44100.0;
    double s0 =  dt;
    double dj = 1.0 / (double)subscale;
    int J = 128 * subscale; // Total Number of scales
    int a0 = 2;//power


    cwt_object wt = cwt_init(wave, param, N, dt, J);

    setCWTScales(wt, s0, dj, type, a0);

    auto start = std::chrono::high_resolution_clock::now();
    cwt(wt, samples.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    qDebug() << "CWT took " << durationMs / 1000.0 << " ms ";

    cwt_summary(wt);

    icwt(wt, outSamples.data());

    auto end2 = std::chrono::high_resolution_clock::now();
    auto durationMs2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - end).count();

    qDebug() << "iCWT took " << durationMs2 / 1000.0 << " ms ";

    //check by inversed


    QImage img = QImage(length, J, QImage::Format_RGB32);

    double max = std::numeric_limits<double>::min();
    double min = std::numeric_limits<double>::max();


    for (size_t i = 0; i < static_cast<size_t>(J) * length; ++i) {
            auto s = wt->output[i]; //TODO std::complex
            const double mag = std::sqrt(s.im * s.im + s.re * s.re);

            const float norm = (mag / 1.5) * 255; //As above normalization..
            QColor c(0, norm, 0); //QColor c(i % 255, norm, 0);


            img.setPixel(i % length, i / length, c.rgb());

            if (max < mag)
                max = mag;
            if (min > mag)
                min = mag;
    }


    auto s = img.scaled(length / 40, J);

    s.save("cwt.jpg");

    auto end3 = std::chrono::high_resolution_clock::now();
    auto durationMs3 = std::chrono::duration_cast<std::chrono::microseconds>(end3 - end2).count();

    qDebug() << "IMAGE took " << durationMs3 / 1000.0 << " ms ";


    qDebug() << "Max " << max << " min " << min;


    std::vector<float> dump(length, 0.f);
    for (size_t i = 0; i < length; ++i)
        dump[i] = outSamples[i];

    WavFile out;
    out.open("cwt.wav", QIODevice::WriteOnly);
    out.writeHeader(44100, 32, dump.size() * sizeof(float), false, true);
    out.write(reinterpret_cast<const char*>(dump.data()), dump.size() * sizeof(float));

    return s;*/
}
