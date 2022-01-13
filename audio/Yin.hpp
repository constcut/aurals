#ifndef YIN_H
#define YIN_H

#include <vector>

// YIN monophonic pitch detection algorithm (http://audition.ens.fr/adc/pdf/2002_JASA_YIN.pdf).


namespace mtherapp {

class Yin {

    public:
        Yin();

        void init(const double sampleRate, const size_t bufferSize);
        double getPitch(const float* buffer);

        double getThreshold() const;
        void setThreshold(const double newThresh);

        size_t getTau() const;

    private:

        double parabolicInterpolation() const;
        bool absoluteThresholdFound();
        void accMeanNormDifference();
        void differenceFunction(const float *buffer);

        double _threshold;
        size_t _bufferSize;
        size_t _halfBufferSize;
        double _sampleRate;
        std::vector<float> _yinBuffer;

        size_t _currentTau;
    };

}


#endif // YIN_H
