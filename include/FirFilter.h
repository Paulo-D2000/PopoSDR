#pragma once

#include <vector>
#include <Types.h>
#include <Buffer.h>
#include <Logging.h>
#include <Block.h>


struct FirRate{
    size_t Interpolation=1;
    size_t Decimation=1;
};

template <typename T>
class FirFilter: public SyncBlock<T,T>
{
public:
    FirFilter(const std::vector<F32>& taps={0.0f}, const FirRate& rate={1,1}, const size_t& BufferSize=0);
    
    void loadTaps(const std::vector<F32>& taps);
    T filter(const T& input);

    size_t work(const size_t& n_inputItems, std::vector<T>&  input, std::vector<T>& output);

    ~FirFilter();

private:
    size_t m_ptr;
    FirRate m_rate;
    std::vector<F32> m_taps;
    std::vector<T> m_buffer;
};

template<typename T>
class PolyPhaseFIR: public SyncBlock<T>
{
public:
    PolyPhaseFIR(const std::vector<F32>& taps={0.0f}, const FirRate& rate={1,1}, const size_t& BufferSize=0);

    size_t work(const size_t& n_inputItems, std::vector<T>&  input, std::vector<T>& output);

    ~PolyPhaseFIR();

private:
    std::vector<FirFilter<T>*> m_filterbank;
    FirRate m_rate;
};
