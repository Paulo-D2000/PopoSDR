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

    size_t SampleRate = 1300*8;
    size_t BaudRate = 1300;
    size_t Sps = SampleRate / BaudRate;

    /* Constellation */
    std::vector<CF32> obj_vec;

    //obj_vec.push_back(CF32(-1,0));
    //obj_vec.push_back(CF32(1,0));

    // QPSK
    //for (size_t i = 0; i < 4; i++)
    //    obj_vec.push_back(std::exp((_1j * 2.0f * M_PI_F * (float)i / 4.0f) + (_1j * M_PI_F * 0.25f)));

    // 16-APSK (or 12-PSK)
    //for (size_t i = 0; i < 12; i++)
    //    obj_vec.push_back(2.0f * std::exp(_1j * 2.0f * M_PI_F * (float)i / 12.0f));

    // 32-APSK (or 16-PSK)
    //for (size_t i = 0; i < 16; i++)
    //    obj_vec.push_back(3.0f * std::exp(_1j * 2.0f * M_PI_F * (float)i / 16.0f));
    
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            obj_vec.push_back(CF32(2*i-7,2*j-7));
        }
    }

    F32 obj_pwr = Vec_AvgPwr(obj_vec);
    for (auto &&point : obj_vec)
        point /= obj_pwr;

    /* LPF taps */
    auto tx_taps = Generate_Root_Raised_Cosine((float)SampleRate, (float)BaudRate, 0.35f, Sps, 3);
    auto rx_taps = Generate_Root_Raised_Cosine((float)SampleRate, (float)BaudRate, 0.35f, 1.0f, 3);

    size_t nfilts = 32;
    auto rrc_pfb_taps = Generate_Root_Raised_Cosine(SampleRate*nfilts, BaudRate, 0.35f, nfilts, 11);
    
    size_t newsize = rrc_pfb_taps.size();
    if(newsize % nfilts != 0){
        newsize += nfilts - (newsize % nfilts);
    }

    std::vector<F32> finalTaps(newsize, 0);
    std::copy(rrc_pfb_taps.begin(), rrc_pfb_taps.end(), finalTaps.begin());

    std::vector<std::vector<F32>> rrc_pfb;
    for (size_t i = 0; i < nfilts; i++)
    { 
        std::vector <F32> tapseg(newsize / nfilts);
        for (size_t j = 0; j < tapseg.size(); j++)
        {
            tapseg[j] = finalTaps[i+j*nfilts];
        }
        rrc_pfb.push_back(tapseg);
    }

    /* Blocks */
    ConstellationMapper psk_mod(obj_vec, 132072);
    ConstellationDemapper psk_demod(obj_vec, 132072);
    FirFilter<CF32> tx_fir(tx_taps, {.Interpolation=Sps, .Decimation=1}, 132072);
    FirFilter<CF32> rx_fir(rx_taps, {.Interpolation=1, .Decimation=1}, 132072);
    TimingPLL<CF32>::TLL_Config cfg = {.Damping = 0.707f / Sps, .LoopBw = 1.0f/(Sps*200.0f), .Type = PFB, .PFB_Taps = rrc_pfb, .Constellation_Obj = Constellation(obj_vec)};
    TimingPLL<CF32> pll(SampleRate, BaudRate, cfg, 132072);

    /* Copy to Rx*/
    std::vector<U8> seq = {1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1};
    printf("Eb/N0, 16-APSK(Theory), 16-APSK(Actual)\n");
    for (float EbNodB = 20.0f; EbNodB < 21.0f; EbNodB+=1.0f)
    {
        //float EbNodB = 20.0f;
        /* Random bits*/
        std::vector<U8> bits;
        for (size_t i = 0; i < 1E5/32; i++)
        {
            U32 word = dist(rng);
            //if(i<16){
            //    word = 0x55555555;
            //}
//
            //if(i==16) word = 0x0;
            
            for (size_t j = 0; j < 32; j++)
            {
                int bit = (word >> (31-j)) & 0x01;
                bits.push_back(bit);
                //for (size_t k = 0; k < 13; k++)
                //{
                //    bits.push_back(bit ^ seq[k]);
                //}
            }
        }

        /* Gen 16-APSK */
        std::vector<CF32> TxSymbols(bits.size());
        size_t nsyms = psk_mod.work(bits.size(), bits, TxSymbols);
        for (size_t i = TxSymbols.size(); i > nsyms; i--)
            TxSymbols.pop_back();
        
        

        /* Filter */
        std::vector<CF32> TxSamples(TxSymbols.size() * Sps);
        tx_fir.work(TxSymbols.size(), TxSymbols, TxSamples);

        std::vector<CF32> RxSamples(TxSamples.size(), {0,0});
        std::vector<CF32> FiltSamples(TxSamples.size(), {0,0});
        std::vector<CF32> pllSamples(bits.size(), {0,0});
        //std::vector<CF32> demodSamples(RxSamples.size());
        //std::vector<CF32> debugSamples(RxSamples.size()/Sps);
        
        //pll.Reset();
        std::copy(TxSamples.begin(), TxSamples.end(), RxSamples.begin());

        /* Add AWGN */
        AwgnChannel(RxSamples, EbNodB, Sps);

        /* Matched Filter */
        rx_fir.work(RxSamples.size(), RxSamples, FiltSamples);

        /* Clock Recovery */
        size_t processed = pll.work(RxSamples.size(), RxSamples, pllSamples);
        auto errs = pll.error_vec;
        for (size_t i = pllSamples.size(); i > processed; i--)
            pllSamples.pop_back();


        /* Demodulate */
        //psk_demod.work()

        /* Measure ber */
        /*
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
        */
        
        //size_t errors = total_bits - correct_bits;
        //float ber = (float)errors / (float)total_bits;
        float ber_theory = bpskBerFunc(pow(10.0, EbNodB/10.0));
        float ber = ber_theory;
        printf("%.1f, %.8f, %.8f\n", EbNodB, ber_theory, ber);
    
        WriteWav("Test-APSK_" + std::to_string((int)(EbNodB)) + "_dB_EbN0.wav", ComplexInterleave(RxSamples), SampleRate, 2, 0.5f); // Scale output by 0.5
        //float spwr = Vec_AvgPwr(RxSamples);
        //LOG_TEST("Avg Power: %.3f V(rms)",spwr);

        WriteWav("Demod-APSK_" + std::to_string((int)(EbNodB)) + "_dB_EbN0.wav", ComplexInterleave(FiltSamples), SampleRate, 2, 0.5f); // Scale output by 0.5
        
        WriteWav("Debug-APSK_" + std::to_string((int)(EbNodB)) + "_dB_EbN0.wav", ComplexInterleave(errs), SampleRate, 2, 0.5f); // Scale output by 0.5
        //spwr = Vec_AvgPwr(filtSamples);
        //LOG_TEST("Avg Power: %.3f V(rms)",spwr);

        WriteWav("Timing-APSK_" + std::to_string((int)(EbNodB)) + "_dB_EbN0.wav", ComplexInterleave(pllSamples), BaudRate, 2, 0.01f); // Scale output by 0.5
        //spwr = Vec_AvgPwr(pllSamples);
        //LOG_TEST("Avg Power: %.3f V(rms)",spwr);
    }
    LOG_INFO("Terminating APSK Modulator");
    
    return 0;
}