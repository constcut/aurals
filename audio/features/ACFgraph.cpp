#include "ACFgraph.hpp"

#include "audio/wave/AudioUtils.hpp"

using namespace aural_sight;



bool ACGraphQML::loadByteArray(QByteArray analyseData) {

    const size_t _numSamples = 4096;
    Q_ASSERT(analyseData.size() == 4096 * 2);
    const char *ptr = analyseData.constData(); //TODO здесь и в spectrum: from preloaded (загружать float сразу, без преобразований)

    for (size_t i=0; i<_numSamples; ++i) {
        const qint16 pcmSample = *reinterpret_cast<const qint16*>(ptr);
        const float realSample = pcmToReal(pcmSample); // Scale down to range [-1.0, 1.0]
        //const float windowedSample = realSample * _window[i];
        _input[i] = realSample;
        //ptr += bytesPerSample;
    }

    //TODO prepare YIN data
    _lastFreq = _yin.process(_input.data());

    return true;
}


void ACGraphQML::paint(QPainter* painter) {
    QRect rect;
    rect.setX(0); rect.setY(0);
    rect.setWidth(this->width());
    rect.setHeight(this->height());
    prepareBackground(*painter, rect);
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
