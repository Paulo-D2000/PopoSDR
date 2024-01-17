#include <TimingPLL.h>

TimmingPLL::TimmingPLL(const size_t& SampleRate, const size_t& SymbolRate, const float& Alpha): SyncBlock(131072, SampleRate/SymbolRate, 1), m_alpha(1.0f-Alpha) {
    double sps = (double)SampleRate / (double)SymbolRate;
    m_step = (I32)((256.0 * 256.0 * 256.0 * 256.0) / sps);

    m_counter = 0;
    m_pcounter = 0;
    m_prevInput = 0.0f;

    LOG_DEBUG("Created Timing Recovery PLL");
    LOG_DEBUG("Alpha: {}",m_alpha);
    LOG_DEBUG("SampleRate: {}",SampleRate);
    LOG_DEBUG("SymbolRate: {}",SymbolRate);
    LOG_DEBUG("Step: {}",m_step);
}

F32 hyst = 0.01;
size_t TimmingPLL::work(const size_t& n_inputItems, std::vector<F32>&  input, std::vector<F32>& output){
    size_t outputIdx = 0;
    bool decision = false, prev_decision = false;
    for (size_t i = 0; i < n_inputItems; i++)
    {
        m_pcounter = m_counter;
        m_counter += m_step; 
        F32 inp = input.at(i); // Get input

        // Counter Overflow -> SAMPLE
        if((m_counter < 0) && (m_pcounter > 0)){
            output.at(outputIdx++) = inp;
        }

        if(inp > hyst){
            decision = 1;
        }
        else if(inp < (-hyst)){
            decision = 0;
        }

        if(m_prevInput > hyst){
            prev_decision = 1;
        }
        else if(m_prevInput < (-hyst)){
            prev_decision = 0;
        }
        
        // Zero Crossing -> Nudge counter
        if(decision != prev_decision){
            m_counter = (I32)((F32)(m_counter) * m_alpha);
        }
        
        // Save state & advance counter
        m_prevInput = inp;
    }
    return outputIdx;
}

TimmingPLL::~TimmingPLL(){
    LOG_DEBUG("Destroyed Timing Recovery PLL");
}