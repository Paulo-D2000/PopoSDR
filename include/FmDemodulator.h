#pragma once

#include <Block.h>
#include <Constants.h>
#include <algorithm>
#include <numeric>

class FmDemodulator: public SyncBlock<CF32,F32>
{
private:
    size_t mNumCoeffs = 5;
    F32 m_deviation;
    std::vector<CF32> mDelayLine;
    std::vector<CF32> mFifo;
    std::vector<F32> mCoeffs;

public:
    FmDemodulator(float DeviationHz=1.0f, size_t SampleRate=1);

    size_t work(const size_t& n_inputItems, std::vector<CF32>&  input, std::vector<F32>& output);

    ~FmDemodulator();
};