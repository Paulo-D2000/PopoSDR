#include <Filters.h>
#include <Logging.h>

std::vector<F32> Generate_Gaussian_LPF(float Ts, float BTb, float Fs){
    int Ntaps = int(2.0f * Fs * Ts);
    if(Ntaps % 2 == 0)
        Ntaps++;
    std::vector<F32> taps(Ntaps);
    int Ntaps_1_2 = Ntaps/2;

    float sigma_2 = 2.0f * logf(2.0f) / ((M_TWOPI_F * BTb)*(M_TWOPI_F * BTb));
    float ampl = 1.0f / sqrtf(sigma_2 * M_PI_F);

    /* Calculate Taps */
    float tapsSum = 0.0f;
    for(int i=0; i<Ntaps; i++){
        float t = (float)(i - Ntaps_1_2) / (float)(Ntaps_1_2);
        t *= Ts;
        taps[i] = ampl * expf(-(t * t) / sigma_2);
        tapsSum += taps[i];
    }

    /* Normalize taps */
    for (size_t i = 0; i < Ntaps; i++)
    {
        taps[i] /= tapsSum;
    }

    LOG_DEBUG("Generated Gaussian FIR Taps.");
    LOG_DEBUG("  Params:");
    LOG_DEBUG("    Ts:  {}",Ts);
    LOG_DEBUG("    Btb: {}",BTb);
    LOG_DEBUG("    Fs:  {}",Fs);
    
    return taps;
}

std::vector<F32> Generate_Generic_LPF(float SampleRate, float Cuttof, int Span, WindowType window){
    float Ts = SampleRate/Cuttof;
    int Ntaps = int(2.0f * (float)Span * Ts);
    if(Ntaps % 2==0)
        Ntaps++;
    std::vector<F32> taps(Ntaps);
    int Ntaps_1_2 = Ntaps / 2;

    /* Calculate Taps */
    float tapsSum = 0.0f;
    for(int i=0; i<Ntaps; i++){
        float t = (float)(i - Ntaps_1_2) / (float)(Ntaps_1_2);
        taps[i] = sinc(2.0f * t * (float)Span) * getWindow(t, window);
        tapsSum += taps[i];
    }
    
    /* Normalize Taps */
    for (size_t i = 0; i < Ntaps; i++)
    {
        taps[i] /= tapsSum;
    }

    LOG_DEBUG("Generated Lowpass FIR Taps.");
    LOG_DEBUG("  Params:");
    LOG_DEBUG("    SampleRate: {} Hz",SampleRate);
    LOG_DEBUG("    Cuttof:     {} Hz",Cuttof);
    LOG_DEBUG("    Span:       {} symbols",Span);
    LOG_DEBUG("    Window:     {}",getWindowName(window));
    LOG_DEBUG("    NumTaps:    {}",Ntaps);
    
    return taps;
}