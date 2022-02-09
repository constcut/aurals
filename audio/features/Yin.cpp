#include "Yin.hpp"

#include <QDebug>

#include "PeaksOperations.hpp"

#include "libs/kiss/kiss_fft.h"
#include <complex>


using namespace aurals;


void YinPP::calcBasicACF(const float* buffer) {

    std::vector<std::complex<float>> b(_bufferSize, 0.f);

    std::transform(buffer, buffer + _bufferSize,
        b.begin(), [](float x) -> std::complex<float> {
            return std::complex(x, static_cast<float>(0.0));
        });

    kiss_fft_cfg fwd = kiss_fft_alloc(_bufferSize, 0, NULL,NULL);
    kiss_fft_cfg inv = kiss_fft_alloc(_bufferSize, 1, NULL,NULL);

    std::vector<std::complex<float>> out(_bufferSize, 0.f);

    kiss_fft(fwd, reinterpret_cast<kiss_fft_cpx*>(b.data()),
             reinterpret_cast<kiss_fft_cpx*>(out.data()));

    std::complex<float> scale = {
        1.0f / (float)(_bufferSize * 2), static_cast<float>(0.0)};

    for (size_t i = 0; i < _bufferSize; ++i)
        out[i] = std::norm(out[i]) * scale;

    kiss_fft(inv, reinterpret_cast<kiss_fft_cpx*>(out.data()),
             reinterpret_cast<kiss_fft_cpx*>(b.data()));

    std::vector<float> realOut(_bufferSize, 0.f); //half?
    std::transform(b.begin(), b.begin() + _bufferSize, realOut.begin(),
        [](std::complex<float> cplx) -> float { return std::real(cplx); });

    acfBufer = realOut;
    sumBufV2 = std::vector<float>(_bufferSize/2, 0.f);

    for (size_t i = 0; i < _bufferSize/2; ++i)  //Для ускоренного вычисления: second term is wrong!
        sumBufV2[i] = 2 * realOut[0]  - 2 * realOut[i];
}


double YinPP::process(const float* buffer) {

    calcBasicACF(buffer);

    _yinBufer = std::vector<float>(_bufferSize/2.0, 0.f); //Delayed better way of flush

    differenceFunction(buffer);
    sumBufer = _yinBufer;

    accMeanDifferenceFunction(_yinBufer);
    accBufer = _yinBufer;

    auto maxE = std::max_element(_yinBufer.begin(), _yinBufer.end());
    float maxVal = *maxE;

    std::vector<double> invBuffer(_yinBufer.size(), 0); //Delayed inversed function for peaks
    for (size_t i = 0; i < invBuffer.size(); ++i)
        invBuffer[i] = maxVal - _yinBufer[i];

    auto idx = peakIndexes<double>(invBuffer, 2);

    size_t startIdx = 0;
    if (idx.empty() == false && idx[0] == 0)
        startIdx = 1;

    float minP = 100.0, maxP = -1.0;
    for (size_t i = startIdx; i < idx.size(); ++i) {
        auto id = idx[i];
        const float v = _yinBufer[id];
        if (v > maxP)
            maxP = v;
        if (v < minP)
            minP = v;
    }

    filteredIdx.clear();
    for (size_t i = startIdx; i < idx.size(); ++i) {
        auto id = idx[i];
        const float val =  _yinBufer[id];
        const float distMax = maxP - val;
        const float distMin = val - minP;
        if (distMax > distMin)
            filteredIdx.push_back(id);
    }

    mineFound = findPeakCommonDistance(filteredIdx, 3);

    size_t tNew = absoluteThreshold(_yinBufer);
    stdFound = parabolicInterpolation(tNew, _yinBufer);

    double foundPitch = _sampleRate / stdFound;

    //TODO оценка гладкости функции - отсечение поиска частот на шумовых звуках
    //+ оценка стадии ноты по уровням прижатости компонентов - разметка ноты на ASDR огибающую

    if (mineFound !=.0)
        foundPitch = _sampleRate /  mineFound;

    if (foundPitch > 660.0) //Yin isn't good and highs -> limit for 1st string fret 12
        foundPitch = -1.0;

    return foundPitch;
}



size_t YinPP::absoluteThreshold(std::vector<float>& v) {
    double minValue = 5000.;
    size_t minTau = 0;
    size_t found = 0;

    _thresholdFound = false;

    for (found = 2; found < v.size() ; found++) {
        if (v[found] < _threshold) {
            while (found + 1 < v.size() && v[found + 1] < v[found])
                ++found;
            _thresholdFound = true;
            return found;
        }
        else
            if (v[found] < minValue) {
                minValue = v[found];
                minTau = found;
            }
    }

    return minTau;
}



size_t YinPP::parabolicInterpolation(size_t t, std::vector<float>& v) {
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



void YinPP::accMeanDifferenceFunction(std::vector<float>& v) {
    v[0] = 1;
    float runningSum = 0;
    for (size_t tau = 1; tau < v.size(); tau++) {
        runningSum += v[tau];
        v[tau] *= tau / runningSum;
    }
}


void YinPP::differenceFunction(const float* signal) {
    const size_t n = _bufferSize / 2;
    for(size_t tau = 0 ; tau < n; tau++)
        for(size_t j = 0; j < n; j++)
        {
            auto d = signal[j] - signal[j + tau];
            _yinBufer[tau] += d * d;
        }
}

