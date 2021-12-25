#include "featureextractor.h"

#include <cmath>


double calc_RMS(const float* data, const size_t N) {
    double squareAmpSum = 0.0;
    for (size_t i = 0; i < N; ++i)
        squareAmpSum += data[i] * data[i];
    return  std::sqrt(squareAmpSum / static_cast<double>(N));
}


double calc_dB(const float* data, const size_t N) {
    return 20 * log10(calc_RMS(data, N));
}


double calc_MidiCents(double f0) {
    double note = 69 + log(f0 / 440.f) * 17.31234; //Some evil magic from lib_extract
    return floor( 0.5f + note * 100 ); //TODO check round(note * 100)?
}
