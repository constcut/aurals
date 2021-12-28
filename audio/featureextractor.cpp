#include "featureextractor.h"

#include <cmath>

#include "yin.h"

#include <QDebug>


double calc_RMS(const float* data, const size_t len) {
    double squareAmpSum = 0.0;
    for (size_t i = 0; i < len; ++i) {
        squareAmpSum += data[i] * data[i];
    }
    return  std::sqrt(squareAmpSum / static_cast<double>(len));
}


double calc_dB(const float* data, const size_t len) {
    auto rms = calc_RMS(data, len);
    if (rms == 0.0)
        return -120.0;
    auto dbs = 20.0 * log10(rms);
    return dbs;
}


double calc_MidiCents(double f0) {
    double note = 69 + log(f0 / 440.f) * 17.31234; //Some evil magic from lib_extract
    return floor( 0.5f + note * 100 ); //TODO check round(note * 100)?
}

//TODO single function with static object

double calc_YinF0(const float* data, const size_t len, double sampleRate) {
    static Yin yin;
    yin.init(sampleRate, len);
    return yin.getPitch(data);
}
