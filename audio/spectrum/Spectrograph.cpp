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

#include "Spectrograph.hpp"

#include <QDebug>
#include <QPainter>

#include <cmath>
#include <unordered_map>

#include "audio/wave/WavFile.hpp"
#include "audio/features/PeaksOperations.hpp"
#include "audio/wave/AudioUtils.hpp"


using namespace aural_sight;


SpectrographPainter::SpectrographPainter() : _barSelected(-1),
    _lowFreq(0.0), _highFreq(0.0)
{
}


void SpectrographPainter::prepareBackground(QPainter &painter, const QRect &rect) const {
    painter.fillRect(rect, Qt::black);
    const int numBars = _bars.count();
    const double barWidth = rect.width()/( static_cast<double>(numBars) );
    auto calcXPos = [&rect, &barWidth](int index) { return (rect.topLeft().x() + index * barWidth); };

    QColor barColor(51, 204, 102);
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

    if (numBars) {
        const int numHorizontalSections = numBars;
        for (int i= 0; i < numHorizontalSections; ++i) {
            QLineF line(rect.topLeft(), rect.bottomLeft());
            line.setP1({calcXPos(i), line.y1()});
            line.setP2({line.x2() + static_cast<int>(barWidth * i), line.y2()});
            painter.drawLine(line);
        }
    }

    const int numVerticalSections = 10;
    QLine line(rect.topLeft(), rect.topRight());
    for (int i = 0; i < numVerticalSections; ++i) {
        line.translate(0, rect.height()/(numVerticalSections));
        painter.drawLine(line);
    }
}


void SpectrographPainter::paintBars(QPainter &painter, const QRect &rect) const {

    const int numBars = _bars.count();
    const double barWidth = rect.width()/( static_cast<double>(numBars) );
    auto calcXPos = [&rect, &barWidth](int index) { return (rect.topLeft().x() + index * barWidth); };
    QColor clipColor(255, 255, 0);
    QColor barColor(51, 204, 102);
    barColor = barColor.lighter();
    barColor.setAlphaF(0.75);
    clipColor.setAlphaF(0.75);

    for (int i = 0; i < numBars; ++i) {
        qreal value = _bars[i].value;
        Q_ASSERT(value >= 0.0 && value <= 1.0);
        QRectF bar = rect;
        bar.setLeft(calcXPos(i)); //rect.left() + leftPaddingWidth + (i * (gapWidth + barWidth)));
        bar.setWidth(barWidth);
        bar.setTop((1.0 - value) * rect.height());
        bar.setBottom(rect.bottom());

        QColor color = barColor;
        if (_bars[i].clipped)
            color = clipColor;
        else {
            if (_peaksIdx.count(i))
                color = QColor("orange");
        }
        painter.fillRect(bar, color);
    }
}


void SpectrographPainter::paintSlope(QPainter &painter, const QRect &rect) const {

    const int numBars = _bars.count();
    const double barWidth = rect.width()/( static_cast<double>(numBars) );
    auto calcXPos = [&rect, &barWidth](int index) { return (rect.topLeft().x() + index * barWidth); };

    //Здесь можно попробовать интерполировать функцию, и получить коэффициент наклона
    painter.setPen(QColor("orange"));
    painter.drawLine(calcXPos(_maxIdx), rect.top() + (1.0 - _maxValue) * rect.height(),
                     calcXPos(_lastIdx), rect.top() + (1.0 - _lastValue) * rect.height());
}


