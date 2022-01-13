#ifndef FFTUNREAL_H
#define FFTUNREAL_H

#include "FFTCommon.hpp"

//TODO check in FFT class is typename float std::is_float<T>
//TODO rename fields and functions


template <class T>
class FFTReal
{

public:

    const uint32_t MAX_BIT_DEPTH = 30;


    explicit FFTReal (long length);
    virtual ~FFTReal () = default;

    uint32_t get_length () const;
    void do_fft (T f [], const T x []) const;
    void do_ifft (const T f [], T x []) const;
    void rescale (T x []) const;
    T* use_buffer () const;


    FFTReal () =  delete;
    FFTReal (const FFTReal &other) = delete;
    FFTReal& operator = (const FFTReal &other) = delete;
    bool operator == (const FFTReal &other) = delete;
    bool operator != (const FFTReal &other) = delete;


private:

    const uint32_t TRIGO_BD_LIMIT = 12; // Over this bit depth, we use direct calculation for sin/cos

    using OscType =	OscSinCos<T>;

    void init_br_lut ();
    void init_trigo_lut ();
    void init_trigo_osc ();

    inline const uint32_t* get_br_ptr () const;
    inline const T* get_trigo_ptr (int level) const;
    inline uint32_t get_trigo_level_index (int level) const;

    inline void	compute_fft_general (T f [], const T x []) const;
    inline void	compute_direct_pass_1_2 (T df [], const T x []) const;
    inline void	compute_direct_pass_3 (T df [], const T sf []) const;
    inline void	compute_direct_pass_n (T df [], const T sf [], int pass) const;
    inline void	compute_direct_pass_n_lut (T df [], const T sf [], int pass) const;
    inline void	compute_direct_pass_n_osc (T df [], const T sf [], int pass) const;

    inline void	compute_ifft_general (const T f [], T x []) const;
    inline void	compute_inverse_pass_n (T df [], const T sf [], int pass) const;
    inline void	compute_inverse_pass_n_osc (T df [], const T sf [], int pass) const;
    inline void	compute_inverse_pass_n_lut (T df [], const T sf [], int pass) const;
    inline void	compute_inverse_pass_3 (T df [], const T sf []) const;
    inline void	compute_inverse_pass_1_2 (T x [], const T sf []) const;


    const uint32_t _length;
    const int _number_bits;
    DynArray <uint32_t> _br_lut;
    DynArray <T> _trigo_lut;
    mutable DynArray <T> _buffer;
    mutable DynArray <OscType> _trigo_osc;


};	// class FFTReal






//FFT impl


static inline bool	FFTReal_is_pow2 (long x) {
    assert (x > 0);
    return  ((x & -x) == x);
}


static inline int	FFTReal_get_next_pow2 (long x) //TODO rechec constexpr
{
    --x;
    int p = 0;
    while ((x & ~0xFFFFL) != 0) {
        p += 16;
        x >>= 16;
    }
    while ((x & ~0xFL) != 0) {
        p += 4;
        x >>= 4;
    }
    while (x > 0) {
        ++p;
        x >>= 1;
    }
    return p;
}



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*
==============================================================================
Name: ctor
Input parameters:
    - length: length of the array on which we want to do a FFT. Range: power of
        2 only, > 0.
Throws: std::bad_alloc
==============================================================================
*/

template <class T>
FFTReal <T>::FFTReal (long length)
    :	_length (length)
    ,	_number_bits (FFTReal_get_next_pow2 (length))
    ,	_br_lut ()
    ,	_trigo_lut ()
    ,	_buffer (length)
    ,	_trigo_osc ()
{
    assert (FFTReal_is_pow2 (length));
    assert (_number_bits <= MAX_BIT_DEPTH);

    init_br_lut ();
    init_trigo_lut ();
    init_trigo_osc ();
}



/*
==============================================================================
Name: get_length
Description:
    Returns the number of points processed by this FFT object.
Returns: The number of points, power of 2, > 0.
Throws: Nothing
==============================================================================
*/

template <class T>
uint32_t FFTReal<T>::get_length() const {
    return (_length);
}


