#include <FmModulator.h>

FmModulator::FmModulator(float DeviationHz, size_t SampleRate) : SyncBlock(131072, 1, 1) {
    m_name = "FmModulator";
    LOG_DEBUG("Created Fm Modulator.");
    m_deviation = M_TWOPI_F * DeviationHz/(float)SampleRate;
    m_phase = 0.0f;
    m_freq = m_deviation;
}


size_t FmModulator::work(const size_t& n_inputItems, std::vector<F32>&  input, std::vector<CF32>& output)
{
    size_t outputIdx = 0;
    for (size_t i = 0; i < n_inputItems; i++)
    {
        F32 inp = input.at(i);
        output.at(outputIdx++) = std::exp(_1j * m_phase);
        m_freq = inp * m_deviation;
        m_phase += m_freq;
    }
    return outputIdx;
}

void FmModulator::updateParams(const float& DeviationHz, const size_t& SampleRate){
    m_deviation = M_TWOPI_F * DeviationHz/(float)SampleRate;
    m_phase = 0.0f;
    m_freq = m_deviation;
}

FmModulator::~FmModulator(){
    LOG_DEBUG("Destroyed Fm Modulator.");
}