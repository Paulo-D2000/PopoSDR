#include <TimingPLL.h>
#include <Utils.h>
#include <ControlLoop.h>
#include <FirFilter.h>
#include <functional>
#include <numeric>
#include <vector>

// MMSE Resampler PFB from GnuRadio
#include <interpolator_taps.h>
#include <interp_differentiator_taps.h>

template <typename T>                                                                                                                               
TimingPLL<T>::TimingPLL(const size_t& SampleRate, const size_t& SymbolRate, const float& Alpha, const size_t& BufferSize): SyncBlock<T>(BufferSize, SampleRate/SymbolRate, 1), m_alpha(1.0f-Alpha)
,error_stream(BufferSize) {
    double sps = m_sps = (double)SampleRate / (double)SymbolRate;
    m_step = (I32)((256.0 * 256.0 * 256.0 * 256.0) / sps);

    m_counter = 0;
    m_pcounter = 0;
    m_prevInput = T(0.0f);
    m_type = MMSE;

    LOG_DEBUG("Created Timing Recovery PLL");
    LOG_DEBUG("Alpha: %f",m_alpha);
    LOG_DEBUG("SampleRate: %d",SampleRate);
    LOG_DEBUG("SymbolRate: %d",SymbolRate);
    LOG_DEBUG("Step: %d",m_step);
}

