#include "Yin.hpp"

#include <QDebug>

#include "libs/fft/FFTurealfix.hpp"

#include "PeaksOperations.hpp"

using namespace aural_sight;


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
    if (absoluteThresholdFound())  {//4. Absolute threshold {

        //qDebug() << "Olderst " << _sampleRate / _currentTau << " " << _currentTau;

        size_t _new = parabolicInterpolation();

        //qDebug() << "Next " << _sampleRate / _new << " pa "  << _new;

        return _sampleRate / _new; //5.Parabolic interpolation
    }
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
        return false; //ELSE find min

    return true;
}



//Better Yin


double YinPP::process(const float* buffer) {

    //TODO remove all old + replace with

    diffFunNew(buffer);
    //diffFunOld(buffer);

    sumBufer = _yinBufer1;
    //compareBuffers();

    accMeanDiff(_yinBufer1);
    accBufer = _yinBufer1;

    //accMeanDiff(_yinBuffer2);

    auto maxE = std::max_element(_yinBufer1.begin(), _yinBufer1.end());
    //auto minE = std::min_element(_yinBuffer1.begin(), _yinBuffer1.end());
    //float minVal = *minE;
    float maxVal = *maxE;

    std::vector<double> invBuffer(_yinBufer1.size(), 0); //TODO inversed function for peaks
    for (size_t i = 0; i < invBuffer.size(); ++i)
        invBuffer[i] = maxVal - _yinBufer1[i];


    auto idx = peakIndexes<double>(invBuffer);

    float minP = 100.0, maxP = -1.0;

    for (size_t i = 1; i < idx.size(); ++i) {
        auto id = idx[i];
        const float v = _yinBufer1[id];
        if (v > maxP)
            maxP = v;
        if (v < minP)
            minP = v;
    }

    float minMaxDist = maxP - minP;
    //qDebug() << "MinMax Dist is " << minMaxDist;

    filteredIdx.clear();
    qDebug() << "_";
    for (size_t i = 1; i < idx.size(); ++i) { //First is always 0 //TODO check + idx shifter

        auto id = idx[i];

        const float val =  _yinBufer1[id];
        const float distMax = maxP - val;
        const float distMin = val - minP;

        //qDebug() << "Dist max " << distMax << " distMin " << distMin;
        if (distMax <= distMin) {
            //qDebug() << "Phantom " << id << " _ " << val;
        }
        else {
            //qDebug() << "Real one " << id << " _ " << val;
            filteredIdx.push_back(id);
        }
    }

    double tX = findPeakCommonDistance(filteredIdx, 3); //dist can be +-5 I think

    size_t tNew = absThreshNew(_yinBufer1);
    double t1 = parabNew(tNew, _yinBufer1);

    //qDebug() << "Std found: " << t1 << " freq= " << _sampleRate / t1;
    //qDebug() << "Mine found: " << tX << " freq= " << _sampleRate / tX;

    //double intp = parabNew(tX, _yinBuffer1);

    //qDebug() << "Mine intp: " << intp << " freq= " << _sampleRate / intp;

    if (tX == 0)
        return 0.0;

    double foundPitch = _sampleRate / t1;
    return foundPitch;
}




size_t YinPP::absThreshNew(std::vector<float>& v) {
    double minValue = 5000.;
    size_t minTau = 0;
    size_t found = 0;


    tSet.clear();

    for (found = 2; found < v.size() ; found++) {
        if (v[found] < 0.15) {
            while (found + 1 < v.size() && v[found + 1] < v[found])
                ++found;
            //qDebug() << "Found adding " << found << " " << v[found];
            tSet.push_back(found);
        }

        if (v[found] < minValue) {
            minValue = v[found];
            minTau = found;
        }
    }

    if (tSet.empty())
        tSet.push_back(minTau); //Usually sub octave

    if (minTau != tSet[0]) {
        qDebug() << "Difference " << minTau << " and " << tSet[0];
        qDebug() << "_" << _sampleRate / minTau << " " << _sampleRate / tSet[0];
        qDebug() << "_";
    }

    return tSet[0];
}



double YinPP::parabOld(double t, std::vector<float>& v) {
    size_t start = t ? t - 1 : t;
    size_t finish = t + 1 < v.size() ? t + 1 : t;

    auto borderResult = [&](size_t idx) {
        if (v[t] <= v[idx])
            return static_cast<double>(t);
        else
            return static_cast<double>(idx);
    };

    if (start == t)
        return borderResult(finish);
    if (finish == t)
        return borderResult(start);

    double begin = v[start];
    double middle = v[t];
    double end = v[finish];
    return t + (end - begin) / (2.0 * (2.0 * middle - end - begin));
}


