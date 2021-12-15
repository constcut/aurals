#include "audio/fft.h"

#include <qmath.h>
#include <cmath>
#include <algorithm>

#include	<cassert>

//using namespace gselib;
//static members init

FreqTable FFT::freqTab = FreqTable();
FreqTable Peak::freqTab = FreqTable();


///frequency table
FreqTable::FreqTable()
{
    double bigOctave[12] = { 65.41, 69.3, 73.91, 77.78,
                         82.41, 87.31, 92.5, 98.0,
                         103.8, 110.0, 116.54, 123.48};

    for (size_t i = 0 ; i < 12; ++i)
        bigOctave[i] /= 2.0;

    //at first generating scaled values
    for (double octave =1.0; octave <= 32.0; octave*=2)
        for (int j = 0; j < 12; ++j)
            scaled.push_back(bigOctave[j]*octave);


    //then from each generating unscaled values
    for (size_t i = 0; i < scaled.size(); ++i)
        for (size_t j = 2; j < 12; ++j) //12 harmonics
        {
            double newValue = scaled[i]*j;
            for (unsigned int k = 0; k < scaled.size(); ++k)
                if (scaled[k] == newValue) goto extraSkip;
            //check is there such in scaled if not then
            unscaled.push_back(newValue);
            extraSkip:
            newValue = 0.0;
        }

    std::sort(unscaled.begin(),unscaled.end());


}

double FreqTable::getNearestUnscaled
(double freq)
{
     for (size_t i = 1; i < unscaled.size()-1; ++i)
     {
         if ((fabs(unscaled[i]-freq) < fabs(unscaled[i-1]-freq))
             &&
             (fabs(unscaled[i]-freq) < fabs(unscaled[i+1]-freq)))
         {
                 return unscaled[i];
         }
     }
     return -0.0;
}

double FreqTable::getNearestScaled(double freq)
{
     for (size_t i = 1; i < scaled.size()-1; ++i)
     {
         if ((fabs(scaled[i]-freq) < fabs(scaled[i-1]-freq))
             &&
             (fabs(scaled[i]-freq) < fabs(scaled[i+1]-freq)))
         {
                 return scaled[i];
         }
     }
     return -0.0;
}

int FreqTable::getScaledIndex(double freq)
{
    for (size_t i = 1; i < scaled.size()-1; ++i)
    {
        if ((fabs(scaled[i]-freq) < fabs(scaled[i-1]-freq))
            &&
            (fabs(scaled[i]-freq) < fabs(scaled[i+1]-freq)))
        {
               return i;
        }
    }
    return -1;
}

double FreqTable::getNearest(double freq)
{

   double f1 = getNearestScaled(freq);
   double f2 = getNearestUnscaled(freq);

   if (f1 == -0.0) return freq;

   if (fabs(f1-freq) < fabs(f2-freq))
       return f1;
   return f2;
}

double FreqTable::getUpperScaled(double freq)
{
    for (size_t i = 1; i < scaled.size()-1; ++i)
    {
        if ((fabs(scaled[i]-freq) <= fabs(scaled[i-1]-freq))
            &&
            (fabs(scaled[i]-freq) <= fabs(scaled[i+1]-freq)))
        {
            return scaled[i+1];
        }
    }
    return -0.0;
}

double FreqTable::getLowerScaled(double freq)
{
    for (size_t i = 1; i < scaled.size()-1; ++i)
    {
        if ((fabs(scaled[i]-freq) <= fabs(scaled[i-1]-freq))
            &&
            (fabs(scaled[i]-freq) <= fabs(scaled[i+1]-freq)))
        {
            return scaled[i-1];
        }
    }
    return -0.0;
}

