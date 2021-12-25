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

void WaveshapeQML::paint(QPainter *painter)
{
    setFillColor(Qt::darkGray);
    paintWaveShape(*painter);
}


void WaveshapePainter::paintWaveShape(QPainter &painter)
{
    QList<ContourEl> zoom64 = waveContour.getZoom64();
    QList<ContourEl> zoom256 = waveContour.getZoom256();



    //=======================Amplitudogramm========
    //thn we can make it symetric

    painter.setPen(QColor(10,20,255));
    for (int i = 0; i < zoom256.size(); ++i)
    {
        ContourEl conturEl = zoom256[i];
        painter.setPen(QColor(10,20,255));
        painter.drawLine(i, 100, i, 100 + 2*conturEl.energy/40000.0);
        painter.drawLine(i, 100, i, 100 - 2*conturEl.energy/40000.0);
        painter.setPen(QColor(10,255,255));
        painter.drawLine(i, 100, i, 100 - conturEl.min / 1000);
        painter.drawLine(i, 100, i, 100 + conturEl.max / 1000);
    }
    painter.setPen(QColor(10,255,20));
    int prevX1=0, prevX2=0, prevY1=0, prevY2=0;
    for (int i = 0; i < zoom64.size(); ++i)
    {
        ContourEl conturEl = zoom64[i];
        //painter.drawLine(i*64, 100, i*64, 100+ conturEl.energy/80000.0);
        //painter.drawLine(i*64, 100, i*64, 100 - conturEl.energy/80000.0);
        painter.drawLine(i*64, 100 - 2*conturEl.energy/80000.0, prevX1, prevY1);
        painter.drawLine(i*64, 100 + 2*conturEl.energy/80000.0, prevX2, prevY2);
        prevX1 = prevX2 = i*64;
        prevY1 = 100 - 2*conturEl.energy/80000.0;
        prevY2 = 100 + 2*conturEl.energy/80000.0;
    }
    if (windowPosition != -1)  {
        painter.setPen(Qt::darkMagenta);
        painter.drawRect(2*windowPosition/125,0,2*windowWidth/125,200);
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
