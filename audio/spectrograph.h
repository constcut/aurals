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

#ifndef SPECTROGRAPH_H
#define SPECTROGRAPH_H

#include "spectrumanalyser.h"

#include <QQuickPaintedItem>
#include <QPainter>

#include <unordered_set>

#include "frequencyspectrum.h"
#include "featureextractor.h"


class SpectrographPainter {

public:
    SpectrographPainter();
    ~SpectrographPainter() = default;

    void paintSpectr(QPainter &painter, QRect &rect) const;
    void setParams(int numBars, qreal lowFreq, qreal highFreq);

protected:
    int barIndex(qreal frequency) const;
    QPair<qreal, qreal> barRange(int barIndex) const;

    virtual void updateBars();

    void prepareBackground(QPainter &painter, QRect &rect) const;
    void paintBars(QPainter &painter, QRect &rect) const;
    void paintSlope(QPainter &painter, QRect &rect) const;

    void findPeaks();
    void findF0();
    void classifySlope();

protected:

    struct Bar {
        Bar() : value(0.0), clipped(false) { }
        qreal   value;
        bool    clipped;
    };

    int _barSelected;
    QVector<Bar>        _bars;
    qreal               _lowFreq;
    qreal               _highFreq;
    FrequencySpectrum   _spectrum;

    double _freqStep;
    bool _gotClipping = false; //TODO group them all
    bool _spectrumGap = false;
    double _gapLevel = 0.0;

    double _spectrumPitch = 0.0;
    double _specPitchAprox = 0.0;

    QVector<qreal> _idxPeaksAmp;
    QVector<int> _idxPeaks;

    QVector<int> _binTable;
    QVector<int> _binCount;

    std::unordered_set<int> peaksIdx;

    double _maxValue = -120;
    int _maxIdx = -1;
    int _lastIdx = - 1;
    double _lastValue = 0.0;
};



class SpectrographQML : public QQuickPaintedItem, public SpectrographPainter
{
        Q_OBJECT
public:
    SpectrographQML(QQuickItem* parent = NULL);
    ~SpectrographQML() {}

    void paint(QPainter* painter);

    virtual void updateBars();

    Q_INVOKABLE void setSamplesAmount(int newNumSamples){
        _analyser.setSamplesAmount(newNumSamples);
        _samplesAmount = newNumSamples;
    }

    Q_INVOKABLE int getSamplesAmount() {
        return _analyser.getSamplesAmount();
    }

    Q_INVOKABLE void setParamsFromQML(int numBars, qreal lowFreq, qreal highFreq) {
        setParams(numBars,lowFreq,highFreq);
    }

    Q_INVOKABLE void changeBarsCount(int barsCount){
        Q_ASSERT(barsCount > 0);
        _bars.resize(barsCount);
        updateBars();
    }

    Q_INVOKABLE void changeHighFreq(qreal newFreq) {
        _highFreq = newFreq;
        updateBars();
    }

    Q_INVOKABLE qreal getFreq1() const { return _barSelected * _freqStep; } //TODO barRange
    Q_INVOKABLE qreal getFreq2() const { return (_barSelected + 1) * _freqStep; }
    Q_INVOKABLE qreal getPitch() const { return _spectrum._pitchYin; }
    Q_INVOKABLE qreal freqToMidi(qreal freq) const { return calc_MidiCents(freq) / 100.0; }
    Q_INVOKABLE qreal getValue() const { return _bars[_barSelected].value; }
    Q_INVOKABLE int getIndex() const { return _barSelected; }

    Q_INVOKABLE qreal getSpectrumF0() const { return _spectrumPitch; }
    Q_INVOKABLE qreal getSpectrumAproxF0() const { return _specPitchAprox; }
    Q_INVOKABLE QVector<int> getBinTable() const { return _binTable; }
    Q_INVOKABLE QVector<int> getBinCount() const { return _binCount; }

    Q_INVOKABLE qreal getRMS() const { return _spectrum._rms; }
    Q_INVOKABLE qreal getRMSNoWindow() const { return _spectrum._rmsNoWindow; }
    Q_INVOKABLE bool clipped() const { return _gotClipping; }
    Q_INVOKABLE bool gotGap() const { return _spectrumGap; }
    Q_INVOKABLE qreal gapLevel() const { return _gapLevel;  }

    Q_INVOKABLE void onPress(int xPress, int yPress, int width, int height);

    Q_INVOKABLE bool loadSpectrum(QString filename, quint64 position);
    Q_INVOKABLE bool loadByteArray(QByteArray analyseData);

    Q_INVOKABLE void setYinLimit(int limit) { _analyser.yinLimit = limit; };
    Q_INVOKABLE void setFFTLimit(int limit) { _analyser.fftLimit = limit; };

    Q_INVOKABLE void setYinThreshold(double threshold) { _analyser.yinThreshold = threshold; }

public slots:
    void spectrumChanged(qint64 position, qint64 length,
                         const FrequencySpectrum &spectrum);

    void spectrumChanged(const FrequencySpectrum &spectrum);


signals:
    void spectrumCalculated();

protected:

    int _samplesAmount;
    void selectBar(int index);

    SpectrumAnalyser _analyser;
};




#endif // SPECTROGRAPH_H
