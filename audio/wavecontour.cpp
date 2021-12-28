#include "wavecontour.h"
#include "wavfile.h"

#include "featureextractor.h"

#include "utils.h"

#include <QDataStream>
#include <QDebug>

WaveContour::WaveContour(QString filename)
{
    if (loadWavFile(filename)==false)
        qDebug() << "Failed to load contour from file "<<filename;
}

ContourEl WaveContour::calculateElement(QVector<qint16> &samples)
{
    ContourEl result;
    result.energy = 0.0;
    result.max = -33000; //more then 32k to be sure
    result.min = 33000; //  16bit complely in,

    qint16 lastSample = 0;
    for (int i = 0; i < samples.size(); ++i)
    {
        qint16 sample = samples[i];

        if (result.max < sample)
            result.max = sample;

        if (result.min > sample)
            result.min = sample;

        result.energy += abs(lastSample-sample);

        lastSample = sample;//preparetion for next time
    }

    return result;
}

qint32 WaveContour::max4(qint32 d1,qint32 d2,qint32 d3,qint32 d4)
{
    if (d1 >= d2 && d1 >= d3 && d1 >= d4)
        return d1;

    if (d2 >= d1 && d2 >= d3 && d2 >= d4)
        return d2;

    if (d3 >= d1 && d3 >= d2 && d3 >= d4)
        return d3;

    if (d4 >= d1 && d4 >= d2 && d4 >= d3)
        return d4;

    qDebug () << "Max4 met trouble " << d1 << " "<< d2 << " "<< d3 << " "<< d4 ;

    return 0;
}

qint32 WaveContour::min4(qint32 d1,qint32 d2,qint32 d3,qint32 d4)
{
    if (d1 <= d2 && d1 <= d3 && d1 <= d4)
        return d1;

    if (d2 <= d1 && d2 <= d3 && d2 <= d4)
        return d2;

    if (d3 <= d1 && d3 <= d2 && d3 <= d4)
        return d3;

    if (d4 <= d1 && d4 <= d2 && d4 <= d3)
        return d4;

    qDebug () << "Min4 met trouble " << d1 << " "<< d2 << " "<< d3 << " "<< d4 ;

    return 0;
}

ContourEl WaveContour::summateElements(const ContourEl &e1,const  ContourEl &e2,const  ContourEl &e3,const  ContourEl &e4)
{
    ContourEl result;

    result.energy = e1.energy + e2.energy + e3.energy + e4.energy;
    result.max = max4(e1.max,e2.max,e3.max,e4.max);
    result.min = min4(e1.min,e2.min,e3.min,e4.min);

    return result;
}


ContourEl WaveContour::summate2Elements(const ContourEl &e1,const  ContourEl &e2)
{
    ContourEl result;

    result.energy = e1.energy + e2.energy;
    result.max = std::max(e1.max,e2.max);
    result.min = std::min(e1.min,e2.min);

    return result;
}

bool WaveContour::loadWavFile(QString filename)
{
    WavFile wav;
    wav.open(filename);

    QByteArray samplesBytes = wav.readAll(); //well some files can be to much - later protect it

    QVector<qint16> samples; //TODO?
    QVector<float> forRms;
    qint16 pcmSample;

    QDataStream dataStream(samplesBytes);

    for (int i = 0; i < samplesBytes.size(); ++i) {        //maybe its rather slow and better to read from here - but yet simplest ways
        dataStream >> pcmSample; //read one
        samples << pcmSample; //put it
    }

    const char *ptr = samplesBytes.constData(); //TODO from preloaded
    for (int i = 0; i < samplesBytes.size()/2; ++i)
    {        //maybe its rather slow and better to read from here - but yet simplest ways
        auto pcmSample2 = *reinterpret_cast<const qint16*>(ptr);
        const float realSample = pcmToReal(pcmSample2);
        forRms.append(realSample);
        ptr += 2; //16 bit audio
    }

    //TODO compare 2 ways

    bool noteStarted = false;
    size_t startPosition=0;
    const double startLimit = -30.0;
    const double fadeLimit = -36.0;

    size_t rmsSize = 125;
    size_t rmsFrames = samples.size()/rmsSize;

    for (size_t step = 0; step < rmsFrames; ++step) {

        auto forRmsLocal = forRms.mid(rmsSize*step,rmsSize);
        auto db = calc_dB(forRmsLocal.data(), forRmsLocal.size());

        rmsLine.append(db);

        if (noteStarted == false && db > startLimit) {
            noteStarted = true;
            startPosition = rmsSize*step;
            //qDebug() << "Starting note " << startPosition;
        }
        else if (noteStarted && db < fadeLimit) {
            auto endPosition = rmsSize*step;
            auto diff = endPosition - startPosition;
            //qDebug() << "Note stop, diff: " << diff/44100.0 << " start " << startPosition/44100.0 << " end " << endPosition/44100.0;
            //FIND BETTER WAY TODO
            noteStarted = false;
        }
    }


    //size_t

    const size_t counterFrameSize = 125;

    unsigned long frames = samples.size()/counterFrameSize;
    for (size_t step = 0; step < frames; ++step)
    {
        auto x64samples = samples.mid(counterFrameSize*step,counterFrameSize);

        //auto rms = calc_RMS(forRmsLocal.data(), forRmsLocal.size());

        auto x256samples1 = x64samples.mid(0,31);
        auto x256samples2 = x64samples.mid(31,31);
        auto x256samples3 = x64samples.mid(62,31);
        auto x256samples4 = x64samples.mid(93,32);

        ContourEl el256x1 = calculateElement(x256samples1);
        ContourEl el256x2 = calculateElement(x256samples2);
        ContourEl el256x3 = calculateElement(x256samples3);
        ContourEl el256x4 = calculateElement(x256samples4);

        ContourEl el64 = summateElements(el256x1,el256x2,el256x3,el256x4);

        ContourEl el128_1 = summate2Elements(el256x1,el256x2);
        ContourEl el128_2 = summate2Elements(el256x1,el256x2);

        zoom64 << el64;
        zoom128 << el128_1 << el128_2;
        zoom256 << el256x1 << el256x2 <<  el256x3 <<  el256x4;
    }

    qDebug() << "Loaded "<<samples.size() <<" samples; "<< zoom64.size() << " z64 " <<zoom256.size() << " z256";

    //this function can be used to rewrite and make 1024th - the smallest measure
    //then 256 - 64 - 16 - 4 - and even maybe 1th (full takt)  from it

    return zoom64.size() && zoom256.size();
}

QVector<ContourEl> WaveContour::summ4Lists(QVector<ContourEl> &source)
{
    QVector<ContourEl> result;

    for (int i = 0; i < source.size()/4; ++i)
    {
        ContourEl el1 = source[i*4 ];
        ContourEl el2 = source[i*4 + 1];
        ContourEl el3 = source[i*4 + 2];
        ContourEl el4 = source[i*4 + 3];

        ContourEl newEl = summateElements(el1,el2,el3,el4);

        result << newEl;
    }

    return result;
}

double WaveContour::findBPM()
{
    return 0.0;
}
