#include "Yin.hpp"

#include <QDebug>

#include "libs/fft/FFTurealfix.hpp"

using namespace mtherapp;


Yin::Yin() { //TODO template
    _threshold = 0.15;
}

size_t Yin::getTau() const {
    return _currentTau;
}

double Yin::getThreshold() const {
    return _threshold;
}

void Yin::setThreshold(const double newThresh) {
    _threshold = newThresh;
}

void Yin::init(double yinSampleRate, const size_t yinBufferSize) {
    if (yinSampleRate != _sampleRate)
        _sampleRate = yinSampleRate;
    if (_bufferSize == yinBufferSize)
        return;
    _bufferSize = yinBufferSize;
    _halfBufferSize = _bufferSize / 2;
    _yinBuffer = std::vector<float>(_halfBufferSize, 0.f);
}


double Yin::getPitch(const float *buffer) {

    _yinBuffer = std::vector<float>(_halfBufferSize, 0.f); //ERROR!

    //1. Autocorrelation ?
    differenceFunction(buffer); //2. Difference function
    accMeanNormDifference(); //3. Cumulative mean normalized difference function
    if (absoluteThresholdFound()) //4. Absolute threshold
        return _sampleRate / parabolicInterpolation(); //5.Parabolic interpolation
    return -1.0;
    //6. Best local estimate ?
}


void Yin::differenceFunction(const float* buffer) {
    float delta = 0.f;
    for(size_t tau = 0 ; tau < _halfBufferSize; tau++) {
        for(size_t index = 0; index < _halfBufferSize; index++){
            delta = buffer[index] - buffer[index + tau];
            _yinBuffer[tau] += delta * delta; //Что если здесь сделать 4ую степень но потом взять от всего корень? Эксперименты!
        }
        if (tau < 15)
            qDebug() << "Slow t " << tau << " " << _yinBuffer[tau];
    }
}


double Yin::parabolicInterpolation() const { //TODO сравнить с другим результатом

    size_t start = _currentTau ? _currentTau - 1 : _currentTau;
    size_t finish = _currentTau + 1 < _halfBufferSize ? _currentTau + 1 : _currentTau;

    auto borderResult = [this](size_t idx) {
        if (_yinBuffer[_currentTau] <= _yinBuffer[idx])
            return _currentTau;
        else
            return idx;
    };

    if (start == _currentTau)
        return borderResult(finish);
    if (finish == _currentTau)
        return borderResult(start);

    double begin = _yinBuffer[start];
    double middle = _yinBuffer[_currentTau];
    double end = _yinBuffer[finish];
    return _currentTau + (end - begin) / (2.0 * (2.0 * middle - end - begin));
}



void Yin::accMeanNormDifference(){
    _yinBuffer[0] = 1;
    float runningSum = 0;
    for (size_t tau = 1; tau < _halfBufferSize; tau++) {
        runningSum += _yinBuffer[tau];
        _yinBuffer[tau] *= tau / runningSum;
    }
}




bool Yin::absoluteThresholdFound(){
    for (_currentTau = 2; _currentTau < _halfBufferSize ; _currentTau++)
        if (_yinBuffer[_currentTau] < _threshold) {
            while (_currentTau + 1 < _halfBufferSize &&
                   _yinBuffer[_currentTau + 1] < _yinBuffer[_currentTau])
                ++_currentTau;
            break;
        }
    if (_currentTau == _halfBufferSize || _yinBuffer[_currentTau] >= _threshold)
        return false;
    return true;
}



//Better Yin

//TODO buffers must be reseted every time!!!


