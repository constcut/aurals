#include "yin.h"

Yin::Yin() {
    threshold = 0.15;
    probability = 0.0;
}

double Yin::getProbability() const {
    return probability;
}

double Yin::getThreshold() const {
    return threshold;
}

void Yin::setProbability(double newProb) {
    probability = newProb;
}

void Yin::setThreshold(double newThresh) {
    threshold = newThresh;
}

void Yin::init(double yinSampleRate, size_t yinBufferSize) {
    if (yinSampleRate != sampleRate)
        sampleRate = yinSampleRate;
    if (bufferSize == yinBufferSize)
        return;
    bufferSize = yinBufferSize;
    halfBufferSize = bufferSize / 2;
    yinBuffer = std::vector<float>(halfBufferSize, 0.f);
}


double Yin::getPitch(const float *buffer) {
    halvesDifference(buffer);
    accMeanNormDifference();
    if (absoluteThresholdFound())
        return sampleRate / parabolicInterpolation();
    return -1.0; //Not found case
}


double Yin::parabolicInterpolation() const {

    size_t start = currentTau ? currentTau - 1 : currentTau;
    size_t finish = currentTau + 1 < halfBufferSize ? currentTau + 1 : currentTau;

    if (start == currentTau) {
        if (yinBuffer[currentTau] <= yinBuffer[finish])
            return currentTau;
        else
            return finish;
    }

    if (finish == currentTau) {
        if (yinBuffer[currentTau] <= yinBuffer[start])
            return currentTau;
        else
            return start;
    }

    double begin = yinBuffer[start];
    double middle = yinBuffer[currentTau];
    double end = yinBuffer[finish];
    return currentTau + (end - begin) / (2 * (2 * middle - end - begin));
}



void Yin::accMeanNormDifference(){
    yinBuffer[0] = 1;
    float runningSum = 0;
    for (size_t tau = 1; tau < halfBufferSize; tau++) {
        runningSum += yinBuffer[tau];
        yinBuffer[tau] *= tau / runningSum;
    }
}


void Yin::halvesDifference(const float* buffer) {
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
