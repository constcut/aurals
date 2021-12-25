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

#ifndef Waveshape_H
#define Waveshape_H

//#include <QAudioFormat>
//#include <QPixmap>
//#include <QScopedPointer>
#include <QQuickPaintedItem>

#include "wavecontour.h"

class WaveshapePainter //: QObject
{
    //Q_OBJECT
public:
    WaveshapePainter():windowPosition(0),windowWidth(4096){}

    void loadContour(QString filename);
    void paintWaveShape(QPainter &painter);

//public slots:
    void audioPositionChanged(qint64 position);

protected:
    WaveContour waveContour;
    qint64 audioPoistion;

    quint64 windowPosition;
    quint16 windowWidth;
};


class WaveshapeQML : public QQuickPaintedItem, public WaveshapePainter
{
    Q_OBJECT
public:
    explicit WaveshapeQML(QQuickItem* parent = NULL) {}
    ~WaveshapeQML() {}

    void paint(QPainter* painter);

    Q_INVOKABLE  void loadFile(QString filename) { loadContour(filename);}

    Q_INVOKABLE void setWindowPosition(quint64 newPosition) { windowPosition = newPosition; update(); }
    Q_INVOKABLE void setWindowWidth(quint64 newWidth) { windowWidth = newWidth; update(); }
};



class WavePositionQML : public QQuickPaintedItem
{
    Q_OBJECT

public:
    explicit WavePositionQML(QQuickItem* parent = NULL){}
    ~WavePositionQML(){}

    void paint(QPainter* painter);

    Q_INVOKABLE void changePosition(qint64 position);

public slots:
    void audioPositionChanged(qint64 position);

private:
    qint64 audioPoistion;
};

#endif // Waveshape_H
