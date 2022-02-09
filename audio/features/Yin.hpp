#ifndef YIN_H
#define YIN_H

#include <vector>

// YIN monophonic pitch detection algorithm (http://audition.ens.fr/adc/pdf/2002_JASA_YIN.pdf).


namespace aurals {

    //Delayed template with check is float + is_same
    class YinPP {

    public:
        void init(const double sampleRate, const size_t bufferSize) {
            _sampleRate = sampleRate;
            _bufferSize = bufferSize;
            _threshold = 0.2;
        }

        void setThreshold(double newThreshold) {
            _threshold = newThreshold;
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
        double mineFound;

    private:

        double _threshold;

        size_t _bufferSize;
        double _sampleRate;

        std::vector<float> _yinBufer;

        bool _thresholdFound = false;

        //+ Amp vatiation implementation
        //+ F0 variation implementation
    };


}


#endif // YIN_H
