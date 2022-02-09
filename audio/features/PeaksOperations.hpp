/*
 This code was delivered from https://github.com/claydergc/find-peaks
 and distributed under MIT lisence
 */
#ifndef PEAKSOPERATIONS_H
#define PEAKSOPERATIONS_H

#include <vector>
#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace aurals {


    template <class T>
    std::vector<size_t> peakIndexes(const std::vector<T>& signal,
                                    const T sensitivity = 2.0);



    inline double findPeakCommonDistance(const std::vector<size_t>& peaks,
                                  int mergeDistance = 1)
    {
        if (peaks.empty())
            return 0.0;

        if (peaks.size() == 1)
            return peaks[0];

        std::unordered_map<int, int> diffCount;
        int prev = -1;
        for (auto p: peaks) {
            if (prev != -1) {
                int diff = p - prev;
                if (diffCount.count(diff))
                    diffCount[diff] += 1;
                else
                    diffCount[diff] = 1;
            }
            prev = p;
        }

        std::vector<std::pair<int,int>>  sorted(diffCount.begin(), diffCount.end());
        std::sort(sorted.begin(), sorted.end(), [](auto& lhs, auto& rhs) { return lhs.second > rhs.second; });

        double foundDistance = 0;

        if (sorted.empty() == false) {
            int mainBin = sorted[0].first;
            foundDistance = mainBin;

            int subBin = -1;
            int subCount = 0;
            for (size_t i = 1; i < sorted.size(); ++i)
                if (std::abs(sorted[i].first - mainBin) == mergeDistance) {
                    subBin = sorted[i].first;
                    subCount = sorted[i].second;
                    break;
                }
            if (subBin != -1) {
                double countCoef = static_cast<double>(sorted[0].second) / subCount;
                double midBin = (static_cast<double>(mainBin) + subBin ) / 2.0;
                double addition = 0.5 - 0.5 / countCoef;
                midBin += addition;
                foundDistance = midBin;
            }
        }

        return foundDistance;
    }


    template <class T>
    std::vector<T> differenceInRange(typename std::vector<T>::const_iterator begin,
                                     typename std::vector<T>::const_iterator end)
    {
        auto len = std::distance(begin, end);
        std::vector<T> diff;

        if (len == 0)
            return diff;

        diff.reserve(len - 1);
        for (auto i = begin; i < end - 1; ++i)
            diff.emplace_back(*(i + 1) - *i);
        return diff;
    }


    template <class T>
    std::vector<T> vectorProduct(typename std::vector<T>::iterator a,
                                 typename std::vector<T>::iterator b, const size_t len)
    {
        std::vector<T> product;
        product.reserve(len);
        for (size_t i = 0; i < len; ++i, ++a, ++b)
            product.emplace_back((*a) * (*b));
        return product;
    }


    template <class T>
    std::vector<size_t> indexesOfDataLessThan(const std::vector<T>& data,
                                               const float threshold)
    {
        std::vector<size_t> ids;
        for (size_t i = 0; i < data.size(); ++i)
            if (data[i] < threshold)
                ids.emplace_back(i + 1);
        return ids;
    }


    template <typename T>
    std::vector<T> selectElementsByIdx(const std::vector<T>& data,
                                  const std::vector<size_t>& ids)
    {
        std::vector<T> selection;
        selection.reserve(ids.size());
        for (auto& i : ids)
            selection.emplace_back(data[i]);
        return selection;
    }


    template <typename T>
    std::vector<int> signVectorElements(const std::vector<T>& data)
    {
        std::vector<int> signs;
        signs.reserve(data.size());
        for (auto& d : data)
            if (d > 0.f)
                signs.emplace_back(1);
            else if (d < 0.f)
                signs.emplace_back(-1);
            else
                signs.emplace_back(0);

        return signs;
    }


    template <class T>
    std::vector<size_t> peakIndexes(const std::vector<T>& signal,
                                    const T sensitivity)
    {
        std::vector<size_t> peakIdx;
        if (signal.size() < 2)
            return peakIdx;

        size_t minIdx = distance(signal.begin(), min_element(signal.begin(), signal.end()));
        size_t maxIdx = distance(signal.begin(), max_element(signal.begin(), signal.end()));

        const T sel = (signal[maxIdx]-signal[minIdx]) / sensitivity;

        auto beginIt = signal.begin();
        auto endIt = signal.end();

        std::vector<double> dx = differenceInRange<T>(beginIt, endIt);

        const T EPS(2.2204e-16);
        replace(dx.begin(), dx.end(), 0.0, -EPS);

        auto dx2 = vectorProduct<T>(dx.begin(), dx.begin() + 1, dx.size() - 1);
        auto indexes = indexesOfDataLessThan<T>(dx2, 0.f); // Find where the derivative changes sign
        auto x = selectElementsByIdx<T>(signal, indexes);

        if (x.empty())
            return {};

        x.insert(x.begin(), signal.front());
        x.insert(x.end(),   signal.back());

        indexes.insert(indexes.begin(), 0);
        indexes.insert(indexes.end(), signal.size());

        const size_t minMagIdx = distance(x.begin(), min_element(x.begin(), x.end()));
        T minMag = x[minMagIdx];
        T leftMin = minMag;

        int len = static_cast<int>(x.size());
        T tempMag = minMag;
        bool foundPeak = false;

        std::vector<T> xDiff = differenceInRange<T>(x.begin(), x.begin() + 3); // tener cuidado subvector
        std::vector<int> signDx = signVectorElements<T>(xDiff);

        if (signDx[0] == signDx[1]){
            if (signDx[0] <= 0){
                x.erase(x.begin() + 1);
                indexes.erase(indexes.begin() + 1);
            }
            else {
                x.erase(x.begin());
                indexes.erase(indexes.begin());
            }
            --len;
        }

        int j = 1;
        if (x[0] >= x[1])
            j = 0;

        float maxPeaks = ceil(len / 2.0);
        std::vector<size_t> peakLoc(maxPeaks, 0);
        std::vector<T> peakMag(maxPeaks, 0.0);
        int cInd = 1;
        int tempLoc = 0;

        while (j < len){
            ++j;

            if (foundPeak){
                tempMag = minMag;
                foundPeak = false;
            }

            int prev = j - 1;
            if (x[prev] > tempMag && x[prev] > leftMin + sel){
                tempLoc = prev;
                tempMag = x[prev];
            }

            if (j >= len)
                break;
            ++j;

            if (!foundPeak && tempMag > sel + x[j - 1]){
                foundPeak = true;
                leftMin = x[j - 1];
                peakLoc[cInd - 1] = tempLoc;
                peakMag[cInd - 1] = tempMag;
                ++cInd;
            }
            else if (x[j - 1] < leftMin)
                leftMin = x[j - 1];
        }


        if (x.back() > tempMag && x.back() > leftMin + sel){
            peakLoc[cInd - 1] = len - 1;
            peakMag[cInd - 1] = x.back();
            ++cInd;
        }

        else if (!foundPeak && tempMag > minMag){
            peakLoc[cInd - 1] = tempLoc;
            peakMag[cInd - 1] = tempMag;
            ++cInd;
        }

        if (cInd > 0){
            peakLoc.erase(peakLoc.begin() + cInd - 1, peakLoc.end());
            if (indexes[peakLoc.back()] >= signal.size())
                peakLoc.pop_back();
            peakIdx = selectElementsByIdx(indexes, peakLoc);
        }

        return peakIdx;
    }

}

#endif
