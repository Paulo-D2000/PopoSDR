#pragma once

#include <Block.h>
#include <FirFilter.h>
#include <ControlLoop.h>

class BandEdgeFLL: public SyncBlock<CF32>
{
private:
    size_t m_sps;
    ControlLoop m_cloop;
    FirFilter<CF32, CF32> m_be_filter_p;
    FirFilter<CF32, CF32> m_be_filter_n;

public:
    BandEdgeFLL(size_t Sps, float Damping = 0.707f, float LoopBw = 0.0628f, const size_t& BufferSize=0);

    size_t work(const size_t& n_inputItems, std::vector<CF32>&  input, std::vector<CF32>& output);

    ~BandEdgeFLL();
};
