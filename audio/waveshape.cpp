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

#include <qglobal.h>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

#include <unordered_set>


void WaveshapeQML::paint(QPainter *painter) {
    setFillColor(Qt::darkGray);
    paintWaveShape(*painter);
}


void WaveshapePainter::makeBackgroungImage(QPainter &painter, int height, double heightCoef) {

    _noImage = false;
    _mainImage =  QImage(painter.device()->width(),painter.device()->height(), QImage::Format_ARGB32);
    QPainter imgPainter(&_mainImage);

    auto rmsStep = _waveContour.getRmsStep();
    auto paintRms = [&](double rmsCoef, double expandCoef,
            const std::vector<double>& container, QColor base, QColor border) {
        double xCoef = rmsStep / (125 * rmsCoef / 2.0);
        double prevValue = 0.0;
        for (size_t i = 0; i < container.size(); ++i) {
            auto localRms = container[i];
            double currentValue = (60.0 + localRms) * heightCoef * expandCoef;
            imgPainter.setPen(base);
            imgPainter.drawLine(i*xCoef, height/2, i*xCoef, height/2 - currentValue);
            imgPainter.drawLine(i*xCoef, height/2, i*xCoef, height/2 + currentValue);
            imgPainter.setPen(border);
            imgPainter.drawLine((i-1)*xCoef, height/2 - prevValue, i*xCoef, height/2 - currentValue);
            imgPainter.drawLine((i-1)*xCoef, height/2 + prevValue, i*xCoef, height/2 + currentValue);
            prevValue = currentValue;
        }
    };

    paintRms(8.0, 1.5, _waveContour.getRMS_8(), QColor("darkgreen"), QColor("chartreuse")); //TODO 8 vs 4 etc
    paintRms(4.0, 1.1, _waveContour.getRMS_4(), QColor("green"), QColor("green"));
    paintRms(2.0, 0.8, _waveContour.getRMS_2(), QColor("green").lighter(110), QColor("green").lighter(110));
    paintMainRms(imgPainter, height, heightCoef);
}


void WaveshapePainter::paintMainRms(QPainter &imgPainter, int height, double heightCoef) {
    auto highs = _waveContour.rmsHigh();
    std::unordered_set<size_t> positionsHigh(highs.begin(), highs.end());
    auto lows = _waveContour.rmsLow();
    std::unordered_set<size_t> positionsLow(lows.begin(), lows.end());
    auto rms = _waveContour.getRMS();

    auto rmsStep = _waveContour.getRmsStep();
    double xCoef = rmsStep / (125 / 2.0);
    double prevValue = -1.0;

    for (size_t i = 0; i < rms.size(); ++i) { //TODO? отрисовать лямбдой, и после этого сделать дополнительную отривоску начал\концов
        if (positionsHigh.count(i))
            imgPainter.setPen(QColor("blue"));
        else if (positionsLow.count(i))
            imgPainter.setPen(QColor("red"));
        else
            imgPainter.setPen(QColor("green").lighter(125));

        double currentValue = (60.0 + rms[i])*heightCoef * 0.4;
        QPolygonF poligon;
        poligon << QPointF((i-1)*xCoef, height/2.0 - prevValue) << QPointF(i * xCoef, height/2.0 - currentValue)
                << QPointF(i*xCoef, height/2 + currentValue) << QPointF((i-1)*xCoef, height/2 + prevValue);

        if (prevValue != -1.0)
            imgPainter.drawPolygon(poligon); //TODO fill this path or minimal rectangle

        if (positionsLow.count(i) || positionsHigh.count(i))
            imgPainter.drawLine(i * xCoef, height/2 + currentValue * 2, i * xCoef, height/2 - currentValue * 2);
        prevValue = currentValue;
    }
}


void WaveshapePainter::drawPitch(QPainter &painter, int height) {
    auto pitchLine = _waveContour.getPitch();
    painter.setPen(QColor("red"));
    double prevPitch = -1;
    auto rmsStep = _waveContour.getRmsStep();
    double coef = (2048.0) / (rmsStep / 2.0);
    for (size_t i = 0; i < pitchLine.size(); ++i) {
        auto pitch = pitchLine[i];
        painter.drawLine((i-1)*coef, height - prevPitch / 4.0, i*coef, height - pitch / 4.0);
        prevPitch = pitch;
    }
}


void WaveshapePainter::drawNoteStartEnd(QPainter &painter, int height) {
    //TODO use data from high and low rms
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
    auto x1 = 2.0 * _windowPosition / (_waveContour.getMinRmsStep());
    auto x2 = 2.0 * _windowWidth / (_waveContour.getMinRmsStep());
    painter.drawRect(x1, 0, x2, height);
}


void WaveshapePainter::audioPositionChanged(qint64 position) {
    _audioPoistion = position;
}


void WaveshapePainter::loadContour(QString filename) {
    _waveContour = WaveContour(filename);
}
