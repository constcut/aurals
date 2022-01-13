#ifndef FFTCOMMON_HPP
#define FFTCOMMON_HPP


#include <cstdint>
#include <cassert>
#include <cmath>
#include <type_traits>


template <class T>
class DynArray //TODO try compare uint\int on many cycles
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


#endif // FFTCOMMON_HPP
