#ifndef WAVECONTOUR_H
#define WAVECONTOUR_H

#include <QAudioFormat>

namespace aurals {

    class WaveContour  {

    public:
        WaveContour(QString filename);
        WaveContour() = default;
        ~WaveContour() = default;

        const std::vector<float>& getFloatSamples() const { return _floatSamples; }

        const std::vector<double>& getRMS() const { return _rmsLine; }
        const std::vector<double>& getRMS_2() const { return _halfRmsLine; }
        const std::vector<double>& getRMS_4() const { return _quaterRmsLine; }
        const std::vector<double>& getRMS_8() const { return _8RmsLine; }

        const std::vector<size_t> rmsHigh() const { return _rmsHigh; }
        const std::vector<size_t> rmsLow() const { return _rmsLow; }

        void changePeakSence(double newSence) {
            _peakSensetivity = newSence;
            calculateRms();
        }

        size_t getRmsStep() const { return _rmsStep; }
        void setRmsStep(size_t newRmsStep) {
            _rmsStep = newRmsStep;
            calculateRms();
        }
        size_t getMinRmsStep() const { return _rmsStep / 4; }

        void calculateF0();
        const std::vector<double>& getPitch() const { return _yinLine; }
        void changeYinTheshold(const double threshold) { _yinTheshold = threshold; }
        double getYinThreshold() const { return _yinTheshold; }

        QAudioFormat getAudioFormat() { return _audioFormat; }
        QByteArray getPCM(const quint64 position, const quint64 samples);
        QByteArray getFloatSamples(const quint64 position, const quint64 samples);

        void setYinWindowSize(const size_t newSize) { _yinWindowSize = newSize; }
        size_t getYinWindowSize() const { return _yinWindowSize; }

        void STFTtoFile(QString filename) const;
        QImage makeSTFT() const;

        QImage makeCQT() const;
        void CQTtoFile(QString filename) const;

        QImage makeCWT() const;

    protected:

        std::vector<float> _floatSamples;

        size_t _rmsStep = 125 * 4;
        std::vector<double> _rmsLine;
        std::vector<double> _halfRmsLine;
        std::vector<double> _quaterRmsLine;
        std::vector<double> _8RmsLine;

        double _peakSensetivity = 4.0;
        std::vector<size_t> _rmsHigh;
        std::vector<size_t> _rmsLow;

        double _yinTheshold = 0.15;
        size_t _yinWindowSize = 2048;
        std::vector<double> _yinLine;

        QAudioFormat _audioFormat; //Если избавиться от этого компонента и переписать WavFile - можно сделать класс pure C++

    private:
        bool loadWavFile(const QString filename);

        void calculateRms();
    };

}

#endif // WAVECONTOUR_H