/*
==============================================================================
Name: do_fft
Description:
    Compute the FFT of the array.
Input parameters:
    - x: pointer on the source array (time).
Output parameters:
    - f: pointer on the destination array (frequencies).
        f [0...length(x)/2] = real values,
        f [length(x)/2+1...length(x)-1] = negative imaginary values of
        coefficents 1...length(x)/2-1.
Throws: Nothing
==============================================================================
*/

template <class T>
void FFTReal <T>::do_fft (T f [], const T x []) const
{
    assert (f != 0);
    assert (f != use_buffer ());
    assert (x != 0);
    assert (x != use_buffer ());
    assert (x != f);

    if (_number_bits > 2) // General case
        compute_fft_general (f, x);
    else if (_number_bits == 2) // 4-point FFT
    {
        f [1] = x [0] - x [2];
        f [3] = x [1] - x [3];
        const T b_0 = x [0] + x [2];
        const T b_2 = x [1] + x [3];
        f [0] = b_0 + b_2;
        f [2] = b_0 - b_2;
    }
    else if (_number_bits == 1) // 2-point FFT
    {
        f [0] = x [0] + x [1];
        f [1] = x [0] - x [1];
    }
    else
        f [0] = x [0]; // 1-point FFT
}



/*
==============================================================================
Name: do_ifft
Description:
    Compute the inverse FFT of the array. Note that data must be post-scaled:
    IFFT (FFT (x)) = x * length (x).
Input parameters:
    - f: pointer on the source array (frequencies).
        f [0...length(x)/2] = real values
        f [length(x)/2+1...length(x)-1] = negative imaginary values of
        coefficents 1...length(x)/2-1.
Output parameters:
    - x: pointer on the destination array (time).
Throws: Nothing
==============================================================================
*/

template <class T>
void FFTReal <T>::do_ifft (const T f [], T x []) const
{
    assert (f != 0);
    assert (f != use_buffer ());
    assert (x != 0);
    assert (x != use_buffer ());
    assert (x != f);

    if (_number_bits > 2) // General case
        compute_ifft_general (f, x);
    else if (_number_bits == 2) // 4-point IFFT
    {
        const T	b_0 = f [0] + f [2];
        const T	b_2 = f [0] - f [2];

        x [0] = b_0 + f [1] * 2;
        x [2] = b_0 - f [1] * 2;
        x [1] = b_2 + f [3] * 2;
        x [3] = b_2 - f [3] * 2;
    }
    else if (_number_bits == 1) // 2-point IFFT
    {
        x [0] = f [0] + f [1];
        x [1] = f [0] - f [1];
    }
    else // 1-point IFFT
        x [0] = f [0];
}



/*
==============================================================================
Name: rescale
Description:
    Scale an array by divide each element by its length. This function should
    be called after FFT + IFFT.
Input parameters:
    - x: pointer on array to rescale (time or frequency).
Throws: Nothing
==============================================================================
*/

template <class T>
void FFTReal <T>::rescale (T x []) const
{
    const T mul = 1.0 / _length;

    if (_length < 4)
    {
        long i = _length - 1;
        do {
            x [i] *= mul;
            --i;
        }
        while (i >= 0);
    }
    else
    {
        assert ((_length & 3) == 0);
        long i = _length - 4; // Could be optimized with SIMD instruction sets (needs alignment check)
        do {
            x [i + 0] *= mul;
            x [i + 1] *= mul;
            x [i + 2] *= mul;
            x [i + 3] *= mul;
            i -= 4;
        }
        while (i >= 0);
    }
}



/*
==============================================================================
Name: use_buffer
Description:
    Access the internal buffer, whose length is the FFT one.
    Buffer content will be erased at each do_fft() / do_ifft() call!
    This buffer cannot be used as:
        - source for FFT or IFFT done with this object
        - destination for FFT or IFFT done with this object
Returns:
    Buffer start address
Throws: Nothing
==============================================================================
*/

template <class T>
T*	FFTReal <T>::use_buffer () const {
    return (&_buffer [0]);
}



