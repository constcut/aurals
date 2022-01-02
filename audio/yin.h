#ifndef YIN_H
#define YIN_H

#include <vector>

class Yin {

public:
    Yin();

    void init(double sampleRate, size_t bufferSize);
    double getPitch(const float* buffer);

    double getProbability() const; //TODO look for funny place
    double getThreshold() const;    //TODO play with configurable parameter

    void setProbability(double newProb);
    void setThreshold(double newThresh);

private:

    double parabolicInterpolation() const;
    bool absoluteThresholdFound();
    void accMeanNormDifference();
    void halvesDifference(const float *buffer);

    double _threshold;
    size_t _bufferSize;
    size_t _halfBufferSize;
    double _sampleRate;
    std::vector<float> _yinBuffer;
    double _probability;

    size_t _currentTau;
};

#endif // YIN_H
