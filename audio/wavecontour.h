#ifndef WAVECONTOUR_H
#define WAVECONTOUR_H
#include <QList>



struct ContourEl{
    qint32 max;
    qint32 min;
    double energy; //TODO альтернативные варианты
};



class WaveContour
{
public:
    WaveContour(QString filename);
    WaveContour() {}

    const QVector<double>& getRMS() const { return rmsLine; }
    const QVector<double>& getPitch() const { return yinLine; }

    const QVector<int>& getNoteStarts() const { return noteStarts; }
    const QVector<int>& getNoteEnds() const { return noteEnds; }

    const QVector<ContourEl>& getZoom64()  const { return zoom64; }
    const QVector<ContourEl>& getZoom128() const { return zoom128; }
    const QVector<ContourEl>& getZoom256() const { return zoom256; }

    void calculateF0();

public:

    ContourEl calculateElement(QVector<qint16> &samples); //TODO static?
    qint32 max4(qint32 d1,qint32 d2,qint32 d3,qint32 d4);
    qint32 min4(qint32 d1,qint32 d2,qint32 d3,qint32 d4);
    ContourEl summateElements(const ContourEl &e1,const  ContourEl &e2,const  ContourEl &e3,const  ContourEl &e4);
    ContourEl summate2Elements(const ContourEl &e1,const  ContourEl &e2);

protected:
    QVector<ContourEl> summ4Lists(QVector<ContourEl> &source);

    QVector<float> floatSamples;
    QVector<double> rmsLine;
    QVector<double> yinLine;

    QVector<int> noteStarts;
    QVector<int> noteEnds;

    QVector<ContourEl> zoom64;
    QVector<ContourEl> zoom128;
    QVector<ContourEl> zoom256;
    QVector<ContourEl> bpm64; //TODO

    double totalBPM;

private:
    double findBPM(); //automation

    bool loadWavFile(QString filename);
};

#endif // WAVECONTOUR_H
