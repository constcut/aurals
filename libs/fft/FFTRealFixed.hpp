#ifndef FFTREALFIXED_H
#define FFTREALFIXED_H


#include <cstdint>
#include <cassert>
#include <cmath>
#include <type_traits>


const double PI = 3.1415926535897932384626433832795;
const double SQRT2 = 1.41421356237309514547462185873883;


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




template <class T, long length>
class Array
{
public:

    Array () = default;

    inline const T& operator [] (long pos) const;
    inline T& operator [] (long pos);
    static inline long size ();

    Array (const Array& other) = delete;
    Array& operator = (const Array& other) = delete;
    bool operator == (const Array& other) = delete;
    bool operator != (const Array& other) = delete;

private:

    T _data_arr[length];

};	// class Array





template <class T>
class OscSinCos
{

public:

    OscSinCos ();

    inline void set_step (double angle_rad);

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


//Array implementation


template <class T, long length>
const T& Array <T, length>::operator [] (long pos) const
{
    assert (pos >= 0);
    assert (pos < length);
    return (_data_arr [pos]);
}


template <class T, long length>
T& Array <T, length>::operator [] (long pos)
{
    assert (pos >= 0);
    assert (pos < length);
    return (_data_arr [pos]);
}


template <class T, long length>
long Array <T, length>::size () {
    return (length);
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


#define DataType float

//First just fast copy


template <int ALGO>
class FFTRealUseTrigo
{

public:

    typedef	OscSinCos <DataType>	OscType;

    inline static void
                        prepare (OscType &osc);
    inline	static void
                        iterate (OscType &osc, DataType &c, DataType &s, const DataType cos_ptr [], long index_c, long index_s);

private:

                        FFTRealUseTrigo ();
                        ~FFTRealUseTrigo ();
                        FFTRealUseTrigo (const FFTRealUseTrigo &other);
    FFTRealUseTrigo &
                        operator = (const FFTRealUseTrigo &other);
    bool				operator == (const FFTRealUseTrigo &other);
    bool				operator != (const FFTRealUseTrigo &other);

};	// class FFTRealUseTrigo




template <int ALGO>
void	FFTRealUseTrigo <ALGO>::prepare (OscType &osc)
{
    osc.clear_buffers ();
}

template <>
inline void	FFTRealUseTrigo <0>::prepare ([[maybe_unused]]OscType &osc)
{
    // Nothing
}



template <int ALGO>
void	FFTRealUseTrigo <ALGO>::iterate (OscType &osc, DataType &c, DataType &s, [[maybe_unused]] const DataType cos_ptr [], [[maybe_unused]] long index_c, [[maybe_unused]] long index_s)
{
    osc.step ();
    c = osc.get_cos ();
    s = osc.get_sin ();
}

template <>
inline void	FFTRealUseTrigo <0>::iterate ([[maybe_unused]] OscType &osc, DataType &c, DataType &s, const DataType cos_ptr [], long index_c, long index_s)
{
    c = cos_ptr [index_c];
    s = cos_ptr [index_s];
}




template <int P>
class FFTRealSelect
{

public:

    inline static float *
                        sel_bin (float *e_ptr, float *o_ptr);

private:

                        FFTRealSelect ();
                        ~FFTRealSelect ();
                        FFTRealSelect (const FFTRealSelect &other);
    FFTRealSelect&	operator = (const FFTRealSelect &other);
    bool				operator == (const FFTRealSelect &other);
    bool				operator != (const FFTRealSelect &other);

};	// class FFTRealSelect




template <int P>
float *	FFTRealSelect <P>::sel_bin ([[maybe_unused]] float *e_ptr, float *o_ptr)
{
    return (o_ptr);
}



template <>
inline float *	FFTRealSelect <0>::sel_bin (float *e_ptr, [[maybe_unused]] float *o_ptr)
{
    return (e_ptr);
}




template <int PASS>
class FFTRealPassInverse
{

public:


    typedef	OscSinCos <DataType>	OscType;

    inline static void
                        process (long len, DataType dest_ptr [], DataType src_ptr [], const DataType f_ptr [], const DataType cos_ptr [],
                                 long cos_len, const uint32_t br_ptr [], OscType osc_list []);
    inline static void
                        process_rec (long len, DataType dest_ptr [], DataType src_ptr [], const DataType cos_ptr [],
                                     long cos_len, const uint32_t br_ptr [], OscType osc_list []);
    inline static void
                        process_internal (long len, DataType dest_ptr [], const DataType src_ptr [], const DataType cos_ptr [],
                                          long cos_len, const uint32_t br_ptr [], OscType osc_list []);


private:

                        FFTRealPassInverse ();
                        FFTRealPassInverse (const FFTRealPassInverse &other);
    FFTRealPassInverse &
                        operator = (const FFTRealPassInverse &other);
    bool				operator == (const FFTRealPassInverse &other);
    bool				operator != (const FFTRealPassInverse &other);

};	// class FFTRealPassInverse


template <int PASS>
void	FFTRealPassInverse <PASS>::process (long len, DataType dest_ptr [], DataType src_ptr [],
                                            const DataType f_ptr [], const DataType cos_ptr [], long cos_len,
                                            const uint32_t br_ptr [], OscType osc_list [])
{
    process_internal (
        len,
        dest_ptr,
        f_ptr,
        cos_ptr,
        cos_len,
        br_ptr,
        osc_list
    );
    FFTRealPassInverse <PASS - 1>::process_rec (
        len,
        src_ptr,
        dest_ptr,
        cos_ptr,
        cos_len,
        br_ptr,
        osc_list
    );
}



template <int PASS>
void	FFTRealPassInverse <PASS>::process_rec (long len, DataType dest_ptr [], DataType src_ptr [],
                                                const DataType cos_ptr [], long cos_len, const uint32_t br_ptr [], OscType osc_list [])
{
    process_internal (
        len,
        dest_ptr,
        src_ptr,
        cos_ptr,
        cos_len,
        br_ptr,
        osc_list
    );
    FFTRealPassInverse <PASS - 1>::process_rec (
        len,
        src_ptr,
        dest_ptr,
        cos_ptr,
        cos_len,
        br_ptr,
        osc_list
    );
}

template <>
inline void	FFTRealPassInverse <0>::process_rec ([[maybe_unused]] long len, [[maybe_unused]]
DataType dest_ptr [], [[maybe_unused]] DataType src_ptr [], [[maybe_unused]] const DataType cos_ptr [],
[[maybe_unused]] long cos_len, [[maybe_unused]] const uint32_t br_ptr [], [[maybe_unused]] OscType osc_list [])
{
    // Stops recursion
}



template <int PASS>
void	FFTRealPassInverse <PASS>::process_internal (long len, DataType dest_ptr [], const DataType src_ptr [],
                                                     const DataType cos_ptr [], long cos_len,
                                                     [[maybe_unused]] const uint32_t br_ptr [], OscType osc_list [])
{
    const uint32_t		dist = 1L << (PASS - 1);
    const uint32_t		c1_r = 0;
    const uint32_t		c1_i = dist;
    const uint32_t		c2_r = dist * 2;
    const uint32_t		c2_i = dist * 3;
    const uint32_t		cend = dist * 4;
    const uint32_t		table_step = cos_len >> (PASS - 1);

   enum {	TRIGO_OSC		= PASS - 12	};
    enum {	TRIGO_DIRECT	= (TRIGO_OSC >= 0) ? 1 : 0	};

    uint32_t				coef_index = 0;
    do
    {
        const DataType	* const	sf = src_ptr + coef_index;
        DataType			* const	df = dest_ptr + coef_index;

        // Extreme coefficients are always real
        df [c1_r] = sf [c1_r] + sf [c2_r];
        df [c2_r] = sf [c1_r] - sf [c2_r];
        df [c1_i] = sf [c1_i] * 2;
        df [c2_i] = sf [c2_i] * 2;

        FFTRealUseTrigo <TRIGO_DIRECT>::prepare (osc_list [TRIGO_OSC]);

        // Others are conjugate complex numbers
        for (uint32_t i = 1; i < dist; ++ i)
        {
            df [c1_r + i] = sf [c1_r + i] + sf [c2_r - i];
            df [c1_i + i] = sf [c2_r + i] - sf [cend - i];

            DataType			c;
            DataType			s;
            FFTRealUseTrigo <TRIGO_DIRECT>::iterate (
                osc_list [TRIGO_OSC],
                c,
                s,
                cos_ptr,
                i * table_step,
                (dist - i) * table_step
            );

            const DataType	vr = sf [c1_r + i] - sf [c2_r - i];
            const DataType	vi = sf [c2_r + i] + sf [cend - i];

            df [c2_r + i] = vr * c + vi * s;
            df [c2_i + i] = vi * c - vr * s;
        }

        coef_index += cend;
    }
    while (coef_index < len);
}

template <>
inline void	FFTRealPassInverse <2>::process_internal (long len, DataType dest_ptr [], const DataType src_ptr [],
                                                      [[maybe_unused]] const DataType cos_ptr [],
        [[maybe_unused]] long cos_len, [[maybe_unused]] const uint32_t br_ptr [], [[maybe_unused]] OscType osc_list [])
{
    // Antepenultimate pass
    const DataType	sqrt2_2 = DataType (SQRT2 * 0.5);

    uint32_t				coef_index = 0;
    do
    {
        dest_ptr [coef_index    ] = src_ptr [coef_index] + src_ptr [coef_index + 4];
        dest_ptr [coef_index + 4] = src_ptr [coef_index] - src_ptr [coef_index + 4];
        dest_ptr [coef_index + 2] = src_ptr [coef_index + 2] * 2;
        dest_ptr [coef_index + 6] = src_ptr [coef_index + 6] * 2;

        dest_ptr [coef_index + 1] = src_ptr [coef_index + 1] + src_ptr [coef_index + 3];
        dest_ptr [coef_index + 3] = src_ptr [coef_index + 5] - src_ptr [coef_index + 7];

        const DataType	vr = src_ptr [coef_index + 1] - src_ptr [coef_index + 3];
        const DataType	vi = src_ptr [coef_index + 5] + src_ptr [coef_index + 7];

        dest_ptr [coef_index + 5] = (vr + vi) * sqrt2_2;
        dest_ptr [coef_index + 7] = (vi - vr) * sqrt2_2;

        coef_index += 8;
    }
    while (coef_index < len);
}

template <>
inline void	FFTRealPassInverse <1>::process_internal (long len, DataType dest_ptr [], const DataType src_ptr [],
                                                      [[maybe_unused]] const DataType cos_ptr [], [[maybe_unused]] long cos_len,
    const uint32_t br_ptr [], [[maybe_unused]] OscType osc_list [])
{
    // Penultimate and last pass at once
    const uint32_t		qlen = len >> 2;

    uint32_t				coef_index = 0;
    do
    {
        const uint32_t		ri_0 = br_ptr [coef_index >> 2];

        const DataType	b_0 = src_ptr [coef_index    ] + src_ptr [coef_index + 2];
        const DataType	b_2 = src_ptr [coef_index    ] - src_ptr [coef_index + 2];
        const DataType	b_1 = src_ptr [coef_index + 1] * 2;
        const DataType	b_3 = src_ptr [coef_index + 3] * 2;

        dest_ptr [ri_0           ] = b_0 + b_1;
        dest_ptr [ri_0 + 2 * qlen] = b_0 - b_1;
        dest_ptr [ri_0 + 1 * qlen] = b_2 + b_3;
        dest_ptr [ri_0 + 3 * qlen] = b_2 - b_3;

        coef_index += 4;
    }
    while (coef_index < len);
}




template <int PASS>
class FFTRealPassDirect
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

    typedef	OscSinCos <DataType>	OscType;

    inline static void
                        process (long len, DataType dest_ptr [], DataType src_ptr [], const DataType x_ptr [],
                                 const DataType cos_ptr [], long cos_len, const uint32_t br_ptr [], OscType osc_list []);


private:

                        FFTRealPassDirect ();
                        FFTRealPassDirect (const FFTRealPassDirect &other);
    FFTRealPassDirect &
                        operator = (const FFTRealPassDirect &other);
    bool				operator == (const FFTRealPassDirect &other);
    bool				operator != (const FFTRealPassDirect &other);

};	// class FFTRealPassDirect





template <>
inline void	FFTRealPassDirect <1>::process (long len, DataType dest_ptr [], [[maybe_unused]] DataType src_ptr [],
const DataType x_ptr [], [[maybe_unused]] const DataType cos_ptr [], [[maybe_unused]] long cos_len, const uint32_t br_ptr [], [[maybe_unused]] OscType osc_list [])
{
    // First and second pass at once
    const uint32_t		qlen = len >> 2;

    uint32_t				coef_index = 0;
    do
    {
        // To do: unroll the loop (2x).
        const uint32_t		ri_0 = br_ptr [coef_index >> 2];
        const uint32_t		ri_1 = ri_0 + 2 * qlen;	// bit_rev_lut_ptr [coef_index + 1];
        const uint32_t		ri_2 = ri_0 + 1 * qlen;	// bit_rev_lut_ptr [coef_index + 2];
        const uint32_t		ri_3 = ri_0 + 3 * qlen;	// bit_rev_lut_ptr [coef_index + 3];

        DataType	* const	df2 = dest_ptr + coef_index;
        df2 [1] = x_ptr [ri_0] - x_ptr [ri_1];
        df2 [3] = x_ptr [ri_2] - x_ptr [ri_3];

        const DataType	sf_0 = x_ptr [ri_0] + x_ptr [ri_1];
        const DataType	sf_2 = x_ptr [ri_2] + x_ptr [ri_3];

        df2 [0] = sf_0 + sf_2;
        df2 [2] = sf_0 - sf_2;

        coef_index += 4;
    }
    while (coef_index < len);
}

template <>
inline void	FFTRealPassDirect <2>::process (long len, DataType dest_ptr [], DataType src_ptr [],
                                            const DataType x_ptr [], const DataType cos_ptr [],
                                            long cos_len, const uint32_t br_ptr [], OscType osc_list [])
{
    // Executes "previous" passes first. Inverts source and destination buffers
    FFTRealPassDirect <1>::process (
        len,
        src_ptr,
        dest_ptr,
        x_ptr,
        cos_ptr,
        cos_len,
        br_ptr,
        osc_list
    );

    // Third pass
    const DataType	sqrt2_2 = SQRT2 * 0.5;
    DataType v;

    uint32_t				coef_index = 0;
    do
    {
        dest_ptr [coef_index    ] = src_ptr [coef_index] + src_ptr [coef_index + 4];
        dest_ptr [coef_index + 4] = src_ptr [coef_index] - src_ptr [coef_index + 4];
        dest_ptr [coef_index + 2] = src_ptr [coef_index + 2];
        dest_ptr [coef_index + 6] = src_ptr [coef_index + 6];

        v = (src_ptr [coef_index + 5] - src_ptr [coef_index + 7]) * sqrt2_2;
        dest_ptr [coef_index + 1] = src_ptr [coef_index + 1] + v;
        dest_ptr [coef_index + 3] = src_ptr [coef_index + 1] - v;

        v = (src_ptr [coef_index + 5] + src_ptr [coef_index + 7]) * sqrt2_2;
        dest_ptr [coef_index + 5] = v + src_ptr [coef_index + 3];
        dest_ptr [coef_index + 7] = v - src_ptr [coef_index + 3];

        coef_index += 8;
    }
    while (coef_index < len);
}

template <int PASS>
void	FFTRealPassDirect <PASS>::process (long len, DataType dest_ptr [], DataType src_ptr [],
                                           const DataType x_ptr [], const DataType cos_ptr [],
                                           long cos_len, const uint32_t br_ptr [], OscType osc_list [])
{
    // Executes "previous" passes first. Inverts source and destination buffers
    FFTRealPassDirect <PASS - 1>::process (
        len,
        src_ptr,
        dest_ptr,
        x_ptr,
        cos_ptr,
        cos_len,
        br_ptr,
        osc_list
    );

    const uint32_t		dist = 1L << (PASS - 1);
    const uint32_t		c1_r = 0;
    const uint32_t		c1_i = dist;
    const uint32_t		c2_r = dist * 2;
    const uint32_t		c2_i = dist * 3;
    const uint32_t		cend = dist * 4;
    const uint32_t		table_step = cos_len >> (PASS - 1);

   enum {	TRIGO_OSC		= PASS - 12	};
    enum {	TRIGO_DIRECT	= (TRIGO_OSC >= 0) ? 1 : 0	};

    uint32_t				coef_index = 0;
    do
    {
        const DataType	* const	sf = src_ptr + coef_index;
        DataType			* const	df = dest_ptr + coef_index;

        // Extreme coefficients are always real
        df [c1_r] = sf [c1_r] + sf [c2_r];
        df [c2_r] = sf [c1_r] - sf [c2_r];
        df [c1_i] = sf [c1_i];
        df [c2_i] = sf [c2_i];

        FFTRealUseTrigo <TRIGO_DIRECT>::prepare (osc_list [TRIGO_OSC]);

        // Others are conjugate complex numbers
        for (uint32_t i = 1; i < dist; ++ i)
        {
            DataType			c;
            DataType			s;
            FFTRealUseTrigo <TRIGO_DIRECT>::iterate (
                osc_list [TRIGO_OSC],
                c,
                s,
                cos_ptr,
                i * table_step,
                (dist - i) * table_step
            );

            const DataType	sf_r_i = sf [c1_r + i];
            const DataType	sf_i_i = sf [c1_i + i];

            const DataType	v1 = sf [c2_r + i] * c - sf [c2_i + i] * s;
            df [c1_r + i] = sf_r_i + v1;
            df [c2_r - i] = sf_r_i - v1;

            const DataType	v2 = sf [c2_r + i] * s + sf [c2_i + i] * c;
            df [c2_r + i] = v2 + sf_i_i;
            df [cend - i] = v2 - sf_i_i;
        }

        coef_index += cend;
    }
    while (coef_index < len);
}





template <int LL2>
class FFTRealFixLen
{
    typedef	int	CompileTimeCheck1 [(LL2 >=  0) ? 1 : -1];
    typedef	int	CompileTimeCheck2 [(LL2 <= 30) ? 1 : -1];

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

