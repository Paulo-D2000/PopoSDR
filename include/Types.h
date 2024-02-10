#pragma once 
#include <cstdint>
#include <complex>

typedef float F32;
typedef std::complex<F32> CF32;

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t  I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

struct CI64{
    I64 real;
    I64 imag;

    void fromComplex(CF32 const& data){
        real = INT32_MAX * data.real();
        imag = INT32_MAX * data.imag();
    }

    CF32 toComplex(double scale = 1.0f){
        CF32 out = CF32( scale * (double)real / (double)INT64_MAX, scale *(double)imag / (double)INT64_MAX);
        return out;
    }

    CI64 operator-(CI64 const& other){
        CI64 result;
        result.real = real - other.real;
        result.imag = imag - other.imag;
        return result;
    }

    CI64 operator+(CI64 const& other){
        CI64 result;
        result.real = real + other.real;
        result.imag = imag + other.imag;
        return result;
    }
};