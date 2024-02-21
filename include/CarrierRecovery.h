#pragma once

#include <Block.h>
#include <ControlLoop.h>

class CarrierRecovery: public SyncBlock<CF32>
{
private:
    ControlLoop m_cloop;
    Constellation m_constellation;

public:
    CarrierRecovery(const Constellation& ConstellationObj, float Damping = 0.707f, float LoopBw = 0.0628f, const size_t& BufferSize=0);

    size_t work(const size_t& n_inputItems, std::vector<CF32>&  input, std::vector<CF32>& output);

    ~CarrierRecovery();
};
