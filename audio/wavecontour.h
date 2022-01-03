#ifndef WAVECONTOUR_H
#define WAVECONTOUR_H

#include <QList>
#include <QObject>


class WaveContour /*: public QObject*/ { //This class could be sepparated from Qt enviroment, just don't enherit QObject, and remove macros
    //Q_OBJECT

public:

    WaveContour(QString filename);
    WaveContour() {}
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

    void calculateF0();
    const std::vector<double>& getPitch() const { return _yinLine; }
    void changeYinTheshold(double threshold) { _yinTheshold = threshold; }

protected:

    std::vector<float> _floatSamples;

    size_t _rmsStep = 125 * 4;
    std::vector<double> _rmsLine; //0, 1, 2, 3
    std::vector<double> _halfRmsLine;
    std::vector<double> _quaterRmsLine; //TODO заменить на обычные вектора,+ сделать объект QObject
    std::vector<double> _8RmsLine;

    double _peakSensetivity = 4.0;
    std::vector<size_t> _rmsHigh;
    std::vector<size_t> _rmsLow;

    double _yinTheshold = 0.15;
    std::vector<double> _yinLine;

private:
    bool loadWavFile(QString filename);

    void calculateRms();
};

#endif // WAVECONTOUR_H
