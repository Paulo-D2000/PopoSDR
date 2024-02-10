#pragma once

#include <Block.h>

template <typename T>
class TimingPLL: public SyncBlock<T>
{
private:
    I32 m_step;
    I32 m_counter;
    I32 m_pcounter;
    
    F32 m_alpha;
    T m_prevInput;
    T m_prevprevInput;
public:
    TimingPLL(const size_t& SampleRate, const size_t& SymbolRate, const float& Alpha=0.75f, const size_t& BufferSize=131072);

    size_t work(const size_t& n_inputItems, std::vector<T>&  input, std::vector<T>& output);

    Stream<CF32> error_stream;

    ~TimingPLL();
};
