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

#include "spectrograph.h"
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QTimerEvent>

#include "wavfile.h"

#include <cmath>

//#include "soundengine.h" //TODO review for Audio handler (ideas)

const int NullTimerId = -1;
const int NullIndex = -1;
const int BarSelectionInterval = 2000;


SpectrographPainter::SpectrographPainter()    :   m_barSelected(NullIndex)
//,   m_timerId(NullTimerId)
,   m_lowFreq(0.0)
,   m_highFreq(0.0)
{
}

void SpectrographPainter::paintSpectr(QPainter &painter, QRect &rect)
{
    painter.fillRect(rect, Qt::black);

    const int numBars = m_bars.count();
    double barWidth = rect.width()/( static_cast<double>(numBars) );

   // qDebug() << "Num bars :"<<numBars;

    // Highlight region of selected bar
    //qDebug() << "BarW1 " << barWidth * 200 << " " << rect.width() << " " << rect.height() ;

    //Removed static cast
    auto calcXPos = [&rect, &barWidth](int index) { return (rect.topLeft().x() + index * barWidth); };

    if (m_barSelected != NullIndex && numBars) {
        QRectF regionRect = rect;
        regionRect.setLeft(calcXPos(m_barSelected));
        regionRect.setWidth(barWidth);
        QColor regionColor(202, 202, 64);
        painter.setBrush(Qt::DiagCrossPattern);
        painter.fillRect(regionRect, regionColor);
        painter.setBrush(Qt::NoBrush);
    }

    QColor barColor(51, 204, 102);
    QColor clipColor(255, 255, 0);

    // Draw the outline
    const QColor gridColor = barColor.darker();
    QPen gridPen(gridColor);
    painter.setPen(gridPen);
    painter.drawLine(rect.topLeft(), rect.topRight());
    painter.drawLine(rect.topRight(), rect.bottomRight());
    painter.drawLine(rect.bottomRight(), rect.bottomLeft());
    painter.drawLine(rect.bottomLeft(), rect.topLeft());

    QVector<qreal> dashes;
    dashes << 2 << 2;
    gridPen.setDashPattern(dashes);
    painter.setPen(gridPen);

    // Draw vertical lines between bars
    if (numBars) {
        const int numHorizontalSections = numBars;

        for (int i= 0; i < numHorizontalSections; ++i) {
            QLineF line(rect.topLeft(), rect.bottomLeft()); //TODO создавать каждый раз новую
            line.setP1({calcXPos(i), line.y1()});
            line.setP2({line.x2() + static_cast<int>(barWidth * i), line.y2()});
            painter.drawLine(line);
        }
    }

    // Draw horizontal lines
    const int numVerticalSections = 10;
    QLine line(rect.topLeft(), rect.topRight());
    for (int i = 0; i < numVerticalSections; ++i) {
        line.translate(0, rect.height()/(numVerticalSections));
        painter.drawLine(line);
    }

    barColor = barColor.lighter();
    barColor.setAlphaF(0.75);
    clipColor.setAlphaF(0.75);

    // Draw the bars
    if (numBars) {

        freqPeaks.clear();
        ampPeaks.clear();

        // Calculate width of bars and gaps
        const int widgetWidth = rect.width();
        const int barPlusGapWidth = widgetWidth / numBars;
        const int barWidth = barPlusGapWidth;
        const int gapWidth =  barPlusGapWidth - barWidth;
        const int barHeight = rect.height() - 2 * gapWidth;

        const qreal bandWidth = freqStep; //(m_highFreq - m_lowFreq) / m_bars.count();

        for (int i=0; i < numBars; ++i) {
            qreal value = m_bars[i].value;
            Q_ASSERT(value >= 0.0 && value <= 1.0);
            QRectF bar = rect;
            bar.setLeft(calcXPos(i)); //rect.left() + leftPaddingWidth + (i * (gapWidth + barWidth)));
            bar.setWidth(barWidth);

            double volume = 20.0 * log10(value);
            if (volume > -30.0) {
                double freq = bandWidth * (i + 0.5);
                freqPeaks.append(freq);
                ampPeaks.append(volume);
            }

            bar.setTop(rect.top() + gapWidth + (1.0 - value) * barHeight);
            bar.setBottom(rect.bottom() - gapWidth);

            QColor color = barColor;
            if (m_bars[i].clipped)
                color = clipColor;

            painter.fillRect(bar, color);
        }
        //qDebug() << "Total peaks " << freqPeaks.size();
    }
    else
        qDebug () << "No bars to draw for qml";
}



void SpectrographPainter::setParams(int numBars, qreal lowFreq, qreal highFreq)
{
    Q_ASSERT(numBars > 0);
    Q_ASSERT(highFreq > lowFreq);
    m_bars.resize(numBars);
    m_lowFreq = lowFreq;
    m_highFreq = highFreq;
    updateBars();
}