void SpectrographPainter::paintSpectr(QPainter &painter, const QRect &rect) {


    if (_bars.count()) {

        if (_imagePainted == false) {

            _mainImage = QImage(painter.device()->width(), painter.device()->height(), QImage::Format_ARGB32);
            QPainter imgPainter(&_mainImage);

            prepareBackground(imgPainter, rect);
            paintBars(imgPainter, rect);
            if (_gapLevel < 0.1)
                paintSlope(imgPainter, rect);

            _imagePainted = true;
        }

        painter.drawImage(QPoint{0,0}, _mainImage);

        const double barWidth = rect.width()/( static_cast<double>(_bars.count()) );
        auto calcXPos = [&rect, &barWidth](int index) { return (rect.topLeft().x() + index * barWidth); };

        if (_barSelected != -1) {
            QRectF regionRect = rect;
            regionRect.setLeft(calcXPos(_barSelected));
            regionRect.setWidth(barWidth);
            QColor regionColor(202, 202, 64);
            painter.setBrush(Qt::DiagCrossPattern);
            painter.fillRect(regionRect, regionColor);
            painter.setBrush(Qt::NoBrush);
        }
    }
    else
        qDebug () << "No bars to draw for qml";
}


void SpectrographPainter::setParams(int numBars, const qreal lowFreq, const qreal highFreq) {
    Q_ASSERT(numBars > 0);
    Q_ASSERT(highFreq > lowFreq);
    _bars.resize(numBars);
    _lowFreq = lowFreq;
    _highFreq = highFreq;
    updateBars();
}


int SpectrographPainter::barIndex(qreal frequency) const {
    Q_ASSERT(frequency >= _lowFreq && frequency < _highFreq);
    const qreal bandWidth = (_highFreq - _lowFreq) / _bars.count();
    const int index = (frequency - _lowFreq) / bandWidth;
    if (index <0 || index >= _bars.count())
        Q_ASSERT(false);
    return index;
}


QPair<qreal, qreal> SpectrographPainter::barRange(int index) const {
    Q_ASSERT(index >= 0 && index < _bars.count());
    const qreal bandWidth = (_highFreq - _lowFreq) / _bars.count();
    return QPair<qreal, qreal>(index * bandWidth, (index+1) * bandWidth);
}


void SpectrographPainter::updateBars()
{
    _bars.fill(Bar());
    FrequencySpectrum::const_iterator i = _spectrum.begin();
    const FrequencySpectrum::const_iterator end = _spectrum.end();
    size_t count = 0;
    double emptyBins = 0.0;

    _freqStep = (i+1)->frequency - i->frequency;
    _gotClipping = false;
    _spectrumGap = false;
    _maxValue = -120;
    _maxIdx = -1;
    _lastIdx = 0;
    _lastValue = 0.0;

    for ( ; i != end; ++i) {
        const FrequencySpectrum::Element e = *i;
        if (e.frequency >= _lowFreq && e.frequency < _highFreq && count < static_cast<size_t>(_bars.size())) {
            Bar &bar = _bars[count]; //Если перенести на std можно делать поиск оптимальней
            bar.value = qMax(bar.value, e.amplitude);
            bar.clipped |= e.clipped;
            _gotClipping |= e.clipped;

            double level = 20 * log10(bar.value);
            if (level < -36.0) {
                if (count >= 5)
                    _spectrumGap = true;
                emptyBins += 1.0;
            }
            if (bar.value > _maxValue) {
                _maxValue = bar.value;
                _maxIdx = count;
            }
            if (bar.value > 0.0) {
                _lastIdx = count;
                _lastValue = bar.value;
            }
        }
        ++count;
    }
    _gapLevel = emptyBins / _bars.size();
    if (_gapLevel < 0.1)
        classifySlope();
    findPeaks();
    _imagePainted = false;
}


