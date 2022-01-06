#include "FeatureExtractor.hpp"

#include <cmath>

#include <QDebug>

#include "Yin.hpp"

using namespace mtherapp;

double mtherapp::calc_RMS(const float* data, const size_t len) {
    double squareAmpSum = 0.0;
    for (size_t i = 0; i < len; ++i)
        squareAmpSum += data[i] * data[i];
    return  std::sqrt(squareAmpSum / static_cast<double>(len));
}


double mtherapp::calc_dB(const float* data, const size_t len) {
    auto rms = calc_RMS(data, len);
    if (rms == 0.0)
        return -120.0;
    auto dbs = 20.0 * log10(rms);
    return dbs;
}


double mtherapp::calc_MidiCentsCustom(const double f0, const double tuneFreq,
                                      const int midiTune)
{
    double note = 12.0 * log2(f0 / tuneFreq) + midiTune;
    return note * 100;
}


double mtherapp::calc_MidiCents(const double f0) {
    const double A1Freq = 440.0; //Note A of first octave = 440Hz
    const int A1Midi = 69; //Midi note number for A of first octave = 69
    return calc_MidiCentsCustom(f0, A1Freq, A1Midi);
}


std::pair<double, double> mtherapp::calc_YinF0(const float* data,  const size_t len,
                                               const double threshold, const double sampleRate) {
    static Yin yin;
    yin.init(sampleRate, len);
    yin.setThreshold(threshold);
    return {yin.getPitch(data), yin.getTau()};
}
