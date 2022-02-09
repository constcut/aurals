#ifndef FEATUREEXTRACTOR_H
#define FEATUREEXTRACTOR_H

#include <string> //Window names in future, now for size_t :)
#include <utility>


namespace aurals {

    double calc_RMS(const float* data, const size_t len);
    double calc_dB(const float* data, const size_t len);

    double calc_MidiCents(const double f0);
    double calc_MidiCentsCustom(const double f0, const double tuneFreq = 440.0,
                                const int midiTune = 69);

    double calc_YinF0(const float* data, const size_t len,
                                         const double threshold=0.15, const double sampleRate=44100.0);

    double calc_Mean(const double* data, const size_t len);
    double calc_Median(const double* data, const size_t len);
    double calc_Mode(const double* data, const size_t len);
    std::pair<double, double> calc_Range(const double* data, const size_t len);

}


#endif // FEATUREEXTRACTOR_H
