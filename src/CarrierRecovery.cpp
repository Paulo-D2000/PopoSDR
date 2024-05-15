#include <CarrierRecovery.h>
#include <Constants.h>
#include <Utils.h>

CarrierRecovery::CarrierRecovery(const Constellation&ConstellationObj, float Damping, float LoopBw, const size_t &BufferSize):
 SyncBlock(BufferSize), m_cloop(Damping, LoopBw, 1.0f, 4.0f * M_PI_F * M_PI_F), m_constellation(ConstellationObj)
{
    this->m_name = "CarrierRecovery";
    LOG_DEBUG("Created Carrier Recovery");
}

size_t CarrierRecovery::work(const size_t &n_inputItems, std::vector<CF32> &input, std::vector<CF32> &output)
{
    size_t OutputIdx = 0;
    for (size_t i = 0; i < n_inputItems; i++)
    {
        CF32 out = output[OutputIdx++] = input[i] * std::exp(-_1j * M_PI_F * m_cloop.getLast());
        float error = std::arg(out * std::conj(m_constellation.make_decision(out)));
        m_cloop.update(error);
    }
    return OutputIdx;
}

CarrierRecovery::~CarrierRecovery()
{
    LOG_DEBUG("Destroyed Carrier Recovery");
}
