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

#include <QPainterPath>


#include <qglobal.h>

void WaveshapeQML::paint(QPainter *painter)
{
    setFillColor(Qt::darkGray);
    paintWaveShape(*painter);
}


void WaveshapePainter::paintWaveShape(QPainter &painter)
{
    //int width = painter.device()->width();
    int height = painter.device()->height();
    #ifdef Q_OS_ANDROID
        height /= 2; //Android is a broke on Qt
    #endif
    double heightCoef = height / 200.0;


    if (noImage) {

        noImage = false;
        mainImage =  QImage(painter.device()->width(),painter.device()->height(), QImage::Format_ARGB32);

        QPainter imgPainter(&mainImage);

        auto zoom64 = waveContour.getZoom64();
        auto zoom256 = waveContour.getZoom128();
        auto zoom128 = waveContour.getZoom256();

        //TODO нужно хранить пропорцию сжатия 125 убрать константу

        imgPainter.setPen(QColor("green"));
        for (int i = 0; i < zoom256.size(); ++i) {
            ContourEl conturEl = zoom256[i];
            const int h = 2*conturEl.energy/(65000.0/heightCoef);
            imgPainter.drawLine(i, height/2, i, height/2 + h);
            imgPainter.drawLine(i, height/2, i, height/2 - h);
        }

        imgPainter.setPen(QColor("chartreuse"));
        int prevX1=0, prevX2=0, prevY1=0, prevY2=0;
        for (int i = 0; i < zoom64.size(); ++i)
        {
            ContourEl conturEl = zoom64[i];
            imgPainter.drawLine(i*2, height/2 - conturEl.energy/(65000.0/heightCoef), prevX1, prevY1);
            imgPainter.drawLine(i*2, height/2 + conturEl.energy/(65000.0/heightCoef), prevX2, prevY2);
            prevX1 = prevX2 = i*2;
            prevY1 = height/2 - conturEl.energy/(65000.0/heightCoef);
            prevY2 = height/2 + conturEl.energy/(65000.0/heightCoef);
        }
        prevX1=0;
        prevX2=0;
        prevY1=0;
        prevY2=0;
        imgPainter.setPen(QColor("darkgreen"));
        for (int i = 0; i < zoom128.size(); ++i)
        {
            ContourEl conturEl = zoom128[i];
            imgPainter.drawLine(i/2, height/2, i/2, height/2 + 2*conturEl.energy/(65000.0/heightCoef));
            imgPainter.drawLine(i/2, height/2, i/2, height/2 - 2*conturEl.energy/(65000.0/heightCoef));
            imgPainter.drawLine(i / 2, height/2 - 2*conturEl.energy/(65000.0/heightCoef), prevX1, prevY1);
            imgPainter.drawLine(i / 2, height/2 + 2*conturEl.energy/(65000.0/heightCoef), prevX2, prevY2);
            prevX1 = prevX2 = i / 2;
            prevY1 = height/2 - 2*conturEl.energy/(65000.0/heightCoef);
            prevY2 = height/2 + 2*conturEl.energy/(65000.0/heightCoef);
        }
        auto rms = waveContour.getRMS();
        imgPainter.setPen(QColor("blue"));
        for (int i = 0; i < rms.size(); ++i) {
            auto localRms = rms[i];
            imgPainter.drawLine(i*2, 0, i*2,  (60.0 + localRms)*heightCoef);
        }
    }

    painter.drawImage(QPoint{0,0}, mainImage);

    //TODO optional
    auto pitchLine = waveContour.getPitch();
    painter.setPen(QColor("red"));
    double prevPitch = -1;
    double coef = (2048.0) / (125.0 / 2.0);
    for (int i = 0; i < pitchLine.size(); ++i) {
        auto pitch = pitchLine[i];
        painter.drawLine((i-1)*coef, height - prevPitch / 4.0, i*coef, height - pitch / 4.0);
        prevPitch = pitch;
    }

    if (showNotes) {
        auto noteStarts = waveContour.getNoteStarts();
        painter.setPen(QColor("yellow"));
        for (int i = 0; i < noteStarts.size(); ++i) {
            auto start = noteStarts[i];
            const double pixelX = 2.0 * start / (125.0);
            painter.drawEllipse(pixelX-5, height/2-5, 10, 10);
        }
        auto noteEnds = waveContour.getNoteEnds();
        painter.setPen(QColor("orange"));
        for (int i = 0; i < noteEnds.size(); ++i) {
            auto start = noteEnds[i];
            const double pixelX = 2.0 * start / (125.0);
            painter.drawEllipse(pixelX-5, height/2-5, 10, 10);
        }
    }

    //painter.restore();
    {
        painter.setPen(Qt::darkMagenta);
        painter.drawRect(2*windowPosition/125,0,2*windowWidth/125,height);
       // qDebug() <<"WP "<<windowPosition<<" WW "<<windowWidth;
    }
}


void WaveshapePainter::audioPositionChanged(qint64 position)
{
    audioPoistion = position;

    //maybe check how ofte we are updated
    //emit update();
}


void WaveshapePainter::loadContour(QString filename)
{
    waveContour = WaveContour(filename);
    //emit update();
}


void WavePositionQML::audioPositionChanged(qint64 position)
{
    audioPoistion = position;
    update();
}

void WavePositionQML::changePosition(qint64 position)
{
    audioPoistion = position;
    update();
}



void WavePositionQML::paint(QPainter *painter)
{
    setFillColor(Qt::lightGray);
    const int verticalPosition = 10;

    painter->setPen(Qt::black);

    qint64 cursorPosition = audioPoistion/125;
    cursorPosition *= 2;

    painter->drawEllipse(cursorPosition,verticalPosition,5,5); //make something better

    for (int i = 0; i < 11; ++i)
        painter->drawLine(i*256,0,i*256,40);
 }
