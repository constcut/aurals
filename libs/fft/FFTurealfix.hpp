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




template <int P, class T>
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



//FFRealSelect

template <int P, class T>
T*	FFTRealSelect <P, T>::sel_bin ([[maybe_unused]] T *e_ptr, T *o_ptr) {
    return (o_ptr);
}



template <>
inline class T* FFTRealSelect <0, T>::sel_bin (T *e_ptr, [[maybe_unused]] T *o_ptr) {
    return (e_ptr);
}




#endif // FFTUREALFIX_HPP
