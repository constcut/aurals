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

#include "SpectrumAnalyser.hpp"

#include <QQuickPaintedItem>
#include <QPainter>
#include <QImage>

#include <unordered_set>

#include "FrequencySpectrum.hpp"
#include "audio/features/FeatureExtractor.hpp"


namespace aurals {

    class SpectrographPainter {

    public:
        SpectrographPainter();
        ~SpectrographPainter() = default;

        void paintSpectr(QPainter &painter, const QRect &rect);
        void setParams(const int numBars, const qreal lowFreq, const qreal highFreq);

    protected:
        int barIndex(const qreal frequency) const;
        QPair<qreal, qreal> barRange(const int barIndex) const;

        virtual void updateBars();

        void prepareBackground(QPainter &painter, const QRect &rect) const;
        void paintBars(QPainter &painter, const QRect &rect) const;
        void paintSlope(QPainter &painter, const QRect &rect) const;

        void findPeaks();
        void findF0();
        void classifySlope();

        void calcChroma();

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

        bool _gotClipping = false;
        bool _spectrumGap = false;
        double _gapLevel = 0.0;

        double _spectrumPitch = 0.0;
        double _specPitchAprox = 0.0;
        std::unordered_set<int> _peaksIdx;

        double _maxValue = -120;
        int _maxIdx = -1;
        int _lastIdx = - 1;
        double _lastValue = 0.0;

        QVector<double> _chroma;
        int _chromaIdx;
        QVector<double> _octaveEnergy;

        QImage _mainImage;
        bool _imagePainted = false;
    };



    class SpectrographQML : public QQuickPaintedItem, public SpectrographPainter
    {
            Q_OBJECT
    public:
        SpectrographQML(QQuickItem* parent = NULL);
        ~SpectrographQML() {}

        void paint(QPainter* painter);

        virtual void updateBars();

        Q_INVOKABLE void setSamplesAmount(const int newNumSamples){
            _analyser.setSamplesAmount(newNumSamples);
            _samplesAmount = newNumSamples;
        }

        Q_INVOKABLE int getSamplesAmount() const {
            return _analyser.getSamplesAmount();
        }

        Q_INVOKABLE void setParamsFromQML(const int numBars, const qreal lowFreq, const qreal highFreq) {
            setParams(numBars, lowFreq, highFreq);
        }

        Q_INVOKABLE void changeBarsCount(const int barsCount){
            Q_ASSERT(barsCount > 0);
            _bars.resize(barsCount);
            updateBars();
        }

        Q_INVOKABLE void changeHighFreq(const qreal newFreq) {
            _highFreq = newFreq;
            updateBars();
        }

        Q_INVOKABLE qreal getFreq1() const { return _barSelected * _freqStep; } //Delayed: barRange даёт смещение на 1, но наш вараинт кажется более правильным, проверить
        Q_INVOKABLE qreal getFreq2() const { return (_barSelected + 1) * _freqStep; }

        Q_INVOKABLE qreal getValue() const { return _bars[_barSelected].value; }
        Q_INVOKABLE int getIndex() const { return _barSelected; }

        Q_INVOKABLE qreal getSpectrumF0() const { return _spectrumPitch; }
        Q_INVOKABLE qreal getSpectrumAproxF0() const { return _specPitchAprox; }

        Q_INVOKABLE QVector<double> getChroma() const { return _chroma; }
        Q_INVOKABLE int getChromaMaxIdx() const { return _chromaIdx; }
        Q_INVOKABLE QVector<double> getOctaveEnergy() const { return _octaveEnergy; }


        Q_INVOKABLE bool clipped() const { return _gotClipping; }
        Q_INVOKABLE bool gotGap() const { return _spectrumGap; }
        Q_INVOKABLE qreal gapLevel() const { return _gapLevel;  }

        Q_INVOKABLE void onPress(const int xPress, const int yPress, const int width, const int height);

        Q_INVOKABLE bool loadSpectrum(QString filename, const quint64 position);
        Q_INVOKABLE bool loadByteArray(QByteArray analyseData);
        Q_INVOKABLE bool loadFloatSamples(QByteArray samples);

        Q_INVOKABLE void setWindowFunction(const int idx) {
            _analyser.setWindowFunction(idx);
        }

        Q_INVOKABLE void saveImage(QString filename) { _mainImage.save(filename); }

        Q_INVOKABLE void changeHalfCut(bool newValue) { _analyser.changeHalfCut(newValue); }

        Q_INVOKABLE void setFilter(int idx, double freq) { _analyser.setFilter(idx, freq); }

    public slots:
        void spectrumChanged(const qint64 position, const qint64 length,
                             const aurals::FrequencySpectrum &spectrum);

        void spectrumChanged(const aurals::FrequencySpectrum &spectrum);

    signals:
        void spectrumCalculated();

    protected:

        int _samplesAmount;
        void selectBar(const int index);

        SpectrumAnalyser _analyser;
    };

}


#endif // SPECTROGRAPH_H
