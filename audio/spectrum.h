/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <qglobal.h>
#include "utils.h"

const int FFTLengthPowerOfTwo = 12;
const int    SpectrumLengthSamples  = PowerOfTwo<FFTLengthPowerOfTwo>::Result;
const int    SpectrumNumBands       = 50;
const qreal  SpectrumLowFreq        = 20.0; // Hz
const qreal  SpectrumHighFreq       = 2000.0; // Hz
const qint64 WaveformWindowDuration = 500 * 1000; //TODO rid of useless
const int   WaveformTileLength      = 4096;
const qreal SpectrumAnalyserMultiplier = 0.15;
const int   NullMessageTimeout      = -1;


enum WindowFunction {
    NoWindow,
    HannWindow
};


const WindowFunction DefaultWindowFunction = HannWindow;


struct Tone {
    Tone(qreal freq = 0.0, qreal amp = 0.0):frequency(freq), amplitude(amp)
    {}
    qreal   frequency; // Start and end frequencies for swept tone generation
    qreal   amplitude; // Amplitude in range [0.0, 1.0]
};


struct SweptTone
{
    SweptTone(qreal start = 0.0, qreal end = 0.0, qreal amp = 0.0)
    :   startFreq(start), endFreq(end), amplitude(amp)
    { Q_ASSERT(end >= start); }

    SweptTone(const Tone &tone)
    :   startFreq(tone.frequency), endFreq(tone.frequency), amplitude(tone.amplitude)
    { }

    // Start and end frequencies for swept tone generation
    qreal   startFreq;
    qreal   endFreq;

    // Amplitude in range [0.0, 1.0]
    qreal   amplitude;
};


#define CHECKED_CONNECT(source, signal, receiver, slot) \
    if (!connect(source, signal, receiver, slot)) \
        qt_assert_x(Q_FUNC_INFO, "CHECKED_CONNECT failed", __FILE__, __LINE__);


#ifdef DISABLE_WAVEFORM
#undef SUPERIMPOSE_PROGRESS_ON_WAVEFORM
#endif

#endif // SPECTRUM_H

