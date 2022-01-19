#ifndef YIN_H
#define YIN_H

#include <vector>

// YIN monophonic pitch detection algorithm (http://audition.ens.fr/adc/pdf/2002_JASA_YIN.pdf).


namespace aural_sight {

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


    //TODO template with check is float + is_same
    class YinPP {

    public:
        void init(const double sampleRate, const size_t bufferSize) {
            _sampleRate = sampleRate;
            _bufferSize = bufferSize;
            _threshold = 0.2;
        }

        void compareBuffers();

        double process(const float* buffer);

        void differenceFunction(const float* signal);
        void accMeanDifferenceFunction(std::vector<float>& v);
        size_t absoluteThreshold(std::vector<float>& v);

        size_t parabolicInterpolation(size_t t, std::vector<float>& v);


        void calcBasicACF(const float* buffer);

    public:


        std::vector<float> acfBufer;
        std::vector<float> sumBufV2;

        std::vector<float> sumBufer;
        std::vector<float> accBufer;
        std::vector<size_t> filteredIdx;

        double stdFound;
        double mineFound; //Almost MPM method (TODO 3 in 1 : MPM, Yin + FFT)

    private:

        double _threshold;
        size_t _bufferSize;
        double _sampleRate;

        std::vector<float> _yinBufer1;

        //+ Amp vatiation implementation
        //+ F0 variation implementation
    };


}


#endif // YIN_H