short FreqTable::getNoteByFreq(double freq)
{
    //forming bit 0-7(do,re,mi,fa,sol,la,si) + bimol flag
    //8-10 octave num (0-7 values)
    //12-15(32 steps of welldone)

    int freqPosition = -1;
    for (size_t i = 1; i < scaled.size()-1; ++i)
        if ((fabs(scaled[i]-freq) < fabs(scaled[i-1]-freq))
            &&
            (fabs(scaled[i]-freq) < fabs(scaled[i+1]-freq)))
        {
                freqPosition = i;
                break;
        }
    int noteValue = freqPosition % 12;
    //0 -do, 2 - re, 4 - mi, 5 - fa
    //7 -sol, 9 - las, 11- si
    //+1 - means bimol

    unsigned short value = 0;

    if (noteValue == 0) value  |= 0x1;
    if (noteValue == 1) value  |= 0x1 | 0X80;
    if (noteValue == 2) value  |= 0x2;
    if (noteValue == 3) value  |= 0x2 | 0X80;
    if (noteValue == 4) value  |= 0x4;
    if (noteValue == 5) value  |= 0x8;
    if (noteValue == 6) value  |= 0x8 | 0X80;
    if (noteValue == 7) value  |= 0x10;
    if (noteValue == 8) value  |= 0x10 | 0X80;
    if (noteValue == 9) value  |= 0x20;
    if (noteValue == 10) value |= 0x20 | 0X80;
    if (noteValue == 11) value |= 0x40;

    return noteValue;
    //unsigned int oct = (freqPosition/12)+1;

    //value |= oct<<8;

    //also good idea to find a collusion

    return value;
}



/*
  ///OLD fft_c stuff
  ///
  ///                     */

#define PI (2.0 * asin(1.0))


void fft_double_old(unsigned int p_nSamples, bool p_bInverseTransform, double *p_lpRealIn, double *p_lpImagIn, double *p_lpRealOut, double *p_lpImagOut);
bool IsPowerOfTwo_old(unsigned int p_nX);
unsigned int NumberOfBitsNeeded_old(unsigned int p_nSamples);
unsigned int ReverseBits_old(unsigned int p_nIndex, unsigned int p_nBits);
double Index_to_frequency_old(unsigned int p_nBaseFreq, unsigned int p_nSamples, unsigned int p_nIndex);

inline double GetFrequencyIntensity(double re, double im)
{
        return sqrt((re*re)+(im*im));
}
#define mag_sqrd(re,im) (re*re+im*im)
#define Decibels(re,im) ((re == 0 && im == 0) ? (0) : 10.0 * log10(double(mag_sqrd(re,im))))
#define Amplitude(re,im,len) (GetFrequencyIntensity(re,im)/(len))
#define AmplitudeScaled(re,im,len,scale) ((int)Amplitude(re,im,len)%scale)


void fft_double_old (unsigned int p_nSamples, bool p_bInverseTransform, double *p_lpRealIn, double *p_lpImagIn, double *p_lpRealOut, double *p_lpImagOut)
{

        if(!p_lpRealIn || !p_lpRealOut || !p_lpImagOut) return;


        unsigned int NumBits;
        unsigned int i, j, k, n;
        unsigned int BlockSize, BlockEnd;

        double angle_numerator = 2.0 * PI;
        double tr, ti;

        if( !IsPowerOfTwo_old(p_nSamples) )
        {
                return;
        }

        if( p_bInverseTransform ) angle_numerator = -angle_numerator;

        NumBits = NumberOfBitsNeeded_old ( p_nSamples );


        for( i=0; i < p_nSamples; i++ )
        {
                j = ReverseBits_old ( i, NumBits );
                p_lpRealOut[j] = p_lpRealIn[i];
                p_lpImagOut[j] = (p_lpImagIn == 0) ? 0.0 : p_lpImagIn[i];
        }


        BlockEnd = 1;
        for( BlockSize = 2; BlockSize <= p_nSamples; BlockSize <<= 1 )
        {
                double delta_angle = angle_numerator / (double)BlockSize;
                double sm2 = sin ( -2 * delta_angle );
                double sm1 = sin ( -delta_angle );
                double cm2 = cos ( -2 * delta_angle );
                double cm1 = cos ( -delta_angle );
                double w = 2 * cm1;
                double ar[3], ai[3];

                for( i=0; i < p_nSamples; i += BlockSize )
                {

                        ar[2] = cm2;
                        ar[1] = cm1;

                        ai[2] = sm2;
                        ai[1] = sm1;

                        for ( j=i, n=0; n < BlockEnd; j++, n++ )
                        {

                                ar[0] = w*ar[1] - ar[2];
                                ar[2] = ar[1];
                                ar[1] = ar[0];

                                ai[0] = w*ai[1] - ai[2];
                                ai[2] = ai[1];
                                ai[1] = ai[0];

                                k = j + BlockEnd;
                                tr = ar[0]*p_lpRealOut[k] - ai[0]*p_lpImagOut[k];
                                ti = ar[0]*p_lpImagOut[k] + ai[0]*p_lpRealOut[k];

                                p_lpRealOut[k] = p_lpRealOut[j] - tr;
                                p_lpImagOut[k] = p_lpImagOut[j] - ti;

                                p_lpRealOut[j] += tr;
                                p_lpImagOut[j] += ti;

                        }
                }

                BlockEnd = BlockSize;

        }


        if( p_bInverseTransform )
        {
                double denom = (double)p_nSamples;

                for ( i=0; i < p_nSamples; i++ )
                {
                        p_lpRealOut[i] /= denom;
                        p_lpImagOut[i] /= denom;
                }
        }

}

