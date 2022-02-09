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

#ifndef SPECTRUMANALYSER_H
#define SPECTRUMANALYSER_H

#include <memory>

#include <QByteArray>
#include <QObject>
#include <QVector>

#include "FrequencySpectrum.hpp"

#include "libs/cqt/dsp/FFT.h"
#include "audio/features/WindowFunction.hpp"

QT_FORWARD_DECLARE_CLASS(QAudioFormat)
QT_FORWARD_DECLARE_CLASS(QThread)

class FFTRealWrapper;
class SpectrumAnalyserThreadPrivate;


namespace aurals {

    class SpectrumAnalyserThread : public QObject {
        Q_OBJECT

    public:
        SpectrumAnalyserThread(QObject *parent);
        ~SpectrumAnalyserThread() = default;

        void setSamplesAmount(int newNumSamples);
        int getSamplesAmount() const { return _numSamples; }

        void changeHalfCut(bool newValue) { _halfCut = newValue; }

        void setFilter(int idx, double freq);

    public slots:
        void setWindowFunction(int idx);
        void calculateSpectrum(const QByteArray &buffer,
                               int inputFrequency,
                               int bytesPerSample);

       void calculateSpectrumFloat(const QByteArray &buffer);

    signals:
        void calculationComplete(const aurals::FrequencySpectrum &spectrum);

    private:
        void calculateWindow();

        void finishSpectrumCalculation(int inputFrequency);


        std::unique_ptr<FFTReal> _fft;
        int _numSamples;
        aurals::WindowFunction _windowFunction;
        bool _halfCut = false;

        int _filterIdx = -1;
        double _filterFreq = 0.0;

        QVector<float> _window;
        QVector<float> _input;
        QVector<float> _output;
        FrequencySpectrum _spectrum;
    };



    class SpectrumAnalyser : public QObject {
        Q_OBJECT

    public:
        SpectrumAnalyser(QObject *parent = 0);
        ~SpectrumAnalyser() = default;

        void setSamplesAmount(int newNumSamples) {
            if (_thread)
                _thread->setSamplesAmount(newNumSamples);
        }
        int getSamplesAmount() const {
            if (_thread)
                return _thread->getSamplesAmount();
            return 0;
        }

        void setWindowFunction(int idx);
        void calculate(const QByteArray& buffer, const QAudioFormat &format);
        void calculateFromFloat(const QByteArray& buffer);

        bool isReady() const;
        void cancelCalculation();


        void changeHalfCut(bool newValue) { _thread->changeHalfCut(newValue); }

        //C++ setters depth composition предложенние
        void setFilter(int idx, double freq) { _thread->setFilter(idx, freq); }

    signals:
        void spectrumChanged(const aurals::FrequencySpectrum &spectrum);

    private slots:
        void calculationComplete(const aurals::FrequencySpectrum &spectrum);

    private:
        void calculateWindow();

        SpectrumAnalyserThread*    _thread;
        enum State {
            Idle,
            Busy,
            Cancelled
        } _state;
    };

}

#endif // SPECTRUMANALYSER_H

