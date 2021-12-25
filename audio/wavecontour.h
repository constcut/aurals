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

    QList<ContourEl> getZoom64() { return zoom64; }
    QList<ContourEl> getZoom256() { return zoom256; }

    QList<ContourEl> getZoom4() { return zoom4; }
    QList<ContourEl> getZoom16() { return zoom16; }

public: //helpers for ContourEl

    ContourEl calculateElement(QList<qint16> &samples);
    qint32 max4(qint32 d1,qint32 d2,qint32 d3,qint32 d4);
    qint32 min4(qint32 d1,qint32 d2,qint32 d3,qint32 d4);
    ContourEl summateElements(const ContourEl &e1,const  ContourEl &e2,const  ContourEl &e3,const  ContourEl &e4);

protected:
    //TODO vector
    QList<ContourEl> zoom4; //2000 samples on 8k
    QList<ContourEl> zoom16; //500 samples on 8k

    QList<ContourEl> summ4Lists(QList<ContourEl> &source);

    QList<ContourEl> zoom64; //125 samples on 8k
    QList<ContourEl> zoom256; //31.25 samples on 8k

    QList<ContourEl> bpm64; //formula calculated parts on folowing bpm

    double totalBPM;

private:
    double findBPM(); //automation

    bool loadWavFile(QString filename);
};

#endif // WAVECONTOUR_H