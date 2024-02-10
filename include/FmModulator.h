#pragma once

#include <Block.h>
#include <Types.h>
#include <Constants.h>
#include <Logging.h>

class FmModulator: public SyncBlock<F32,CF32>
{
private:
    F32 m_deviation;
    F32 m_phase;
    F32 m_freq;
public:
    FmModulator(float DeviationHz=1.0f, size_t SampleRate=1);

    size_t work(const size_t& n_inputItems, std::vector<F32>&  input, std::vector<CF32>& output);

    void updateParams(const float& DeviationHz, const size_t& SampleRate);

    ~FmModulator();
};