    typedef	OscSinCos <DataType>	OscType;

    enum {			FFT_LEN_L2	= LL2	};
    enum {			FFT_LEN		= 1 << FFT_LEN_L2	};

                        FFTRealFixLen ();

    inline uint32_t		get_length () const;
    void				do_fft (DataType f [], const DataType x []);
    void				do_ifft (const DataType f [], DataType x []);
    void				rescale (DataType x []) const;



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

    enum {			TRIGO_BD_LIMIT	= 12	};

    enum {			BR_ARR_SIZE_L2	= ((FFT_LEN_L2 - 3) < 0) ? 0 : (FFT_LEN_L2 - 2)	};
    enum {			BR_ARR_SIZE		= 1 << BR_ARR_SIZE_L2	};

   enum {			TRIGO_BD			=   ((FFT_LEN_L2 - TRIGO_BD_LIMIT) < 0)
                                              ? (int)FFT_LEN_L2
                                              : (int)TRIGO_BD_LIMIT };
    enum {			TRIGO_TABLE_ARR_SIZE_L2	= (LL2 < 4) ? 0 : (TRIGO_BD - 2)	};
    enum {			TRIGO_TABLE_ARR_SIZE	= 1 << TRIGO_TABLE_ARR_SIZE_L2	};

    enum {			NBR_TRIGO_OSC			= FFT_LEN_L2 - TRIGO_BD	};
    enum {			TRIGO_OSC_ARR_SIZE	=	(NBR_TRIGO_OSC > 0) ? NBR_TRIGO_OSC : 1	};

