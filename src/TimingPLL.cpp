#include <TimingPLL.h>
#include <Utils.h>
#include <ControlLoop.h>
#include <FirFilter.h>

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

static ControlLoop cloop(0.707f, 1/200.0f, 8, 1.0f);
static std::vector<FirFilter<CF32>*>interp;
static std::vector<FirFilter<CF32>*>interp_diff;
static Constellation const_obj({{-1,-1},{-1,1},{1,-1},{1,1}});//({{0.316228, -0.316228}, {-0.316228, -0.316228}, {0.948683, -0.948683}, {-0.948683, -0.948683}, {-0.948683, -0.316228}, {0.948683, -0.316228}, {-0.316228, -0.948683}, {0.316228, -0.948683}, {-0.948683, 0.948683}, {0.948683, 0.948683}, {-0.316228, 0.316228}, {0.316228, 0.316228}, {0.316228, 0.948683}, {-0.316228, 0.948683}, {0.948683, 0.316228}, {-0.948683, 0.316228}});
static bool build_firs = true;
static bool enable = true;
static float d_mu = 0.5;
static float counter = 0.0f;
static float error = 0.0f;
static float d_rate = 0.125f;
static CF32 d_decision[2] = {0,0};
static CF32 d_input_derivative[2] = {0,0};
static CF32 d_input[2] = {0,0};
static float cmd = 0.0f;
template <>
size_t TimingPLL<CF32>::work(const size_t& n_inputItems, std::vector<CF32>&  input, std::vector<CF32>& output){
    size_t m_k = 0;
    size_t outputIdx = 0, i = 0;
    size_t sym_idx = 0;
    if(build_firs){
        for (size_t nf = 0; nf < NSTEPS+1; nf++)
        {
            std::vector<float> t(&taps[nf][0], &taps[nf][NTAPS-1]);
            interp.push_back(new FirFilter<CF32>(t));

            std::vector<float> dt(&Dtaps[nf][0], &Dtaps[nf][DNTAPS-1]);
            interp_diff.push_back(new FirFilter<CF32>(dt));
        }
        build_firs = false;
    }
    while((i < n_inputItems) && (outputIdx < output.size()))
    {
        if(enable){
            // Compute interpolator index
            int imu = (int)rint(NSTEPS*(1-d_mu));

            // check bounds
            if((imu < 0) || (imu > NSTEPS)){
                LOG_ERROR("{} Out of range(0,{})",imu,NSTEPS);
                imu = 0;
            }

            // Update History && Interpolate input
            d_input[1] = d_input[0];
            d_input[0] = interp[imu]->filter(input.at(m_k));
            output.at(outputIdx++) = d_input[0];

            // Update History && Compute derivative
            d_input_derivative[1] = d_input_derivative[0];
            d_input_derivative[0] = interp_diff[imu]->filter(input.at(m_k));

            // Compute sample decisions (closest sample from constellation)
            d_decision[0] = const_obj.make_decision(d_input[0]);
            d_decision[1] = const_obj.make_decision(d_input[1]);

            // Calculate error (MM TED)
            error = (d_decision[1].real() * d_input[0].real() -
                           d_decision[0].real() * d_input[1].real()) +
                          (d_decision[1].imag() * d_input[0].imag() -
                           d_decision[0].imag() * d_input[1].imag());
            
            // Maybe Try (Sig * Sig') ML-TED
            if(1){
            error = (d_input[0].real() * d_input_derivative[0].real() +
                     d_input[0].imag() * d_input_derivative[0].imag()) / 
                     2.0f;
            }
            
            error = branchless_clip(error, 1.0f);
        }else{
            error = 0.0f;
        }

        // Update PI Loop Filter
        cmd = cloop.update(error);

        // Update rate estimate && keep deviation < 2.5%
        d_rate = (1.0f / m_sps) +  branchless_clip(cmd, 0.025f / m_sps);

        // Modulo-1 Counter (NCO)
        enable = counter < d_rate;
        if(enable){
            d_mu = (counter / d_rate) - 0.5;
            d_mu -= floorf(d_mu);
            m_k = i;
            error_stream.writeToBuffer({CF32(2*d_mu-1,50*m_sps*cmd*128)},1);
        }
        
        counter -= d_rate;
        counter = counter - floorf(counter);
        i++;
    }
    return outputIdx;
}

template <typename T>
TimingPLL<T>::~TimingPLL(){
    LOG_DEBUG("Destroyed Timing Recovery PLL");
}

template class TimingPLL<CF32>;
template class TimingPLL<F32>;