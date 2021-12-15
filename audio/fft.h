#ifndef FFT_H
#define FFT_H

#include <vector>

class FreqTable
{
    std::vector<double> scaled;
    std::vector<double> unscaled;

public:
    FreqTable();
    double getNearestUnscaled(double freq);
    double getNearestScaled(double freq);
    double getNearest(double freq);
    double getUpperScaled(double freq);
    double getLowerScaled(double freq);
    short  getNoteByFreq(double freq);

    int getScaledIndex(double freq);
};


class Peak
{
private:
    void findNearest();

  public:

    static FreqTable freqTab;
    double position;
    double freq;
    double amplitude;

    Peak() : position(0), freq(0) ,amplitude(0.0)
    {

    }

    Peak(int inposition, double newAmplitude, double coef=7.8125): //was 5.383 is 8192 for 44100
        position(inposition),

        amplitude(newAmplitude)

    {
        double fr = position*coef;
        freq = freqTab.getNearest(fr);
        //freq = fr; //table was ill)
    }

    double getPosition()
    {
        return position;
    }

    bool isFrequencyInside(unsigned short freq)
    {
        return false;
    }

    double getAmplitude()
    {
        return amplitude;
    }

    double  getFreq()
    {
        return freq;
    }

};

class FFT
{
public:
    std::vector<Peak> peaks;
    unsigned short N;

    static FreqTable freqTab;

    std::vector<double> decibels;
    std::vector<double> amplitude;
    std::vector<double> amplitudeScale;
    std::vector<double> sqrd;
    std::vector<double> halfFoubricatedSource;

    std::vector<double> destReal;
    std::vector<double> destImg;


public:
    FFT(int size);
    ~FFT();

    bool transform(short int *place);

    bool countDecibels();
    bool countAmplitude();
    bool countAmplitudeScale();
    bool countSQRD();

    double *getDecibels();
    double *getAmplitude();
    double *getAmplitudeScale();
    double *getSQRD();
    double *getRealPart();
    double *getImgPart();

    int findPeaks(double coefRate=7.8125);
    std::vector<Peak>& getPeaks();

    struct AmplitudeValue
    {
        double value;
        int freq;
    };

    AmplitudeValue getMaxAmplitude();
};

//COPY PASTE

/*****************************************************************************
*                                                                            *
*       DIGITAL SIGNAL PROCESSING TOOLS                                      *
*       Version 1.03, 2001/06/15                                             *
*       (c) 1999 - Laurent de Soras                                          *
*                                                                            *
*       FFTReal.h                                                            *
*       Fourier transformation of real number arrays.                        *
*       Portable ISO C++                                                     *
*                                                                            *
* Tab = 3                                                                    *
*****************************************************************************/



#if defined (FFTReal_CURRENT_HEADER)
    #error Recursive inclusion of FFTReal header file.
#endif
#define	FFTReal_CURRENT_HEADER

#if ! defined (FFTReal_HEADER_INCLUDED)
#define	FFTReal_HEADER_INCLUDED



#if defined (_MSC_VER)
#pragma pack (push, 8)
#endif	// _MSC_VER



class	FFTReal
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

    /* Change this typedef to use a different floating point type in your FFTs
        (i.e. float, double or long double). */
    typedef float	flt_t;

    explicit			FFTReal (const long length);
                        ~FFTReal ();
    void				do_fft (flt_t f [], const flt_t x []) const;
    void				do_ifft (const flt_t f [], flt_t x []) const;
    void				rescale (flt_t x []) const;



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

    /* Bit-reversed look-up table nested class */
    class BitReversedLUT
    {
    public:
        explicit			BitReversedLUT (const int nbr_bits);
                            ~BitReversedLUT ();
        const long *	get_ptr () const
        {
            return (_ptr);
        }
    private:
        long *			_ptr;
    };

    /* Trigonometric look-up table nested class */
    class	TrigoLUT
    {
    public:
        explicit			TrigoLUT (const int nbr_bits);
                            ~TrigoLUT ();
        const flt_t	*	get_ptr (const int level) const
        {
            return (_ptr + (1L << (level - 1)) - 4);
        };
    private:
        flt_t	*			_ptr;
    };

    const BitReversedLUT	_bit_rev_lut;
    const TrigoLUT	_trigo_lut;
    const flt_t		_sqrt2_2;
    const long		_length;
    const int		_nbr_bits;
    flt_t *			_buffer_ptr;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

                        FFTReal (const FFTReal &other);
    const FFTReal&	operator = (const FFTReal &other);
    int				operator == (const FFTReal &other);
    int				operator != (const FFTReal &other);
};



#if defined (_MSC_VER)
#pragma pack (pop)
#endif	// _MSC_VER



#endif	// FFTReal_HEADER_INCLUDED

#undef FFTReal_CURRENT_HEADER



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




#endif // FFT_H
