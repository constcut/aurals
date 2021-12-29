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

#include "frequencyspectrum.h"

#include "featureextractor.h"

#include <QQuickPaintedItem>
#include <QPainter>
/**
 * Widget which displays a spectrograph showing the frequency spectrum
 * of the window of audio samples most recently analyzed by the Engine.
 */

class SpectrographPainter
{
public:
    SpectrographPainter();
    ~SpectrographPainter() {}

    void paintSpectr(QPainter &painter, QRect &rect);

    void setParams(int numBars, qreal lowFreq, qreal highFreq);

    /*
signals:
    void infoMessage(const QString &message, int intervalMs);
    */

protected:
    int barIndex(qreal frequency) const;
    QPair<qreal, qreal> barRange(int barIndex) const;


    virtual void updateBars();

protected:

    struct Bar {
        Bar() : value(0.0), clipped(false) { }
        qreal   value;
        bool    clipped;
    };

    int m_barSelected;
    QVector<Bar>        m_bars;
    qreal               m_lowFreq;
    qreal               m_highFreq;
    FrequencySpectrum   m_spectrum;

    QVector<qreal> freqPeaks;
    QVector<qreal> ampPeaks;
};


class Engine;

class SpectrographQML : public QQuickPaintedItem, public SpectrographPainter
{
        Q_OBJECT
public:
    SpectrographQML(QQuickItem* parent = NULL);
    ~SpectrographQML() {}

    void paint(QPainter* painter);

    virtual void updateBars();

    Q_INVOKABLE void setSamplesAmount(int newNumSamples){
        analyser.setSamplesAmount(newNumSamples);
        samplesAmount = newNumSamples;
    }

    Q_INVOKABLE int getSamplesAmount() {
        return analyser.getSamplesAmount();
    }

    Q_INVOKABLE void setSoundEngine(QObject *eng);

    Q_INVOKABLE void setParamsFromQML(int numBars, qreal lowFreq, qreal highFreq)
    { setParams(numBars,lowFreq,highFreq);}

    Q_INVOKABLE void changeBarsCount(int barsCount)
    {
        Q_ASSERT(barsCount > 0);
        m_bars.resize(barsCount);
        updateBars();
    }

    Q_INVOKABLE qreal getFreq1() { const qreal bandWidth = (m_highFreq - m_lowFreq) / m_bars.count();
                                     return m_barSelected * bandWidth; }

    Q_INVOKABLE qreal getFreq2() { const qreal bandWidth = (m_highFreq - m_lowFreq) / m_bars.count();
                                     return (m_barSelected + 1) * bandWidth; }

    Q_INVOKABLE qreal getValue()  { return m_bars[m_barSelected].value; }
    Q_INVOKABLE qreal getRMS() { return m_spectrum.rms; }
    Q_INVOKABLE qreal getRMSNoWindow() { return m_spectrum.rmsNoWindow; }
    Q_INVOKABLE qreal getPitch() { return m_spectrum.pitch; }


    Q_INVOKABLE void onPress(int xPress, int yPress, int width, int height);
    Q_INVOKABLE bool loadSpectrum(QString filename, quint64 position);

    Q_INVOKABLE void setYinLimit(int limit) { analyser.yinLimit = limit; };
    Q_INVOKABLE void setFFTLimit(int limit) { analyser.fftLimit = limit; };

    Q_INVOKABLE qreal freqToMidi(qreal freq) {
        return calc_MidiCents(freq) / 100.0;
    }

    Q_INVOKABLE int peaksCount() { return freqPeaks.size(); }
    Q_INVOKABLE QVector<qreal> getFreqPeaks() { return freqPeaks; }
    Q_INVOKABLE QVector<qreal> getAmpPeaks() { return ampPeaks; }

public slots:
    void spectrumChanged(qint64 position, qint64 length,
                         const FrequencySpectrum &spectrum);

    void spectrumChanged(const FrequencySpectrum &spectrum);


signals:

    void spectrumCalculated();

protected:

    int samplesAmount;
    void selectBar(int index);

    Engine *soundEngine; //TODO собрать лучшее из ideas
    SpectrumAnalyser analyser;
};




#endif // SPECTROGRAPH_H
