/*
 This code was delivered from https://github.com/claydergc/find-peaks
 and distributed under MIT lisence
 */
#ifndef FINDPEAKS_H
#define FINDPEAKS_H

#include <vector>

namespace mtherapp {

    std::vector<size_t> peakIndexesInData(const std::vector<double>& signal, const float sensitivity = 2.f);

}

#endif
