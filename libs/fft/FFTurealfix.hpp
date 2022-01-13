#ifndef FFTUREALFIX_HPP
#define FFTUREALFIX_HPP

#include "FFTCommon.hpp"


template <class T, int LL2>
class FFTRealFixLen
{
    typedef	int	CompileTimeCheck1 [(LL2 >=  0) ? 1 : -1];
    typedef	int	CompileTimeCheck2 [(LL2 <= 30) ? 1 : -1];

    using OscType =	OscSinCos<T>;

public:

    static const uint32_t FFT_LEN_L2 = LL2;
    static const uint32_t FFT_LEN = 1 << FFT_LEN_L2;

    FFTRealFixLen ();

    inline long get_length () const;
    void do_fft (T f [], const T x []);
    void do_ifft (const T f [], T x []);
    void rescale (T x []) const;

    FFTRealFixLen (const FFTRealFixLen &other) = delete;
    FFTRealFixLen& operator = (const FFTRealFixLen &other) = delete;
    bool operator == (const FFTRealFixLen &other) = delete;
    bool operator != (const FFTRealFixLen &other) = delete;

private:

    static const uint32_t TRIGO_BD_LIMIT = 12; //TODO into common

    static const uint32_t BR_ARR_SIZE_L2 = ((FFT_LEN_L2 - 3) < 0) ? 0 : (FFT_LEN_L2 - 2); //TODO rename all
    static const uint32_t BR_ARR_SIZE = 1 << BR_ARR_SIZE_L2;
    static const uint32_t TRIGO_BD	= ((FFT_LEN_L2 - TRIGO_BD_LIMIT) < 0) ? (int)FFT_LEN_L2 : (int)TRIGO_BD_LIMIT;
    static const uint32_t TRIGO_TABLE_ARR_SIZE_L2	= (LL2 < 4) ? 0 : (TRIGO_BD - 2);
    static const uint32_t TRIGO_TABLE_ARR_SIZE	= 1 << TRIGO_TABLE_ARR_SIZE_L2;
    static const uint32_t NBR_TRIGO_OSC = FFT_LEN_L2 - TRIGO_BD;
    static const uint32_t TRIGO_OSC_ARR_SIZE = (NBR_TRIGO_OSC > 0) ? NBR_TRIGO_OSC : 1;

    void build_br_lut();
    void build_trigo_lut();
    void build_trigo_osc();

    DynArray<T> _buffer;
    DynArray<long> _br_data;
    DynArray<T> _trigo_data;
    Array<OscType, TRIGO_OSC_ARR_SIZE> _trigo_osc;

};	// class FFTRealFixLen




template <class T, int PASS>
class FFTRealPassDirect
{

public:
    using OscType =	OscSinCos<T>;

    inline static void process (long len, T dest_ptr [], T src_ptr [], const T x_ptr [],
        const T cos_ptr [], long cos_len, const long br_ptr [], OscType osc_list []);

    FFTRealPassDirect () = delete;
    FFTRealPassDirect (const FFTRealPassDirect& other) = delete;
    FFTRealPassDirect& operator = (const FFTRealPassDirect& other) = delete;
    bool operator == (const FFTRealPassDirect& other) = delete;
    bool operator != (const FFTRealPassDirect& other) = delete;
};




template <class T, int PASS>
class FFTRealPassInverse
{

public:

    using OscType =	OscSinCos<T>;

    inline static void process (long len, T dest_ptr [], T src_ptr [], const T f_ptr [],
                        const T cos_ptr [], long cos_len, const long br_ptr [], OscType osc_list []);

    inline static void process_rec (long len, T dest_ptr [], T src_ptr [],
                        const T cos_ptr [], long cos_len, const long br_ptr [], OscType osc_list []);

    inline static void process_internal (long len, T dest_ptr [], const T src_ptr [], const T cos_ptr [],
                                          long cos_len, const long br_ptr [], OscType osc_list []);

    FFTRealPassInverse () = delete;
    FFTRealPassInverse (const FFTRealPassInverse &other) = delete;
    FFTRealPassInverse& operator = (const FFTRealPassInverse &other) = delete;
    bool operator == (const FFTRealPassInverse &other) = delete;
    bool operator != (const FFTRealPassInverse &other) = delete;

};	// class FFTRealPassInverse




template <class T, int P>
class FFTRealSelect
{

public:

    inline static T* sel_bin(T* e_ptr, T* o_ptr);

