/*
 This code was delivered from https://github.com/claydergc/find-peaks
 and distributed under MIT lisence
 */
#ifndef FINDPEAKS_H
#define FINDPEAKS_H

#include <vector>

std::vector<size_t> peakIndexesInData(const std::vector<float>& signal, float sensitivity = 2.f);

#endif
