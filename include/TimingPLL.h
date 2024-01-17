#pragma once

#include <Block.h>

class TimmingPLL: public SyncBlock<F32>
{
private:
    I32 m_step;
    I32 m_counter;
    I32 m_pcounter;
    
    F32 m_alpha;
    F32 m_prevInput;
public:
    TimmingPLL(const size_t& SampleRate, const size_t& SymbolRate, const float& Alpha=0.75f);

    size_t work(const size_t& n_inputItems, std::vector<F32>&  input, std::vector<F32>& output);

    ~TimmingPLL();
};
