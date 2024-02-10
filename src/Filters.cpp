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

std::vector<F32> Generate_Generic_LPF(float SampleRate, float Cutoff, float Gain, int Span, WindowType window){
    float Ts = SampleRate/Cutoff;
    int Ntaps = int(2.0f * (float)Span * Ts);
    if(Ntaps % 2==0)
        Ntaps++;
    std::vector<F32> taps(Ntaps);
    int Ntaps_1_2 = Ntaps / 2;

    /* Calculate Taps */
    float tapsSum = 0.0f;
    for(int i=0; i<Ntaps; i++){
        float t = (float)(i - Ntaps_1_2) / (float)(Ntaps_1_2);
        taps[i] = sinc(2.0f * t * (float)Span) * getWindow(t * 0.5f + 0.5f, window);
        tapsSum += taps[i];
    }
    
    /* Normalize Taps */
    for (size_t i = 0; i < Ntaps; i++)
    {
        taps[i] *= Gain / tapsSum;
    }

    LOG_DEBUG("Generated Lowpass FIR Taps.");
    LOG_DEBUG("  Params:");
    LOG_DEBUG("    SampleRate: {} Hz",SampleRate);
    LOG_DEBUG("    Cutoff:     {} Hz",Cutoff);
    LOG_DEBUG("    Span:       {} symbols",Span);
    LOG_DEBUG("    Window:     {}",getWindowName(window));
    LOG_DEBUG("    NumTaps:    {}",Ntaps);
    
    return taps;
}

std::vector<F32> Generate_Root_Raised_Cosine(float SampleRate, float SymbolRate, float ExcessBw, float Gain, int Span)
{
    float Ts = SampleRate/SymbolRate;
    int Ntaps = int(2.0f * (float)Span * Ts);
    if(Ntaps % 2==0)
        Ntaps++;
    std::vector<F32> taps(Ntaps);
    int Ntaps_1_2 = Ntaps / 2;

    /* Calculate Taps */
    float tapsSum = 0.0f;
    for(int i=0; i<Ntaps; i++){
        float xindx = i - Ntaps_1_2;
        float x1 = M_PI_F * xindx / Ts;
        float x2 = 4.0f * ExcessBw * xindx / Ts;
        float x3 = x2 * x2 - 1.0f;
        float num, den;

        if(fabsf(x3) >= 0.000001f){
            if(i != Ntaps_1_2)
                num = cosf((1.0f+ExcessBw) * x1) + sinf((1.0f-ExcessBw) / (4.0f * ExcessBw * xindx / Ts));
            else
                num = cosf((1.0f + ExcessBw) * x1) + (1.0f - ExcessBw) * M_PI_F / (4.0f * ExcessBw);
            den = x3 * M_PI_F;
        }else{
            if(ExcessBw == 1.0f){
                taps[i] = -1;
                tapsSum += taps[i];
                continue;
            }
            x3 = (1.0f - ExcessBw) * x1;
            x2 = (1.0f + ExcessBw) * x1;
            num = (sinf(x2) * (1.0f + ExcessBw) * M_PI_F -
            cosf(x3) * ((1.0f - ExcessBw) * M_PI_F * Ts) / (4.0f * ExcessBw * xindx) +
            sinf(x3) * Ts * Ts / (4.0f * ExcessBw * xindx * xindx));
            den = -32.0f * M_PI_F * ExcessBw * ExcessBw * xindx / Ts;
        }
        taps[i] = 4.0f * ExcessBw * num / den;
        tapsSum += taps[i];
    }
    
    /* Normalize Taps */
    for (size_t i = 0; i < Ntaps; i++)
    {
        taps[i] *= Gain / tapsSum;
    }

    LOG_DEBUG("Generated RRC (Root Raised Cosine) FIR Taps.");
    LOG_DEBUG("  Params:");
    LOG_DEBUG("    SampleRate: {} Hz",SampleRate);
    LOG_DEBUG("    SymbolRate: {} Hz",SymbolRate);
    LOG_DEBUG("    ExcessBw:   {}",ExcessBw);
    LOG_DEBUG("    Span:       {} symbols",Span);
    LOG_DEBUG("    NumTaps:    {}",Ntaps);
    
    return taps;
}
