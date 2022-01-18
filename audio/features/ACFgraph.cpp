#include "ACFgraph.hpp"

#include "audio/wave/AudioUtils.hpp"
#include "audio/features/WindowFunction.hpp"


using namespace aural_sight;


bool ACGraphQML::loadByteArray(QByteArray analyseData) {

    const size_t _numSamples = 4096;
    const int bytesPerSample  = 2;
    Q_ASSERT(analyseData.size() == _numSamples * bytesPerSample);
    const char *ptr = analyseData.constData();

    for (size_t i=0; i<_numSamples; ++i) {
        const qint16 pcmSample = *reinterpret_cast<const qint16*>(ptr);
        const float realSample = pcmToReal(pcmSample);
        _input[i] = realSample;
         ptr += bytesPerSample;
    }

    _yin.init(44100, _numSamples);
    _lastFreq = _yin.process(_input.data());
    update();
    return true;
}

void ACGraphQML::loadFloatSamples(QByteArray samples) {
    const size_t _numSamples = 4096;
    const float* ptr = reinterpret_cast<const float*>(samples.constData());

    //TODO samples data and window fun

    _yin.init(44100, _numSamples);
    _lastFreq = _yin.process(ptr);
    update();
}


Q_INVOKABLE QByteArray ACGraphQML::getACF() {

    size_t samplesCount = 4096;
    std::vector<float> buf(samplesCount, 0.f);

    const std::vector<float>& src = _yin.acfBufer;

    for (size_t i = 0; i < src.size(); ++i)
        buf[i] = hannWindow(i, samplesCount) * src[i];

    QByteArray yinData = QByteArray(
                reinterpret_cast<const char*>(&buf[0]),
                samplesCount * sizeof(float));

    return yinData;
}


void ACGraphQML::paintACFbufer(QPainter& painter, const std::vector<float>& bufer,
                               size_t size, QString color, float heightPos, float scaleCoef) {
    QColor c(color);
    QPen pen(c);
    pen.setWidth(3);
    painter.setPen(pen);
    float prevValue = heightPos;
    for (size_t i = 0; i < size; ++i) {
        const double value = heightPos - bufer[i] * scaleCoef;
        painter.drawLine(i-1, prevValue, i, value);
        prevValue = value;
    }
}


void ACGraphQML::paint(QPainter* painter) {

    prepareBackground(*painter);

    QPen gPen(QColor("green"));
    gPen.setWidth(3);
    painter->setPen(gPen);

    int h = height();
    for (size_t i = 0; i < _yin.filteredIdx.size(); ++i) {
        const auto idx = _yin.filteredIdx[i];
        painter->drawLine(idx, 0, idx, h);
    }

    size_t uiSize = _yin.accBufer.size();
    paintACFbufer(*painter, _yin.acfBufer, uiSize, "green", h/2, 14);
    paintACFbufer(*painter, _yin.accBufer, uiSize, "red", h, 28);
    paintACFbufer(*painter, _yin.sumBufer, uiSize, "blue", h, 7);

    painter->setPen(QColor("orange"));
    painter->drawLine(_cursorPos, 0, _cursorPos, h);

    QPen wPen(QColor("white"));
    wPen.setWidth(3);
    painter->setPen(wPen);
    painter->drawLine(_yin.mineFound, 0, _yin.mineFound, h/2);

    QPen blackPen(QColor("black"));
    blackPen.setWidth(3);
    painter->setPen(blackPen);
    painter->drawLine(_yin.stdFound, 0, _yin.stdFound, h/2);
}




void ACGraphQML::prepareBackground(QPainter &painter) const {

    QRect rect;
    rect.setX(0); rect.setY(0);
    rect.setWidth(this->width());
    rect.setHeight(this->height());

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
