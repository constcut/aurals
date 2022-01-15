/*
 This code was delivered from https://github.com/claydergc/find-peaks
 and distributed under MIT lisence
 */
#ifndef FINDPEAKS_H
#define FINDPEAKS_H

#include <vector>
#include <algorithm>
#include <cmath>

namespace aural_sight {

    std::vector<size_t> peakIndexesInData(const std::vector<double>& signal, const float sensitivity = 2.f);


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
    std::vector<T> vectorProduct_(typename std::vector<T>::iterator a,
                                 typename std::vector<T>::iterator b, const size_t len)
    {
        std::vector<T> product;
        product.reserve(len);
        for (size_t i = 0; i < len; ++i, ++a, ++b)
            product.emplace_back((*a) * (*b));
        return product;
    }


    template <class T>
    std::vector<size_t> indexesOfDataLessThan_(const std::vector<T>& data,
                                               const float threshold)
    {
        std::vector<size_t> ids;
        for (size_t i = 0; i < data.size(); ++i)
            if (data[i] < threshold)
                ids.emplace_back(i + 1);
        return ids;
    }


    template <typename T>
    std::vector<T> selectElements_(const std::vector<T>& data,
                                  const std::vector<size_t>& ids)
    {
        std::vector<T> selection;
        selection.reserve(ids.size());
        for (auto& i : ids)
            selection.emplace_back(data[i]);
        return selection;
    }


    template <typename T>
    std::vector<int> signVector_(const std::vector<T>& data)
    {
        std::vector<int> signs; //TODO replace on bool?
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
                                    const T sensitivity = 2.f)
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

        auto dx2 = vectorProduct_<T>(dx.begin(), dx.begin() + 1, dx.size() - 1);
        auto indexes = indexesOfDataLessThan_<T>(dx2, 0.f); // Find where the derivative changes sign
        auto x = selectElements_<T>(signal, indexes);

        if (x.empty())
            return {};

        x.insert(x.begin(), signal.front());
        x.insert(x.end(),   signal.back());

        indexes.insert(indexes.begin(), 0);
        indexes.insert(indexes.end(), signal.size());

        const size_t minMagIdx = distance(x.begin(), min_element(x.begin(), x.end()));
        T minMag = x[minMagIdx];
        T leftMin = minMag;

        int len = static_cast<int>(x.size()); //TODO check why size_t fails?
        T tempMag = minMag;
        bool foundPeak = false;

        // Deal with first point a little differently since tacked it on
        // Calculate the sign of the derivative since we tacked the first
        // point on it does not neccessarily alternate like the rest.
        std::vector<T> xDiff = differenceInRange<T>(x.begin(), x.begin() + 3); // tener cuidado subvector
        std::vector<int> signDx = signVector_<T>(xDiff);

        if (signDx[0] == signDx[1]){ // Want alternating signs
            if (signDx[0] <= 0){  // The first point is larger or equal to the second
                x.erase(x.begin() + 1);
                indexes.erase(indexes.begin() + 1);
            }
            else { // First point is smaller than the second
                x.erase(x.begin());
                indexes.erase(indexes.begin());
            }
            --len;
        }

        int ii = 1;
        if (x[0] >= x[1])
            ii = 0;

        float maxPeaks = ceil(len / 2.0);
        std::vector<size_t> peakLoc(maxPeaks, 0);
        std::vector<T> peakMag(maxPeaks, 0.0);
        int cInd = 1;
        int tempLoc = 0;

        while (ii < len){
            ++ii; // This is a peak

            // Reset peak finding if we had a peak and the next peak is bigger
            // than the last or the left min was small enough to reset.
            if (foundPeak){
                tempMag = minMag;
                foundPeak = false;
            }

            // Found new peak that was lager than temp mag and selectivity larger
            // than the minimum to its left.
            int prev = ii - 1;
            if (x[prev] > tempMag && x[prev] > leftMin + sel){
                tempLoc = prev;
                tempMag = x[prev];
            }

            // Make sure we don't iterate past the length of our vector
            if (ii >= len)
                break; // We assign the last point differently out of the loop

            ++ii; // Move onto the valley

            // Come down at least sel from peak
            if (!foundPeak && tempMag > sel + x[ii - 1]){
                foundPeak = true;
                leftMin = x[ii - 1];
                peakLoc[cInd - 1] = tempLoc; // Add peak to index
                peakMag[cInd - 1] = tempMag;
                ++cInd;
            }
            else if (x[ii - 1] < leftMin) // New left minima
                leftMin = x[ii - 1];
        }

        // Check end point
        if (x.back() > tempMag && x.back() > leftMin + sel){
            peakLoc[cInd - 1] = len - 1;
            peakMag[cInd - 1] = x.back();
            ++cInd;
        }
        else if (!foundPeak && tempMag > minMag){ // Check if we still need to add the last point
            peakLoc[cInd - 1] = tempLoc;
            peakMag[cInd - 1] = tempMag;
            ++cInd;
        }

        if (cInd > 0){
            peakLoc.erase(peakLoc.begin() + cInd - 1, peakLoc.end());
            if (indexes[peakLoc.back()] >= signal.size())
                peakLoc.pop_back(); // иногда последний индекс == длине
            peakIdx = selectElements_(indexes, peakLoc);
        }

        return peakIdx;
    }

}

#endif
