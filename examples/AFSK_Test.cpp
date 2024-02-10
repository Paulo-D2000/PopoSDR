// General includes
#include <iostream>
#include <thread>
#include <random>

// Modulator includes
#include <Utils.h>
#include <WaveFile.h>
#include <AfskModulator.h>

// Demodulator includes
#include <FmDemodulator.h>
#include <TimingPLL.h>

/* Init random generator */
std::mt19937 rng(std::random_device{}());
// Random bytes distribuition
std::uniform_int_distribution<U32> dist(0, UINT32_MAX);
std::normal_distribution<F32> ndist(0.0f,1.0f);

int main(){
    LOG_INFO("Starting AFSK Modulator");
    
    AfskModulator mod(1200, (int)48e3);

    Stream<CF32>* stream = mod.GetStream();
    
    for (size_t j = 0; j < 5; j++)
    {
        /* Send packet */
        std::vector<U8> pkt({0x7E,0x39,0x29,0x76,0x05,0x02,0x02,0x07,0x39,0x76,0x19,0x51,0x59,0x02,0x86,0xC0,0x0F,0x2A,0x16,0xA6,0x04,0x8E,0xAE,0x96,0xC6,0xD6,0x04,0x46,0x4E,0xF6,0xEE,0x76,0x04,0x66,0xF6,0x1E,0x04,0x56,0xAE,0xB6,0x0E,0xCE,0x04,0xF6,0x6E,0xA6,0x4E,0x04,0x2E,0x16,0xA6,0x04,0x36,0x86,0x5E,0x9E,0x04,0x26,0xF6,0xE6,0x75,0xF3,0x3F,0x00});
        mod.sendPacket(pkt, {.Preamble = 0x7E, .PreambleLen = 50, .PostambleLen = 7});
    }

    mod.start();

    mod.stop();

    std::vector<CF32> inputSamples;
    std::vector<CF32> item(1);
    while(!stream->isEmpty()){
        stream->readFromBuffer(item,1);
        inputSamples.push_back(item.at(0));
    }

    WriteWav("Test-AFSK.wav", ComplexInterleave(inputSamples), mod.GetSampleRate(), 2, 0.5f); // Scale output by 0.5
    float spwr = Vec_AvgPwr(inputSamples);
    LOG_TEST("Avg Power: {:.3f} V(rms)",spwr);

    /* Add AWGN */
    float SNRdB = 20.0f;

    float npwr = powf(10.0f,-SNRdB/10.0f) * spwr / sqrtf(2.0f);
    LOG_INFO("SNR: {:.1f}dB = {} V(rms)",SNRdB,npwr);
    for (size_t i = 0; i < inputSamples.size(); i++)
    {
        inputSamples[i] = inputSamples[i] + CF32(npwr*ndist(rng),npwr*ndist(rng));
    }

    /* FM Demodulator */
    FmDemodulator demod(500.0f, mod.GetSampleRate());

    std::vector<F32> demodSamples(inputSamples.size());
    demod.work(inputSamples.size(), inputSamples, demodSamples);
    
    WriteWav("Demod-AFSK.wav", demodSamples,  mod.GetSampleRate(), 1, 0.5f); // Scale output by 0.5
    spwr = Vec_AvgPwr(demodSamples);
    LOG_TEST("Avg Power: {:.3f} V(rms)",spwr);

    /* Test Timing Recovery PLL */
    TimmingPLL<F32> pll(mod.GetSampleRate(), mod.GetBaudRate(), 0.9f);

    std::vector<F32> pllSamples(demodSamples.size()*mod.GetBaudRate()/mod.GetSampleRate());
    pll.work(demodSamples.size(), demodSamples, pllSamples);

    bool nrzi = 1;
    U8 shiftRegister = 0;
    for (size_t i = 0; i < pllSamples.size(); i++)
    {
        // LOAD BIT
        bool ibit = Sign(pllSamples.at(i)) > 0.5;
        bool bit = (ibit == nrzi);
        nrzi = ibit;
        shiftRegister = ((shiftRegister << 1) + bit) & 0xFF;
        if(shiftRegister == 0x7E){
            LOG_DEBUG("Found 7E Flag");
        }
    }

    WriteWav("Timing-AFSK.wav", pllSamples, mod.GetSampleRate(), 1, 0.5f); // Scale output by 0.5
    spwr = Vec_AvgPwr(pllSamples);
    LOG_TEST("Avg Power: {:.3f} V(rms)",spwr);

    LOG_INFO("Terminating AFSK Modulator");

    return 0;
}