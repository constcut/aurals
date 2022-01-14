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


    //TODO template with check is float + is_same
    class YinPP { //Yin ++ better version with step 1, step 1 opt, step 2 on step 1, and step 6

    public:
        void init(const double sampleRate, const size_t bufferSize) {
            _sampleRate = sampleRate;
            _bufferSize = bufferSize;
            _threshold = 0.2;
            _yinBuffer1 = std::vector<float>(_bufferSize/2.0, 0.f);
            _yinBuffer2 = std::vector<float>(_bufferSize/2.0, 0.f);
        }

        void compareBuffers();

        double process(const float* buffer);

        //TODO access to buffers for visualization + mode to save all the results
        void diffFunNew(const float* signal);
        void diffFunOld(const float* buffer);

        void accMeanDiff(std::vector<float>& v);

        size_t absThreshOld(std::vector<float>& v);
        size_t absThreshNew(std::vector<float>& v);

        size_t parabNew(size_t t, std::vector<float>& v);
        size_t parabOld(size_t t, std::vector<float>& v);

    private:

        double _threshold;
        size_t _bufferSize;
        double _sampleRate;

        std::vector<float> _yinBuffer1;
        std::vector<float> _yinBuffer2;

        //STEP1:
        //TODO autocorrelation FFT method

        //STEP2:
        //TODO diff function on acf

        //STEP3:
        //TODO accMeanNormDifference

        //STEP4:
        //TODO threshold

        //STEP5: parabolic inerpolation

        //STEP6: estimate

        //+ Amp vatiation implementation
        //+ F0 variation implementation
        void autoCorrelateionSlow1(const float* buffer, size_t tau, size_t W);
        void autoCorrelateionSlow2(const float* buffer, size_t tau, size_t W);
        void autoCorrelationFast(const float* buffer);
        void diffSlow(const float* yinBuf, size_t W);
        void diffFast(const float* yinBuf, size_t W);


    };



}


#endif // YIN_H
