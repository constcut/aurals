/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "SpectrumAnalyser.hpp"

#include <qmath.h>
#include <qmetatype.h>
#include <QAudioFormat>

#include "libs/fft/fftreal_wrapper.h"
#include "audio/features/FeatureExtractor.hpp"
#include "audio/wave/AudioUtils.hpp"


using namespace aural_sight;


SpectrumAnalyserThread::SpectrumAnalyserThread(QObject *parent)
    :   QObject(parent)
    ,   _numSamples(SpectrumLengthSamples)
    ,   _windowFunction(DefaultWindowFunction)
    ,   _window(4096*4, 0.0) //SpectrumLengthSamples
    ,   _input(4096*4, 0.0)
    ,   _output(4096*4, 0.0)
    ,   _spectrum(4096*4) //16 before
{
    _fft = std::make_unique<FFTRealWrapper>();
    calculateWindow();
}

void SpectrumAnalyserThread::setWindowFunction(aural_sight::WindowFunction type) {
    _windowFunction = type;
    calculateWindow();
}

void SpectrumAnalyserThread::calculateWindow() {
    for (int i=0; i<_numSamples; ++i) {
        float x = 0.0;
        switch (_windowFunction) {
        case NoWindow:
            x = 1.0;
            break;
        case HannWindow:
            x = 0.5 * (1 - qCos((2 * M_PI * i) / (_numSamples - 1)));
            break;
        default:
            Q_ASSERT(false);
        }
        _window[i] = x;
    }
}

void SpectrumAnalyserThread::calculateSpectrum(const QByteArray &buffer,
                                                int inputFrequency,
                                                int bytesPerSample) {

    //TODO sepparate into sub function + accept different types both 16 bit and 32
    Q_ASSERT(buffer.size() == _numSamples * bytesPerSample);
    const char *ptr = buffer.constData(); //Delayed: from preloaded (загружать float сразу, без преобразований)
    for (int i=0; i<_numSamples; ++i) {

        if (i > _fftLimit) {
            _input[i] = 0.f;
        }
        else {
            const qint16 pcmSample = *reinterpret_cast<const qint16*>(ptr);
            const float realSample = pcmToReal(pcmSample); // Scale down to range [-1.0, 1.0]
            const float windowedSample = realSample * _window[i];
            _input[i] = windowedSample;
        }
        ptr += bytesPerSample;
    }

    auto start = std::chrono::high_resolution_clock::now();
    _fft->calculateFFT(_output.data(), _input.data()); //m_noWindowInput m_input
    auto end = std::chrono::high_resolution_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    //qDebug() << "FFT done for " << durationMs;


    for (int i=0; i<=_numSamples/2; ++i) {
        _spectrum[i].frequency = qreal(i * inputFrequency) / (_numSamples);
        const qreal real = _output[i];
        qreal imag = 0.0;
        if (i>0 && i<_numSamples/2)
            imag = _output[_numSamples/2 + i];
        const qreal magnitude = qSqrt(real*real + imag*imag);
        qreal amplitude = SpectrumAnalyserMultiplier * qLn(magnitude);

        _spectrum[i].clipped = (amplitude > 1.0); // Bound amplitude to [0.0, 1.0]
        amplitude = qMax(qreal(0.0), amplitude);
        amplitude = qMin(qreal(1.0), amplitude);
        _spectrum[i].amplitude = amplitude;
    }
    emit calculationComplete(_spectrum);
}


//=============================================================================
// SpectrumAnalyser
//=============================================================================

SpectrumAnalyser::SpectrumAnalyser(QObject *parent)
    :   QObject(parent)
    ,   _thread(new SpectrumAnalyserThread(this))
    ,   _state(Idle)
{
    CHECKED_CONNECT(_thread, SIGNAL(calculationComplete(aural_sight::FrequencySpectrum)),
                    this, SLOT(calculationComplete(aural_sight::FrequencySpectrum)));
}



//-----------------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------------

void SpectrumAnalyser::setWindowFunction(WindowFunction type) {
    const bool b = QMetaObject::invokeMethod(_thread, "setWindowFunction",
                              Qt::AutoConnection,
                              Q_ARG(WindowFunction, type));
    Q_ASSERT(b);
    Q_UNUSED(b) // suppress warnings in release builds
}

void SpectrumAnalyser::calculate(const QByteArray &buffer,
                         const QAudioFormat &format) {

    //qDebug() << "SpectrumAnalyser::calculate" << QThread::currentThread() << "state" << _state;
    if (isReady()) {
        Q_ASSERT(isPCMS16LE(format));
        const int bytesPerSample = format.sampleSize() * format.channelCount() / 8;

        _state = Busy;
        _thread->setFFTLimit(_fftLimit);
        const bool b = QMetaObject::invokeMethod(_thread, "calculateSpectrum",
                                  Qt::AutoConnection,
                                  Q_ARG(QByteArray, buffer),
                                  Q_ARG(int, format.sampleRate()),
                                  Q_ARG(int, bytesPerSample));
        Q_ASSERT(b);
        Q_UNUSED(b) // suppress warnings in release builds
    }
}


bool SpectrumAnalyser::isReady() const {
    return (Idle == _state);
}


void SpectrumAnalyser::cancelCalculation() {
    if (Busy == _state)
        _state = Cancelled;
}


void SpectrumAnalyser::calculationComplete(const aural_sight::FrequencySpectrum &spectrum)
{
    Q_ASSERT(Idle != _state);
    if (Busy == _state)
        emit spectrumChanged(spectrum);
    _state = Idle;
}
