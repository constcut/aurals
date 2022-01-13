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
    long _len;

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

    long get_length () const;
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

    inline const long* get_br_ptr () const;
    inline const T* get_trigo_ptr (int level) const;
    inline long get_trigo_level_index (int level) const;

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


    const long _length;
    const int _nbr_bits;
    DynArray <long> _br_lut; //TODO review is there long or uint32_t?
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
    static_assert (_len > 0, "DynArray zero len not expected");
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




#endif // FFTUNREAL_H