    FFTRealSelect () = delete;
    ~FFTRealSelect () = delete;
    FFTRealSelect (const FFTRealSelect& other) = delete;
    FFTRealSelect& operator = (const FFTRealSelect& other) = delete;
    bool operator == (const FFTRealSelect& other) = delete;
    bool operator != (const FFTRealSelect& other) = delete;

};	// class FFTRealSelect




template <class T, int ALGO>
class FFTRealUseTrigo
{

public:

    using OscType =	OscSinCos<T>;

    inline static void prepare (OscType &osc);
    inline static void iterate (OscType &osc, T &c, T &s,
                                const T cos_ptr [], long index_c, long index_s);

    FFTRealUseTrigo () = delete;
    ~FFTRealUseTrigo () = delete;
    FFTRealUseTrigo (const FFTRealUseTrigo &other) = delete;
    FFTRealUseTrigo& operator = (const FFTRealUseTrigo &other) = delete;
    bool operator == (const FFTRealUseTrigo &other) = delete;
    bool operator != (const FFTRealUseTrigo &other) = delete;

};	// class FFTRealUseTrigo





//FFRealSelect impl

template <class T, int P>
T*	FFTRealSelect <T, P>::sel_bin ([[maybe_unused]] T *e_ptr, T *o_ptr) {
    return o_ptr;
}



template <>
inline class T* FFTRealSelect <T, 0>::sel_bin (T *e_ptr, [[maybe_unused]] T *o_ptr) {
    return e_ptr;
}



//F invers impl


template <class T, int PASS>
void FFTRealPassInverse <T, PASS>::process (long len, T dest_ptr [], T src_ptr [],
                const T f_ptr [], const T cos_ptr [], long cos_len, const long br_ptr [], OscType osc_list [])
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
    FFTRealPassInverse <T, PASS - 1>::process_rec (
        len,
        src_ptr,
        dest_ptr,
        cos_ptr,
        cos_len,
        br_ptr,
        osc_list
    );
}



