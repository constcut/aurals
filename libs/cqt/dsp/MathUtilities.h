/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */
/*
    Constant-Q library
    Copyright (c) 2013-2014 Queen Mary, University of London

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Except as contained in this notice, the names of the Centre for
    Digital Music; Queen Mary, University of London; and Chris Cannam
    shall not be used in advertising or otherwise to promote the sale,
    use or other dealings in this Software without prior written
    authorization.
*/

#ifndef MATHUTILITIES_H
#define MATHUTILITIES_H

#include <vector>

#include "nan-inf.h"
#include "pi.h"

/**
 * Static helper functions for simple mathematical calculations.
 */
class MathUtilities  
{
public:	
    /**
     * Round x to the nearest integer.
     */
    static float round( float x );

    /**
     * Return through min and max pointers the highest and lowest
     * values in the given array of the given length.
     */
    static void	  getFrameMinMax( const float* data, unsigned int len,  float* min, float* max );

    /**
     * Return the mean of the given array of the given length.
     */
    static float mean( const float* src, unsigned int len );

    /**
     * Return the mean of the subset of the given vector identified by
     * start and count.
     */
    static float mean( const std::vector<float> &data,
                        unsigned int start, unsigned int count );
    
    /**
     * Return the sum of the values in the given array of the given
     * length.
     */
    static float sum( const float* src, unsigned int len );

    /**
     * Return the median of the values in the given array of the given
     * length. If the array is even in length, the returned value will
     * be half-way between the two values adjacent to median.
     */
    static float median( const float* src, unsigned int len );

    /**
     * The principle argument function. Map the phase angle ang into
     * the range [-pi,pi).
     */
    static float princarg( float ang );

    /**
     * Floating-point division modulus: return x % y.
     */
    static float mod( float x, float y);

    static void	  getAlphaNorm(const float *data, unsigned int len, unsigned int alpha, float* ANorm);
    static float getAlphaNorm(const std::vector <float> &data, unsigned int alpha );

    static void   circShift( float* data, int length, int shift);

    static int	  getMax( float* data, unsigned int length, float* max = 0 );
    static int	  getMax( const std::vector<float> &data, float* max = 0 );
    static int    compareInt(const void * a, const void * b);

    enum NormaliseType {
        NormaliseNone,
        NormaliseUnitSum,
        NormaliseUnitMax
    };

    static void normalise(float *data, int length,
                          NormaliseType n = NormaliseUnitMax);

    static void normalise(std::vector<float> &data,
                          NormaliseType n = NormaliseUnitMax);

    /**
     * Threshold the input/output vector data against a moving-mean
     * average filter.
     */
    static void adaptiveThreshold(std::vector<float> &data);

    /** 
     * Return true if x is 2^n for some integer n >= 0.
     */
    static bool isPowerOfTwo(int x);

    /**
     * Return the next higher integer power of two from x, e.g. 1300
     * -> 2048, 2048 -> 2048.
     */
    static int nextPowerOfTwo(int x);

    /**
     * Return the next lower integer power of two from x, e.g. 1300 ->
     * 1024, 2048 -> 2048.
     */
    static int previousPowerOfTwo(int x);

    /**
     * Return the nearest integer power of two to x, e.g. 1300 -> 1024,
     * 12 -> 16 (not 8; if two are equidistant, the higher is returned).
     */
    static int nearestPowerOfTwo(int x);

    /**
     * Return x!
     */
    static float factorial(int x); // returns float in case it is large

    /**
     * Return the greatest common divisor of natural numbers a and b.
     */
    static int gcd(int a, int b);
};

#endif
