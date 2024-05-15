// General includes
#include <iostream>
#include <thread>
#include <random>

// Modulator includes
#include <Utils.h>
#include <WaveFile.h>
#include <FirFilter.h>
#include <Filters.h>
#include <FmModulator.h>

// Demodulator includes
#include <FmDemodulator.h>
#include <TimingPLL.h>

/* Init random generator */
std::mt19937 rng(std::random_device{}());
// Random bytes distribuition
std::uniform_int_distribution<U32> dist(0, UINT32_MAX);
std::normal_distribution<F32> ndist(0.0f,1.0f);

void AwgnChannel(std::vector<CF32>& inputSamples, float EbN0_dB, float Sps){
    float mean_Pwr = Vec_AvgPwr(inputSamples);
    const float SNR_dB = EbN0_dB - 10.*log10f(Sps);
    const float S_dB = 10*log10f(mean_Pwr);
    const float N_dB = S_dB - SNR_dB;
    const float npwr = pow(10., N_dB/10.) / sqrt(2);
    LOG_INFO("S_dB %1fdB = %f V(rms)",S_dB,mean_Pwr);
    LOG_INFO("SNR: %1fdB = %f V(rms)",EbN0_dB,npwr);
    for (size_t i = 0; i < inputSamples.size(); i++)
    {
        inputSamples[i] = inputSamples[i] + CF32(npwr*ndist(rng),npwr*ndist(rng));
    }
}

int main(){
    LOG_INFO("Starting AFSK Modulator");

    size_t SampleRate = 9.6e3;
    size_t BaudRate = 300;
    size_t Sps = SampleRate / BaudRate;

    /* LPF taps */
    auto interp_taps = Generate_Generic_LPF((float)SampleRate, 0.5f * (float)BaudRate, 0.5f, 6, Kaiser);
    auto rx_taps = Generate_Generic_LPF((float)SampleRate, 0.5f*(float)BaudRate, 1.0f, 6, Kaiser);

    /* Blocks */
    FmModulator fmmod(100.f, SampleRate);
    FmDemodulator fmdemod(100.0f, SampleRate);
    FirFilter<F32> demodlpf(interp_taps, {.Interpolation=1, .Decimation=1}, 132072);
    FirFilter<CF32> rxlpf(rx_taps, {.Interpolation=1, .Decimation=1}, 132072);
    TimingPLL<F32> pll(SampleRate, BaudRate, 0.1f);

    /* Copy to Rx*/
    printf("C/N, BER\n");
    //for (float SNRdB = 0.0f; SNRdB < 20.0f; SNRdB+=0.5f)
    //{
    float SNRdB = 14.0f;
        /* Random bits*/
        std::vector<U8> bits;
        for (size_t i = 0; i < 1024; i++)
        {
            U32 word = dist(rng);
            if(i<16){
                word = 0xAAAAAAAA;
            }

            for (size_t j = 0; j < 32; j++)
            {
                int bit = (word >> (31-j)) & 0x01;
                bits.push_back(bit);
            }
        }

        /* Gen 2-FSK */
        std::vector<F32> TxSymbols;
        for (size_t i = 0; i < bits.size(); i++)
        {
            float sample = 2.0f * (float)bits[i] - 1.0f;
            for (size_t j = 0; j < Sps; j++)
            {
                TxSymbols.push_back(sample);
            }
        }

        /* Modulate */
        std::vector<CF32> TxSamples(TxSymbols.size());
        fmmod.Reset();
        fmmod.work(TxSymbols.size(), TxSymbols, TxSamples);

        std::vector<CF32> RxSamples(TxSamples.size(), {0,0});
        std::vector<F32> pllSamples(bits.size());
        std::vector<F32> demodSamples(RxSamples.size());
        std::vector<F32> filtSamples(RxSamples.size());
        
        fmdemod.Reset();
        pll.Reset();
        //std::copy(TxSamples.begin(), TxSamples.end(), RxSamples.begin());
        float pw = Vec_AvgPwr(TxSamples);
        for (size_t i = 0; i < TxSamples.size(); i++)
        {
            RxSamples[i] = TxSamples[i] / pw;
        }
        

        /* Add AWGN */
        AwgnChannel(RxSamples, SNRdB, Sps);

        /* Filter */
        rxlpf.work(RxSamples.size(), RxSamples, RxSamples);

        /* Demodulate */
        fmdemod.work(RxSamples.size(), RxSamples, demodSamples);

        /* Filter */
        //demodlpf.work(demodSamples.size(), demodSamples, filtSamples);

        /* Clock Recovery */
        //pll.work(filtSamples.size(), filtSamples, pllSamples);

        /* Measure ber */
        size_t correct_bits = 0;
        size_t total_bits = 0;
        U8 SR = 0x00;
        bool sync = 0;
        for (size_t i = 0.5*Sps + 2; i < demodSamples.size(); i+=Sps)
        {
            // LOAD BIT
            pllSamples[i/Sps] = demodSamples[i];
            bool bit = Sign(demodSamples[i]) > 0;
            SR = ((SR << 1) | bit) & 0xFF;
            if(bit == bits[i/Sps]){
                correct_bits++;
            }
            total_bits++;
        }
        
        size_t errors = total_bits - correct_bits;
        float ber = (float)errors / (float)total_bits;
        printf("%.1f, %.8f\n", SNRdB, ber);
    //}

    
    WriteWav("Test-AFSK.wav", ComplexInterleave(RxSamples), SampleRate, 2, 0.5f); // Scale output by 0.5
    float spwr = Vec_AvgPwr(RxSamples);
    LOG_TEST("Avg Power: %.3f V(rms)",spwr);
    
    WriteWav("Demod-AFSK.wav", demodSamples, SampleRate, 1, 0.5f); // Scale output by 0.5
    spwr = Vec_AvgPwr(demodSamples);
    LOG_TEST("Avg Power: %.3f V(rms)",spwr);

    WriteWav("Timing-AFSK.wav", pllSamples, BaudRate, 1, 0.5f); // Scale output by 0.5
    spwr = Vec_AvgPwr(pllSamples);
    LOG_TEST("Avg Power: %.3f V(rms)",spwr);
    
    LOG_INFO("Terminating AFSK Modulator");

    return 0;
}