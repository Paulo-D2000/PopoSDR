#include <Types.h>
#include <Constants.h>

#include <vector>

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