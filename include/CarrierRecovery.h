#pragma once

#include <Block.h>
#include <ControlLoop.h>

class CarrierRecovery: public SyncBlock<CF32>
{
private:
    ControlLoop m_cloop;
    std::vector<CF32> m_constellation;

    CF32 make_decision(const CF32& sample);

public:
    CarrierRecovery(const std::vector<CF32>& ConstellationPoints, float Damping = 0.707f, float LoopBw = 0.0628f, const size_t& BufferSize=0);

    size_t work(const size_t& n_inputItems, std::vector<CF32>&  input, std::vector<CF32>& output);

    ~CarrierRecovery();
};