template <class T>
void FFTReal <T>::init_br_lut ()
{
    const long length = 1L << _number_bits;

    _br_lut.resize (length);
    _br_lut [0] = 0;

    long br_index = 0;
    for (long cnt = 1; cnt < length; ++cnt) {
        long bit = length >> 1;
        while (((br_index ^= bit) & bit) == 0)
            bit >>= 1;
        _br_lut [cnt] = br_index;
    }
}



template <class T>
void FFTReal <T>::init_trigo_lut ()
{
    if (_number_bits > 3)
    {
        const long total_len = (1L << (_number_bits - 1)) - 4;
        _trigo_lut.resize (total_len);

        for (int level = 3; level < _number_bits; ++level)
        {
            const long level_len = 1L << (level - 1);
            T* const level_ptr = &_trigo_lut [get_trigo_level_index (level)];

            const double mul = PI / (level_len << 1);
            for (long i = 0; i < level_len; ++ i)
                level_ptr [i] = std::cos (i * mul);
        }
    }
}



template <class T>
void FFTReal <T>::init_trigo_osc ()
{
    const int nbr_osc = _number_bits - TRIGO_BD_LIMIT;
    if (nbr_osc > 0)
    {
        _trigo_osc.resize (nbr_osc);

        for (int osc_cnt = 0; osc_cnt < nbr_osc; ++osc_cnt)
        {
            OscType& osc = _trigo_osc [osc_cnt];
            const long len = 1L << (TRIGO_BD_LIMIT + osc_cnt);
            const double mul = (0.5 * PI) / len;
            osc.set_step (mul);
        }
    }
}



template <class T>
const uint32_t* FFTReal<T>::get_br_ptr() const {
    return &_br_lut [0]; //TODO make another function just to access ptr
}



template <class T>
const T* FFTReal <T>::get_trigo_ptr (int level) const {
    assert (level >= 3);
    return (&_trigo_lut [get_trigo_level_index (level)]);//TODO make another function just to access ptr
}



template <class T>
uint32_t FFTReal<T>::get_trigo_level_index(int level) const {
    assert (level >= 3);
    return ((1L << (level - 1)) - 4);
}



// Transform in several passes
template <class T>
void FFTReal <T>::compute_fft_general (T f [], const T x []) const
{
    assert (f != 0);
    assert (f != use_buffer ());
    assert (x != 0);
    assert (x != use_buffer ());
    assert (x != f);

    T* sf;
    T* df;

    if ((_number_bits & 1) != 0) {
        df = use_buffer ();
        sf = f;
    }
    else {
        df = f;
        sf = use_buffer ();
    }

    compute_direct_pass_1_2 (df, x);
    compute_direct_pass_3 (sf, df);

    for (int pass = 3; pass < _number_bits; ++ pass)
    {
        compute_direct_pass_n (df, sf, pass);

        T* const temp_ptr = df;
        df = sf;
        sf = temp_ptr;
    }
}



template <class T>
void FFTReal <T>::compute_direct_pass_1_2 (T df [], const T x []) const
{
    assert (df != 0);
    assert (x != 0);
    assert (df != x);

    const uint32_t * const	bit_rev_lut_ptr = get_br_ptr (); //TODO maybe back
    uint32_t				coef_index = 0;
    do
    {
        const uint32_t		rev_index_0 = bit_rev_lut_ptr [coef_index];
        const uint32_t		rev_index_1 = bit_rev_lut_ptr [coef_index + 1];
        const uint32_t		rev_index_2 = bit_rev_lut_ptr [coef_index + 2];
        const uint32_t		rev_index_3 = bit_rev_lut_ptr [coef_index + 3];

        T	* const	df2 = df + coef_index;
        df2 [1] = x [rev_index_0] - x [rev_index_1];
        df2 [3] = x [rev_index_2] - x [rev_index_3];

        const T	sf_0 = x [rev_index_0] + x [rev_index_1];
        const T	sf_2 = x [rev_index_2] + x [rev_index_3];

        df2 [0] = sf_0 + sf_2;
        df2 [2] = sf_0 - sf_2;

        coef_index += 4;
    }
    while (coef_index < _length);
}