template <class T, int PASS>
void FFTRealPassInverse <T, PASS>::process_rec (long len, T dest_ptr [], T src_ptr [],
                                const T cos_ptr [], long cos_len, const long br_ptr [], OscType osc_list [])
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
    FFTRealPassInverse <T, PASS - 1>::process_rec (
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
inline void	FFTRealPassInverse <T, 0>::process_rec ([[maybe_unused]] long len, [[maybe_unused]] T dest_ptr [],
    [[maybe_unused]] T src_ptr [], [[maybe_unused]] const T cos_ptr [], [[maybe_unused]] long cos_len,
    [[maybe_unused]] const long br_ptr [], [[maybe_unused]] OscType osc_list [])
{
} // Stops recursion



template <class T, int PASS>
void FFTRealPassInverse <T, PASS>::process_internal (long len, T dest_ptr [], const T src_ptr [], const T cos_ptr [],
                                                  long cos_len, [[maybe_unused]] const long br_ptr [], OscType osc_list [])
{
    const long dist = 1L << (PASS - 1);
    const long c1_r = 0;
    const long c1_i = dist;
    const long c2_r = dist * 2;
    const long c2_i = dist * 3;
    const long cend = dist * 4;
    const long table_step = cos_len >> (PASS - 1);

    const uint32_t TRIGO_OSC = PASS - 12; //FFTRealFixLenParam::TRIGO_BD_LIMIT TODO to FFTCommon
    const uint32_t TRIGO_DIRECT = (TRIGO_OSC >= 0) ? 1 : 0;

    long coef_index = 0;

    do
    {
        const T* const sf = src_ptr + coef_index;
        T* const df = dest_ptr + coef_index;

        df [c1_r] = sf [c1_r] + sf [c2_r]; // Extreme coefficients are always real
        df [c2_r] = sf [c1_r] - sf [c2_r];
        df [c1_i] = sf [c1_i] * 2;
        df [c2_i] = sf [c2_i] * 2;

        FFTRealUseTrigo <T, TRIGO_DIRECT>::prepare (osc_list [TRIGO_OSC]);

        for (long i = 1; i < dist; ++ i) // Others are conjugate complex numbers
        {
            df [c1_r + i] = sf [c1_r + i] + sf [c2_r - i];
            df [c1_i + i] = sf [c2_r + i] - sf [cend - i];

            T c;
            T s;
            FFTRealUseTrigo <T, TRIGO_DIRECT>::iterate (
                osc_list [TRIGO_OSC],
                c, s, cos_ptr, i * table_step,
                (dist - i) * table_step
            );

            const T vr = sf [c1_r + i] - sf [c2_r - i];
            const T vi = sf [c2_r + i] + sf [cend - i];

            df [c2_r + i] = vr * c + vi * s;
            df [c2_i + i] = vi * c - vr * s;
        }

        coef_index += cend;
    }
    while (coef_index < len);
}


/*
template<>
inline void	FFTRealPassInverse <T, 2>::process_internal (long len, T dest_ptr [], const T src_ptr [],
                    [[maybe_unused]] const T cos_ptr [], [[maybe_unused]] long cos_len,
                    [[maybe_unused]] const long br_ptr [], [[maybe_unused]] OscType osc_list [])
{
    // Antepenultimate pass
    const T sqrt2_2 = SQRT2 * 0.5;

    long				coef_index = 0;
    do
    {
        dest_ptr [coef_index    ] = src_ptr [coef_index] + src_ptr [coef_index + 4];
        dest_ptr [coef_index + 4] = src_ptr [coef_index] - src_ptr [coef_index + 4];
        dest_ptr [coef_index + 2] = src_ptr [coef_index + 2] * 2;
        dest_ptr [coef_index + 6] = src_ptr [coef_index + 6] * 2;

        dest_ptr [coef_index + 1] = src_ptr [coef_index + 1] + src_ptr [coef_index + 3];
        dest_ptr [coef_index + 3] = src_ptr [coef_index + 5] - src_ptr [coef_index + 7];

        const T	vr = src_ptr [coef_index + 1] - src_ptr [coef_index + 3];
        const T	vi = src_ptr [coef_index + 5] + src_ptr [coef_index + 7];

        dest_ptr [coef_index + 5] = (vr + vi) * sqrt2_2;
        dest_ptr [coef_index + 7] = (vi - vr) * sqrt2_2;

        coef_index += 8;
    }
    while (coef_index < len);
}


template <>
inline void	FFTRealPassInverse <T, 1>::process_internal (long len, T dest_ptr [], const T src_ptr [],
                        [[maybe_unused]] const T cos_ptr [], [[maybe_unused]] long cos_len, const long br_ptr [],
                        [[maybe_unused]] OscType osc_list [])
{
    // Penultimate and last pass at once
    const long qlen = len >> 2;
    long coef_index = 0;

    do
    {
        const long ri_0 = br_ptr [coef_index >> 2];

        const T	b_0 = src_ptr [coef_index    ] + src_ptr [coef_index + 2];
        const T	b_2 = src_ptr [coef_index    ] - src_ptr [coef_index + 2];
        const T	b_1 = src_ptr [coef_index + 1] * 2;
        const T	b_3 = src_ptr [coef_index + 3] * 2;

        dest_ptr [ri_0           ] = b_0 + b_1;
        dest_ptr [ri_0 + 2 * qlen] = b_0 - b_1;
        dest_ptr [ri_0 + 1 * qlen] = b_2 + b_3;
        dest_ptr [ri_0 + 3 * qlen] = b_2 - b_3;

        coef_index += 4;
    }
    while (coef_index < len);
}
*/

//Failed yet to implement <> functions

template <class T, int PASS>
void FFTRealPassDirect <T, PASS>::process (long len, T dest_ptr [], T src_ptr [], const T x_ptr [],
                            const T cos_ptr [], long cos_len, const long br_ptr [], OscType osc_list [])
{
    // Executes "previous" passes first. Inverts source and destination buffers

    if (PASS == 0) {
        return;
    }

    if (PASS == 1) {
        const long qlen = len >> 2;
        long coef_index = 0;
        do
        {
            // To do: unroll the loop (2x).
            const long		ri_0 = br_ptr [coef_index >> 2];
            const long		ri_1 = ri_0 + 2 * qlen;	// bit_rev_lut_ptr [coef_index + 1];
            const long		ri_2 = ri_0 + 1 * qlen;	// bit_rev_lut_ptr [coef_index + 2];
            const long		ri_3 = ri_0 + 3 * qlen;	// bit_rev_lut_ptr [coef_index + 3];

            T	* const	df2 = dest_ptr + coef_index;
            df2 [1] = x_ptr [ri_0] - x_ptr [ri_1];
            df2 [3] = x_ptr [ri_2] - x_ptr [ri_3];

            const T	sf_0 = x_ptr [ri_0] + x_ptr [ri_1];
            const T	sf_2 = x_ptr [ri_2] + x_ptr [ri_3];

            df2 [0] = sf_0 + sf_2;
            df2 [2] = sf_0 - sf_2;

            coef_index += 4;
        }
        while (coef_index < len);
        return;
    }

    if (PASS == 2) {
        FFTRealPassDirect <T, 1>::process (
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
        const T	sqrt2_2 = T (SQRT2 * 0.5);

        long				coef_index = 0;
        do
        {
            dest_ptr [coef_index    ] = src_ptr [coef_index] + src_ptr [coef_index + 4];
            dest_ptr [coef_index + 4] = src_ptr [coef_index] - src_ptr [coef_index + 4];
            dest_ptr [coef_index + 2] = src_ptr [coef_index + 2];
            dest_ptr [coef_index + 6] = src_ptr [coef_index + 6];

            T v;

            v = (src_ptr [coef_index + 5] - src_ptr [coef_index + 7]) * sqrt2_2;
            dest_ptr [coef_index + 1] = src_ptr [coef_index + 1] + v;
            dest_ptr [coef_index + 3] = src_ptr [coef_index + 1] - v;

            v = (src_ptr [coef_index + 5] + src_ptr [coef_index + 7]) * sqrt2_2;
            dest_ptr [coef_index + 5] = v + src_ptr [coef_index + 3];
            dest_ptr [coef_index + 7] = v - src_ptr [coef_index + 3];

            coef_index += 8;
        }
        while (coef_index < len);
        return;
    }

    FFTRealPassDirect <T, PASS - 1>::process (
        len,
        src_ptr,
        dest_ptr,
        x_ptr,
        cos_ptr,
        cos_len,
        br_ptr,
        osc_list
    );

    const long dist = 1L << (PASS - 1);
    const long c1_r = 0;
    const long c1_i = dist;
    const long c2_r = dist * 2;
    const long c2_i = dist * 3;
    const long cend = dist * 4;
    const long table_step = cos_len >> (PASS - 1);

    const uint32_t TRIGO_OSC = PASS - 12; //TODO const
    const uint32_t TRIGO_DIRECT	= (TRIGO_OSC >= 0) ? 1 : 0;

    long				coef_index = 0;
    do
    {
        const T* const sf = src_ptr + coef_index;
        T* const df = dest_ptr + coef_index;


        df [c1_r] = sf [c1_r] + sf [c2_r]; // Extreme coefficients are always real
        df [c2_r] = sf [c1_r] - sf [c2_r];
        df [c1_i] = sf [c1_i];
        df [c2_i] = sf [c2_i];

        FFTRealUseTrigo <T, TRIGO_DIRECT>::prepare (osc_list [TRIGO_OSC]);

        for (long i = 1; i < dist; ++ i) // Others are conjugate complex numbers
        {
            T c;
            T s;
            FFTRealUseTrigo <T, TRIGO_DIRECT>::iterate (
                osc_list [TRIGO_OSC],
                c, s, cos_ptr, i * table_step,
                (dist - i) * table_step
            );

            const T sf_r_i = sf [c1_r + i];
            const T sf_i_i = sf [c1_i + i];

            const T v1 = sf [c2_r + i] * c - sf [c2_i + i] * s;
            df [c1_r + i] = sf_r_i + v1;
            df [c2_r - i] = sf_r_i - v1;

            const T v2 = sf [c2_r + i] * s + sf [c2_i + i] * c;
            df [c2_r + i] = v2 + sf_i_i;
            df [cend - i] = v2 - sf_i_i;
        }

        coef_index += cend;
    }
    while (coef_index < len);
}





template <class T, int LL2>
FFTRealFixLen <T, LL2>::FFTRealFixLen ()
    :	_buffer (FFT_LEN)
    ,	_br_data (BR_ARR_SIZE)
    ,	_trigo_data (TRIGO_TABLE_ARR_SIZE)
    ,	_trigo_osc ()
{
    build_br_lut ();
    build_trigo_lut ();
    build_trigo_osc ();
}



template <class T, int LL2>
long	FFTRealFixLen <T, LL2>::get_length () const {
    return FFT_LEN;
}



// General case
template <class T, int LL2>
void	FFTRealFixLen <T, LL2>::do_fft (T f [], const T x [])
{

    if (LL2 == 2) {
        assert (f != 0);
        assert (x != 0);
        assert (x != f);

        f [1] = x [0] - x [2];
        f [3] = x [1] - x [3];

        const T	b_0 = x [0] + x [2];
        const T	b_2 = x [1] + x [3];

        f [0] = b_0 + b_2;
        f [2] = b_0 - b_2;
        return;
    }

    if (LL2 == 1) {

        assert (f != 0);
        assert (x != 0);
        assert (x != f);

        f [0] = x [0] + x [1];
        f [1] = x [0] - x [1];
        return;
    }

    if (LL2 == 0) {
        assert (f != 0);
        assert (x != 0);

        f [0] = x [0];
        return;
    }

    assert (f != 0);
    assert (x != 0);
    assert (x != f);
    assert (FFT_LEN_L2 >= 3);

    // Do the transform in several passes
    const T* cos_ptr = &_trigo_data [0];
    const long* br_ptr = &_br_data [0];

    FFTRealPassDirect <T, FFT_LEN_L2 - 1>::process (
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




// General case
template <class T, int LL2>
void	FFTRealFixLen <T, LL2>::do_ifft (const T f [], T x [])
{

    if (LL2 == 2) {
        assert (f != 0);
        assert (x != 0);
        assert (x != f);

        const T	b_0 = f [0] + f [2];
        const T	b_2 = f [0] - f [2];

        x [0] = b_0 + f [1] * 2;
        x [2] = b_0 - f [1] * 2;
        x [1] = b_2 + f [3] * 2;
        x [3] = b_2 - f [3] * 2;
        return;
    }
    if (LL2 == 1) {
        assert (f != 0);
        assert (x != 0);
        assert (x != f);
        x [0] = f [0] + f [1];
        x [1] = f [0] - f [1];
        return;
    }
    if (LL2 == 0) {
        assert (f != 0);
        assert (x != 0);
        assert (x != f);
        x [0] = f [0];
        return;
    }

    assert (f != 0);
    assert (x != 0);
    assert (x != f);
    assert (FFT_LEN_L2 >= 3);

    if (LL2 == 2) {
        ;
    }

    T* s_ptr = FFTRealSelect <T, FFT_LEN_L2 & 1>::sel_bin (&_buffer [0], x); // Do the transform in several passes
    T* d_ptr = FFTRealSelect <T, FFT_LEN_L2 & 1>::sel_bin (x, &_buffer [0]);
    const T* cos_ptr = &_trigo_data [0];
    const long* br_ptr = &_br_data [0];

    FFTRealPassInverse <T, FFT_LEN_L2 - 1>::process (
        FFT_LEN, d_ptr, s_ptr,
        f, cos_ptr,
        TRIGO_TABLE_ARR_SIZE,
        br_ptr, &_trigo_osc [0]
    );
}


template <class T, int LL2>
void FFTRealFixLen <T, LL2>::rescale (T x []) const
{
    assert (x != 0);

    const T	mul = 1.0 / FFT_LEN;

    if (FFT_LEN < 4)
    {
        long i = FFT_LEN - 1;
        do {
            x [i] *= mul;
            --i;
        }
        while (i >= 0);
    }

    else
    {
        assert ((FFT_LEN & 3) == 0);
        long i = FFT_LEN - 4;
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




template <class T, int LL2>
void	FFTRealFixLen <T, LL2>::build_br_lut ()
{
    _br_data [0] = 0;
    for (long cnt = 1; cnt < BR_ARR_SIZE; ++cnt)
    {
        long index = cnt << 2;
        long br_index = 0;

        int bit_cnt = FFT_LEN_L2;
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



template <class T, int LL2>
void FFTRealFixLen <T, LL2>::build_trigo_lut ()
{
    const double mul = (0.5 * PI) / TRIGO_TABLE_ARR_SIZE;
    for (long i = 0; i < TRIGO_TABLE_ARR_SIZE; ++ i)
        _trigo_data [i] = std::cos (i * mul);
}



template <class T, int LL2>
void FFTRealFixLen <T, LL2>::build_trigo_osc ()
{
    for (int i = 0; i < NBR_TRIGO_OSC; ++i) {
        OscType& osc = _trigo_osc [i];
        const long len = static_cast <long> (TRIGO_TABLE_ARR_SIZE) << (i + 1);
        const double mul = (0.5 * PI) / len;
        osc.set_step (mul);
    }
}






#endif // FFTUREALFIX_HPP
