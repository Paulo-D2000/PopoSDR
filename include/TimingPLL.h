#pragma once

#include <Block.h>

enum TLL_Type {MMSE, PFB};

template <typename T>
class TimingPLL: public SyncBlock<T>
{
private:
    F32 m_sps;
    I32 m_step;
    I32 m_counter;
    I32 m_pcounter;
    
    F32 m_alpha;
    T m_prevInput;
    T m_prevprevInput;
    std::vector<std::vector<F32>> m_userTaps;
    TLL_Type m_type;
public:
    TimingPLL(const size_t& SampleRate, const size_t& SymbolRate, const float& Alpha=0.75f, const size_t& BufferSize=131072);

    TimingPLL(const size_t& SampleRate, const size_t& SymbolRate, TLL_Type Type=MMSE, std::vector<std::vector<F32>> PFB_Taps={}, const size_t& BufferSize=131072);

    size_t work(const size_t& n_inputItems, std::vector<T>&  input, std::vector<T>& output);

    Stream<CF32> error_stream;

    void Reset();

    ~TimingPLL();
};
