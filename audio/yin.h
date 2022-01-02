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


// YIN monophonic pitch detection algorithm (http://audition.ens.fr/adc/pdf/2002_JASA_YIN.pdf).

class Yin2 {
public:

    Yin2();

    float getFundamentalFrequency(const std::vector<float>& audio, float sampleRate);

    void setThreshold(float);
    float getThreshold() const;

    float getTau() const;
    float getFreq() const;
    const std::vector<float>& getData() const;

    float indexToFreq(float i, float sampleRate) const;
    size_t freqToIndex(float f, float sampleRate) const;

private:

    float _threshold;

    std::vector<float> _yinData;
    float _tau;
    float _freq;

    std::vector<float> buildYinData(const std::vector<float>& audio) const;
    std::vector<float> sumOfDeltas(const std::vector<float>& audio) const;
    float parabolicInterpolation(const std::vector<float>& yinData, int tau) const;
    int absoluteThreshold(const std::vector<float>& yinData) const;
    void cumulativeDifference(std::vector<float> &yinData) const;

};


#endif // YIN_H