template <class T>
void	FFTReal <T>::compute_direct_pass_3 (T df [], const T sf []) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);

    const T	sqrt2_2 = SQRT2 * 0.5;
    long coef_index = 0;

    T v;
    do
    {
        df [coef_index] = sf [coef_index] + sf [coef_index + 4];
        df [coef_index + 4] = sf [coef_index] - sf [coef_index + 4];
        df [coef_index + 2] = sf [coef_index + 2];
        df [coef_index + 6] = sf [coef_index + 6];

        v = (sf [coef_index + 5] - sf [coef_index + 7]) * sqrt2_2;
        df [coef_index + 1] = sf [coef_index + 1] + v;
        df [coef_index + 3] = sf [coef_index + 1] - v;

        v = (sf [coef_index + 5] + sf [coef_index + 7]) * sqrt2_2;
        df [coef_index + 5] = v + sf [coef_index + 3];
        df [coef_index + 7] = v - sf [coef_index + 3];

        coef_index += 8;
    }
    while (coef_index < _length);
}



template <class DT>
void FFTReal <DT>::compute_direct_pass_n (DT df [], const DT sf [], int pass) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);
    assert (pass >= 3);
    assert (pass < _number_bits);

    if (pass <= TRIGO_BD_LIMIT)
        compute_direct_pass_n_lut (df, sf, pass);
    else
        compute_direct_pass_n_osc (df, sf, pass);
}



template <class T>
void FFTReal <T>::compute_direct_pass_n_lut (T df [], const T sf [], int pass) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);
    assert (pass >= 3);
    assert (pass < _number_bits);

    const long nbr_coef = 1 << pass;
    const long h_nbr_coef = nbr_coef >> 1;
    const long d_nbr_coef = nbr_coef << 1;
    long coef_index = 0;
    const T* const cos_ptr = get_trigo_ptr (pass);

    T v;
    do
    {
        const T* const sf1r = sf + coef_index;
        const T* const sf2r = sf1r + nbr_coef;
        T* const dfr = df + coef_index;
        T* const dfi = dfr + nbr_coef;

        dfr [0] = sf1r [0] + sf2r [0];  // Extreme coefficients are always real
        dfi [0] = sf1r [0] - sf2r [0];	// dfr [nbr_coef] =
        dfr [h_nbr_coef] = sf1r [h_nbr_coef];
        dfi [h_nbr_coef] = sf2r [h_nbr_coef];

        const T* const	sf1i = sf1r + h_nbr_coef; // Others are conjugate complex numbers
        const T* const	sf2i = sf1i + nbr_coef;

        for (long i = 1; i < h_nbr_coef; ++ i)
        {
            const T	c = cos_ptr [i];					// cos (i*PI/nbr_coef);
            const T	s = cos_ptr [h_nbr_coef - i];	// sin (i*PI/nbr_coef);

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
}



template <class T>
void FFTReal <T>::compute_direct_pass_n_osc (T df [], const T sf [], int pass) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);
    assert (pass > TRIGO_BD_LIMIT);
    assert (pass < _number_bits);

    const long nbr_coef = 1 << pass;
    const long h_nbr_coef = nbr_coef >> 1;
    const long d_nbr_coef = nbr_coef << 1;
    long coef_index = 0;
    OscType& osc = _trigo_osc [pass - (TRIGO_BD_LIMIT + 1)];

    T v;
    do
    {
        const T* const sf1r = sf + coef_index;
        const T* const sf2r = sf1r + nbr_coef;
        T* const dfr = df + coef_index;
        T* const dfi = dfr + nbr_coef;
        osc.clear_buffers ();

        dfr [0] = sf1r [0] + sf2r [0];
        dfi [0] = sf1r [0] - sf2r [0];
        dfr [h_nbr_coef] = sf1r [h_nbr_coef];
        dfi [h_nbr_coef] = sf2r [h_nbr_coef];

        const T* const sf1i = sf1r + h_nbr_coef;
        const T* const sf2i = sf1i + nbr_coef;

        for (long i = 1; i < h_nbr_coef; ++ i)
        {
            osc.step ();
            const T	c = osc.get_cos ();
            const T	s = osc.get_sin ();

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
}



// Transform in several pass
template <class T>
void FFTReal <T>::compute_ifft_general (const T f [], T x []) const
{
    assert (f != 0);
    assert (f != use_buffer ());
    assert (x != 0);
    assert (x != use_buffer ());
    assert (x != f);

    const T* sf = f; // const_cast <DT *> (f); //TODO!
    T* df;
    T* df_temp;

    if (_number_bits & 1) {
        df = use_buffer ();
        df_temp = x;
    }
    else {
        df = x;
        df_temp = use_buffer ();
    }

    for (int pass = _number_bits - 1; pass >= 3; -- pass)
    {
        compute_inverse_pass_n (df, sf, pass);

        if (pass < _number_bits - 1)
        {
            T* const temp_ptr = df;
            df = sf;
            sf = temp_ptr;
        }
        else
        {
            sf = df;
            df = df_temp;
        }
    }

    compute_inverse_pass_3 (df, sf);
    compute_inverse_pass_1_2 (x, df);
}



template <class T>
void FFTReal <T>::compute_inverse_pass_n (T df [], const T sf [], int pass) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);
    assert (pass >= 3);
    assert (pass < _number_bits);

    if (pass <= TRIGO_BD_LIMIT)
        compute_inverse_pass_n_lut (df, sf, pass);
    else
        compute_inverse_pass_n_osc (df, sf, pass);
}



