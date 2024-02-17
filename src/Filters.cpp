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
    int Ntaps = int((float)Span * Ts);
    if(Ntaps % 2==0)
        Ntaps++;
    std::vector<F32> taps(Ntaps);
    int Ntaps_1_2 = Ntaps / 2;

    /* Calculate Taps */
    double tapsSum = 0;
    for(int i=0; i<Ntaps; i++){
        double x1, x2, x3, num, den;
        double xindx = i - Ntaps / 2;
        x1 = M_PI * xindx / Ts;
        x2 = 4 * ExcessBw * xindx / Ts;
        x3 = x2 * x2 - 1;

        if (fabs(x3) >= 0.000001) { // Avoid Rounding errors...
            if (i != Ntaps / 2)
                num = cos((1 + ExcessBw) * x1) +
                      sin((1 - ExcessBw) * x1) / (4 * ExcessBw * xindx / Ts);
            else
                num = cos((1 + ExcessBw) * x1) + (1 - ExcessBw) * M_PI / (4 * ExcessBw);
            den = x3 * M_PI;
        } else {
            if (ExcessBw == 1) {
                taps[i] = -1;
                tapsSum += taps[i];
                continue;
            }
            x3 = (1 - ExcessBw) * x1;
            x2 = (1 + ExcessBw) * x1;
            num = (sin(x2) * (1 + ExcessBw) * M_PI -
                   cos(x3) * ((1 - ExcessBw) * M_PI * Ts) / (4 * ExcessBw * xindx) +
                   sin(x3) * Ts * Ts / (4 * ExcessBw * xindx * xindx));
            den = -32 * M_PI * ExcessBw * ExcessBw * xindx / Ts;
        }
        taps[i] = 4 * ExcessBw * num / den;
        tapsSum += taps[i];
    }
    
    /* Normalize Taps */
    for (size_t i = 0; i < Ntaps; i++)
    {
        taps[i] = taps[i] * Gain / tapsSum;
    }

    LOG_DEBUG("Generated RRC (Root Raised Cosine) FIR Taps.");
    LOG_DEBUG("  Params:");
    LOG_DEBUG("    SampleRate: {} Hz",SampleRate);
    LOG_DEBUG("    SymbolRate: {} Hz",SymbolRate);
    LOG_DEBUG("    ExcessBw:   {}",ExcessBw);
    LOG_DEBUG("    Span:       {} symbols",Span);
    LOG_DEBUG("    NumTaps:    {}",Ntaps);
    
    std::cerr << "[";
    for (size_t i = 0; i < Ntaps; i++)
    {
        std::cerr << taps[i] << ", ";
    }
    std::cerr << "]\n";
    
    return taps;
}
