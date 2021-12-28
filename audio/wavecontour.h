#ifndef WAVECONTOUR_H
#define WAVECONTOUR_H
#include <QList>

struct ContourEl{
    qint32 max;
    qint32 min; //as module - to be sure to hold 24bit

    double energy;
    //TODO rms
};

class WaveContour
{
public:
    WaveContour(QString filename);
    WaveContour() {}

    const QVector<double>& getRMS() const { return rmsLine; }

    const QVector<ContourEl>& getZoom64()  const { return zoom64; }
    const QVector<ContourEl>& getZoom128() const { return zoom128; }
    const QVector<ContourEl>& getZoom256() const { return zoom256; }


public: //helpers for ContourEl

    ContourEl calculateElement(QVector<qint16> &samples);
    qint32 max4(qint32 d1,qint32 d2,qint32 d3,qint32 d4);
    qint32 min4(qint32 d1,qint32 d2,qint32 d3,qint32 d4);
    ContourEl summateElements(const ContourEl &e1,const  ContourEl &e2,const  ContourEl &e3,const  ContourEl &e4);
    ContourEl summate2Elements(const ContourEl &e1,const  ContourEl &e2);

protected:
    //TODO vector


    QVector<ContourEl> summ4Lists(QVector<ContourEl> &source);

    QVector<double> rmsLine;

    QVector<ContourEl> zoom64;
    QVector<ContourEl> zoom128;
    QVector<ContourEl> zoom256;

    QVector<ContourEl> bpm64; //formula calculated parts on folowing bpm

    double totalBPM;

private:
    double findBPM(); //automation

    bool loadWavFile(QString filename);
};

#endif // WAVECONTOUR_H