void SpectrographPainter::findF0() { //TODO findF0 classifySlope findPeaks использовать спектр, а не бары для отрисовки
    if (_bars.size() < 100)
        return;

    //bool usePlusMinusMode = true;
    std::unordered_map<int, double> table;
    std::unordered_map<int, std::vector<int>> sequences;

    const double lowestFreq = 20.0;
    const int lowBorder = (lowestFreq / _freqStep) + 1.0;

    for (int i = lowBorder; i < 100; ++i) {
        double summ = 0.0;
        std::vector<int> sequence;

        for (int n = 1; n < 13; ++n) {
            int currentPosition = i * n;
            if (currentPosition >= _bars.size())
                break;

            if (n > 1) { //usePlusMinusMode &&

                double localMax = _bars[currentPosition].value;
                int index = currentPosition;

                if (_bars[currentPosition - 1].value > localMax) { //Можно оформить лямбдами
                    localMax = _bars[currentPosition - 1].value;
                    index = currentPosition - 1;
                }
                if (_bars.size() > currentPosition + 1 && _bars[currentPosition + 1].value > localMax) {
                    localMax = _bars[currentPosition + 1].value;
                    index = currentPosition + 1;
                }
                if (_bars[currentPosition - 2].value > localMax) {
                    localMax = _bars[currentPosition - 2].value;
                    index = currentPosition - 2;
                }
                if (_bars.size() > currentPosition + 2 && _bars[currentPosition + 2].value > localMax) {
                    //localMax = _bars[currentPosition + 2].value; //Use only if continue checking +-3
                    index = currentPosition + 2;
                }
                summ += _bars[index].value;
                sequence.push_back(index);
            }
        }

        table[i] = summ;
        sequences[i] = sequence;
    }

    std::vector<std::pair<int,double>> sortedTable(table.begin(), table.end());
    std::sort(sortedTable.begin(), sortedTable.end(), [](auto lhs, auto rhs){ return lhs.second > rhs.second;});

    size_t count = 0;
    for (auto& [n, summ]: sortedTable) {
        if (count < 2) {
            qDebug() << "Spectral summ " << n << " " << summ << " " << n*_freqStep;
            qDebug() << "Seq: " << sequences[n];
        }
        if (++count > 20)
            break;
    }

    //Нужно сохранять дополнительную информацию - какой пик из +- был максимальным
    //Нужно сохранять процент не пустых пиков, так как например если субгармоника содержит все гармоники, но и пропуски - её рейтинг должен падать
    _spectrumPitch = (sortedTable[0].first + 0.5) * _freqStep;

     //Нужно использовать максимальный пик из + - тогда получится повысить точность
    _specPitchAprox = (sortedTable[0].first + 0.5) * _freqStep +
            (sortedTable[0].first * 2 + 0.5) * _freqStep +
            (sortedTable[0].first * 3 + 0.5) * _freqStep;
    _specPitchAprox /= 6.0;

    int lowBin = 0;
    int highBin = 0;
    if (sortedTable[0].first < sortedTable[1].first) {
        lowBin = sortedTable[0].first;
        highBin = sortedTable[1].first;
    }
    else {
        highBin = sortedTable[1].first;
        lowBin = sortedTable[0].first;
    }

    int mod = highBin % lowBin;
    if (mod == 0 || mod == 1 || mod == lowBin - 1) {
        //qDebug() << "First 2 are harmonics";
        //_specPitchAprox = ((lowBin + highBin + 1.0) / 3.0) * freqStep;
        //Вместо этого элемента использовать глобальный поиск, нужен ещё 1 контейнер для хранения череды
        //MAP[n] = {n*2, n*3 - 1, n*4 +1};
    }
    //Другой случай если первые 2 очень рядом
    if (highBin - lowBin == 1) {
        _specPitchAprox = ((lowBin + highBin + 1.0) / 2.0) * _freqStep;
    }
}




void SpectrographPainter::findPeaks() {

    if (_bars.size() < 7) {
        qDebug() << "Spectrograph bars size issue";
        return;
    }

    std::vector<double> amps;
    for (auto& bar: _bars)
        amps.push_back(bar.value);
    auto peaks = aural_sight::peakIndexes<double>(amps, 6.0);

    bool searchTinyPeaks = true;
    if (searchTinyPeaks) {
        for (size_t i = 80; i < static_cast<size_t>(_bars.size()) - 6; ++i) {
            size_t emptyCount = 0;
            size_t maxIdx = 0;
            double maxValue = 0.0;
            for (size_t j = 0; j < 12; ++j) {
                if (_bars[i - 5 + j].value > maxValue) {
                    maxValue = _bars[i - 5 + j].value;
                    maxIdx = j;
                }
                if (_bars[i - 5 + j].value <= 0.0)
                    emptyCount += 1;
            }
            if (maxIdx == 5 && emptyCount >= 6)
                peaks.push_back(i);
        }
        std::sort(peaks.begin(), peaks.end());
        auto removeIt = std::unique(peaks.begin(), peaks.end());
        peaks.erase(removeIt, peaks.end());
    }

    _peaksIdx.clear();
    for (auto p: peaks)
        _peaksIdx.insert(p);

    _spectrumPitch = findPeakCommonDistance(peaks) * _freqStep;
}


