#include <TimingPLL.h>
#include <Utils.h>
#include <ControlLoop.h>

template <typename T>                                                                                                                               
TimingPLL<T>::TimingPLL(const size_t& SampleRate, const size_t& SymbolRate, const float& Alpha, const size_t& BufferSize): SyncBlock<T>(BufferSize, SampleRate/SymbolRate, 1), m_alpha(1.0f-Alpha)
,error_stream(BufferSize) {
    double sps = (double)SampleRate / (double)SymbolRate;
    m_step = (I32)((256.0 * 256.0 * 256.0 * 256.0) / sps);

    m_counter = 0;
    m_pcounter = 0;
    m_prevInput = T(0.0f);

    LOG_DEBUG("Created Timing Recovery PLL");
    LOG_DEBUG("Alpha: {}",m_alpha);
    LOG_DEBUG("SampleRate: {}",SampleRate);
    LOG_DEBUG("SymbolRate: {}",SymbolRate);
    LOG_DEBUG("Step: {}",m_step);
}

F32 hyst = 0.1f;
template <>
size_t TimingPLL<F32>::work(const size_t& n_inputItems, std::vector<F32>&  input, std::vector<F32>& output){
    size_t outputIdx = 0;
    bool decision = false, prev_decision = false;
    for (size_t i = 0; i < n_inputItems; i++)
    {
        F32 inp = input.at(i); // Get input

        // Counter Overflow -> SAMPLE
        if((m_counter < 0) && (m_pcounter > 0)){
            if(outputIdx >= output.size()) break;
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
        m_prevprevInput = m_prevInput;
        m_pcounter = m_counter;
        m_counter += m_step; 
    }
    return outputIdx;
}

ControlLoop cloop(0.707f, 0.0628/10.0);
template <>
size_t TimingPLL<CF32>::work(const size_t& n_inputItems, std::vector<CF32>&  input, std::vector<CF32>& output){
    size_t outputIdx = 0;
    F32 error = 0, cmd = 0, mu = 0, frac = 0;
    float sps = 10;
    float dev = 0.005;
    bool decision = false, prev_decision = false;
    for (size_t i = 0; i < n_inputItems; i++)
    {
        CF32 inp = input.at(i); // Get input 

        // Counter Overflow -> SAMPLE
        if((m_counter < 0) && (m_pcounter > 0)){
            if(outputIdx >= output.size()) break;
            output.at(outputIdx++) = inp;
            error = (m_prevprevInput.real() - inp.real()) * m_prevInput.real() +
                    (m_prevprevInput.imag() - inp.imag()) * m_prevInput.imag();
            cmd = std::clamp(cloop.update(error), -1.0f, 1.0f);
            //error_stream.writeToBuffer({{mu,error}}, 1);
            mu = floorf(cmd);
            frac = cmd - mu;
            m_step = I32(std::pow(256.0, 4.0) / std::clamp(sps-mu, sps-sps*dev, sps+sps*dev));
            //m_counter = I32(mu * 0.05 * INT32_MAX);
        }
        
        m_prevInput = inp;
        m_prevprevInput = m_prevInput;
        m_pcounter = m_counter;
        m_counter += m_step;
    }
    return outputIdx;
}

template <typename T>
TimingPLL<T>::~TimingPLL(){
    LOG_DEBUG("Destroyed Timing Recovery PLL");
}

template class TimingPLL<CF32>;
template class TimingPLL<F32>;