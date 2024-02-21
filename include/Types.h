#pragma once 
#include <cstdint>
#include <complex>
#include <vector>

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

inline F32 Magn2(const CF32& x){
    return x.real()*x.real() +  x.imag()*x.imag();
}

inline F32 Magn2(const F32& x){
    return x*x;
}

struct Constellation{
    std::vector<CF32> points;

    Constellation(const std::vector<CF32>& Points): points(std::move(Points)){}

    float compute_distance(const CF32& sample, size_t& min_idx){
        min_idx = 0;
        float min_distance = Magn2(sample - points[0]);
        for (size_t idx = 1; idx < points.size(); idx++)
        {
            float dist = Magn2(sample - points[idx]);
            if(dist < min_distance){
                min_distance = dist;
                min_idx = idx;
            }
        }
        return min_distance;
    }

    CF32 make_decision(const CF32 &sample)
    {
        // Compute (squared) distance from sample to const points
        size_t min_idx = 0;
        compute_distance(sample, min_idx);
        return points[min_idx];
    }
};