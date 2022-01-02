#include "yin.h"

#include <cmath>


Yin::Yin() {
    _threshold = 0.15; //TODO play with + check funny place
    _probability = 0.0;
}

double Yin::getProbability() const {
    return _probability;
}

double Yin::getThreshold() const {
    return _threshold;
}

void Yin::setProbability(double newProb) {
    _probability = newProb;
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
    if (_currentTau == _halfBufferSize || _yinBuffer[_currentTau] >= _threshold) {
        _probability = 0;
        return false;
    }
    return true;
}


// Experiment with simmiliar implementation


using namespace std;

Yin2::Yin2() {
    setThreshold(0.4f);
    _tau = 0.f;
    _freq = 0.f;
}

void Yin2::setThreshold(float t) { _threshold = t; }

float Yin2::getThreshold() const { return _threshold; }

float Yin2::getTau() const { return _tau; }

float Yin2::getFreq() const { return _freq; }

const std::vector<float>& Yin2::getData() const { return _yinData; }


float Yin2::getFundamentalFrequency(const vector<float>& audio, float sampleRate) {
    if (audio.size() < 2)
        return 0.f;

    _yinData = buildYinData(audio);
    const auto tau = absoluteThreshold(_yinData);

    if (tau == 0)
        return 0;

    _tau = parabolicInterpolation(_yinData, tau);
    _freq = indexToFreq(_tau, sampleRate);

    //assert(_freq >= 0.f);
    return _freq;
}


vector<float> Yin2::buildYinData(const vector<float>& audio) const {
    auto yinData = sumOfDeltas(audio);
    cumulativeDifference(yinData);
    return yinData;
}


vector<float> Yin2::sumOfDeltas(const vector<float>& audio) const {
    size_t sz = audio.size() / 2;
    vector<float> deltas(sz, 0.f);

    for (size_t s = 0; s != sz; ++s)
        for (size_t i = 0; i != sz; ++i) {
            auto d = audio[i] - audio[i+s];
            deltas[s] += d * d;
        }

    return deltas;
}


void Yin2::cumulativeDifference(vector<float>& yin) const {
    yin[0] = 1;
    float runningSum = 0.f;

    for (size_t tau = 1; tau != yin.size(); ++tau) {
        runningSum += yin[tau];
        if (runningSum == 0)
            yin[tau] = 1;
        else
            yin[tau] *= (tau) / runningSum;
    }
}


int Yin2::absoluteThreshold(const vector<float>& yin) const {
    int tau = 2;
    int minTau = 0;
    float minVal = 1000.f;

    while (tau < yin.size()) {
        if (yin[tau] < _threshold) {
            while ((tau + 1) < yin.size() && yin[tau + 1] < yin[tau])
                ++tau;
            return tau;
        }
        else if (yin[tau] < minVal) {
            minVal = yin[tau];
            minTau = tau;
        }
        ++tau;
    }

    return -minTau;
}


float Yin2::parabolicInterpolation(const vector<float>& yin, int tau) const {
    if (tau == yin.size())
        return (float) tau;

    float betterTau = 0.f;

    if (tau > 0 && tau < yin.size() - 1) {
        float s0 = yin[tau - 1];
        float s1 = yin[tau];
        float s2 = yin[tau + 1];

        float adjustment = (s2 - s0) / (2.f * (2.f * s1 - s2 - s0));

        if (abs(adjustment) > 1.f)
            adjustment = 0.f;

        betterTau = (float) tau + adjustment;
    }
    else
        betterTau = (float) tau;

    return abs(betterTau);
}


float Yin2::indexToFreq(float i, float sampleRate) const {
    //assert(i > 0.f);
    //assert(sampleRate > 0.f);
    return sampleRate / i;
}

size_t Yin2::freqToIndex(float f, float sampleRate) const {
    return sampleRate / f;
}

