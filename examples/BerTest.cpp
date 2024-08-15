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
#include <Constellation.h>
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
    const float npwr = sqrtf(pow(10., N_dB/10.)/2.0f);
    LOG_INFO("S_dB %1fdB = %f V(rms)",S_dB,mean_Pwr);
    LOG_INFO("SNR: %1fdB = %f V(rms)",EbN0_dB,npwr);
    for (size_t i = 0; i < inputSamples.size(); i++)
    {
        inputSamples[i] = inputSamples[i] + CF32(npwr*ndist(rng),npwr*ndist(rng));
    }
}

double fskBerFunc(double EbNo){
    return 0.5 * erfc(sqrt(0.5*EbNo));
}

double bpskBerFunc(double EbNo){
    return 0.5 * erfc(sqrt(EbNo));
}

int main(){
    LOG_INFO("Starting AFSK Modulator");

    size_t SampleRate = 1500;
    size_t BaudRate = 300;
    size_t Sps = SampleRate / BaudRate;

    /* LPF taps */
    auto tx_taps = Generate_Root_Raised_Cosine((float)SampleRate, (float)BaudRate, 1.0, Sps, 3);
    auto rx_taps = Generate_Root_Raised_Cosine((float)SampleRate, (float)BaudRate, 1.0, 0.5, 3);

    /* Blocks */
    FmModulator fmmod(1.0f * BaudRate, SampleRate);
    FmDemodulator fmdemod(1.0f * BaudRate, SampleRate);
    FirFilter<F32> tx_fir(tx_taps, {.Interpolation=1, .Decimation=1}, 132072);
    FirFilter<F32> rx_fir(rx_taps, {.Interpolation=1, .Decimation=1}, 132072);
    TimingPLL<F32> pll(SampleRate, BaudRate, 0.1f);

    /* Copy to Rx*/
    printf("Eb/N0, FSK(Theory), FSK(Lyons2), FSK(Arctan2)\n");
    for (float EbNodB = -2.0f; EbNodB < 21.0f; EbNodB+=1.0f)
    {
        //float EbNodB = 20.0f;
        /* Random bits*/
        std::vector<U8> bits;
        for (size_t i = 0; i < 1E6/32; i++)
        {
            U32 word = dist(rng);
            if(i<16){
                word = 0x55555555;
            }

            if(i==16) word = 0x0;

            for (size_t j = 0; j < 32; j++)
            {
                int bit = (word >> (31-j)) & 0x01;
                bits.push_back(bit);
            }
        }

        /* Gen 2-FSK */
        //std::vector<F32> PreTxSymbols;
        std::vector<F32> TxSymbols;
        for (size_t i = 0; i < bits.size(); i++)
        {
            float sample = 2.0f * (float)bits[i] - 1.0f;
            for (size_t j = 0; j < Sps; j++)
            {
                TxSymbols.push_back(sample);
                //PreTxSymbols.push_back(sample);
                //sample = 0;
            }
        }
        //std::vector<F32> TxSymbols(PreTxSymbols.size());
        //tx_fir.work(PreTxSymbols.size(), PreTxSymbols, TxSymbols);

        /* Modulate */
        std::vector<CF32> TxSamples(TxSymbols.size());
        fmmod.Reset();
        fmmod.work(TxSymbols.size(), TxSymbols, TxSamples);

        std::vector<CF32> RxSamples(TxSamples.size(), {0,0});
        //std::vector<F32> pllSamples(bits.size());
        std::vector<F32> demodSamples(RxSamples.size());
        std::vector<F32> demodSamples_slow(RxSamples.size());
        std::vector<F32> filtSamples(RxSamples.size());
        std::vector<F32> filtSamples_slow(RxSamples.size());
        std::vector<CF32> debugSamples(RxSamples.size()/Sps);
        std::vector<CF32> debugSamples_slow(RxSamples.size()/Sps);
        
        //fmdemod.Reset();
        //pll.Reset();
        std::copy(TxSamples.begin(), TxSamples.end(), RxSamples.begin());
        //float pw = Vec_AvgPwr(TxSamples);
        //for (size_t i = 0; i < TxSamples.size(); i++)
        //{
        //    RxSamples[i] = TxSamples[i] / pw;
        //}
        

        /* Add AWGN */
        AwgnChannel(RxSamples, EbNodB, Sps);

        /* Filter */
        //rxlpf.work(RxSamples.size(), RxSamples, RxSamples);

        /* Demodulate */
        fmdemod.Reset();
        fmdemod.work(RxSamples.size(), RxSamples, demodSamples);
        fmdemod.Reset();
        fmdemod.work_slow(RxSamples.size(), RxSamples, demodSamples_slow);
        
        /* Filter */
        //rx_fir.work(demodSamples.size(), demodSamples, filtSamples);
        
        std::vector<F32> buf(Sps,0.0f);
        for (size_t i = 0; i < demodSamples.size(); i++)
        {
            buf.insert(buf.begin(), demodSamples[i]);
            buf.pop_back();
            filtSamples[i] = std::accumulate(buf.begin(), buf.end(), 0.0f) / Sps;
        }

        std::vector<F32> buf_slow(Sps,0.0f);
        for (size_t i = 0; i < demodSamples_slow.size(); i++)
        {
            buf_slow.insert(buf_slow.begin(), demodSamples_slow[i]);
            buf_slow.pop_back();
            filtSamples_slow[i] = std::accumulate(buf_slow.begin(), buf_slow.end(), 0.0f) / Sps;
        }
        

        /* Clock Recovery */
        //pll.work(filtSamples.size(), filtSamples, pllSamples);

        /* Measure ber */
        size_t correct_bits = 0;
        size_t total_bits = 0;
        size_t j = 0;
        U8 SR = 0x00;
        for (size_t i = 1.5*Sps; i < filtSamples.size(); i+=Sps)
        {
            // LOAD BIT
            //pllSamples[j] = filtSamples[i];
            bool bit = Sign(filtSamples[i]) > 0;
            SR = ((SR << 1) | bit) & 0xFF;
            if(bit == bits[j]){
                correct_bits++;
            }
            debugSamples[j] = CF32(filtSamples[i], 2.0f * bits[j] - 1.0f);
            total_bits++;
            j++;
        }

        j = 0;
        size_t correct_bits_slow = 0;
        size_t total_bits_slow = 0;
        for (size_t i = 5; i < filtSamples_slow.size(); i+=Sps)
        {
            // LOAD BIT
            //pllSamples[j] = filtSamples[i];
            bool bit = Sign(filtSamples_slow[i]) > 0;
            SR = ((SR << 1) | bit) & 0xFF;
            if(bit == bits[j]){
                correct_bits_slow++;
            }
            debugSamples_slow[j] = CF32(filtSamples_slow[i], 2.0f * bits[j] - 1.0f);
            total_bits_slow++;
            j++;
        }
        
        size_t errors = total_bits - correct_bits;
        size_t errors_slow = total_bits_slow - correct_bits_slow;
        float ber = (float)errors / (float)total_bits;
        float ber_slow = (float)errors_slow / (float)total_bits_slow;
        float ber_theory = fskBerFunc(pow(10.0, EbNodB/10.0));
        printf("%.1f, %.8f, %.8f, %.8f\n", EbNodB, ber_theory, ber, ber_slow);
    
        //WriteWav("Test-AFSK_" + std::to_string((int)(EbNodB)) + "_dB_EbN0.wav", ComplexInterleave(RxSamples), SampleRate, 2, 0.01f); // Scale output by 0.5
        //float spwr = Vec_AvgPwr(RxSamples);
        //LOG_TEST("Avg Power: %.3f V(rms)",spwr);
        
        //WriteWav("Demod-AFSK_" + std::to_string((int)(EbNodB)) + "_dB_EbN0.wav", filtSamples, SampleRate, 1, 0.01f); // Scale output by 0.5
        //spwr = Vec_AvgPwr(filtSamples);
        //LOG_TEST("Avg Power: %.3f V(rms)",spwr);

        //WriteWav("Timing-AFSK_" + std::to_string((int)(EbNodB)) + "_dB_EbN0.wav", ComplexInterleave(debugSamples_slow), BaudRate, 2, 0.01f); // Scale output by 0.5
        //spwr = Vec_AvgPwr(pllSamples);
        //LOG_TEST("Avg Power: %.3f V(rms)",spwr);
        
    }
    LOG_INFO("Terminating AFSK Modulator");
    
    return 0;
}