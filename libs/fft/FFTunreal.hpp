#ifndef FFTUNREAL_H
#define FFTUNREAL_H

#include <cstdint>
#include <cassert>
#include <cmath>

#include <type_traits>


const double	PI		= 3.1415926535897932384626433832795;
const double	SQRT2	= 1.41421356237309514547462185873883;


//TODO check in FFT class is typename float std::is_float<T>
//TODO force inline option

template <class T>
class DynArray
{

public:

    DynArray ();
    explicit DynArray (uint32_t size);
    ~DynArray ();

    inline uint32_t size () const;
    inline void resize (uint32_t new_size);

    inline const T& operator [] (uint32_t pos) const;
    inline T& operator [] (uint32_t pos);

    DynArray(const DynArray& other) = delete;
    DynArray& operator = (const DynArray& other) = delete;
    bool operator == (const DynArray& other) = delete;
    bool operator != (const DynArray& other) = delete;

private:

    T* _data_ptr;
    uint32_t _len;

};



template <class T>
class OscSinCos
{

public:

    OscSinCos ();

    inline void set_step (double angle_rad); //TODO T

    inline T get_cos () const;
    inline T get_sin () const;
    inline void step ();
    inline void clear_buffers ();

    OscSinCos (const OscSinCos& other) = delete;
    OscSinCos&	operator = (const OscSinCos& other) = delete;
    bool operator == (const OscSinCos& other) = delete;
    bool operator != (const OscSinCos& other) = delete;

private:

    T _pos_cos;		// Current phase expressed with sin and cos. [-1 ; 1]
    T _pos_sin;		// -
    T _step_cos;	// Phase increment per step, [-1 ; 1]
    T _step_sin;	// -

};	// class OscSinCos





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


    FFTReal ();
    FFTReal (const FFTReal &other);
    FFTReal& operator = (const FFTReal &other);
    bool operator == (const FFTReal &other);
    bool operator != (const FFTReal &other);


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
    const int _nbr_bits;
    DynArray <uint32_t> _br_lut;
    DynArray <T> _trigo_lut;
    mutable DynArray <T> _buffer;
    mutable DynArray <OscType> _trigo_osc;


};	// class FFTReal




//Dynamic array implementation

template <class T>
DynArray <T>::DynArray()
    :	_data_ptr (nullptr), _len (0) {}


template <class T>
DynArray <T>::DynArray(uint32_t size)
    : _data_ptr(nullptr), _len (size)
{
    //static_assert (_len > 0, "DynArray zero len not expected");
    _data_ptr = new T[_len];
}

template <class T>
DynArray <T>::~DynArray()
{
    delete [] _data_ptr;
    _data_ptr = nullptr;
    _len = 0;
}

template <class T>
uint32_t DynArray <T>::size() const {
    return (_len);
}


template <class T>
void DynArray <T>::resize(uint32_t new_size)
{
    assert (new_size > 0);
    _len = new_size;
    delete [] _data_ptr;
    _data_ptr = new T[_len];
}

template <class T>
const T& DynArray <T>::operator[] (uint32_t pos) const
{
    assert (pos >= 0);
    assert (pos < _len);
    return (_data_ptr [pos]);
}

template <class T>
T&	DynArray <T>::operator[] (uint32_t pos)
{
    assert (pos >= 0);
    assert (pos < _len);
    return (_data_ptr [pos]);
}



//OscSinCos implementation

template <class T>
OscSinCos <T>::OscSinCos () : _pos_cos(1),
    _pos_sin (0), _step_cos(1), _step_sin (0) {}


template <class T>
void OscSinCos <T>::set_step (double angle_rad)
{
    _step_cos = std::cos(angle_rad);
    _step_sin = std::sin(angle_rad);
}


template <class T>
T OscSinCos <T>::get_cos () const {
    return _pos_cos;
}


template <class T>
T OscSinCos <T>::get_sin () const {
    return _pos_sin;
}


template <class T>
void OscSinCos <T>::step ()
{
    const T	old_cos = _pos_cos;
    const T	old_sin = _pos_sin;

    _pos_cos = old_cos * _step_cos - old_sin * _step_sin;
    _pos_sin = old_cos * _step_sin + old_sin * _step_cos;
}



template <class T>
void OscSinCos <T>::clear_buffers ()
{
    if (std::is_same<float, T>()) {
        _pos_cos = 1.f;
        _pos_sin = 0.f;
    }
    if (std::is_same<double, T>()) {
        _pos_cos = 1.;
        _pos_sin = 0.;
    }
}




//FFT impl


