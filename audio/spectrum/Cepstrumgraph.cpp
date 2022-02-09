#include "Cepstrumgraph.hpp"

#include "audio/wave/AudioUtils.hpp"
#include "audio/features/WindowFunction.hpp"
#include "libs/cqt/dsp/FFT.h"


using namespace aurals;


bool CepstrumgraphQML::loadByteArray(QByteArray analyseData) {

    const size_t bytesPerSample  = 2;
    Q_ASSERT(static_cast<size_t>(analyseData.size()) == _windowSize * bytesPerSample);
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

    process();
    _imagePainted = false;
    update();
    return true;
}

void CepstrumgraphQML::loadFloatSamples(QByteArray samples) {
    float* ptr = reinterpret_cast<float*>(samples.data());

    if (_window != WindowFunction::NoWindow) {
        for (size_t i = 0; i < _windowSize; ++i)
            _input[i] = ptr[i] * _windowBufer[i];
    }
    else
        _input = std::vector<float>(ptr, ptr + _windowSize);

    process();
    _imagePainted = false;
    update();
}


void CepstrumgraphQML::process() {
    FFTReal fft(_windowSize);

    std::vector<float> magnitude(_windowSize, 0.f);
    fft.forwardMagnitude(_input.data(), magnitude.data());
    for (auto& s: magnitude)
        s = std::log(s * s); //std::abs(s)

    std::vector<float> complexZeroes(_windowSize, 0.f);
    _cepstrum = std::vector<float>(_windowSize, 0.f);

    FFTReal fftHalf(_windowSize/2);

    fftHalf.inverse(magnitude.data(), complexZeroes.data(),
                _cepstrum.data());

    auto maxIt = std::max_element(_cepstrum.begin(), _cepstrum.end());
    auto minIt = std::min_element(_cepstrum.begin(), _cepstrum.end());
    qDebug() << "1) Max " << *maxIt << " min " << *minIt;

    _cepstrumV2 = std::vector<float>(_windowSize, 0.f);
    fftHalf.forward(magnitude.data(), _cepstrumV2.data(), complexZeroes.data());

    auto maxIt2 = std::max_element(_cepstrumV2.begin(), _cepstrumV2.end());
    auto minIt2 = std::min_element(_cepstrumV2.begin(), _cepstrumV2.end());
    qDebug() << "2) Max " << *maxIt2 << " min " << *minIt2;

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
    pen.setWidth(2);
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

    //paintBufer(imgPainter, _cepstrumV2, _windowSize /2, "red", height() / 2, 0.25f);
    paintBufer(imgPainter, _cepstrum, _windowSize /2, "green", height() / 2, 500.f);
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
