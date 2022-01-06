#ifndef FEATUREEXTRACTOR_H
#define FEATUREEXTRACTOR_H

#include <string> //Window names in future, now for size_t :)
#include <utility>


namespace mtherapp {

    double calc_RMS(const float* data, const size_t len);
    double calc_dB(const float* data, const size_t len);

    double calc_MidiCents(double f0);

    //Returns pair F0, tau
    std::pair<double, double> calc_YinF0(const float* data, const size_t len, double threshold=0.15, double sampleRate=44100.0);

}

//TODO mean, median, envelop

#endif // FEATUREEXTRACTOR_H