    void				build_br_lut ();
    void				build_trigo_lut ();
    void				build_trigo_osc ();

    DynArray <DataType>
                        _buffer;
    DynArray <uint32_t>
                        _br_data;
    DynArray <DataType>
                        _trigo_data;
   Array <OscType, TRIGO_OSC_ARR_SIZE>
                        _trigo_osc;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

                        FFTRealFixLen (const FFTRealFixLen &other);
    FFTRealFixLen&	operator = (const FFTRealFixLen &other);
    bool				operator == (const FFTRealFixLen &other);
    bool				operator != (const FFTRealFixLen &other);

};	// class FFTRealFixLen




template <int LL2>
FFTRealFixLen <LL2>::FFTRealFixLen ()
:	_buffer (FFT_LEN)
,	_br_data (BR_ARR_SIZE)
,	_trigo_data (TRIGO_TABLE_ARR_SIZE)
,	_trigo_osc ()
{
    build_br_lut ();
    build_trigo_lut ();
    build_trigo_osc ();
}



template <int LL2>
uint32_t FFTRealFixLen<LL2>::get_length() const
{
    return (FFT_LEN);
}



// General case
template <int LL2>
void	FFTRealFixLen <LL2>::do_fft (DataType f [], const DataType x [])
{
    assert (f != 0);
    assert (x != 0);
    assert (x != f);
    assert (FFT_LEN_L2 >= 3);

    // Do the transform in several passes
    const DataType	*	cos_ptr = &_trigo_data [0];
    const uint32_t *	br_ptr = &_br_data [0];

    FFTRealPassDirect <FFT_LEN_L2 - 1>::process (
        FFT_LEN,
        f,
        &_buffer [0],
        x,
        cos_ptr,
        TRIGO_TABLE_ARR_SIZE,
        br_ptr,
        &_trigo_osc [0]
    );
}

// 4-point FFT
template <>
inline void	FFTRealFixLen <2>::do_fft (DataType f [], const DataType x [])
{
    assert (f != 0);
    assert (x != 0);
    assert (x != f);

    f [1] = x [0] - x [2];
    f [3] = x [1] - x [3];

    const DataType	b_0 = x [0] + x [2];
    const DataType	b_2 = x [1] + x [3];

    f [0] = b_0 + b_2;
    f [2] = b_0 - b_2;
}

// 2-point FFT
template <>
inline void	FFTRealFixLen <1>::do_fft (DataType f [], const DataType x [])
{
    assert (f != 0);
    assert (x != 0);
    assert (x != f);

    f [0] = x [0] + x [1];
    f [1] = x [0] - x [1];
}

// 1-point FFT
template <>
inline void	FFTRealFixLen <0>::do_fft (DataType f [], const DataType x [])
{
    assert (f != 0);
    assert (x != 0);

    f [0] = x [0];
}



// General case
template <int LL2>
void	FFTRealFixLen <LL2>::do_ifft (const DataType f [], DataType x [])
{
    assert (f != 0);
    assert (x != 0);
    assert (x != f);
    assert (FFT_LEN_L2 >= 3);

    // Do the transform in several passes
    DataType *		s_ptr =
        FFTRealSelect <FFT_LEN_L2 & 1>::sel_bin (&_buffer [0], x);
    DataType *		d_ptr =
        FFTRealSelect <FFT_LEN_L2 & 1>::sel_bin (x, &_buffer [0]);
    const DataType	*	cos_ptr = &_trigo_data [0];
    const uint32_t *	br_ptr = &_br_data [0];

    FFTRealPassInverse <FFT_LEN_L2 - 1>::process (
        FFT_LEN,
        d_ptr,
        s_ptr,
        f,
        cos_ptr,
        TRIGO_TABLE_ARR_SIZE,
        br_ptr,
        &_trigo_osc [0]
    );
}

// 4-point IFFT
template <>
inline void	FFTRealFixLen <2>::do_ifft (const DataType f [], DataType x [])
{
    assert (f != 0);
    assert (x != 0);
    assert (x != f);

    const DataType	b_0 = f [0] + f [2];
    const DataType	b_2 = f [0] - f [2];

    x [0] = b_0 + f [1] * 2;
    x [2] = b_0 - f [1] * 2;
    x [1] = b_2 + f [3] * 2;
    x [3] = b_2 - f [3] * 2;
}

// 2-point IFFT
template <>
inline void	FFTRealFixLen <1>::do_ifft (const DataType f [], DataType x [])
{
    assert (f != 0);
    assert (x != 0);
    assert (x != f);

    x [0] = f [0] + f [1];
    x [1] = f [0] - f [1];
}

// 1-point IFFT
template <>
inline void	FFTRealFixLen <0>::do_ifft (const DataType f [], DataType x [])
{
    assert (f != 0);
    assert (x != 0);
    assert (x != f);

    x [0] = f [0];
}




template <int LL2>
void	FFTRealFixLen <LL2>::rescale (DataType x []) const
{
    assert (x != 0);

    const DataType	mul = 1.0 / FFT_LEN;

    if (FFT_LEN < 4)
    {
        long				i = FFT_LEN - 1;
        do
        {
            x [i] *= mul;
            --i;
        }
        while (i >= 0);
    }

    else
    {
        assert ((FFT_LEN & 3) == 0);

        // Could be optimized with SIMD instruction sets (needs alignment check)
        long				i = FFT_LEN - 4;
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



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



template <int LL2>
void	FFTRealFixLen <LL2>::build_br_lut ()
{
    _br_data [0] = 0;
    for (uint32_t cnt = 1; cnt < BR_ARR_SIZE; ++cnt)
    {
        uint32_t				index = cnt << 2;
        uint32_t				br_index = 0;

        int				bit_cnt = FFT_LEN_L2;
        do
        {
            br_index <<= 1;
            br_index += (index & 1);
            index >>= 1;

            -- bit_cnt;
        }
        while (bit_cnt > 0);

        _br_data [cnt] = br_index;
    }
}



template <int LL2>
void	FFTRealFixLen <LL2>::build_trigo_lut ()
{
    const double	mul = (0.5 * PI) / TRIGO_TABLE_ARR_SIZE;
    for (uint32_t i = 0; i < TRIGO_TABLE_ARR_SIZE; ++ i)
    {
        using namespace std;

        _trigo_data [i] = DataType (cos (i * mul));
    }
}



template <int LL2>
void	FFTRealFixLen <LL2>::build_trigo_osc ()
{
    for (int i = 0; i < NBR_TRIGO_OSC; ++i)
    {
        OscType &		osc = _trigo_osc [i];

        const uint32_t		len =  (TRIGO_TABLE_ARR_SIZE) << (i + 1);
        const double	mul = (0.5 * PI) / len;
        osc.set_step (mul);
    }
}





#endif // FFTREALFIXED_H
