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

#include "audio/features/FeatureExtractor.hpp"
#include "audio/wave/AudioUtils.hpp"
#include "audio/features/WindowFunction.hpp"

#include "libs/filters/Iir.h"


using namespace aural_sight;


SpectrumAnalyserThread::SpectrumAnalyserThread(QObject *parent)
    :   QObject(parent)
    ,   _numSamples(4096)
    ,   _windowFunction(HannWindow)
    ,   _window(4096*4, 0.f)
    ,   _input(4096*4, 0.f)
    ,   _output(4096*4, 0.f)
    ,   _spectrum(4096*4) //Biggest buffers we may use in our code
{
    _fft = std::make_unique<FFTReal>(_numSamples);
    calculateWindow();
}


void SpectrumAnalyserThread::setWindowFunction(int idx) {
    if (_windowFunction != static_cast<aural_sight::WindowFunction>(idx)) {
        _windowFunction = static_cast<aural_sight::WindowFunction>(idx);
        calculateWindow();
    }
}


void SpectrumAnalyserThread::setSamplesAmount(int newNumSamples) {
    if (newNumSamples != _numSamples) {
        _numSamples = newNumSamples;
        _fft = std::make_unique<FFTReal>(_numSamples);
        calculateWindow();
    }
}



void SpectrumAnalyserThread::calculateWindow() {
    for (int i=0; i<_numSamples; ++i) {
        float x = 0.0;
        switch (_windowFunction) {
        case NoWindow:
            x = 1.0;
            break;
        case HannWindow:
            x =  hannWindow(i, _numSamples); //0.5 * (1 - qCos((2 * M_PI * i) / (_numSamples - 1)));
            break;
        case GausWindow:
            x = gausWindow(i, _numSamples);
            break;
        case BlackmanWindow:
            x = blackmanWindow(i, _numSamples);
            break;
        case HammWindow:
            x = hammWindow(i, _numSamples);
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

    Q_ASSERT(buffer.size() == _numSamples * bytesPerSample);
    const char *ptr = buffer.constData(); //Delayed: from preloaded (загружать float сразу, без преобразований)

    //TODO realsamples count as below

    for (int i=0; i<_numSamples ; ++i) {

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

    finishSpectrumCalculation(inputFrequency);
}


void SpectrumAnalyserThread::calculateSpectrumFloat(const QByteArray &buffer) {

    const float *ptr = reinterpret_cast<const float*>(buffer.constData());

    const int realSamplesCount = _halfCut ? _numSamples / 2 : _numSamples;

    for (int i = 0; i< realSamplesCount; ++i) {
        const float windowedSample = ptr[i] * _window[i];
        _input[i] = windowedSample;
    }

    if (_filterIdx != -1) {

        auto setupAndFilter = [&](auto& filter) {
            filter.setup(44100.0, _filterFreq);
            for (int i = 0; i< realSamplesCount; ++i)
                _input[i] = filter.filter(_input[i]);
        };

        if (_filterIdx == 0) {
            Iir::Butterworth::LowPass<4> f;
            setupAndFilter(f);
        }
        if (_filterIdx == 1) {
            Iir::Butterworth::HighPass<4> f;
            setupAndFilter(f);
        }

    }

    if (_halfCut)
        for (int i = realSamplesCount; i < _numSamples; ++i)
            _input[i] = 0.f;

    finishSpectrumCalculation(44100);
}


void SpectrumAnalyserThread::setFilter(int idx, double freq) {
    _filterIdx = idx;
    _filterFreq = freq;

    qDebug() << "Setten filter " << idx << " with freq " << freq;
}



void SpectrumAnalyserThread::finishSpectrumCalculation(int inputFrequency) {

    _fft->forwardMagnitude(_input.data(), _output.data());

    const double SpectrumAnalyserMultiplier = 0.15;

    for (int i=0; i<= _numSamples / 2; ++i) {
        _spectrum[i].frequency = qreal(i * inputFrequency) / (_numSamples);

        const double magnitude = _output[i]; //fullsize??
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


#define CHECKED_CONNECT(source, signal, receiver, slot) \
    if (!connect(source, signal, receiver, slot)) \
        qt_assert_x(Q_FUNC_INFO, "CHECKED_CONNECT failed", __FILE__, __LINE__);


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

void SpectrumAnalyser::setWindowFunction(int idx) {
    /*
    const bool b = QMetaObject::invokeMethod(_thread, "setWindowFunction",
                              Qt::AutoConnection,
                              Q_ARG(WindowFunction, type));
    Q_ASSERT(b);
    Q_UNUSED(b) // suppress warnings in release builds*/

    _thread->setWindowFunction(idx);
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


void SpectrumAnalyser::calculateFromFloat(const QByteArray& buffer)
{

    if (isReady()) {

        _state = Busy;
        _thread->setFFTLimit(_fftLimit);
        const bool b = QMetaObject::invokeMethod(_thread, "calculateSpectrumFloat",
                                  Qt::AutoConnection,
                                  Q_ARG(QByteArray, buffer));
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