/*void Spectrograph::timerEvent(QTimerEvent *event)
{
    Q_ASSERT(event->timerId() == m_timerId);
    Q_UNUSED(event) // suppress warnings in release builds
    killTimer(m_timerId);
    m_timerId = NullTimerId;
    m_barSelected = NullIndex;
    update();
    //may be one more update fade
}*/

//=============================

int SpectrographPainter::barIndex(qreal frequency) const
{
    Q_ASSERT(frequency >= m_lowFreq && frequency < m_highFreq);
    const qreal bandWidth = (m_highFreq - m_lowFreq) / m_bars.count();
    const int index = (frequency - m_lowFreq) / bandWidth;
    if (index <0 || index >= m_bars.count())
        Q_ASSERT(false);
    return index;
}

QPair<qreal, qreal> SpectrographPainter::barRange(int index) const
{
    Q_ASSERT(index >= 0 && index < m_bars.count());
    const qreal bandWidth = (m_highFreq - m_lowFreq) / m_bars.count();
    return QPair<qreal, qreal>(index * bandWidth, (index+1) * bandWidth);
}

void SpectrographPainter::updateBars()
{
    m_bars.fill(Bar());
    FrequencySpectrum::const_iterator i = m_spectrum.begin();
    const FrequencySpectrum::const_iterator end = m_spectrum.end();

    size_t count = 0; //skipped in calculation
    double prevFreq = 0;
    double diffFreq = 0;

    gotClipping = false;

    for ( ; i != end; ++i) {
        const FrequencySpectrum::Element e = *i;
        if (e.frequency >= m_lowFreq && e.frequency < m_highFreq && count < m_bars.size()) {
            //auto idx = barIndex(e.frequency); //Точный размер
            Bar &bar = m_bars[count];
            bar.value = qMax(bar.value, e.amplitude);
            bar.clipped |= e.clipped;
            gotClipping |= e.clipped;

            diffFreq = e.frequency - prevFreq;
            prevFreq = e.frequency; //TODO сделать адекватней
        }
        ++count;
    }

    freqStep = diffFreq;

    //update();
}


//========================================================

SpectrographQML::SpectrographQML(QQuickItem* parent):soundEngine(0),samplesAmount(4096)
{
    setParams(SpectrumNumBands, SpectrumLowFreq, SpectrumHighFreq);
    //defaults

    QObject::connect(&analyser,SIGNAL(spectrumChanged(FrequencySpectrum)),
                     this,SLOT(spectrumChanged(FrequencySpectrum)));

}

void SpectrographQML::paint(QPainter* painter)
{
   QRect rect;
   rect.setX(0); rect.setY(0);
   rect.setWidth(this->width());
   rect.setHeight(this->height());

   paintSpectr(*painter,rect);

   emit spectrumCalculated();
}

void SpectrographQML::updateBars()
{
    SpectrographPainter::updateBars();
    update();
}


void SpectrographQML::spectrumChanged(qint64 position, qint64 length,
                                      const FrequencySpectrum &spectrum)
{
    m_spectrum = spectrum;
    SpectrographQML::updateBars();
}

void SpectrographQML::spectrumChanged(const FrequencySpectrum &spectrum)
{
    m_spectrum = spectrum;
    SpectrographQML::updateBars();
}


void SpectrographQML::setSoundEngine(QObject *eng)
{
    //soundEngine = qobject_cast<Engine*>(eng);

    qDebug() << "UNDONE: TODO implement ideas sound engine connection to audio handler";
    //CHECKED_CONNECT(soundEngine, SIGNAL(spectrumChanged(qint64, qint64, const FrequencySpectrum &)),
            //this, SLOT(spectrumChanged(qint64, qint64, const FrequencySpectrum &)));
}


void SpectrographQML::selectBar(int index) {
    Q_ASSERT(index >= 0 && index < m_bars.count());
    m_barSelected = index;
    //qDebug() << "Selected index " << index;
    update();
}

void SpectrographQML::onPress(int xPress, int yPress, int width, int height)
{
    QRect rect(0,0,width,height);
    qreal barWidth = static_cast<double>(width) / m_bars.count();
    const int index = (static_cast<double>(xPress) / barWidth);
    selectBar(index);
}


 bool SpectrographQML::loadSpectrum(QString filename, quint64 position) //TODO использовать уже загруженный буфер
 {
    WavFile wav;
    if ( wav.open(filename) == false)
       return false;
    quint64 afterHeaderPosition = wav.pos();
    wav.seek(afterHeaderPosition + position*2);
    QByteArray analyseData = wav.read(samplesAmount*2);
    if (analyseData.size() != samplesAmount*2)
        return false;
    analyser.calculate(analyseData, wav.audioFormat());
    return true;
 }
