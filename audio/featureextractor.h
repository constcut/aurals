#ifndef FEATUREEXTRACTOR_H
#define FEATUREEXTRACTOR_H

#include <string> //Window names in future, now for size_t :)

double calc_RMS(const float* data, const size_t N);
double calc_dB(const float* data, const size_t N);

double calc_MidiCents(double f0);

//TODO libextract features slop, sharp etc to find ADSR?

#endif // FEATUREEXTRACTOR_H