template <class T>
void FFTReal <T>::compute_inverse_pass_n_lut (T df [], const T sf [], int pass) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);
    assert (pass >= 3);
    assert (pass < _number_bits);

    const long nbr_coef = 1 << pass;
    const long h_nbr_coef = nbr_coef >> 1;
    const long d_nbr_coef = nbr_coef << 1;
    long coef_index = 0;
    const T* const cos_ptr = get_trigo_ptr (pass);
    do
    {
        const T* const sfr = sf + coef_index;
        const T* const sfi = sfr + nbr_coef;
        T* const df1r = df + coef_index;
        T* const df2r = df1r + nbr_coef;


        df1r [0] = sfr [0] + sfi [0];   // Extreme coefficients are always real // + sfr [nbr_coef]
        df2r [0] = sfr [0] - sfi [0];   // - sfr [nbr_coef]
        df1r [h_nbr_coef] = sfr [h_nbr_coef] * 2;
        df2r [h_nbr_coef] = sfi [h_nbr_coef] * 2;

        T* const df1i = df1r + h_nbr_coef; // Others are conjugate complex numbers
        T* const df2i = df1i + nbr_coef;
        for (long i = 1; i < h_nbr_coef; ++ i)
        {
            df1r [i] = sfr [i] + sfi [-i];		// + sfr [nbr_coef - i]
            df1i [i] = sfi [i] - sfi [nbr_coef - i];

            const T c = cos_ptr [i];					// cos (i*PI/nbr_coef);
            const T s = cos_ptr [h_nbr_coef - i];	// sin (i*PI/nbr_coef);
            const T vr = sfr [i] - sfi [-i];		// - sfr [nbr_coef - i]
            const T vi = sfi [i] + sfi [nbr_coef - i];

            df2r [i] = vr * c + vi * s;
            df2i [i] = vi * c - vr * s;
        }
        coef_index += d_nbr_coef;
    }
    while (coef_index < _length);
}



