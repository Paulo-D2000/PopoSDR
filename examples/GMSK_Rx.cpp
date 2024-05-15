// General includes
#include <iostream>
#include <thread>
#include <random>
#include <cstring>
#include <iterator>

// Demodulator includes
#include <FmDemodulator.h>
#include <TimingPLL.h>
#include <FirFilter.h>
#include <Filters.h>
#include <Utils.h>
#include <WaveFile.h>
#include <Agc.h>
#include <Networking.h>

void _writeFloat(F32 data){
    I16 val = (short)(data * (float)INT16_MAX);
    std::cout << (U8)(val & 0xFF) << (U8)((val >> 8) & 0xFF);
}

void WriteCout(F32 data){
    _writeFloat(data);
}

void WriteCout(CF32 data){
    _writeFloat(data.real());
    _writeFloat(data.imag());
}

int main(){
    size_t SampleRate = 1e6;  // 1000kHz
    size_t DecimRate  = 50e3; // 50Khz
    size_t FmRate     = 48e3; // 48kHz
    size_t Cutoff     = 12.5e3; // 12.5KHz
    size_t BaudRate   = 1200; //1200 Bd

    // Bufer Sizes
    size_t PacketSize = 4096;
    size_t BufferSize = 131072;

    // Blocks
    std::vector<Block*> blocks;

    SocketSource<CF32, UDP> src(1234, PacketSize, false, BufferSize);

    Agc<CF32> agc(1e-6f, 0.707f, BufferSize);

    Agc<F32> agc2(1e-5f, 0.5f, BufferSize);

    FmDemodulator fm(0.25f * (float)BaudRate, FmRate);

    TimingPLL<F32> pll(FmRate, BaudRate, 0.98f, BufferSize);

    // filters
    FirRate decim_rate = {1, SampleRate / DecimRate};
    auto decim_taps = Generate_Generic_LPF(SampleRate, DecimRate, 1.0f, 3, Kaiser);
    PolyPhaseFIR<CF32> decim_inp(decim_taps, decim_rate, BufferSize);
    
    // Resample
    size_t mul = std::gcd(DecimRate, FmRate);
    size_t up = FmRate / mul, down = DecimRate / mul;

    FirRate up_rate = {up, 1};
    auto up_taps = Generate_Generic_LPF(DecimRate*up, FmRate/2, up, 3, Kaiser);
    PolyPhaseFIR<CF32> up_resamp(up_taps, up_rate, BufferSize);

    FirRate down_rate = {1, down};
    auto down_taps = Generate_Generic_LPF(DecimRate*up, FmRate/2, 1.0f, 3, Kaiser);
    PolyPhaseFIR<CF32> down_resamp(down_taps, down_rate, BufferSize);

    FirRate out_rate = {FmRate / BaudRate, 1};
    auto out_taps = Generate_Generic_LPF(FmRate, BaudRate/2, 0.5f*FmRate/BaudRate, 3, Kaiser);
    PolyPhaseFIR<F32> out_resamp(out_taps, out_rate, BufferSize);

    // Lpf after FM
    auto lpf_taps = Generate_Generic_LPF(FmRate, Cutoff/2, 1.0f, 6, Kaiser);
    FirFilter<F32> fm_lpf(lpf_taps, {1,1}, BufferSize);
    
    // connect
    agc.connect(src);
    decim_inp.connect(agc);
    up_resamp.connect(decim_inp);
    down_resamp.connect(up_resamp);
    fm.connect(down_resamp);
    fm_lpf.connect(fm);
    agc2.connect(fm_lpf);
    pll.connect(agc2);
    out_resamp.connect(pll);

    // start blocks
    blocks.push_back(&src);
    blocks.push_back(&agc);
    blocks.push_back(&decim_inp);
    blocks.push_back(&up_resamp);
    blocks.push_back(&down_resamp);
    blocks.push_back(&fm);
    blocks.push_back(&fm_lpf);
    blocks.push_back(&agc2);
    blocks.push_back(&pll);
    blocks.push_back(&out_resamp);

    for (auto &blk : blocks)
        blk->start();

    // simulate some delay
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // DEBUG -> Write to stdout
    auto stream = pll.getOutputStream();
    std::vector<F32> inputSamples(4096, 0);
    auto stream_out = out_resamp.getOutputStream();
    std::vector<F32> outputSamples(4096, 0);


    std::string syncword = "00011010110011111111110000011101";
    std::reverse(syncword.begin(), syncword.end());
    std::vector<int> sw;
    for (size_t i = 0; i < syncword.length(); i++)
    {
        sw.push_back(syncword.at(i)-'0');
    }
    std::vector<int> sbuf(sw.size(),0);
    LOG_DEBUG("SW SIZE: %d", sw.size());

    int maxcorr = 0;
    size_t bitpos = 0;
    U8 byte = 0x00;
    std::vector<int> pkt(1024, 0);
    std::vector<U8> msg(pkt.size()/8, 0);
    size_t counter = 0;
    while(1){
        // read demod samples
        size_t nread = stream->readFromBuffer(inputSamples);

        // try to frame
        for (size_t i = 0; i < nread; i++)
        {
            pkt.erase(pkt.end());
            sbuf.erase(sbuf.end());
            int bit = Sign(inputSamples.at(i)) > 0.01f;
            sbuf.insert(sbuf.begin(), bit);
            pkt.insert(pkt.begin(), bit);
            int newcorr = 0;
            for (size_t j = 0; j < sw.size(); j++)
            {
                newcorr += !(sbuf.at(j) ^ sw.at(j));
            }
            maxcorr = std::max<int>(maxcorr, newcorr);
            if(newcorr > (syncword.size()-4)){
                std::stringstream result;
                std::copy(sbuf.begin(), sbuf.end(), std::ostream_iterator<int>(result, ""));
                LOG_INFO("New Corr: %d, New data: %s, SW: %s",newcorr,result.str().c_str(),syncword.c_str());
                result.str(std::string());
                for (size_t k = 0; k < pkt.size(); k+=8)
                {
                    byte = 0x00;
                    for(bitpos = 0; bitpos < 8; bitpos++){
                        byte |= pkt[k+bitpos] << bitpos;
                    }
                    msg[k/8] = byte;
                }
                std::reverse(msg.begin(), msg.end());
                for (size_t k = 0; k < 59; k++)
                {
                    char chr =  (char)(msg[k]);
                    if(isascii(chr)){
                        result << chr;
                    }else{
                        result << string_format(" 0x%02x ", msg[k]);
                    }
                }
                LOG_INFO("Packet data: %s", result.str().c_str());
            }
        }
    }

    // simulate some delay
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // stop
    for (auto &blk : blocks)
        blk->stop();

    return 0;
}