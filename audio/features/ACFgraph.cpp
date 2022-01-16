#include "ACFgraph.hpp"

#include "audio/wave/AudioUtils.hpp"

using namespace aural_sight;



bool ACGraphQML::loadByteArray(QByteArray analyseData) {

    const size_t _numSamples = 4096;
    const int bytesPerSample  = 2;
    Q_ASSERT(analyseData.size() == _numSamples * bytesPerSample);
    const char *ptr = analyseData.constData(); //TODO здесь и в spectrum: from preloaded (загружать float сразу, без преобразований)

    for (size_t i=0; i<_numSamples; ++i) {
        const qint16 pcmSample = *reinterpret_cast<const qint16*>(ptr);
        const float realSample = pcmToReal(pcmSample); // Scale down to range [-1.0, 1.0]
        //const float windowedSample = realSample * _window[i];
        _input[i] = realSample;
         ptr += bytesPerSample;
    }

    //TODO prepare YIN data
    _yin.init(44100, 4096);
    _lastFreq = _yin.process(_input.data());
    update();
    return true;
}


void ACGraphQML::paint(QPainter* painter) {
    QRect rect;
    rect.setX(0); rect.setY(0);
    rect.setWidth(this->width());
    rect.setHeight(this->height());
    prepareBackground(*painter, rect);

    //PAINT ACC Buffer first, from 0 to max, lets max would be first 5

    QPen gPen(QColor("green"));
    gPen.setWidth(3);

    painter->setPen(gPen);
    for (size_t i = 0; i < _yin.filteredIdx.size(); ++i) {
        const auto idx = _yin.filteredIdx[i];
        painter->drawLine(idx, 0, idx, rect.height());
    }

    QPen rPen(QColor("red")); //TODO just function with position, color, width to paint line!
    rPen.setWidth(3);
    QPen bPen(QColor("blue"));
    bPen.setWidth(3);
    QPen cPen(QColor("cyan"));
    cPen.setWidth(3);


    double prevYacc = rect.height();
    double prevDiff = rect.height();
    double prevAcc = rect.height() / 2;
    double prevDiff2 = rect.height();

    for (size_t i = 0; i < _yin.accBufer.size(); ++i) {

        painter->setPen(gPen);
        const double newAC = rect.height() / 2 - _yin.acfBufer[i] * 14;
        painter->drawLine(i-1, prevAcc, i, newAC);
        prevAcc = newAC;

        painter->setPen(rPen);
        const double newYacc = rect.height() - _yin.accBufer[i] * 28; //TODO scale
        painter->drawLine(i-1, prevYacc, i, newYacc);
        prevYacc = newYacc;

        painter->setPen(bPen);
        const double newDiff = rect.height() - _yin.sumBufer[i] * 7;
        painter->drawLine(i-1, prevDiff, i, newDiff);
        prevDiff = newDiff;

        painter->setPen(cPen);
        const double newDiff2 = rect.height() - _yin.sumBufV2[i] * 7;
        painter->drawLine(i-1, prevDiff2, i, newDiff2);
        prevDiff2 = newDiff2;

    }

    painter->setPen(QColor("orange"));
    painter->drawLine(_cursorPos, 0, _cursorPos, rect.height());

    QPen wPen(QColor("white"));
    wPen.setWidth(3);
    painter->setPen(wPen);
    painter->drawLine(_yin.mineFound, 0, _yin.mineFound, rect.height()/2);

    QPen blackPen(QColor("black"));
    blackPen.setWidth(3);
    painter->setPen(blackPen);
    painter->drawLine(_yin.stdFound, 0, _yin.stdFound, rect.height()/2);
}



void ACGraphQML::prepareBackground(QPainter &painter, const QRect &rect) const {
    painter.fillRect(rect, Qt::darkGray);
    const int numBars = 200;
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
