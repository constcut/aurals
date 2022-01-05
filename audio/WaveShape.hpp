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

#include <QQuickPaintedItem>
#include <QImage>

#include "WaveContour.hpp"

namespace mtherapp {


    class WaveshapePainter  {

    public:
        WaveshapePainter():_windowPosition(0),_windowWidth(4096){}

        void loadContour(QString filename);
        void paintWaveShape(QPainter& painter);

        double calculateWindowRmsDb();
        double calculateWindowYin();

    protected:
        WaveContour _waveContour;

        quint64 _windowPosition;
        quint16 _windowWidth;

        bool _noImage = true;
        QImage _mainImage;

        bool _showNotes = false;

        void makeBackgroungImage(QPainter &painter, int height, double heightCoef);
        void paintMainRms(QPainter &painter, int height, double heightCoef);

        void drawPitch(QPainter &painter, int height);
        void drawNoteStartEnd(QPainter &painter, int height);

        void invertShowNotes() { _showNotes = !_showNotes;}

        void changePeakSence(double newSence) {
            _waveContour.changePeakSence(newSence);
            resetImage();
        }

        void changeRmsStep(size_t newStep) {
            _waveContour.setRmsStep(newStep);
            resetImage();
        }

        void changeYinTheshold(double threshold) {
            _waveContour.changeYinTheshold(threshold);
        }

        void resetImage() {
            _mainImage = QImage();
            _noImage = true;
        }
    };



    class WaveshapeQML : public QQuickPaintedItem, public WaveshapePainter {
        Q_OBJECT
    public:
        explicit WaveshapeQML() = default; //[[maybe_unused]] QQuickItem* parent = NULL OMG mingw fails in 2022
        ~WaveshapeQML() = default;

        Q_INVOKABLE int getPixelsLength() { return _waveContour.getRMS_8().size();  } // TODO rewrite names
        Q_INVOKABLE qreal getRMS(int index) const { return _waveContour.getRMS()[index]; }
        Q_INVOKABLE int getRmsSize() const { return _waveContour.getRMS().size(); }
        Q_INVOKABLE int getRmsStep() const { return _waveContour.getRmsStep();}
        Q_INVOKABLE int getMinRmsStep() const { return _waveContour.getMinRmsStep(); }

        void paint(QPainter* painter);

        Q_INVOKABLE  void loadFile(QString filename) { loadContour(filename);}

        Q_INVOKABLE void setWindowPosition(quint64 newPosition) { _windowPosition = newPosition; update(); }
        Q_INVOKABLE quint64 getWindowPosition() const { return _windowPosition; }
        Q_INVOKABLE void setWindowWidth(quint64 newWidth) { _windowWidth = newWidth; update(); }

        Q_INVOKABLE void calculateF0() { _waveContour.calculateF0(); update(); }
        Q_INVOKABLE void setYinWindowSize(int newSize) { _waveContour.setYinWindowSize(newSize); }

        Q_INVOKABLE void showNotes() { invertShowNotes(); update();}

        Q_INVOKABLE void setPeakSence(double newSence) { changePeakSence(newSence); update();}
        Q_INVOKABLE void setRmsStep(int newStep) { changeRmsStep(newStep); update(); }

        Q_INVOKABLE void setYinThreshold(double threshold) { changeYinTheshold(threshold); update(); }

        Q_INVOKABLE QByteArray getPCM(quint64 position, quint64 samples) { return _waveContour.getPCM(position, samples); }

        Q_INVOKABLE double getWindowRmsDb() { return calculateWindowRmsDb(); }
        Q_INVOKABLE double getWindowYinF0() { return calculateWindowYin(); }
        Q_INVOKABLE qreal freqToMidi(qreal freq) const;
    };

}


#endif // Waveshape_H
