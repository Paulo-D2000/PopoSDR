#include <Types.h>
#include <Constants.h>

#include <vector>

template<typename T>
inline int Sign(const T& val){
    return (T(0) < val) - (val < T(0));
}

inline CF32 Sign_CF(const CF32& val){
    return CF32((0.0f < val.real()) - (val.real() < 0.0f), (0.0f < val.imag()) - (val.imag() < 0.0f));
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

/* This bounds x by +/- clip without a branch */
static inline float branchless_clip(float x, float clip)
{
    return 0.5 * (std::abs(x + clip) - std::abs(x - clip));
}