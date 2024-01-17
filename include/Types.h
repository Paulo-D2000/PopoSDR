#pragma once 
#include <complex>
#include <vector>

typedef float F32;
typedef std::complex<F32> CF32;

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;

typedef int8_t  I8;
typedef int16_t I16;
typedef int32_t I32;

inline F32 Magn2(const CF32& x){
    return x.real()*x.real() +  x.imag()*x.imag();
}

inline F32 Magn2(const F32& x){
    return x*x;
}

template<typename T>
inline int Sign(const T& val){
    return (T(0) < val) - (val < T(0));
}

template<typename T>
inline F32 Vec_AvgPwr(const std::vector<T> &inputData){
    float spwr = 0.0f;
    for (auto &s : inputData)
        spwr += Magn2(s);
    spwr /= (float) inputData.size();
    return sqrtf(spwr);
}

template<typename T>
inline F32 Vec_PeakPwr(const std::vector<T> &inputData){
    float spwr = 0.0f;
    for (auto &s : inputData)
        spwr = std::max<F32>(spwr,std::abs(s));
    return spwr;
}