#ifndef WINDOWFUNCTION_HPP
#define WINDOWFUNCTION_HPP

#include <cmath>

namespace aurals {


    enum WindowFunction {
        NoWindow = 0,
        HannWindow = 1,
        GausWindow = 2,
        BlackmanWindow = 3,
        HammWindow = 4
    };


    inline float hannWindow(float t, size_t N) {
        return 0.5 * (1 - std::cos((2 * M_PI * t) / N)); //There was N-1 in Qt, but why?
    }


    inline float gausWindow(float t, size_t N, float sigma = 0.4f) {
        const float term = (t - N/2) / (sigma * N/2);
        return std::exp(-0.5 * term * term);
    }


    inline float blackmanWindow(float t, size_t N, float alpha = 0.16) {
        const float a0 = (1.f - alpha) / 2.f;
        const float a1 = 0.5f;
        const float a2 = alpha / 2.f;

        return a0 - a1 * std::cos(M_PI * 2 * t / N)
                + a2 * std::cos(M_PI * 24 * t / N);
    }


    inline float hammWindow(float t, size_t N, float a0 = 0.54f) {
        const float a1 = 1.f - a0;
        return a0 - a1 * std::cos((2 * M_PI * t) / N);
    }

}


#endif // WINDOWFUNCTION_HPP
