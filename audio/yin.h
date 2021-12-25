#ifndef YIN_H
#define YIN_H

#include <vector>

class Yin {

public:
    Yin();

    void init(double sampleRate, size_t bufferSize);
    double getPitch(const float* buffer);

    double getProbability() const;
    double getThreshold() const;

    void setProbability(double newProb);
    void setThreshold(double newThresh);

private:

    double parabolicInterpolation() const;
    bool absoluteThresholdFound();
    void accMeanNormDifference();
    void halvesDifference(const float *buffer);

    double threshold;
    size_t bufferSize;
    size_t halfBufferSize;
    double sampleRate;
    std::vector<float> yinBuffer;
    double probability;

    size_t currentTau;
};
#endif // YIN_H