void YinPP::process(const float* buffer) {

    //size_t W = std::ceil(_sampleRate/40.0); // Integration window is dependent on the lowest frequency to be detected

    _yinBuffer1 = std::vector<float>(_bufferSize/2.0, 0.f);
    _yinBuffer2 = std::vector<float>(_bufferSize/2.0, 0.f);

    size_t W = _bufferSize / 2;

    autoCorrelateionSlow1(buffer, 0, W);
    autoCorrelateionSlow2(buffer, 0, W);

    //TODO fast

    compareBuffers();

    //diffFast(buffer, W);
    diffSlow(buffer, W);

    //compareBuffers();

    _yinBuffer1[0] = 1;
    float runningSum = 0;
    for (size_t tau = 1; tau < W; tau++) {
        runningSum += _yinBuffer1[tau];
        _yinBuffer1[tau] *= tau / runningSum;
    }

    size_t _currentTau = 0;
    for (_currentTau = 2; _currentTau < W ; _currentTau++)
        if (_yinBuffer1[_currentTau] < 0.2) {
            while (_currentTau + 1 < W &&
                   _yinBuffer1[_currentTau + 1] < _yinBuffer1[_currentTau])
                ++_currentTau;
            break;
        }

    //TODO parabolic also
    double foundPitch = _sampleRate / _currentTau;
    qDebug() << "New way pitch " << foundPitch;
}



void YinPP::autoCorrelateionSlow1(const float* buffer, size_t tau, size_t W) {
    const size_t n = _bufferSize / 2;

    for (size_t t = 0; t < n; t++) {

        _yinBuffer1[t] = 0;

        for (size_t j = t + 1; j < (t + W); j++) {
            _yinBuffer1[t] += buffer[j] * buffer[j + tau];
        }
    }

}

void YinPP::autoCorrelateionSlow2(const float* buffer, size_t tau, size_t W) {
    const size_t n = _bufferSize / 2;
    //TODO buffer must be zeroes
    for (size_t t = 0; t < n; t++)
        for (size_t j = t + 1; j < (t + W - tau); j++)
            _yinBuffer2[t] += buffer[j] * buffer[j + tau];

}

void YinPP::diffSlow(const float* yinBuf, size_t W) {

    const size_t n = _bufferSize / 2; //Or w??

    for(size_t tau = 0 ; tau < n; tau++) {

        _yinBuffer1[tau] = 0;

        for(size_t j = 0; j < n; j++)
        {
            auto delta = yinBuf[j] - yinBuf[j + tau];
            _yinBuffer1[tau] += delta * delta;
        }

        if (tau < 15)
            qDebug() << "Slow " << _yinBuffer1[tau];
    }
}//Что если здесь сделать 4ую степень но потом взять от всего корень? Эксперименты!


void YinPP::diffFast(const float* buffer, size_t W) {

    const size_t n = _bufferSize / 2; //Or w??

    auto first = _yinBuffer1[0];

    for(size_t tau = 0 ; tau < n; tau++) {

        float second = 0.f;
        for (size_t j = 1; j < W; j++)
            second += buffer[j] * buffer[j + tau];

         _yinBuffer2[tau] = first + second - _yinBuffer1[tau];

         //qDebug() << "Fast t " << tau << " " << _yinBuffer2[tau];
    }
}



void YinPP::autoCorrelationFast(const float* buffer) {
    static FFTRealFixLen<12> _fft;

    //TODO review with fftw!

    auto buf = std::vector<float>(_bufferSize , 0.f);

    _fft.do_fft(buf.data(), buffer); //TODO make inplace version

    const size_t n = _bufferSize / 2;

    for (size_t i = 0; i < n; ++i) { //conjugate
        buf[i] = buf[i] * buf[i];
        buf[n + i] = buf[n + i] * buf[n + i];
    }

    _yinBuffer2 = std::vector<float>(_bufferSize, 0.f);

    _fft.do_ifft(buf.data(), _yinBuffer2.data());
    _fft.rescale(_yinBuffer2.data());

}


void YinPP::compareBuffers() {

    float eps = 10e-7;

    const size_t n = _bufferSize / 2;

    size_t failesCount = 0;

    for (size_t i = 0; i < n; i++) {
        if (std::abs(_yinBuffer1[i] - _yinBuffer2[i]) > eps) {

            qDebug() << i << " unequal " << _yinBuffer1[i] << " " << _yinBuffer2[i];

            ++failesCount;
            //if (failesCount > 10)
                //break;
        }
        //else
            //qDebug() << i << "is fine" << _yinBuffer1[i];
    }

    qDebug() << "Maybe equal!";
}