size_t YinPP::parabNew(size_t t, std::vector<float>& v) {
    if (t == v.size())
        return t;

    size_t betterTau = 0.f;
    if (t > 0 && t < v.size() - 1) {
        float s0 = v[t - 1];
        float s1 = v[t];
        float s2 = v[t + 1];
        float adjustment = (s2 - s0) / (2.f * (2.f * s1 - s2 - s0));
        if (abs(adjustment) > 1.f)
            adjustment = 0.f;
        betterTau = t + adjustment;
    }
    else
        betterTau =  t;

    return betterTau;
}


size_t YinPP::absThreshOld(std::vector<float>& v) {

    size_t found = 0;
    for (found = 2; found < v.size() ; found++)

        if (v[found] < _threshold) {
            while (found + 1 < v.size() &&
                   v[found + 1] < v[found])
                ++found;
            break;
        }
    return found;
}




void YinPP::accMeanDiff(std::vector<float>& v) {
    v[0] = 1;
    float runningSum = 0;
    for (size_t tau = 1; tau < v.size(); tau++) {
        runningSum += v[tau];
        v[tau] *= tau / runningSum;
    }
}


void YinPP::diffFunNew(const float* signal) {
    const size_t n = _bufferSize / 2;
    for(size_t tau = 0 ; tau < n; tau++)
        for(size_t j = 0; j < n; j++)
        {
            auto d = signal[j] - signal[j + tau];
            _yinBufer1[tau] += d * d;
        }
}

void YinPP::diffFunOld(const float* buffer) {
    const size_t n = _bufferSize / 2;
    float delta = 0.f;
    for(size_t tau = 0 ; tau < n; tau++) {
        for(size_t index = 0; index < n; index++){
            delta = buffer[index] - buffer[index + tau];
            _yinBufer2[tau] += delta * delta; //Что если здесь сделать 4ую степень но потом взять от всего корень? Эксперименты!
        }
    }
}




//===========================Experiments===========================

void YinPP::autoCorrelateionSlow1(const float* buffer, size_t tau, size_t W) {
    const size_t n = _bufferSize / 2;

    for (size_t t = 0; t < n; t++) {

        _yinBufer1[t] = 0;

        for (size_t j = t + 1; j < (t + W); j++) {
            _yinBufer1[t] += buffer[j] * buffer[j + tau];
        }
    }

}

void YinPP::autoCorrelateionSlow2(const float* buffer, size_t tau, size_t W) {
    const size_t n = _bufferSize / 2;
    //TODO buffer must be zeroes
    for (size_t t = 0; t < n; t++)
        for (size_t j = t + 1; j < (t + W - tau); j++)
            _yinBufer2[t] += buffer[j] * buffer[j + tau];

}

void YinPP::diffSlow(const float* yinBuf, size_t W) {

    const size_t n = _bufferSize / 2; //Or w??

    for(size_t tau = 0 ; tau < n; tau++) {

        for(size_t j = 0; j < n; j++)
        {
            auto d = yinBuf[j] - yinBuf[j + tau];
            _yinBufer1[tau] += d * d;
        }
    }
}//Что если здесь сделать 4ую степень но потом взять от всего корень? Эксперименты!


void YinPP::diffFast(const float* buffer, size_t W) {

    const size_t n = _bufferSize / 2; //Or w??

    auto first = _yinBufer1[0];

    for(size_t tau = 0 ; tau < n; tau++) {

        float second = 0.f;
        for (size_t j = 1; j < W; j++)
            second += buffer[j] * buffer[j + tau];

         _yinBufer2[tau] = first + second - _yinBufer1[tau];

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

    _yinBufer2 = std::vector<float>(_bufferSize, 0.f);

    _fft.do_ifft(buf.data(), _yinBufer2.data());
    _fft.rescale(_yinBufer2.data());

}


void YinPP::compareBuffers() {

    float eps = 10e-7;

    const size_t n = _bufferSize / 2;

    size_t failesCount = 0;

    for (size_t i = 0; i < n; i++) {
        if (std::abs(_yinBufer1[i] - _yinBufer2[i]) > eps) {

            qDebug() << i << " unequal " << _yinBufer1[i] << " " << _yinBufer2[i];

            ++failesCount;
            //if (failesCount > 10)
                //break;
        }
        //else
            //qDebug() << i << "is fine" << _yinBuffer1[i];
    }

    //qDebug() << "Maybe equal!";
}