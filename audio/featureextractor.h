#ifndef FEATUREEXTRACTOR_H
#define FEATUREEXTRACTOR_H

#include <string> //Window names in future, now for size_t :)
#include <vector>

double calc_RMS(const float* data, const size_t N);
double calc_dB(const float* data, const size_t N);

double calc_MidiCents(double f0);

//TODO libextract features slop, sharp etc to find ADSR?

class Yin {

public:
    Yin();

    void initialize(double sampleRate, size_t bufferSize);
    double getPitch(float* buffer);

    double getProbability();
    double getThreshold();

    void setProbability(double newProb);
    void setThreshold(double newThresh);

private:

    float parabolicInterpolation(int tauEstimate);
    bool absoluteThresholdFound();
    void accMeanNormDifference();
    void halvesDifference(float* buffer);

    double threshold;
    size_t bufferSize;
    size_t halfBufferSize;
    double sampleRate;
    std::vector<float> yinBuffer;
    double probability;

    size_t currentTau;
};

#endif // FEATUREEXTRACTOR_H