static inline bool	FFTReal_is_pow2 (long x) {
    assert (x > 0);
    return  ((x & -x) == x);
}



static inline int	FFTReal_get_next_pow2 (long x)
{
    --x;
    int				p = 0;
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
    return (p);
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

template <class DT>
FFTReal <DT>::FFTReal (long length)
:	_length (length)
,	_nbr_bits (FFTReal_get_next_pow2 (length))
,	_br_lut ()
,	_trigo_lut ()
,	_buffer (length)
,	_trigo_osc ()
{
    assert (FFTReal_is_pow2 (length));
    assert (_nbr_bits <= MAX_BIT_DEPTH);

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

template <class DT>
uint32_t FFTReal<DT>::get_length() const
{
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

template <class DT>
void	FFTReal <DT>::do_fft (DT f [], const DT x []) const
{
    assert (f != 0);
    assert (f != use_buffer ());
    assert (x != 0);
    assert (x != use_buffer ());
    assert (x != f);

    // General case
    if (_nbr_bits > 2)
    {
        compute_fft_general (f, x);
    }

    // 4-point FFT
    else if (_nbr_bits == 2)
    {
        f [1] = x [0] - x [2];
        f [3] = x [1] - x [3];

        const DT	b_0 = x [0] + x [2];
        const DT	b_2 = x [1] + x [3];

        f [0] = b_0 + b_2;
        f [2] = b_0 - b_2;
    }

    // 2-point FFT
    else if (_nbr_bits == 1)
    {
        f [0] = x [0] + x [1];
        f [1] = x [0] - x [1];
    }

    // 1-point FFT
    else
    {
        f [0] = x [0];
    }
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

template <class DT>
void	FFTReal <DT>::do_ifft (const DT f [], DT x []) const
{
    assert (f != 0);
    assert (f != use_buffer ());
    assert (x != 0);
    assert (x != use_buffer ());
    assert (x != f);

    // General case
    if (_nbr_bits > 2)
    {
        compute_ifft_general (f, x);
    }

    // 4-point IFFT
    else if (_nbr_bits == 2)
    {
        const DT	b_0 = f [0] + f [2];
        const DT	b_2 = f [0] - f [2];

        x [0] = b_0 + f [1] * 2;
        x [2] = b_0 - f [1] * 2;
        x [1] = b_2 + f [3] * 2;
        x [3] = b_2 - f [3] * 2;
    }

    // 2-point IFFT
    else if (_nbr_bits == 1)
    {
        x [0] = f [0] + f [1];
        x [1] = f [0] - f [1];
    }

    // 1-point IFFT
    else
    {
        x [0] = f [0];
    }
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

template <class DT>
void	FFTReal <DT>::rescale (DT x []) const
{
    const DT mul = DT (1.0 / _length); //TODO

    if (_length < 4)
    {
        long				i = _length - 1;
        do
        {
            x [i] *= mul;
            --i;
        }
        while (i >= 0);
    }

    else
    {
        assert ((_length & 3) == 0);

        // Could be optimized with SIMD instruction sets (needs alignment check)
        long				i = _length - 4;
        do
        {
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

template <class DT>
DT*	FFTReal <DT>::use_buffer () const {
    return (&_buffer [0]);
}



template <class DT>
void	FFTReal <DT>::init_br_lut ()
{
    const long		length = 1L << _nbr_bits;
    _br_lut.resize (length);

    _br_lut [0] = 0;
    long				br_index = 0;
    for (long cnt = 1; cnt < length; ++cnt)
    {
        // ++br_index (bit reversed)
        long				bit = length >> 1;
        while (((br_index ^= bit) & bit) == 0)
        {
            bit >>= 1;
        }

        _br_lut [cnt] = br_index;
    }
}



template <class DT>
void	FFTReal <DT>::init_trigo_lut ()
{
    using namespace std;

    if (_nbr_bits > 3)
    {
        const long		total_len = (1L << (_nbr_bits - 1)) - 4;
        _trigo_lut.resize (total_len);

        for (int level = 3; level < _nbr_bits; ++level)
        {
            const long		level_len = 1L << (level - 1);

            DT	* const	level_ptr =
                &_trigo_lut [get_trigo_level_index (level)];

            const double	mul = PI / (level_len << 1);

            for (long i = 0; i < level_len; ++ i)
            {
                level_ptr [i] = std::cos (i * mul);
            }
        }
    }
}



template <class DT>
void	FFTReal <DT>::init_trigo_osc ()
{
    const int		nbr_osc = _nbr_bits - TRIGO_BD_LIMIT;
    if (nbr_osc > 0)
    {
        _trigo_osc.resize (nbr_osc);

        for (int osc_cnt = 0; osc_cnt < nbr_osc; ++osc_cnt)
        {
            OscType &		osc = _trigo_osc [osc_cnt];

            const long		len = 1L << (TRIGO_BD_LIMIT + osc_cnt);
            const double	mul = (0.5 * PI) / len;
            osc.set_step (mul);
        }
    }
}



template <class DT>
const uint32_t *FFTReal<DT>::get_br_ptr() const
{
    return (&_br_lut [0]);
}



template <class DT>
const DT*	FFTReal <DT>::get_trigo_ptr (int level) const {
    assert (level >= 3);
    return (&_trigo_lut [get_trigo_level_index (level)]);
}



template <class DT>
uint32_t FFTReal<DT>::get_trigo_level_index(int level) const {
    assert (level >= 3);
    return ((1L << (level - 1)) - 4);
}



// Transform in several passes
template <class DT>
void	FFTReal <DT>::compute_fft_general (DT f [], const DT x []) const
{
    assert (f != 0);
    assert (f != use_buffer ());
    assert (x != 0);
    assert (x != use_buffer ());
    assert (x != f);

    DT *		sf;
    DT *		df;

    if ((_nbr_bits & 1) != 0)
    {
        df = use_buffer ();
        sf = f;
    }
    else
    {
        df = f;
        sf = use_buffer ();
    }

    compute_direct_pass_1_2 (df, x);
    compute_direct_pass_3 (sf, df);

    for (int pass = 3; pass < _nbr_bits; ++ pass)
    {
        compute_direct_pass_n (df, sf, pass);

        DT * const	temp_ptr = df;
        df = sf;
        sf = temp_ptr;
    }
}



template <class DT>
void	FFTReal <DT>::compute_direct_pass_1_2 (DT df [], const DT x []) const
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

        DT	* const	df2 = df + coef_index;
        df2 [1] = x [rev_index_0] - x [rev_index_1];
        df2 [3] = x [rev_index_2] - x [rev_index_3];

        const DT	sf_0 = x [rev_index_0] + x [rev_index_1];
        const DT	sf_2 = x [rev_index_2] + x [rev_index_3];

        df2 [0] = sf_0 + sf_2;
        df2 [2] = sf_0 - sf_2;

        coef_index += 4;
    }
    while (coef_index < _length);
}



template <class DT>
void	FFTReal <DT>::compute_direct_pass_3 (DT df [], const DT sf []) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);

    const DT	sqrt2_2 = SQRT2 * 0.5;
    long coef_index = 0;
    DT v;

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
void	FFTReal <DT>::compute_direct_pass_n (DT df [], const DT sf [], int pass) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);
    assert (pass >= 3);
    assert (pass < _nbr_bits);

    if (pass <= TRIGO_BD_LIMIT)
    {
        compute_direct_pass_n_lut (df, sf, pass);
    }
    else
    {
        compute_direct_pass_n_osc (df, sf, pass);
    }
}



template <class DT>
void	FFTReal <DT>::compute_direct_pass_n_lut (DT df [], const DT sf [], int pass) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);
    assert (pass >= 3);
    assert (pass < _nbr_bits);

    const long		nbr_coef = 1 << pass;
    const long		h_nbr_coef = nbr_coef >> 1;
    const long		d_nbr_coef = nbr_coef << 1;
    long				coef_index = 0;
    const DT	* const	cos_ptr = get_trigo_ptr (pass);
    do
    {
        const DT	* const	sf1r = sf + coef_index;
        const DT	* const	sf2r = sf1r + nbr_coef;
        DT			* const	dfr = df + coef_index;
        DT			* const	dfi = dfr + nbr_coef;

        // Extreme coefficients are always real
        dfr [0] = sf1r [0] + sf2r [0];
        dfi [0] = sf1r [0] - sf2r [0];	// dfr [nbr_coef] =
        dfr [h_nbr_coef] = sf1r [h_nbr_coef];
        dfi [h_nbr_coef] = sf2r [h_nbr_coef];

        // Others are conjugate complex numbers
        const DT * const	sf1i = sf1r + h_nbr_coef;
        const DT * const	sf2i = sf1i + nbr_coef;

        DT	 		v;
        for (long i = 1; i < h_nbr_coef; ++ i)
        {
            const DT	c = cos_ptr [i];					// cos (i*PI/nbr_coef);
            const DT	s = cos_ptr [h_nbr_coef - i];	// sin (i*PI/nbr_coef);

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



template <class DT>
void	FFTReal <DT>::compute_direct_pass_n_osc (DT df [], const DT sf [], int pass) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);
    assert (pass > TRIGO_BD_LIMIT);
    assert (pass < _nbr_bits);

    const long		nbr_coef = 1 << pass;
    const long		h_nbr_coef = nbr_coef >> 1;
    const long		d_nbr_coef = nbr_coef << 1;
    long				coef_index = 0;
    OscType &		osc = _trigo_osc [pass - (TRIGO_BD_LIMIT + 1)];
    do
    {
        const DT	* const	sf1r = sf + coef_index;
        const DT	* const	sf2r = sf1r + nbr_coef;
        DT			* const	dfr = df + coef_index;
        DT			* const	dfi = dfr + nbr_coef;

        osc.clear_buffers ();

        // Extreme coefficients are always real
        dfr [0] = sf1r [0] + sf2r [0];
        dfi [0] = sf1r [0] - sf2r [0];	// dfr [nbr_coef] =
        dfr [h_nbr_coef] = sf1r [h_nbr_coef];
        dfi [h_nbr_coef] = sf2r [h_nbr_coef];

        // Others are conjugate complex numbers
        const DT * const	sf1i = sf1r + h_nbr_coef;
        const DT * const	sf2i = sf1i + nbr_coef;
        DT	 		v;

        for (long i = 1; i < h_nbr_coef; ++ i)
        {
            osc.step ();
            const DT	c = osc.get_cos ();
            const DT	s = osc.get_sin ();

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
template <class DT>
void	FFTReal <DT>::compute_ifft_general (const DT f [], DT x []) const
{
    assert (f != 0);
    assert (f != use_buffer ());
    assert (x != 0);
    assert (x != use_buffer ());
    assert (x != f);

    const DT *		sf = f; // const_cast <DT *> (f); //TODO!
    DT *		df;
    DT *		df_temp;

    if (_nbr_bits & 1)
    {
        df = use_buffer ();
        df_temp = x;
    }
    else
    {
        df = x;
        df_temp = use_buffer ();
    }

    for (int pass = _nbr_bits - 1; pass >= 3; -- pass)
    {
        compute_inverse_pass_n (df, sf, pass);

        if (pass < _nbr_bits - 1)
        {
            DT	* const	temp_ptr = df;
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



template <class DT>
void	FFTReal <DT>::compute_inverse_pass_n (DT df [], const DT sf [], int pass) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);
    assert (pass >= 3);
    assert (pass < _nbr_bits);

    if (pass <= TRIGO_BD_LIMIT)
    {
        compute_inverse_pass_n_lut (df, sf, pass);
    }
    else
    {
        compute_inverse_pass_n_osc (df, sf, pass);
    }
}



template <class DT>
void	FFTReal <DT>::compute_inverse_pass_n_lut (DT df [], const DT sf [], int pass) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);
    assert (pass >= 3);
    assert (pass < _nbr_bits);

    const long		nbr_coef = 1 << pass;
    const long		h_nbr_coef = nbr_coef >> 1;
    const long		d_nbr_coef = nbr_coef << 1;
    long				coef_index = 0;
    const DT * const	cos_ptr = get_trigo_ptr (pass);
    do
    {
        const DT	* const	sfr = sf + coef_index;
        const DT	* const	sfi = sfr + nbr_coef;
        DT			* const	df1r = df + coef_index;
        DT			* const	df2r = df1r + nbr_coef;

        // Extreme coefficients are always real
        df1r [0] = sfr [0] + sfi [0];		// + sfr [nbr_coef]
        df2r [0] = sfr [0] - sfi [0];		// - sfr [nbr_coef]
        df1r [h_nbr_coef] = sfr [h_nbr_coef] * 2;
        df2r [h_nbr_coef] = sfi [h_nbr_coef] * 2;

        // Others are conjugate complex numbers
        DT * const	df1i = df1r + h_nbr_coef;
        DT * const	df2i = df1i + nbr_coef;
        for (long i = 1; i < h_nbr_coef; ++ i)
        {
            df1r [i] = sfr [i] + sfi [-i];		// + sfr [nbr_coef - i]
            df1i [i] = sfi [i] - sfi [nbr_coef - i];

            const DT	c = cos_ptr [i];					// cos (i*PI/nbr_coef);
            const DT	s = cos_ptr [h_nbr_coef - i];	// sin (i*PI/nbr_coef);
            const DT	vr = sfr [i] - sfi [-i];		// - sfr [nbr_coef - i]
            const DT	vi = sfi [i] + sfi [nbr_coef - i];

            df2r [i] = vr * c + vi * s;
            df2i [i] = vi * c - vr * s;
        }

        coef_index += d_nbr_coef;
    }
    while (coef_index < _length);
}



template <class DT>
void	FFTReal <DT>::compute_inverse_pass_n_osc (DT df [], const DT sf [], int pass) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);
    assert (pass > TRIGO_BD_LIMIT);
    assert (pass < _nbr_bits);

    const long		nbr_coef = 1 << pass;
    const long		h_nbr_coef = nbr_coef >> 1;
    const long		d_nbr_coef = nbr_coef << 1;
    long				coef_index = 0;
    OscType &		osc = _trigo_osc [pass - (TRIGO_BD_LIMIT + 1)];
    do
    {
        const DT	* const	sfr = sf + coef_index;
        const DT	* const	sfi = sfr + nbr_coef;
        DT			* const	df1r = df + coef_index;
        DT			* const	df2r = df1r + nbr_coef;

        osc.clear_buffers ();

        // Extreme coefficients are always real
        df1r [0] = sfr [0] + sfi [0];		// + sfr [nbr_coef]
        df2r [0] = sfr [0] - sfi [0];		// - sfr [nbr_coef]
        df1r [h_nbr_coef] = sfr [h_nbr_coef] * 2;
        df2r [h_nbr_coef] = sfi [h_nbr_coef] * 2;

        // Others are conjugate complex numbers
        DT * const	df1i = df1r + h_nbr_coef;
        DT * const	df2i = df1i + nbr_coef;
        for (long i = 1; i < h_nbr_coef; ++ i)
        {
            df1r [i] = sfr [i] + sfi [-i];		// + sfr [nbr_coef - i]
            df1i [i] = sfi [i] - sfi [nbr_coef - i];

            osc.step ();
            const DT	c = osc.get_cos ();
            const DT	s = osc.get_sin ();
            const DT	vr = sfr [i] - sfi [-i];		// - sfr [nbr_coef - i]
            const DT	vi = sfi [i] + sfi [nbr_coef - i];

            df2r [i] = vr * c + vi * s;
            df2i [i] = vi * c - vr * s;
        }

        coef_index += d_nbr_coef;
    }
    while (coef_index < _length);
}



template <class DT>
void	FFTReal <DT>::compute_inverse_pass_3 (DT df [], const DT sf []) const
{
    assert (df != 0);
    assert (sf != 0);
    assert (df != sf);

    const DT	sqrt2_2 = SQRT2 * 0.5;
    long				coef_index = 0;
    do
    {
        df [coef_index] = sf [coef_index] + sf [coef_index + 4];
        df [coef_index + 4] = sf [coef_index] - sf [coef_index + 4];
        df [coef_index + 2] = sf [coef_index + 2] * 2;
        df [coef_index + 6] = sf [coef_index + 6] * 2;

        df [coef_index + 1] = sf [coef_index + 1] + sf [coef_index + 3];
        df [coef_index + 3] = sf [coef_index + 5] - sf [coef_index + 7];

        const DT	vr = sf [coef_index + 1] - sf [coef_index + 3];
        const DT	vi = sf [coef_index + 5] + sf [coef_index + 7];

        df [coef_index + 5] = (vr + vi) * sqrt2_2;
        df [coef_index + 7] = (vi - vr) * sqrt2_2;

        coef_index += 8;
    }
    while (coef_index < _length);
}



template <class DT>
void	FFTReal <DT>::compute_inverse_pass_1_2 (DT x [], const DT sf []) const
{
    assert (x != 0);
    assert (sf != 0);
    assert (x != sf);

    const long *	bit_rev_lut_ptr = get_br_ptr ();
    const DT *	sf2 = sf;
    long				coef_index = 0;
    do
    {
        {
            const DT	b_0 = sf2 [0] + sf2 [2];
            const DT	b_2 = sf2 [0] - sf2 [2];
            const DT	b_1 = sf2 [1] * 2;
            const DT	b_3 = sf2 [3] * 2;

            x [bit_rev_lut_ptr [0]] = b_0 + b_1;
            x [bit_rev_lut_ptr [1]] = b_0 - b_1;
            x [bit_rev_lut_ptr [2]] = b_2 + b_3;
            x [bit_rev_lut_ptr [3]] = b_2 - b_3;
        }
        {
            const DT	b_0 = sf2 [4] + sf2 [6];
            const DT	b_2 = sf2 [4] - sf2 [6];
            const DT	b_1 = sf2 [5] * 2;
            const DT	b_3 = sf2 [7] * 2;

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





#endif // FFTUNREAL_H
