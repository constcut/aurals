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

#include "waveshape.h"
//#include "utils.h"
#include <QPainter>
#include <QResizeEvent>
#include <QDebug>

#include <set>

#include <qglobal.h>


void WaveshapeQML::paint(QPainter *painter) {
    setFillColor(Qt::darkGray);
    paintWaveShape(*painter);
}


void WaveshapePainter::makeBackgroungImage(QPainter &painter, int height, double heightCoef) {

    _noImage = false;
    _mainImage =  QImage(painter.device()->width(),painter.device()->height(), QImage::Format_ARGB32);

    QPainter imgPainter(&_mainImage);

    //imgPainter.setPen(QColor("green"));
    //imgPainter.setPen(QColor("chartreuse"));
    //imgPainter.setPen(QColor("darkgreen"));
    auto rmsStep = _waveContour.getRmsStep();


    auto rms_8 = _waveContour.getRMS_8();
    double xCoef = rmsStep / (125 * 4.0);
    double prevValue = 0.0;
    imgPainter.setPen(QColor("darkgreen"));
    for (int i = 0; i < rms_8.size(); ++i) {
        auto localRms = rms_8[i];
        double currentValue = (60.0 + localRms)*heightCoef * 1.6;
        imgPainter.drawLine(i*xCoef, height/2, i*xCoef, height/2 - currentValue);
        imgPainter.drawLine((i-1)*xCoef, height/2 - prevValue, i*xCoef, height/2 - currentValue);
        imgPainter.drawLine(i*xCoef, height/2, i*xCoef, height/2 + currentValue);
        imgPainter.drawLine((i-1)*xCoef, height/2 + prevValue, i*xCoef, height/2 + currentValue);
        prevValue = currentValue;
    }

    auto rms_4 = _waveContour.getRMS_4();
    xCoef = rmsStep / (125 * 2.0);
    prevValue = 0.0;
    imgPainter.setPen(QColor("green"));
    for (int i = 0; i < rms_4.size(); ++i) {
        auto localRms = rms_4[i];
        double currentValue = (60.0 + localRms)*heightCoef * 1.4;
        imgPainter.drawLine(i*xCoef, height/2, i*xCoef, height/2 - currentValue);
        imgPainter.drawLine((i-1)*xCoef, height/2 - prevValue, i*xCoef, height/2 - currentValue);
        imgPainter.drawLine(i*xCoef, height/2, i*xCoef, height/2 + currentValue);
        imgPainter.drawLine((i-1)*xCoef, height/2 + prevValue, i*xCoef, height/2 + currentValue);
        prevValue = currentValue;
    }

    auto rms_2 = _waveContour.getRMS_2(); //TODO rename better
    xCoef = rmsStep / (125);
    prevValue = 0.0;
    imgPainter.setPen(QColor("green").lighter(110));
    for (int i = 0; i < rms_2.size(); ++i) {
        auto localRms = rms_2[i];
        double currentValue = (60.0 + localRms)*heightCoef * 1.2;
        imgPainter.drawLine(i*xCoef, height/2, i*xCoef, height/2 - currentValue);
        imgPainter.drawLine((i-1)*xCoef, height/2 - prevValue, i*xCoef, height/2 - currentValue);
        imgPainter.drawLine(i*xCoef, height/2, i*xCoef, height/2 + currentValue);
        imgPainter.drawLine((i-1)*xCoef, height/2 + prevValue, i*xCoef, height/2 + currentValue);
        prevValue = currentValue;
    }



    auto highs = _waveContour.rmsHigh();
    std::set<size_t> positionsHigh(highs.begin(), highs.end());
    auto lows = _waveContour.rmsLow();
    std::set<size_t> positionsLow(lows.begin(), lows.end());

    auto rms = _waveContour.getRMS();
    xCoef = rmsStep / (125 / 2.0);
    prevValue = 0.0;
    for (int i = 0; i < rms.size(); ++i) {
        if (positionsHigh.count(i))
            imgPainter.setPen(QColor("blue"));
        else if (positionsLow.count(i))
            imgPainter.setPen(QColor("red"));
        else
            imgPainter.setPen(QColor("darkgray"));
        auto localRms = rms[i];

        double currentValue = (60.0 + localRms)*heightCoef;
        imgPainter.drawLine(i*xCoef, height/2, i*xCoef, height/2 - currentValue);
        imgPainter.drawLine((i-1)*xCoef, height/2 - prevValue, i*xCoef, height/2 - currentValue);
        imgPainter.drawLine(i*xCoef, height/2, i*xCoef, height/2 + currentValue);
        imgPainter.drawLine((i-1)*xCoef, height/2 + prevValue, i*xCoef, height/2 + currentValue);
        prevValue = currentValue;
    }
}

void WaveshapePainter::drawPitch(QPainter &painter, int height) {
    auto pitchLine = _waveContour.getPitch();
    painter.setPen(QColor("red"));
    double prevPitch = -1;
    double coef = (2048.0) / (125.0 / 2.0);
    for (int i = 0; i < pitchLine.size(); ++i) {
        auto pitch = pitchLine[i];
        painter.drawLine((i-1)*coef, height - prevPitch / 4.0, i*coef, height - pitch / 4.0);
        prevPitch = pitch;
    }
}


void WaveshapePainter::drawNoteStartEnd(QPainter &painter, int height) {
    auto noteStarts = _waveContour.getNoteStarts();
    painter.setPen(QColor("yellow"));
    for (int i = 0; i < noteStarts.size(); ++i) {
        auto start = noteStarts[i];
        const double pixelX = 2.0 * start / (125.0);
        painter.drawEllipse(pixelX-5, height/2-5, 10, 10);
    }
    auto noteEnds = _waveContour.getNoteEnds();
    painter.setPen(QColor("orange"));
    for (int i = 0; i < noteEnds.size(); ++i) {
        auto start = noteEnds[i];
        const double pixelX = 2.0 * start / (125.0);
        painter.drawEllipse(pixelX-5, height/2-5, 10, 10);
    }
}


void WaveshapePainter::paintWaveShape(QPainter &painter)
{
    int height = painter.device()->height();
    #ifdef Q_OS_ANDROID
        height /= 2; //Android is a broke on Qt
    #endif
    double heightCoef = height / 200.0;

    if (_noImage)
        makeBackgroungImage(painter, height, heightCoef);
    painter.drawImage(QPoint{0,0}, _mainImage);

    drawPitch(painter, height);

    if (_showNotes)
        drawNoteStartEnd(painter, height);

    painter.setPen(Qt::darkMagenta);
    painter.drawRect(2*_windowPosition/125,0,2*_windowWidth/125,height);
}


void WaveshapePainter::audioPositionChanged(qint64 position) {
    _audioPoistion = position;
    //maybe check how ofte we are updated
    //emit update();
}


void WaveshapePainter::loadContour(QString filename) {
    _waveContour = WaveContour(filename);
    //emit update();
}


void WavePositionQML::audioPositionChanged(qint64 position) {
    _audioPoistion = position;
    update();
}

void WavePositionQML::changePosition(qint64 position) {
    _audioPoistion = position;
    update();
}


void WavePositionQML::paint(QPainter *painter)  {
    setFillColor(Qt::lightGray);
    const int verticalPosition = 10;

    painter->setPen(Qt::black);

    qint64 cursorPosition = _audioPoistion/125;
    cursorPosition *= 2;

    painter->drawEllipse(cursorPosition,verticalPosition,5,5); //make something better

    for (int i = 0; i < 11; ++i)
        painter->drawLine(i*256,0,i*256,40);
 }