template <class T>
void FFTReal <T>::compute_inverse_pass_n_osc (T df [], const T sf [], int pass) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);
    assert (pass > TRIGO_BD_LIMIT);
    assert (pass < _number_bits);

    const long nbr_coef = 1 << pass;
    const long h_nbr_coef = nbr_coef >> 1;
    const long d_nbr_coef = nbr_coef << 1;
    long coef_index = 0;
    OscType& osc = _trigo_osc [pass - (TRIGO_BD_LIMIT + 1)];
    do
    {
        const T* const sfr = sf + coef_index;
        const T* const sfi = sfr + nbr_coef;
        T* const df1r = df + coef_index;
        T* const df2r = df1r + nbr_coef;

        df1r [0] = sfr [0] + sfi [0];   // Extreme coefficients are always real	// + sfr [nbr_coef]
        df2r [0] = sfr [0] - sfi [0];   // - sfr [nbr_coef]
        df1r [h_nbr_coef] = sfr [h_nbr_coef] * 2;
        df2r [h_nbr_coef] = sfi [h_nbr_coef] * 2;

        osc.clear_buffers ();
        T* const df1i = df1r + h_nbr_coef; // Others are conjugate complex numbers
        T* const df2i = df1i + nbr_coef;
        for (long i = 1; i < h_nbr_coef; ++ i)
        {
            df1r [i] = sfr [i] + sfi [-i];		// + sfr [nbr_coef - i]
            df1i [i] = sfi [i] - sfi [nbr_coef - i];

            osc.step ();
            const T c = osc.get_cos ();
            const T s = osc.get_sin ();
            const T vr = sfr [i] - sfi [-i];		// - sfr [nbr_coef - i]
            const T vi = sfi [i] + sfi [nbr_coef - i];

            df2r[i] = vr * c + vi * s;
            df2i[i] = vi * c - vr * s;
        }

        coef_index += d_nbr_coef;
    }
    while (coef_index < _length);
}



template <class T>
void FFTReal <T>::compute_inverse_pass_3 (T df [], const T sf []) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);

    const T sqrt2_2 = SQRT2 * 0.5;
    long coef_index = 0;
    do
    {
        df[coef_index] = sf[coef_index] + sf[coef_index + 4];
        df[coef_index + 4] = sf[coef_index] - sf[coef_index + 4];
        df[coef_index + 2] = sf[coef_index + 2] * 2.0; //TODO is_same
        df[coef_index + 6] = sf[coef_index + 6] * 2.0;

        df[coef_index + 1] = sf [coef_index + 1] + sf[coef_index + 3];
        df[coef_index + 3] = sf [coef_index + 5] - sf[coef_index + 7];

        const T vr = sf[coef_index + 1] - sf [coef_index + 3];
        const T vi = sf[coef_index + 5] + sf [coef_index + 7];

        df[coef_index + 5] = (vr + vi) * sqrt2_2;
        df[coef_index + 7] = (vi - vr) * sqrt2_2;

        coef_index += 8;
    }
    while (coef_index < _length);
}



template <class T>
void FFTReal <T>::compute_inverse_pass_1_2 (T x [], const T sf []) const
{
    assert (x != 0);
    assert (sf != 0);
    assert (x != sf);

    const long* bit_rev_lut_ptr = get_br_ptr ();
    const T* sf2 = sf;
    long coef_index = 0;
    do
    {
        {
            const T b_0 = sf2 [0] + sf2 [2];
            const T b_2 = sf2 [0] - sf2 [2];
            const T b_1 = sf2 [1] * 2;
            const T b_3 = sf2 [3] * 2;

            x[bit_rev_lut_ptr [0]] = b_0 + b_1;
            x[bit_rev_lut_ptr [1]] = b_0 - b_1;
            x[bit_rev_lut_ptr [2]] = b_2 + b_3;
            x[bit_rev_lut_ptr [3]] = b_2 - b_3;
        }
        {
            const T b_0 = sf2 [4] + sf2 [6];
            const T b_2 = sf2 [4] - sf2 [6];
            const T b_1 = sf2 [5] * 2;
            const T b_3 = sf2 [7] * 2;

            x[bit_rev_lut_ptr [4]] = b_0 + b_1;
            x[bit_rev_lut_ptr [5]] = b_0 - b_1;
            x[bit_rev_lut_ptr [6]] = b_2 + b_3;
            x[bit_rev_lut_ptr [7]] = b_2 - b_3;
        }

        sf2 += 8;
        coef_index += 8;
        bit_rev_lut_ptr += 8;
    }
    while (coef_index < _length);
}



#endif // FFTUNREAL_H
