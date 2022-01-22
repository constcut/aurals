#include "Cepstrumgraph.hpp"

#include "audio/wave/AudioUtils.hpp"
#include "audio/features/WindowFunction.hpp"


using namespace aural_sight;


bool CepstrumgraphQML::loadByteArray(QByteArray analyseData) {

    const size_t bytesPerSample  = 2;
    Q_ASSERT(analyseData.size() == _windowSize * bytesPerSample);
    const char *ptr = analyseData.constData();

    for (size_t i=0; i<_windowSize; ++i) {
        const qint16 pcmSample = *reinterpret_cast<const qint16*>(ptr);
        const float realSample = pcmToReal(pcmSample);

        if (_window != WindowFunction::NoWindow)
            _input[i] = realSample * _windowBufer[i];
        else
            _input[i] = realSample;
         ptr += bytesPerSample;
    }

    //TODO process
    _imagePainted = false;
    update();
    return true;
}

void CepstrumgraphQML::loadFloatSamples(QByteArray samples) {
    float* ptr = reinterpret_cast<float*>(samples.data());

    if (_window != WindowFunction::NoWindow)
        for (size_t i = 0; i < _windowSize; ++i)
            ptr[i] *= _windowBufer[i];

    //TODO process
    _imagePainted = false;
    update();
}


void CepstrumgraphQML::updateWindowFunction() {
    if (_window == WindowFunction::NoWindow)
        return;

    for (size_t i = 0; i < _windowSize; ++i) {
        if (_window == WindowFunction::HannWindow)
            _windowBufer[i] = hannWindow(i, _windowSize);
        else if (_window == WindowFunction::GausWindow)
            _windowBufer[i] = gausWindow(i, _windowSize);
        else if (_window == WindowFunction::BlackmanWindow)
            _windowBufer[i] = blackmanWindow(i, _windowSize);
        else if (_window == WindowFunction::HammWindow)
            _windowBufer[i] = hammWindow(i, _windowSize);
    }
}


void CepstrumgraphQML::changeSampleRate(int newSampleRate) {
    _sampleRate = newSampleRate;
}

void CepstrumgraphQML::changeWindowSize(int newWindowSize) {
    _windowSize = newWindowSize;

    _windowBufer = std::vector(newWindowSize, 0.f);
    _input = std::vector(newWindowSize, 0.f);

    updateWindowFunction();
}



void CepstrumgraphQML::paintBufer(QPainter& painter, const std::vector<float>& bufer,
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


void CepstrumgraphQML::paintImage(QPainter& painter) {
    _mainImage =  QImage(painter.device()->width(), painter.device()->height(), QImage::Format_ARGB32);
    QPainter imgPainter(&_mainImage);

    prepareBackground(imgPainter);

    QPen gPen(QColor("green"));
    gPen.setWidth(3);
    imgPainter.setPen(gPen);

}


void CepstrumgraphQML::paint(QPainter* painter) {

    if (_imagePainted == false) {
        paintImage(*painter);
        _imagePainted = true;
    }

    painter->drawImage(QPoint{0,0}, _mainImage);
    painter->setPen(QColor("orange"));
    painter->drawLine(_cursorPos, 0, _cursorPos, height());
}



void CepstrumgraphQML::prepareBackground(QPainter &painter) const {

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
