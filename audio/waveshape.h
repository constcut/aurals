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
#include <QImage>

#include "wavecontour.h"

class WaveshapePainter  {

public:
    WaveshapePainter():_windowPosition(0),_windowWidth(4096){}

    void loadContour(QString filename);
    void paintWaveShape(QPainter& painter);

    void audioPositionChanged(qint64 position);
    void invertShowNotes() { _showNotes = !_showNotes;}

protected:
    WaveContour _waveContour;
    qint64 _audioPoistion;

    quint64 _windowPosition;
    quint16 _windowWidth;

    bool _noImage = true;
    QImage _mainImage;

    bool _showNotes = false;

    void makeBackgroungImage(QPainter &painter, int height, double heightCoef);
    void drawPitch(QPainter &painter, int height);
    void drawNoteStartEnd(QPainter &painter, int height);
};


class WaveshapeQML : public QQuickPaintedItem, public WaveshapePainter {
    Q_OBJECT
public:
    explicit WaveshapeQML([[maybe_unused]] QQuickItem* parent = NULL) {}
    ~WaveshapeQML() = default;

    Q_INVOKABLE int getPixelsLength() {
        return _waveContour.getZoom64().size();
    }
    Q_INVOKABLE qreal getRMS(int index) const {
        return _waveContour.getRMS()[index];
    }
    Q_INVOKABLE int getRmsSize() const {
        return _waveContour.getRMS().size();
    }
    Q_INVOKABLE int getRmsStep() const {
        return _waveContour.getRmsStep();
    }

    void paint(QPainter* painter);

    Q_INVOKABLE  void loadFile(QString filename) { loadContour(filename);}

    Q_INVOKABLE void setWindowPosition(quint64 newPosition) { _windowPosition = newPosition; update(); }
    Q_INVOKABLE quint64 getWindowPosition() const { return _windowPosition; }
    Q_INVOKABLE void setWindowWidth(quint64 newWidth) { _windowWidth = newWidth; update(); }
    Q_INVOKABLE void calculateF0() { _waveContour.calculateF0(); update(); }
    Q_INVOKABLE void showNotes() { invertShowNotes(); update();}
};



class WavePositionQML : public QQuickPaintedItem
{
    Q_OBJECT

public:
    explicit WavePositionQML([[maybe_unused]] QQuickItem* parent = NULL) {}
    ~WavePositionQML() = default;

    void paint(QPainter* painter);
    Q_INVOKABLE void changePosition(qint64 position);

public slots:
    void audioPositionChanged(qint64 position);

private:
    qint64 _audioPoistion;
};

#endif // Waveshape_H
