#ifndef WAVEHOLDER_H
#define WAVEHOLDER_H

#include <QObject>

#include "wavecontour.h"

class WaveHolder : public QObject { //TODO template class if we need transmit other structures through QML
     Q_OBJECT

public:
    WaveHolder(): _ptr(nullptr) {}
    WaveHolder(WaveContour& w) : _ptr(&w) {}
    WaveContour* getPtr() { return _ptr; }
    WaveHolder (const WaveHolder& other):_ptr(other._ptr) {}
    WaveHolder& operator=(const WaveHolder& other){ _ptr = other._ptr; return *this; }

private:
    WaveContour* _ptr;
};


#endif // WAVEHOLDER_H
