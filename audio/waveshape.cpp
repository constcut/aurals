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
    QList<ContourEl> zoom4 = waveContour.getZoom4();
    QList<ContourEl> zoom16 = waveContour.getZoom16();
    QList<ContourEl> zoom64 = waveContour.getZoom64();
    QList<ContourEl> zoom256 = waveContour.getZoom256();

    //=======================Amplitudogramm========
    //thn we can make it symetric

    painter.setPen(QColor(10,20,255));
    for (int i = 0; i < zoom256.size(); ++i)
    {
        ContourEl conturEl = zoom256[i];
        painter.drawLine(i, 100, i, 100 + conturEl.energy/40000.0);
        painter.drawLine(i, 100, i, 100 - conturEl.energy/40000.0);
    }
    painter.setPen(QColor(10,255,20));
    for (int i = 0; i < zoom64.size(); ++i)
    {
        ContourEl conturEl = zoom64[i];
        painter.drawLine(i*4, 100, i*4, 100+ conturEl.energy/80000.0);
        painter.drawLine(i*4, 100, i*4, 100 - conturEl.energy/80000.0);
    }
    painter.setPen(Qt::gray);
    for (int i = 0; i < zoom16.size(); ++i)
    {
        ContourEl conturEl = zoom16[i];
        painter.drawLine(i*16, 100, i*16, 100+ conturEl.energy/160000.0);
        painter.drawLine(i*16, 100, i*16, 100- conturEl.energy/160000.0);
    }
    painter.setPen(Qt::lightGray);
    for (int i = 0; i < zoom4.size(); ++i)
    {
        ContourEl conturEl = zoom4[i];
        painter.drawLine(i*64, 100, i*64, 100+ conturEl.energy/320000.0);
        painter.drawLine(i*64, 100, i*64, 100- conturEl.energy/320000.0);
    }
    //needed optimization - each step by one line

    //=======================Amplitudogramm========


    //=======================Oscialogramm==========


    /*
    ContourEl prevEl;
    for (int i = 0; i < zoom256.size(); ++i)
    {
        ContourEl conturEl = zoom256[i];

        qint32 heightOfEl = conturEl.max - conturEl.min;
        heightOfEl /= 1024;

        qint32 yOff = (conturEl.min + conturEl.max)/1024;

        painter.setPen(QColor(10,20,255));
        painter.drawLine(i,300+yOff,i,300+yOff+heightOfEl);

        qint32 yOffPrev = (prevEl.min + prevEl.max)/1024;
        qint32 heightPrev = (conturEl.max - conturEl.min)/1024;

        painter.setPen(QColor(10,255,20));
        painter.drawLine(i, 300 +yOff, i-1,300+yOffPrev);
        painter.drawLine(i, 300 +yOff + heightOfEl, i-1,300+yOffPrev + heightPrev);

        prevEl = conturEl;
    }
    */

    /*
    painter.setPen(QColor(10,255,20));
    for (int i = 0; i < zoom64.size(); ++i)
    {
        ContourEl conturEl = zoom64[i];

        qint32 heightOfEl = conturEl.max - conturEl.min;
        heightOfEl /= 1024;

        qint32 yOff = (conturEl.min + 32535)/1024;

        painter.drawRect(i*4,200+yOff,4,heightOfEl);
    }

    painter.setPen(Qt::gray);
    for (int i = 0; i < zoom16.size(); ++i)
    {
        ContourEl conturEl = zoom16[i];

        qint32 heightOfEl = conturEl.max - conturEl.min;
        heightOfEl /= 1024;

        qint32 yOff = (conturEl.min + 32535)/1024;

        painter.drawRect(i*16,200+yOff,16,heightOfEl);
    }
    painter.setPen(Qt::lightGray);
    for (int i = 0; i < zoom4.size(); ++i)
    {
        ContourEl conturEl = zoom4[i];

        qint32 heightOfEl = conturEl.max - conturEl.min;
        heightOfEl /= 1024;

        qint32 yOff = (conturEl.min + 32535)/1024;


        painter.drawRect(i*64,200+yOff,64,heightOfEl);
    }*/



    if (windowPosition != -1)
    {
        painter.setPen(Qt::darkMagenta);
        painter.drawRect(2*windowPosition/125,0,2*windowWidth/125,200);


       // qDebug() <<"WP "<<windowPosition<<" WW "<<windowWidth;
    }

    /*
    painter.setPen(Qt::black);
    qint64 cursorPosition = audioPoistion/125;
    cursorPosition *= 2;

    const int verticalPosition = 200;

    painter.drawEllipse(cursorPosition,verticalPosition,5,5); //make something better

     painter.setPen(Qt::red);
     painter.drawEllipse(cursorPosition+2,verticalPosition+2,2,2);
    */

    //=======================Oscialogramm==========

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