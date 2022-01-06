#include "Yin.hpp"

using namespace mtherapp;


Yin::Yin() {
    _threshold = 0.15;
}

size_t Yin::getTau() const {
    return _currentTau;
}

double Yin::getThreshold() const {
    return _threshold;
}

void Yin::setThreshold(double newThresh) {
    _threshold = newThresh;
}

void Yin::init(double yinSampleRate, size_t yinBufferSize) {
    if (yinSampleRate != _sampleRate)
        _sampleRate = yinSampleRate;
    if (_bufferSize == yinBufferSize)
        return;
    _bufferSize = yinBufferSize;
    _halfBufferSize = _bufferSize / 2;
    _yinBuffer = std::vector<float>(_halfBufferSize, 0.f);
}


double Yin::getPitch(const float *buffer) {
    halvesDifference(buffer);
    accMeanNormDifference();
    if (absoluteThresholdFound())
        return _sampleRate / parabolicInterpolation();
    return -1.0; //Not found case
}


double Yin::parabolicInterpolation() const {

    size_t start = _currentTau ? _currentTau - 1 : _currentTau;
    size_t finish = _currentTau + 1 < _halfBufferSize ? _currentTau + 1 : _currentTau;

    if (start == _currentTau) {
        if (_yinBuffer[_currentTau] <= _yinBuffer[finish])
            return _currentTau;
        else
            return finish;
    }

    if (finish == _currentTau) {
        if (_yinBuffer[_currentTau] <= _yinBuffer[start])
            return _currentTau;
        else
            return start;
    }

    double begin = _yinBuffer[start];
    double middle = _yinBuffer[_currentTau];
    double end = _yinBuffer[finish];
    return _currentTau + (end - begin) / (2 * (2 * middle - end - begin));
}



void Yin::accMeanNormDifference(){
    _yinBuffer[0] = 1;
    float runningSum = 0;
    for (size_t tau = 1; tau < _halfBufferSize; tau++) {
        runningSum += _yinBuffer[tau];
        _yinBuffer[tau] *= tau / runningSum;
    }
}


void Yin::halvesDifference(const float* buffer) {
    float delta = 0.f;
    for(size_t tau = 0 ; tau < _halfBufferSize; tau++)
        for(size_t index = 0; index < _halfBufferSize; index++){
            delta = buffer[index] - buffer[index + tau];
            _yinBuffer[tau] += delta * delta;
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
