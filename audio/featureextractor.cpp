#include "featureextractor.h"

#include <cmath>


double calc_RMS(const float* data, const size_t N) {
    double squareAmpSum = 0.0;
    for (size_t i = 0; i < N; ++i)
        squareAmpSum += data[i] * data[i];
    return  std::sqrt(squareAmpSum / static_cast<double>(N));
}


double calc_dB(const float* data, const size_t N) {
    return 20 * log10(calc_RMS(data, N));
}


double calc_MidiCents(double f0) {
    double note = 69 + log(f0 / 440.f) * 17.31234; //Some evil magic from lib_extract
    return floor( 0.5f + note * 100 ); //TODO check round(note * 100)?
}

//TODO single function with static object



Yin::Yin() {
    threshold = 0.15;
    probability = 0.0;
}

double Yin::getProbability() {
    return probability;
}

double Yin::getThreshold() {
    return threshold;
}

void Yin::setProbability(double newProb) {
    probability = newProb;
}

void Yin::setThreshold(double newThresh) {
    threshold = newThresh;
}

void Yin::initialize(double yinSampleRate, size_t yinBufferSize) {
    if (yinSampleRate != sampleRate)
        sampleRate = yinSampleRate;
    if (bufferSize == yinBufferSize)
        return;
    bufferSize = yinBufferSize;
    halfBufferSize = bufferSize / 2;
    yinBuffer = std::vector<float>(halfBufferSize, 0.f);
}


double Yin::getPitch(float* buffer) {
    halvesDifference(buffer);
    accMeanNormDifference();
    if (absoluteThresholdFound())
        return sampleRate / parabolicInterpolation(currentTau);
    return -1.0; //Not found
}


float Yin::parabolicInterpolation(int tauEstimate) {
    float betterTau;
    int x0;
    int x2;

    if (tauEstimate < 1) {
        x0 = tauEstimate;
    }
    else {
        x0 = tauEstimate - 1;
    }
    if (tauEstimate + 1 < halfBufferSize) {
        x2 = tauEstimate + 1;
    }
    else {
        x2 = tauEstimate;
    }
    if (x0 == tauEstimate) {
        if (yinBuffer[tauEstimate] <= yinBuffer[x2]) {
            betterTau = tauEstimate;
        }
        else {
            betterTau = x2;
        }
    }
    else if (x2 == tauEstimate) {
        if (yinBuffer[tauEstimate] <= yinBuffer[x0]) {
            betterTau = tauEstimate;
        }
        else {
            betterTau = x0;
        }
    }
    else {
        float s0, s1, s2;
        s0 = yinBuffer[x0];
        s1 = yinBuffer[tauEstimate];
        s2 = yinBuffer[x2];
        // fixed AUBIO implementation, thanks to Karl Helgason:
        // (2.0f * s1 - s2 - s0) was incorrectly multiplied with -1
        betterTau = tauEstimate + (s2 - s0) / (2 * (2 * s1 - s2 - s0));
    }
    return betterTau;
}



void Yin::accMeanNormDifference(){
    yinBuffer[0] = 1;
    float runningSum = 0;
    for (size_t tau = 1; tau < halfBufferSize; tau++) {
        runningSum += yinBuffer[tau];
        yinBuffer[tau] *= tau / runningSum;
    }
}


void Yin::halvesDifference(float* buffer) {
    float delta = 0.f;
    for(size_t tau = 0 ; tau < halfBufferSize; tau++)
        for(size_t index = 0; index < halfBufferSize; index++){
            delta = buffer[index] - buffer[index + tau];
            yinBuffer[tau] += delta * delta;
        }
}


bool Yin::absoluteThresholdFound(){
    for (currentTau = 2; currentTau < halfBufferSize ; currentTau++)
        if (yinBuffer[currentTau] < threshold) {
            while (currentTau + 1 < halfBufferSize &&
                   yinBuffer[currentTau + 1] < yinBuffer[currentTau])
                ++currentTau;
            break;
        }
    if (currentTau == halfBufferSize || yinBuffer[currentTau] >= threshold) {
        probability = 0;
        return false;
    }
    return true;
}
