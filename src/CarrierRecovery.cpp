#include <CarrierRecovery.h>
#include <Constants.h>
#include <Utils.h>

CarrierRecovery::CarrierRecovery(const std::vector<CF32> &ConstellationPoints, float Damping, float LoopBw, const size_t &BufferSize) : SyncBlock(BufferSize), m_cloop(Damping, LoopBw), m_constellation(std::move(ConstellationPoints))
{
    this->m_name = "CarrierRecovery";
    LOG_DEBUG("Created Carrier Recovery");
}

CF32 CarrierRecovery::make_decision(const CF32 &sample)
{
    // Point
    CF32 dec = m_constellation[0];
    
    // Compute (squared) distance from sample to const points
    float distance = 1000000.0f;
    size_t idx = 0;
    for (size_t i = 0; i < m_constellation.size(); i++)
    {
        dec = m_constellation[i];
        float dist = Magn2(sample - dec);
        if(dist < distance){
            distance = dist;
            idx = i;
        }
    }

    return m_constellation[idx];
}

size_t CarrierRecovery::work(const size_t &n_inputItems, std::vector<CF32> &input, std::vector<CF32> &output)
{
    size_t OutputIdx = 0;
    for (size_t i = 0; i < n_inputItems; i++)
    {
        CF32 out = output[OutputIdx++] = input[i] * std::exp(-_1j * M_PI_F * m_cloop.getLast());
        float error = std::arg(out * std::conj(make_decision(out)));
        m_cloop.update(error);
    }
    return OutputIdx;
}

CarrierRecovery::~CarrierRecovery()
{
    LOG_DEBUG("Destroyed Carrier Recovery");
}