template <typename T>                                                                                                                               
TimingPLL<T>::TimingPLL(const size_t& SampleRate, const size_t& SymbolRate, TLL_Type Type, std::vector<std::vector<F32>> PFB_Taps, const size_t& BufferSize): SyncBlock<T>(BufferSize, SampleRate/SymbolRate, 1), m_type(Type), m_userTaps(PFB_Taps)
,error_stream(BufferSize) {
    double sps = m_sps = (double)SampleRate / (double)SymbolRate;
    m_step = (I32)((256.0 * 256.0 * 256.0 * 256.0) / sps);

    m_counter = 0;
    m_pcounter = 0;
    m_prevInput = T(0.0f);

    LOG_DEBUG("Created Timing Recovery PLL");
    LOG_DEBUG("Alpha: %f",m_alpha);
    LOG_DEBUG("SampleRate: %d",SampleRate);
    LOG_DEBUG("SymbolRate: %d",SymbolRate);
    LOG_DEBUG("Step: %d",m_step);
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

static ControlLoop cloop(0.707f, 1.0f/100.0f, 8.0f, 8.0f);
static std::vector<FirFilter<CF32>*>interp;
static std::vector<FirFilter<CF32>*>interp_diff;
static Constellation const_obj({{-1,-1},{-1,1},{1,-1},{1,1}});//({{0.316228, -0.316228}, {-0.316228, -0.316228}, {0.948683, -0.948683}, {-0.948683, -0.948683}, {-0.948683, -0.316228}, {0.948683, -0.316228}, {-0.316228, -0.948683}, {0.316228, -0.948683}, {-0.948683, 0.948683}, {0.948683, 0.948683}, {-0.316228, 0.316228}, {0.316228, 0.316228}, {0.316228, 0.948683}, {-0.316228, 0.948683}, {0.948683, 0.316228}, {-0.948683, 0.316228}});
static bool build_firs = true;
static bool enable = true;
static float d_mu = 0.5f;
static float counter = 0.0f;
static float counter_next = 0.0f;
static float d_mu_next = 0.5f;
static float error = 0.0f;
static float d_rate = 0.125f;
static CF32 d_decision[2] = {0,0};
static CF32 d_input_derivative[2] = {0,0};
static CF32 d_input[2] = {0,0};
static float cmd = 0.0f;
template <>
size_t TimingPLL<CF32>::work(const size_t& n_inputItems, std::vector<CF32>&  input, std::vector<CF32>& output){
    if(build_firs){
        if(m_type == MMSE){
            for (size_t nf = 0; nf < NSTEPS+1; nf++)
            {
                std::vector<float> t(&taps[nf][0], &taps[nf][NTAPS-1]);
                interp.push_back(new FirFilter<CF32>(t));

                std::vector<float> dt(&Dtaps[nf][0], &Dtaps[nf][DNTAPS-1]);
                interp_diff.push_back(new FirFilter<CF32>(dt));
            }
        }
        else if(m_type == PFB){
            for (size_t nf = 0; nf < m_userTaps.size(); nf++)
            {
                interp.push_back(new FirFilter<CF32>(m_userTaps[nf]));

                std::vector<float> dt(m_userTaps[nf].size());
                for (size_t i = 1; i < dt.size()-1; i++)
                {
                    dt[i] = m_userTaps[nf][i+1] - m_userTaps[nf][i-1];
                }
                interp_diff.push_back(new FirFilter<CF32>(dt));
            }
        }
        else
        {
            LOG_ERROR("Uknown TLL Type!\n"); // ignored
        }
        build_firs = false;
    }
    size_t m_k = m_sps/2;
    size_t outputIdx = 0, i = 0;
    enable = true;
    while((i < (n_inputItems - interp[0]->getTaps().size())) && (outputIdx < output.size()))
    {
        counter = counter_next;
        d_mu = d_mu_next;

        // Compute interpolator index
        int imu = (int)rint((interp.size()-1)*d_mu);

        // check bounds
        if((imu < 0) || (imu > (interp.size()-1))){
            LOG_ERROR("%d Out of range(0,%d)",imu,interp.size()-1);
            imu = 0;
        }

        if(enable){
            // Update History && Interpolate input
            d_input[1] = d_input[0];
            //d_input[0] = interp[imu]->filter(input.at(m_k));
            auto itaps = interp[imu]->getTaps();
            auto dtaps = interp_diff[imu]->getTaps();
            d_input[0] = std::inner_product(input.begin()+m_k,
                                            input.begin()+m_k+itaps.size(),
                                            itaps.begin(), CF32(0,0));

            output.at(outputIdx++) = d_input[0];

            // Update History && Compute derivative
            d_input_derivative[1] = d_input_derivative[0];
            d_input_derivative[0] = std::inner_product(input.begin()+m_k,
                                            input.begin()+m_k+dtaps.size(),
                                            dtaps.begin(), CF32(0,0));

            // Compute sample decisions (closest sample from constellation)
            d_decision[0] = const_obj.make_decision(d_input[0]);
            d_decision[1] = const_obj.make_decision(d_input[1]);

            // Calculate error (MM TED)
            /*
            error = (d_decision[1].real() * d_input[0].real() -
                           d_decision[0].real() * d_input[1].real()) +
                          (d_decision[1].imag() * d_input[0].imag() -
                           d_decision[0].imag() * d_input[1].imag());
            */
            // Maybe Try Sign(x) * x' ML-TED
            /*
            error = ((d_input[0].real() < 0.0f ? -d_input_derivative[0].real()
                                               : d_input_derivative[0].real()) +
                     (d_input[0].imag() < 0.0f ? -d_input_derivative[0].imag()
                                               : d_input_derivative[0].imag())) / 
                    2.0f;
            */
            error = (d_input[0].real() * d_input_derivative[0].real() +
                    d_input[0].imag() * d_input_derivative[0].imag()) /
                    2.0f;
            
            error = branchless_clip(error, 1.0f);
            //error_stream.writeToBuffer({CF32(2.0f*d_mu-1.0f, error)},1);
        }else{
            error = 0.0f;
        }

        // Update PI Loop Filter
        float v = cloop.update(error);

        // Update rate estimate
        float W = (1.0f/m_sps) + branchless_clip(v, 0.05f/m_sps);

        // Modulo-1 Counter (NCO)
        counter_next = counter - W;
        if(counter_next < 0.0f){
            counter_next = 1.0f + counter_next;
            enable = 1;
            d_mu_next = counter / m_sps;
            m_k = i + m_sps/2 + 1;
        }
        else{
            d_mu_next = d_mu;
            enable = 0;
        }
        i++;
    }
    return outputIdx;
}

template <typename T>
void TimingPLL<T>::Reset(){
     m_step = (I32)((256.0 * 256.0 * 256.0 * 256.0) / m_sps);

    m_counter = 0;
    m_pcounter = 0;
    m_prevInput = T(0.0f);
}

template <typename T>
TimingPLL<T>::~TimingPLL(){
    LOG_DEBUG("Destroyed Timing Recovery PLL");
}

template class TimingPLL<CF32>;
template class TimingPLL<F32>;