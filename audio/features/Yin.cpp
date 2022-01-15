#include "Yin.hpp"

#include <QDebug>

#include "libs/fft/FFTurealfix.hpp"

#include "FindPeaks.hpp"

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

    diffFunNew(buffer);
    diffFunOld(buffer);

    compareBuffers();

    accMeanDiff(_yinBuffer1);
    accMeanDiff(_yinBuffer2);

    auto maxE = std::max_element(_yinBuffer1.begin(), _yinBuffer1.end());
    float maxVal = *maxE;

    auto minE = std::min_element(_yinBuffer1.begin(), _yinBuffer1.end());
    float minVal = *minE;

    std::vector<double> invBuffer(_yinBuffer1.size(), 0);
    for (size_t i = 0; i < invBuffer.size(); ++i)
        invBuffer[i] = maxVal - _yinBuffer1[i];

    //Find best ways to find local minima

    auto idx = peakIndexes<double>(invBuffer);

    qDebug() << "_";
    for (auto id: idx) {
        if (_yinBuffer1[id] > minVal*5) { //TODO K mean
            //qDebug() << "Sub place";
        }
        else
            qDebug() << id << " one of ids " << _yinBuffer1[id] << " f0 "
                     << _sampleRate / id;
    }


    size_t tNew = absThreshNew(_yinBuffer1);
    size_t tOld = absThreshOld(_yinBuffer2);

    double preF1 = _sampleRate / tNew;
    double preF2 = _sampleRate / tOld;

    //qDebug() << "Pre " << preF1 << " " << preF2;

    size_t t1 = parabNew(tNew, _yinBuffer1); //TODO replace with new
    size_t t2 = parabOld(tOld, _yinBuffer2);

    //qDebug() << "Para new " << t1 << " para old " << t2;

    qDebug() << "And found is " << t1;

    //for (auto ts: tSet)
        //qDebug() << "Th set " << ts;

    //TODO estimate

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



size_t YinPP::parabOld(size_t t, std::vector<float>& v) {
    size_t start = t ? t - 1 : t;
    size_t finish = t + 1 < v.size() ? t + 1 : t;

    auto borderResult = [&](size_t idx) {
        if (v[t] <= v[idx])
            return t;
        else
            return idx;
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
            _yinBuffer1[tau] += d * d;
        }
}

void YinPP::diffFunOld(const float* buffer) {
    const size_t n = _bufferSize / 2;
    float delta = 0.f;
    for(size_t tau = 0 ; tau < n; tau++) {
        for(size_t index = 0; index < n; index++){
            delta = buffer[index] - buffer[index + tau];
            _yinBuffer2[tau] += delta * delta; //Что если здесь сделать 4ую степень но потом взять от всего корень? Эксперименты!
        }
    }
}




//===========================Experiments===========================

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

        for(size_t j = 0; j < n; j++)
        {
            auto d = yinBuf[j] - yinBuf[j + tau];
            _yinBuffer1[tau] += d * d;
        }
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

    //qDebug() << "Maybe equal!";
}
