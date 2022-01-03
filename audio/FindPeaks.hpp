/*
 This code was delivered from https://github.com/claydergc/find-peaks
 and distributed under MIT lisence
 */
#ifndef FINDPEAKS_H
#define FINDPEAKS_H

#include <vector>

//TODO move into feature extractor
std::vector<size_t> peakIndexesInData(const std::vector<double>& signal, float sensitivity = 2.f);

#endif
