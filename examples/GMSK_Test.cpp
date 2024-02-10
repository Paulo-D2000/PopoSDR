// General includes
#include <iostream>
#include <thread>
#include <random>
#include <cstring>
#include <iterator>

// Modulator includes
#include <Utils.h>
#include <WaveFile.h>
#include <GmskModulator.h>
#include <Filters.h>

// Demodulator includes
#include <FmDemodulator.h>
#include <TimingPLL.h>

/* Init random generator */
std::mt19937 rng(std::random_device{}());
// Random bytes distribuition
std::uniform_int_distribution<U32> dist(0, UINT32_MAX);
std::normal_distribution<F32> ndist(0.0f,1.0f);

// TODO: Move to Utils.h...
std::vector<U8> String2Packet(const char* str){
    std::vector<U8> pkt;
     for (size_t i = 0; i < strlen(str); i++)
    {
        pkt.push_back(str[i]);
    }
    return pkt;
}

int main(){
    LOG_INFO("Starting GMSK Modulator");
    
    GmskModulator mod(1200, (int)48e3);

    Stream<CF32>* stream = mod.GetStream();
    
    for (size_t j = 0; j < 1; j++)
    {
        /* Send packet */
        std::vector<U8> pkt = String2Packet("CCSDS PACKET TEST | AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA | - COUNTER: 0\n");
        //pkt.push_back(std::to_string(32).c_str()[0]);
        //pkt.insert(pkt.end(), '\n');
        //pkt.insert(pkt.end(), std::to_string(j).c_str()[0]);
        mod.sendPacket(pkt, { .Preamble = 0x33, .PreambleLen = 50, .Syncword=0x1ACFFC1D, .Scrambler = true });
    }

    mod.start();

    mod.stop();

    std::vector<CF32> inputSamples;
    std::vector<CF32> item(1);
    while(!stream->isEmpty()){
        stream->readFromBuffer(item,1);
        inputSamples.push_back(item.at(0)); 
    }

    WriteWav("Test.wav", ComplexInterleave(inputSamples), mod.GetSampleRate(), 2, 0.5f); // Scale output by 0.5
    float spwr = Vec_AvgPwr(inputSamples);
    LOG_TEST("Avg Power: {:.3f} V(rms)",spwr);

    auto samples = inputSamples;
    

    size_t OverRate = mod.GetSampleRate()*25;
    size_t DownRate = OverRate / 24;
    std::vector<CF32> oversamp(samples.size()*25);
    std::vector<CF32> downsamp(oversamp.size()/24);
    std::vector<CF32> finalsamp(downsamp.size()*20);
    PolyPhaseFIR<CF32> up25x(Generate_Generic_LPF(OverRate, mod.GetSampleRate()/2, 25.0f, 3, Kaiser), {25, 1}, 0);
    PolyPhaseFIR<CF32> down24x(Generate_Generic_LPF(OverRate, mod.GetSampleRate()/2, 1.0f, 3, Kaiser), {1,24}, 0);
    PolyPhaseFIR<CF32> final20x(Generate_Generic_LPF(OverRate*20, mod.GetSampleRate()/2, 20.0f, 3, Kaiser), {20, 1}, 0);
    up25x.work(samples.size(), samples, oversamp);
    down24x.work(oversamp.size(), oversamp, downsamp);
    final20x.work(downsamp.size(), downsamp, finalsamp);

    auto fname = std::format("GMSK_{}Bd_{}Hz.wav",mod.GetBaudRate(),DownRate*20);
    WriteWav(fname, ComplexInterleave(finalsamp), DownRate, 2, 0.5f); // Scale output by 0.5
    spwr = Vec_AvgPwr(finalsamp);
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
    FmDemodulator demod(0.25f*(float)mod.GetBaudRate(), mod.GetSampleRate());

    std::vector<F32> demodSamples(inputSamples.size());
    demod.work(inputSamples.size(), inputSamples, demodSamples);
    
    WriteWav("Demod.wav", demodSamples,  mod.GetSampleRate(), 1, 0.5f); // Scale output by 0.5
    spwr = Vec_AvgPwr(demodSamples);
    LOG_TEST("Avg Power: {:.3f} V(rms)",spwr);

    /* Test Timing Recovery PLL */
    TimingPLL<F32> pll(mod.GetSampleRate(), mod.GetBaudRate(), 0.9f);

    std::vector<F32> pllSamples(demodSamples.size()*mod.GetBaudRate()/mod.GetSampleRate());
    pll.work(demodSamples.size(), demodSamples, pllSamples);
    
    std::string syncword = "00011010110011111111110000011101";
    std::vector<int> sw;
    for (size_t i = 0; i < syncword.length(); i++)
    {
        sw.push_back(syncword.at(i)-'0');
    }
    std::vector<int> sbuf(sw.size(),0);
    LOG_DEBUG("SW SIZE: {}", sw.size());

    
    int maxcorr = 0;
    for (size_t i = 0; i < pllSamples.size(); i++)
    {
        sbuf.erase(sbuf.begin());
        int bit = Sign(pllSamples.at(i));
        sbuf.push_back(bit > 0.5);
        int newcorr = 0;
        for (size_t j = 0; j < sw.size(); j++)
        {
            newcorr += !(sbuf.at(j) ^ sw.at(j));
        }
        maxcorr = std::max<int>(maxcorr, newcorr);
        if(newcorr > (syncword.size()-4)){
            std::stringstream result;
            std::copy(sbuf.begin(), sbuf.end(), std::ostream_iterator<int>(result, ""));
            std::stringstream sz;
            for (size_t k = 0; k < 8; k++)
            {
                sz << std::to_string(Sign(pllSamples.at(i+k)) > 0.01f);
            }
            
            LOG_INFO("New Corr: {}, New data: {}, SW: {}, PktSz: {}",newcorr,result.str(),syncword,sz.str());
        }
    }

    LOG_INFO("Max Correlation: {}",maxcorr);
    
    WriteWav("Timing.wav", pllSamples, mod.GetSampleRate(), 1, 0.5f); // Scale output by 0.5
    spwr = Vec_AvgPwr(pllSamples);
    LOG_TEST("Avg Power: {:.3f} V(rms)",spwr);

    LOG_INFO("Terminating GMSK Modulator");

    return 0;
}