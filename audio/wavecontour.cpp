#include "wavecontour.h"
#include "wavfile.h"

#include "featureextractor.h"

#include "utils.h"

#include <array>

#include <QDataStream>
#include <QDebug>

WaveContour::WaveContour(QString filename)
{
    if (loadWavFile(filename)==false)
        qDebug() << "Failed to load contour from file "<<filename;
}

void WaveContour::calculateF0() {
    yinLine.clear();
    size_t yinSize = 2048;
    size_t yinFrames = floatSamples.size() / yinSize;

    for (size_t step = 0; step < yinFrames; ++step) {
        auto forLocalYin = floatSamples.mid(yinSize * step, yinSize);
        if (forLocalYin.empty())
            break;
        auto pitch = calc_YinF0(forLocalYin.data(), forLocalYin.size());
        //qDebug() << step << " Yin tracking " << pitch;
        yinLine.append(pitch);
    }
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

    QByteArray samplesBytes = wav.readAll();
    QVector<qint16> samples;
    qint16 pcmSample;
    QDataStream dataStream(samplesBytes);
    for (int i = 0; i < samplesBytes.size(); ++i) {
        dataStream >> pcmSample;
        samples << pcmSample;
    }

    floatSamples.clear();
    const char *ptr = samplesBytes.constData();
    for (int i = 0; i < samplesBytes.size()/2; ++i) {
        auto pcmSample2 = *reinterpret_cast<const qint16*>(ptr);
        const float realSample = pcmToReal(pcmSample2);
        floatSamples.append(realSample);
        ptr += 2; //16 bit audio
    }

    size_t rmsSize = 125;
    size_t rmsFrames = samples.size()/rmsSize;

    QList<double> lastRms;

    bool noteIsStarted = false;

    for (size_t step = 0; step < rmsFrames; ++step) {
        auto forRmsLocal = floatSamples.mid(rmsSize*step,rmsSize);
        auto db = calc_dB(forRmsLocal.data(), forRmsLocal.size());
        rmsLine.append(db);
        lastRms.append(db);

        const int checkLimit = 12;

        if (lastRms.size() == checkLimit) { //TODO subfunction

            double prev = lastRms[0];

            std::array<int, checkLimit> states = {};

            const double goodBorder = 6.0;
            const double perfectBorder = 9.0; //12?

            double max = -120.0;
            int maxIdx = -1;
            double min = 0.0;
            int minIdx = -1;

            for (int i = 1; i < lastRms.size(); ++i) {
                double current = lastRms[i];
                if (current - prev > goodBorder)
                    states[i] = 1; //TODO enumerate states
                if (current - prev > perfectBorder)
                    states[i] = 2;

                if (max < current) {
                    max = current;
                    maxIdx = i;
                }
                if (min > current) {
                    min = current;
                    minIdx = i;
                }
                prev = current;
            }

            const int foundPosition = rmsSize * step;

            if (noteIsStarted)
                if (minIdx == checkLimit - 1 & min < -36.0) {
                    if (max - min > 12.0) {
                        qDebug() << "Note end " << foundPosition / 44100.0;
                        noteEnds.append(foundPosition);
                        noteIsStarted = false;


                    }
                }

            if (noteIsStarted == false)
                if (maxIdx == checkLimit - 1 & max > -28.0) { //TODO configurable param (first set, then calculate)

                    bool foundGood = false;
                    int goodDist = -1;
                    bool foundPerfect = false;
                    int perfectDist = -1;

                    for (int i = maxIdx; i != -1; --i) {
                        if (states[i] == 2) {
                            foundPerfect = true;
                            perfectDist = maxIdx - i;
                            break;
                        }
                        if (foundGood == false && states[i] == 1) {
                            foundGood = true;
                            goodDist = maxIdx - i;
                        }
                    }

                    if (foundPerfect) {
                        qDebug() << "+Note start " << foundPosition / 44100.0 << " dist " << perfectDist;
                        noteStarts.append(foundPosition);
                        noteIsStarted = true;
                    }
                    else if (foundPerfect) {
                        qDebug() << "+Note start " << foundPosition / 44100.0 << " dist " << goodDist;
                        noteStarts.append(foundPosition);
                        noteIsStarted = true;
                    }
                    else {
                        if (max - min > perfectBorder) {
                            qDebug() << "MaxMin note " << foundPosition / 44100.0 << " " << (max - min);
                            noteStarts.append(foundPosition);
                            noteIsStarted = true;
                        }
                    }
                }

            lastRms.pop_front();
        }
    }

    const size_t counterFrameSize = 125;
    unsigned long frames = samples.size()/counterFrameSize;
    for (size_t step = 0; step < frames; ++step) {
        auto x64samples = samples.mid(counterFrameSize*step,counterFrameSize);
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

    return zoom64.size() && zoom256.size();
}

QVector<ContourEl> WaveContour::summ4Lists(QVector<ContourEl> &source) {
    QVector<ContourEl> result;
    for (int i = 0; i < source.size()/4; ++i) {
        ContourEl el1 = source[i*4 ];
        ContourEl el2 = source[i*4 + 1];
        ContourEl el3 = source[i*4 + 2];
        ContourEl el4 = source[i*4 + 3];
        ContourEl newEl = summateElements(el1,el2,el3,el4);
        result << newEl;
    }
    return result;
}

double WaveContour::findBPM() {
    return 0.0;
}
