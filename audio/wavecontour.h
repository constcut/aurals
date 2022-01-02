#ifndef WAVECONTOUR_H
#define WAVECONTOUR_H
#include <QList>


struct ContourEl{
    qint32 max;
    qint32 min;
    double energy; //TODO альтернативные варианты
};


class WaveContour {

public:
    WaveContour(QString filename);
    WaveContour() {}

    const QVector<double>& getRMS() const { return _rmsLine; }
    const QVector<double>& getPitch() const { return _yinLine; }

    const QVector<int>& getNoteStarts() const { return _noteStarts; }
    const QVector<int>& getNoteEnds() const { return _noteEnds; }

    const QVector<ContourEl>& getZoom64()  const { return _zoom64; }
    const QVector<ContourEl>& getZoom128() const { return _zoom128; }
    const QVector<ContourEl>& getZoom256() const { return _zoom256; }

    const std::vector<size_t> rmsHigh() const { return _rmsHigh; }
    const std::vector<size_t> rmsLow() const { return _rmsLow; }

    size_t getRmsStep() const { return _rmsStep; }
    void setRmsStep(size_t newRmsStep) {
        _rmsStep = newRmsStep;
        calculateRms();
    }

    void calculateF0();

    ContourEl calculateElement(QVector<qint16> &samples) const; //TODO static?
    qint32 max4(qint32 d1,qint32 d2,qint32 d3,qint32 d4) const;
    qint32 min4(qint32 d1,qint32 d2,qint32 d3,qint32 d4) const;
    ContourEl summateElements(const ContourEl &e1,const  ContourEl &e2,
                              const  ContourEl &e3,const  ContourEl &e4) const;
    ContourEl summate2Elements(const ContourEl &e1,const  ContourEl &e2) const;

    void changePeakSence(double newSence) {
        _peakSensetivity = newSence;
        calculateRms();
    }

    void changeYinTheshold(double threshold) {
        _yinTheshold = threshold;
    }

protected:
    QVector<ContourEl> summ4Lists(QVector<ContourEl> &source) const;

    QVector<float> _floatSamples;

    size_t _rmsStep = 125 * 4;
    QVector<double> _rmsLine;

    double _peakSensetivity = 4.0;
    std::vector<size_t> _rmsHigh;
    std::vector<size_t> _rmsLow;

    double _yinTheshold = 0.15;
    QVector<double> _yinLine;

    QVector<int> _noteStarts;
    QVector<int> _noteEnds;

    QVector<ContourEl> _zoom64;
    QVector<ContourEl> _zoom128;
    QVector<ContourEl> _zoom256;

    QVector<ContourEl> _bpm64; //TODO
    double _totalBPM;


private:

    double findBPM(); //TODO
    bool loadWavFile(QString filename);

    void calculateRms();

    void createCountour(QByteArray& samplesBytes);

    void markNotesStartEnd(QList<double>& lastRms, bool& noteIsStarted, size_t step);


};

#endif // WAVECONTOUR_H
