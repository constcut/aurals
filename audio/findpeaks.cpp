// This code was derived from https://github.com/claydergc/find-peaks
// and distributed under MIT lisence


#include <iostream>
#include <algorithm>
#include <cmath>
#include "findpeaks.hpp"

#include <list>
#include <algorithm>



using std::vector;
using std::distance;

const double EPS(2.2204e-16);

typedef vector<float>::const_iterator cit;
typedef vector<float>::iterator it;


vector<float> diffsInRange(cit begin, cit end){
    
    auto len = std::distance(begin, end);
    //assert(begin < end);
    vector<float> diff;
    
    if (len == 0) {
        //LOG << "Met 0 len in diffsInRange";
        return diff;
    }
    
    diff.reserve(len - 1);
    for (auto i = begin; i < end - 1; ++i)
        diff.emplace_back(*(i + 1) - *i);
    return diff;
}


vector<float> vectorProduct(it a, it b, size_t len){
    vector<float> product;
    product.reserve(len);
    for (size_t i = 0; i < len; ++i, ++a, ++b)
        product.emplace_back((*a) * (*b));
    return product;
}


vector<size_t> indexesOfDataLessThan(const vector<float>& data, float threshold){
    vector<size_t> ids;
    for (size_t i = 0; i < data.size(); ++i)
        if (data[i] < threshold)
            ids.emplace_back(i + 1);
    return ids;
}


template <typename D>
vector<D> selectElements(const vector<D>& data, const vector<size_t>& ids){
    vector<D> selection;
    selection.reserve(ids.size());
    for (auto& i : ids)
        selection.emplace_back(data[i]);
    return selection;
}


vector<int> signVector(const vector<float>& data){
    vector<int> signs;
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


vector<size_t> peakIndexesInData(const vector<float>& signal, float sensitivity){
    
    vector<size_t> peakIndexes;
    if (signal.size() < 2) {
        //LOG << "Attention! signal in peakIndexesInData is too small " << signal.size();
        return peakIndexes;
    }
    
    size_t minIdx = distance(signal.begin(), min_element(signal.begin(), signal.end()));
    size_t maxIdx = distance(signal.begin(), max_element(signal.begin(), signal.end()));
    
    const float sel = (signal[maxIdx]-signal[minIdx]) / sensitivity;
    
    cit beginIt = signal.begin();
    cit endIt = signal.end();
    
    vector<float> dx = diffsInRange(beginIt, endIt);
    replace(dx.begin(), dx.end(), 0.0, -EPS);
    
    auto dx2 = vectorProduct(dx.begin(), dx.begin() + 1, dx.size() - 1);
    auto indexes = indexesOfDataLessThan(dx2, 0.f); // Find where the derivative changes sign
    auto x = selectElements(signal, indexes);
    
    if (x.empty())
        return {};
    
    x.insert(x.begin(), signal.front());
    x.insert(x.end(),   signal.back());
    
    indexes.insert(indexes.begin(), 0);
    indexes.insert(indexes.end(), signal.size());
    
    const size_t minMagIdx = distance(x.begin(), min_element(x.begin(), x.end()));
    float minMag = x[minMagIdx];
    float leftMin = minMag;
    
    int len = (int) x.size();
    float tempMag = minMag;
    bool foundPeak = false;
    
    // Deal with first point a little differently since tacked it on
    // Calculate the sign of the derivative since we tacked the first
    // point on it does not neccessarily alternate like the rest.
    vector<float> xDiff = diffsInRange(x.begin(), x.begin() + 3); // tener cuidado subvector
    vector<int> signDx = signVector(xDiff);
    
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
    
    float maxPeaks = ceil((float) len / 2.f);
    vector<size_t> peakLoc(maxPeaks, 0);
    vector<float> peakMag(maxPeaks, 0.f);
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
        peakIndexes = selectElements(indexes, peakLoc);
    }
    
    //for (auto p : peakIndexes)
        //assert(p < signal.size());
    
    return peakIndexes;
}
    
    