bool IsPowerOfTwo_old( unsigned int p_nX )
{

        if( p_nX < 2 ) return false;

        if( p_nX & (p_nX-1) ) return false;

    return true;

}

unsigned int NumberOfBitsNeeded_old( unsigned int p_nSamples )
{

        int i;

        if( p_nSamples < 2 )
        {
                return 0;
        }

        for ( i=0; ; i++ )
        {
                if( p_nSamples & (1 << i) ) return i;
    }

}

unsigned int ReverseBits_old(unsigned int p_nIndex, unsigned int p_nBits)
{

        unsigned int i, rev;

        for(i=rev=0; i < p_nBits; i++)
        {
                rev = (rev << 1) | (p_nIndex & 1);
                p_nIndex >>= 1;
        }

        return rev;

}

double Index_to_frequency_old(unsigned int p_nBaseFreq, unsigned int p_nSamples, unsigned int p_nIndex)
{

        if(p_nIndex >= p_nSamples)
        {
                return 0.0;
        }
        else if(p_nIndex <= p_nSamples/2)
        {
                return ( (double)p_nIndex / (double)p_nSamples * p_nBaseFreq );
        }
        else
        {
                return ( -(double)(p_nSamples-p_nIndex) / (double)p_nSamples * p_nBaseFreq );
        }
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////


FFT::FFT(int size) : N(size)
{
    destImg = std::vector<double>(N, 0.0);
    destReal = std::vector<double>(N, 0.0);
    amplitude = std::vector<double>(N/2, 0.0);
    decibels = std::vector<double>(N/2, 0.0);
    sqrd = std::vector<double>(N/2, 0.0);
    amplitudeScale = std::vector<double>(N/2, 0.0);
    halfFoubricatedSource = std::vector<double>(N, 0.0);

}


FFT::~FFT()
{

}

bool FFT::transform(short int *place)
{
    if (place!=0) {
        for (int i=0; i < N; ++i)
          halfFoubricatedSource[i]=(double)(place[i]);
        fft_double_old(N,false,halfFoubricatedSource.data(),0,destReal.data(),destImg.data());
    }
    else
        return false;
    return true;
}

bool FFT::countDecibels()
{
    for (size_t i = 0; i < N/2; ++i)
        decibels[i] = Decibels(this->destReal[i],this->destImg[i])/256;
    return true;
}

bool FFT::countAmplitude()
{
    for (size_t i = 0; i < N/2; ++i)
        this->amplitude[i] = Amplitude(this->destReal[i],this->destImg[i],N)/256;
    return true;
}

FFT::AmplitudeValue FFT::getMaxAmplitude()
{
    AmplitudeValue am;
    //am.value = maxAmplitude;
    //am.freq = maxAmplitudeIndex*(44100.0/N);
    return am;
}

bool FFT::countAmplitudeScale()
{
    for (size_t i = 0; i < N/2; ++i)
        amplitudeScale[i] = AmplitudeScaled(this->destReal[i],this->destImg[i],N,256);
    return true; //scale?
}

bool FFT::countSQRD()
{

    for (size_t i = 0; i < N/2; ++i)
        sqrd[i] = mag_sqrd(this->destReal[i],this->destImg[i]);
    return true;
}

double *FFT::getDecibels()
{
     return decibels.data();
}

double *FFT::getAmplitude()
{
     return amplitude.data();
}

double *FFT::getAmplitudeScale()
{
     return amplitudeScale.data();
}

double *FFT::getSQRD()
{
     return sqrd.data();
}

double *FFT::getRealPart()
{
     return destReal.data();
}

double *FFT::getImgPart()
{
    return destImg.data();
}

int FFT::findPeaks(double coefRate)
{
    peaks.clear();

    unsigned int skippedCount = 0;
    double max = 0;
    double maxPos = -1;
    double *source = getAmplitude();

    source = getAmplitude();

    iteration:

    max = 0;
    maxPos = -1;


    for (int i=1;i < 3*(this->N-1)/8; ++i)
    {
        bool needSkip = false;
        if (source[i]>max)
        {
            if (peaks.empty() == false)
                for (size_t j = 0; j < peaks.size(); ++j)
                   if (peaks[j].getPosition() == i)  {
                       needSkip = true;
                       break;
                   }
            if (needSkip == false) {
                max = source[i];
                maxPos = i;
            }
        }
        if (needSkip == true)
            ++skippedCount;
    }

    {
        Peak p(maxPos,max,coefRate);

        //if ((maxPos*7.8125) > 20000.0) return peaks.size(); //check this condition 5.383

        peaks.push_back(p);
        //return peaks.size();

        //or 14 instead 21? - 10 is ok!
        if (peaks.size() < 14)  goto iteration; //check check
        if (peaks.size() >= 14)  return peaks.size(); //for each 2 waiting notes (28 for 4?)
        //if (max > 7*(*peaks)[0].getAmplitude()/100)
            //goto iteration;
    }


    return peaks.size();

}

std::vector<Peak>& FFT::getPeaks()
{
    return peaks;
}
/*****************************************************************************
*                                                                            *
*       DIGITAL SIGNAL PROCESSING TOOLS                                      *
*       Version 1.03, 2001/06/15                                             *
*       (c) 1999 - Laurent de Soras                                          *
*                                                                            *
*       FFTReal.cpp                                                          *
*       Fourier transformation of real number arrays.                        *
*       Portable ISO C++                                                     *
*                                                                            *
* Tab = 3                                                                    *
*****************************************************************************/



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PUBLIC MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*==========================================================================*/
/*      Name: Constructor                                                   */
/*      Input parameters:                                                   */
/*        - length: length of the array on which we want to do a FFT.       */
/*                  Range: power of 2 only, > 0.                            */
/*      Throws: std::bad_alloc, anything                                    */
/*==========================================================================*/

FFTReal::FFTReal (const long length)
:	_length (length)
,	_nbr_bits (int (floor (log (length) / log (2) + 0.5)))
,	_bit_rev_lut (int (floor (log (length) / log (2) + 0.5)))
,	_trigo_lut (int (floor (log (length) / log (2) + 0.5)))
,	_sqrt2_2 (flt_t (sqrt (2) * 0.5))
{
    assert ((1L << _nbr_bits) == length);

    _buffer_ptr = 0;
    if (_nbr_bits > 2)
    {
        _buffer_ptr = new flt_t [_length];
    }
}



/*==========================================================================*/
/*      Name: Destructor                                                    */
/*==========================================================================*/

FFTReal::~FFTReal (void)
{
    delete [] _buffer_ptr;
    _buffer_ptr = 0;
}



/*==========================================================================*/
/*      Name: do_fft                                                        */
/*      Description: Compute the FFT of the array.                          */
/*      Input parameters:                                                   */
/*        - x: pointer on the source array (time).                          */
/*      Output parameters:                                                  */
/*        - f: pointer on the destination array (frequencies).              */
/*             f [0...length(x)/2] = real values,                           */
/*             f [length(x)/2+1...length(x)-1] = imaginary values of        */
/*               coefficents 1...length(x)/2-1.                             */
/*      Throws: Nothing                                                     */
/*==========================================================================*/

void	FFTReal::do_fft (flt_t f [], const flt_t x []) const
{

/*______________________________________________
 *
 * General case
 *______________________________________________
 */

    if (_nbr_bits > 2)
    {
        flt_t *			sf;
        flt_t *			df;

        if (_nbr_bits & 1)
        {
            df = _buffer_ptr;
            sf = f;
        }
        else
        {
            df = f;
            sf = _buffer_ptr;
        }

        /* Do the transformation in several pass */
        {
            int		pass;
            long		nbr_coef;
            long		h_nbr_coef;
            long		d_nbr_coef;
            long		coef_index;

            /* First and second pass at once */
            {
                const long * const	bit_rev_lut_ptr = _bit_rev_lut.get_ptr ();
                coef_index = 0;
                do
                {
                    const long		rev_index_0 = bit_rev_lut_ptr [coef_index];
                    const long		rev_index_1 = bit_rev_lut_ptr [coef_index + 1];
                    const long		rev_index_2 = bit_rev_lut_ptr [coef_index + 2];
                    const long		rev_index_3 = bit_rev_lut_ptr [coef_index + 3];

                    flt_t	* const	df2 = df + coef_index;
                    df2 [1] = x [rev_index_0] - x [rev_index_1];
                    df2 [3] = x [rev_index_2] - x [rev_index_3];

                    const flt_t		sf_0 = x [rev_index_0] + x [rev_index_1];
                    const flt_t		sf_2 = x [rev_index_2] + x [rev_index_3];

                    df2 [0] = sf_0 + sf_2;
                    df2 [2] = sf_0 - sf_2;

                    coef_index += 4;
                }
                while (coef_index < _length);
            }

            /* Third pass */
            {
                coef_index = 0;
                const flt_t		sqrt2_2 = _sqrt2_2;
                do
                {
                    flt_t				v;

                    sf [coef_index] = df [coef_index] + df [coef_index + 4];
                    sf [coef_index + 4] = df [coef_index] - df [coef_index + 4];
                    sf [coef_index + 2] = df [coef_index + 2];
                    sf [coef_index + 6] = df [coef_index + 6];

                    v = (df [coef_index + 5] - df [coef_index + 7]) * sqrt2_2;
                    sf [coef_index + 1] = df [coef_index + 1] + v;
                    sf [coef_index + 3] = df [coef_index + 1] - v;

                    v = (df [coef_index + 5] + df [coef_index + 7]) * sqrt2_2;
                    sf [coef_index + 5] = v + df [coef_index + 3];
                    sf [coef_index + 7] = v - df [coef_index + 3];

                    coef_index += 8;
                }
                while (coef_index < _length);
            }

            /* Next pass */
            for (pass = 3; pass < _nbr_bits; ++pass)
            {
                coef_index = 0;
                nbr_coef = 1 << pass;
                h_nbr_coef = nbr_coef >> 1;
                d_nbr_coef = nbr_coef << 1;
                const flt_t	* const	cos_ptr = _trigo_lut.get_ptr (pass);
                do
                {
                    long				i;
                    const flt_t	*	const sf1r = sf + coef_index;
                    const flt_t	*	const sf2r = sf1r + nbr_coef;
                    flt_t *			const dfr = df + coef_index;
                    flt_t *			const dfi = dfr + nbr_coef;

                    /* Extreme coefficients are always real */
                    dfr [0] = sf1r [0] + sf2r [0];
                    dfi [0] = sf1r [0] - sf2r [0];	// dfr [nbr_coef] =
                    dfr [h_nbr_coef] = sf1r [h_nbr_coef];
                    dfi [h_nbr_coef] = sf2r [h_nbr_coef];

                    /* Others are conjugate complex numbers */
                    const flt_t	* const	sf1i = sf1r + h_nbr_coef;
                    const flt_t	* const	sf2i = sf1i + nbr_coef;
                    for (i = 1; i < h_nbr_coef; ++ i)
                    {
                        const flt_t		c = cos_ptr [i];					// cos (i*PI/nbr_coef);
                        const flt_t		s = cos_ptr [h_nbr_coef - i];	// sin (i*PI/nbr_coef);
                        flt_t				v;

                        v = sf2r [i] * c - sf2i [i] * s;
                        dfr [i] = sf1r [i] + v;
                        dfi [-i] = sf1r [i] - v;	// dfr [nbr_coef - i] =

                        v = sf2r [i] * s + sf2i [i] * c;
                        dfi [i] = v + sf1i [i];
                        dfi [nbr_coef - i] = v - sf1i [i];
                    }

                    coef_index += d_nbr_coef;
                }
                while (coef_index < _length);

                /* Prepare to the next pass */
                {
                    flt_t	* const		temp_ptr = df;
                    df = sf;
                    sf = temp_ptr;
                }
            }
        }
    }

/*______________________________________________
 *
 * Special cases
 *______________________________________________
 */

    /* 4-point FFT */
    else if (_nbr_bits == 2)
    {
        f [1] = x [0] - x [2];
        f [3] = x [1] - x [3];

        const flt_t			b_0 = x [0] + x [2];
        const flt_t			b_2 = x [1] + x [3];

        f [0] = b_0 + b_2;
        f [2] = b_0 - b_2;
    }

    /* 2-point FFT */
    else if (_nbr_bits == 1)
    {
        f [0] = x [0] + x [1];
        f [1] = x [0] - x [1];
    }

    /* 1-point FFT */
    else
    {
        f [0] = x [0];
    }
}



/*==========================================================================*/
/*      Name: do_ifft                                                       */
/*      Description: Compute the inverse FFT of the array. Notice that      */
/*                   IFFT (FFT (x)) = x * length (x). Data must be          */
/*                   post-scaled.                                           */
/*      Input parameters:                                                   */
/*        - f: pointer on the source array (frequencies).                   */
/*             f [0...length(x)/2] = real values,                           */
/*             f [length(x)/2+1...length(x)] = imaginary values of          */
/*               coefficents 1...length(x)-1.                               */
/*      Output parameters:                                                  */
/*        - x: pointer on the destination array (time).                     */
/*      Throws: Nothing                                                     */
/*==========================================================================*/

void	FFTReal::do_ifft (const flt_t f [], flt_t x []) const
{

/*______________________________________________
 *
 * General case
 *______________________________________________
 */

    if (_nbr_bits > 2)
    {
        flt_t *			sf = const_cast <flt_t *> (f);
        flt_t *			df;
        flt_t *			df_temp;

        if (_nbr_bits & 1)
        {
            df = _buffer_ptr;
            df_temp = x;
        }
        else
        {
            df = x;
            df_temp = _buffer_ptr;
        }

        /* Do the transformation in several pass */
        {
            int			pass;
            long			nbr_coef;
            long			h_nbr_coef;
            long			d_nbr_coef;
            long			coef_index;

            /* First pass */
            for (pass = _nbr_bits - 1; pass >= 3; --pass)
            {
                coef_index = 0;
                nbr_coef = 1 << pass;
                h_nbr_coef = nbr_coef >> 1;
                d_nbr_coef = nbr_coef << 1;
                const flt_t	*const cos_ptr = _trigo_lut.get_ptr (pass);
                do
                {
                    long				i;
                    const flt_t	*	const sfr = sf + coef_index;
                    const flt_t	*	const sfi = sfr + nbr_coef;
                    flt_t *			const df1r = df + coef_index;
                    flt_t *			const df2r = df1r + nbr_coef;

                    /* Extreme coefficients are always real */
                    df1r [0] = sfr [0] + sfi [0];		// + sfr [nbr_coef]
                    df2r [0] = sfr [0] - sfi [0];		// - sfr [nbr_coef]
                    df1r [h_nbr_coef] = sfr [h_nbr_coef] * 2;
                    df2r [h_nbr_coef] = sfi [h_nbr_coef] * 2;

                    /* Others are conjugate complex numbers */
                    flt_t * const	df1i = df1r + h_nbr_coef;
                    flt_t * const	df2i = df1i + nbr_coef;
                    for (i = 1; i < h_nbr_coef; ++ i)
                    {
                        df1r [i] = sfr [i] + sfi [-i];		// + sfr [nbr_coef - i]
                        df1i [i] = sfi [i] - sfi [nbr_coef - i];

                        const flt_t		c = cos_ptr [i];					// cos (i*PI/nbr_coef);
                        const flt_t		s = cos_ptr [h_nbr_coef - i];	// sin (i*PI/nbr_coef);
                        const flt_t		vr = sfr [i] - sfi [-i];		// - sfr [nbr_coef - i]
                        const flt_t		vi = sfi [i] + sfi [nbr_coef - i];

                        df2r [i] = vr * c + vi * s;
                        df2i [i] = vi * c - vr * s;
                    }

                    coef_index += d_nbr_coef;
                }
                while (coef_index < _length);

                /* Prepare to the next pass */
                if (pass < _nbr_bits - 1)
                {
                    flt_t	* const	temp_ptr = df;
                    df = sf;
                    sf = temp_ptr;
                }
                else
                {
                    sf = df;
                    df = df_temp;
                }
            }

            /* Antepenultimate pass */
            {
                const flt_t		sqrt2_2 = _sqrt2_2;
                coef_index = 0;
                do
                {
                    df [coef_index] = sf [coef_index] + sf [coef_index + 4];
                    df [coef_index + 4] = sf [coef_index] - sf [coef_index + 4];
                    df [coef_index + 2] = sf [coef_index + 2] * 2;
                    df [coef_index + 6] = sf [coef_index + 6] * 2;

                    df [coef_index + 1] = sf [coef_index + 1] + sf [coef_index + 3];
                    df [coef_index + 3] = sf [coef_index + 5] - sf [coef_index + 7];

                    const flt_t		vr = sf [coef_index + 1] - sf [coef_index + 3];
                    const flt_t		vi = sf [coef_index + 5] + sf [coef_index + 7];

                    df [coef_index + 5] = (vr + vi) * sqrt2_2;
                    df [coef_index + 7] = (vi - vr) * sqrt2_2;

                    coef_index += 8;
                }
                while (coef_index < _length);
            }

            /* Penultimate and last pass at once */
            {
                coef_index = 0;
                const long *	bit_rev_lut_ptr = _bit_rev_lut.get_ptr ();
                const flt_t	*	sf2 = df;
                do
                {
                    {
                        const flt_t		b_0 = sf2 [0] + sf2 [2];
                        const flt_t		b_2 = sf2 [0] - sf2 [2];
                        const flt_t		b_1 = sf2 [1] * 2;
                        const flt_t		b_3 = sf2 [3] * 2;

                        x [bit_rev_lut_ptr [0]] = b_0 + b_1;
                        x [bit_rev_lut_ptr [1]] = b_0 - b_1;
                        x [bit_rev_lut_ptr [2]] = b_2 + b_3;
                        x [bit_rev_lut_ptr [3]] = b_2 - b_3;
                    }
                    {
                        const flt_t		b_0 = sf2 [4] + sf2 [6];
                        const flt_t		b_2 = sf2 [4] - sf2 [6];
                        const flt_t		b_1 = sf2 [5] * 2;
                        const flt_t		b_3 = sf2 [7] * 2;

                        x [bit_rev_lut_ptr [4]] = b_0 + b_1;
                        x [bit_rev_lut_ptr [5]] = b_0 - b_1;
                        x [bit_rev_lut_ptr [6]] = b_2 + b_3;
                        x [bit_rev_lut_ptr [7]] = b_2 - b_3;
                    }

                    sf2 += 8;
                    coef_index += 8;
                    bit_rev_lut_ptr += 8;
                }
                while (coef_index < _length);
            }
        }
    }

/*______________________________________________
 *
 * Special cases
 *______________________________________________
 */

    /* 4-point IFFT */
    else if (_nbr_bits == 2)
    {
        const flt_t		b_0 = f [0] + f [2];
        const flt_t		b_2 = f [0] - f [2];

        x [0] = b_0 + f [1] * 2;
        x [2] = b_0 - f [1] * 2;
        x [1] = b_2 + f [3] * 2;
        x [3] = b_2 - f [3] * 2;
    }

    /* 2-point IFFT */
    else if (_nbr_bits == 1)
    {
        x [0] = f [0] + f [1];
        x [1] = f [0] - f [1];
    }

    /* 1-point IFFT */
    else
    {
        x [0] = f [0];
    }
}



/*==========================================================================*/
/*      Name: rescale                                                       */
/*      Description: Scale an array by divide each element by its length.   */
/*                   This function should be called after FFT + IFFT.       */
/*      Input/Output parameters:                                            */
/*        - x: pointer on array to rescale (time or frequency).             */
/*      Throws: Nothing                                                     */
/*==========================================================================*/

void	FFTReal::rescale (flt_t x []) const
{
    const flt_t		mul = flt_t (1.0 / _length);
    long				i = _length - 1;

    do
    {
        x [i] *= mul;
        --i;
    }
    while (i >= 0);
}



/*\\\ NESTED CLASS MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*==========================================================================*/
/*      Name: Constructor                                                   */
/*      Input parameters:                                                   */
/*        - nbr_bits: number of bits of the array on which we want to do a  */
/*                    FFT. Range: > 0                                       */
/*      Throws: std::bad_alloc                                              */
/*==========================================================================*/

FFTReal::BitReversedLUT::BitReversedLUT (const int nbr_bits)
{
    long				length;
    long				cnt;
    long				br_index;
    long				bit;

    length = 1L << nbr_bits;
    _ptr = new long [length];

    br_index = 0;
    _ptr [0] = 0;
    for (cnt = 1; cnt < length; ++cnt)
    {
        /* ++br_index (bit reversed) */
        bit = length >> 1;
        while (((br_index ^= bit) & bit) == 0)
        {
            bit >>= 1;
        }

        _ptr [cnt] = br_index;
    }
}



/*==========================================================================*/
/*      Name: Destructor                                                    */
/*==========================================================================*/

FFTReal::BitReversedLUT::~BitReversedLUT (void)
{
    delete [] _ptr;
    _ptr = 0;
}



/*==========================================================================*/
/*      Name: Constructor                                                   */
/*      Input parameters:                                                   */
/*        - nbr_bits: number of bits of the array on which we want to do a  */
/*                    FFT. Range: > 0                                       */
/*      Throws: std::bad_alloc, anything                                    */
/*==========================================================================*/

FFTReal::TrigoLUT::TrigoLUT (const int nbr_bits)
{
    long		total_len;

    _ptr = 0;
    if (nbr_bits > 3)
    {
        total_len = (1L << (nbr_bits - 1)) - 4;
        _ptr = new flt_t [total_len];

        const double	PI_ = atan (1) * 4;
        for (int level = 3; level < nbr_bits; ++level)
        {
            const long		level_len = 1L << (level - 1);
            flt_t	* const	level_ptr = const_cast<flt_t *> (get_ptr (level));
            const double	mul = PI_ / (level_len << 1);

            for (long i = 0; i < level_len; ++ i)
            {
                level_ptr [i] = (flt_t) cos (i * mul);
            }
        }
    }
}



/*==========================================================================*/
/*      Name: Destructor                                                    */
/*==========================================================================*/

FFTReal::TrigoLUT::~TrigoLUT (void)
{
    delete [] _ptr;
    _ptr = 0;
}



#if defined (_MSC_VER)
#pragma pack (pop)
#endif	// _MSC_VER



/*****************************************************************************

    LEGAL

    Source code may be freely used for any purpose, including commercial
    applications. Programs must display in their "About" dialog-box (or
    documentation) a text telling they use these routines by Laurent de Soras.
    Modified source code can be distributed, but modifications must be clearly
    indicated.

    CONTACT

    Laurent de Soras
    92 avenue Albert 1er
    92500 Rueil-Malmaison
    France

    ldesoras@club-internet.fr

*****************************************************************************/



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/


// //COPY PASTE
