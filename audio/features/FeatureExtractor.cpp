#include "FeatureExtractor.hpp"

#include <cmath>
#include <algorithm>
#include <unordered_map>

#include <QDebug>

#include "Yin.hpp"

using namespace aurals;

double aurals::calc_RMS(const float* data, const size_t len) {
    double squareAmpSum = 0.0;
    for (size_t i = 0; i < len; ++i)
        squareAmpSum += data[i] * data[i];
    return  std::sqrt(squareAmpSum / static_cast<double>(len));
}


double aurals::calc_dB(const float* data, const size_t len) {
    auto rms = calc_RMS(data, len);
    if (rms == 0.0)
        return -120.0;
    auto dbs = 20.0 * log10(rms);
    return dbs;
}


double aurals::calc_MidiCentsCustom(const double f0, const double tuneFreq,
                                      const int midiTune)
{
    double note = 12.0 * log2(f0 / tuneFreq) + midiTune;
    return note * 100;
}


double aurals::calc_MidiCents(const double f0) {
    const double A1Freq = 440.0; //Note A of first octave = 440Hz
    const int A1Midi = 69; //Midi note number for A of first octave = 69
    return calc_MidiCentsCustom(f0, A1Freq, A1Midi);
}


double aurals::calc_YinF0(const float* data,  const size_t len,
                            const double threshold, const double sampleRate)
{
    static YinPP yin2; //TO
    yin2.init(sampleRate, len);
    yin2.setThreshold(threshold);
    auto p = yin2.process(data);
    return p;
}


double calc_Mean(const double* data, const size_t len) {
    double summ = 0.0;
    for (size_t i = 0; i < len; ++i)
        summ += data[i];
    return summ / len;
}


double calc_Median(const double* data, const size_t len) {
    if (len == 0)
        return -0.0;

    std::vector<double> input(data, data + len);
    std::sort(input.begin(), input.end());
    size_t middlePosition = len/2;
    return input[middlePosition];
}


double calc_Mode(const double* data, const size_t len) { //maybe need to add here epsilom? yet unused
    if (len == 0)
        return -0.0;

    std::unordered_map<double, size_t> usedMap;
    for (size_t i = 0; i < len; ++i) {
        if (usedMap.count(data[i]))
            usedMap[data[i]] += 1;
        else
            usedMap[data[i]] = 1;
    }
    std::vector<std::pair<double, size_t>> usedVec(usedMap.begin(), usedMap.end());
    std::sort(usedVec.begin(), usedVec.end(), [](auto& lhs, auto& rhs){ return lhs.second > rhs.second; });
    return usedVec[0].first;
}


std::pair<double, double> calc_Range(const double* data, const size_t len) {
    if (len == 0)
        return {0.0, 0.0};
    const auto [min, max] = std::minmax_element(data, data + len);
    return {*min, *max};
}