void SpectrographPainter::classifySlope() {
    double y1 = (1.0 - _maxValue);
    double y2 = (1.0 - _lastValue);
    double x1 = _maxIdx;
    double x2 = _lastIdx;
    double vK = y2 - y1;
    double hK = x2 - x1;
    double angle1 = atan(vK/hK)  * 180.0 / 3.141592653;
    double angle2 = atan(hK/vK)  * 180.0 / 3.141592653;
    //qDebug() << " Vertial katet "<< vK << " horizontal katet " << hK << " angle1 " << angle1 << " angle2 " << angle2;
    //Использовать -k чтобы получить ту же картинку, т.к. одна из осей перевернута
    //Возможно сделать расчёт по всем корзинам, и наблюдать как меняется угол
    //Вначале больше похоже на розовый шум, потом на белый
    //Другой способ анализа это проверка как меняется сигнал по окатвам, если идёт падение на 3 дБ - это розовый шум
}



//========================================================

SpectrographQML::SpectrographQML([[maybe_unused]] QQuickItem* parent) :  _samplesAmount(4096) {
    setParams(SpectrumNumBands, SpectrumLowFreq, SpectrumHighFreq);
    QObject::connect(&_analyser,SIGNAL(spectrumChanged(aural_sight::FrequencySpectrum)),
                     this,SLOT(spectrumChanged(aural_sight::FrequencySpectrum)));
}


void SpectrographQML::paint(QPainter* painter) {
   QRect rect;
   rect.setX(0); rect.setY(0);
   rect.setWidth(this->width());
   rect.setHeight(this->height());
   paintSpectr(*painter,rect);
   emit spectrumCalculated(); //emh?
}


void SpectrographQML::updateBars() {
    SpectrographPainter::updateBars();
    update();
}

void SpectrographQML::spectrumChanged([[maybe_unused]] const qint64 position, [[maybe_unused]] const qint64 length,
                                      const aural_sight::FrequencySpectrum &spectrum) {
    _spectrum = spectrum;
    SpectrographQML::updateBars();
}


void SpectrographQML::spectrumChanged(const aural_sight::FrequencySpectrum &spectrum) {
    _spectrum = spectrum;
    SpectrographQML::updateBars();
}


void SpectrographQML::selectBar(const int index) {
    Q_ASSERT(index >= 0 && index < _bars.count());
    _barSelected = index;
    update();
}


void SpectrographQML::onPress(const int xPress, [[maybe_unused]]const  int yPress, const  int width, [[maybe_unused]] const int height) {
    qreal barWidth = static_cast<double>(width) / _bars.count();
    const int index = (static_cast<double>(xPress) / barWidth);
    selectBar(index);
}


bool SpectrographQML::loadSpectrum(const QString filename, const quint64 position) {
    WavFile wav;
    if ( wav.open(filename) == false)
       return false;
    const int bytesInSample = 2;
    quint64 afterHeaderPosition = wav.pos();
    wav.seek(afterHeaderPosition + position * bytesInSample);
    QByteArray analyseData = wav.read(_samplesAmount * bytesInSample);
    if (analyseData.size() != _samplesAmount * bytesInSample)
        return false;
    _analyser.calculate(analyseData, wav.audioFormat());
    return true;
}


bool SpectrographQML::loadByteArray(QByteArray analyseData) {
    QAudioFormat format;;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");

    const int bytesInSample = 2;
    if (analyseData.size() != _samplesAmount * bytesInSample)
        return false;
    _analyser.calculate(analyseData, format);
    return true;
}


bool SpectrographQML::loadFloatSamples(QByteArray samples) {
    _analyser.calculateFromFloat(samples);
    return true;
}
