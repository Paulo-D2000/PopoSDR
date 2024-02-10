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
#include <CarrierRecovery.h>

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
    size_t FmRate     = 48e3; // 48kHz
    size_t BaudRate   = FmRate/10; //4800 Bd

    // Bufer Sizes
    size_t PacketSize = 16384;
    size_t BufferSize = 131072;

    // Blocks
    std::vector<Block*> blocks;

    SocketSource<CF32, TCP> src(1234, PacketSize, true, 2*BufferSize);

    Agc<CF32> agc(1e-4f, 1.0f, BufferSize);

    TimingPLL<CF32> pll(FmRate, BaudRate, 1.0f-0.98f, BufferSize);

    // 16-QAM Constellation from GnuRadio
    std::vector<CF32> const_points = {{0.316228, -0.316228}, {-0.316228, -0.316228}, {0.948683, -0.948683}, {-0.948683, -0.948683}, {-0.948683, -0.316228}, {0.948683, -0.316228}, {-0.316228, -0.948683}, {0.316228, -0.948683}, {-0.948683, 0.948683}, {0.948683, 0.948683}, {-0.316228, 0.316228}, {0.316228, 0.316228}, {0.316228, 0.948683}, {-0.316228, 0.948683}, {0.948683, 0.316228}, {-0.948683, 0.316228}};
    CarrierRecovery crec(const_points, 0.707f, 0.0628f/120.0f, BufferSize);

    FirRate out_rate = {FmRate / BaudRate, 1};
    auto out_taps = Generate_Generic_LPF(FmRate, BaudRate/2, 0.5f*FmRate/BaudRate, 3, Kaiser);
    PolyPhaseFIR<CF32> out_resamp(out_taps, out_rate, BufferSize);

    auto rrc_taps = Generate_Root_Raised_Cosine(FmRate, BaudRate, 0.5f, 1.0f, 11.0f);
    FirFilter<CF32> rrc_fir(rrc_taps, {1,1}, BufferSize);

    // connect
    agc.connect(src);
    rrc_fir.connect(agc);
    pll.connect(rrc_fir);
    crec.connect(pll);
    //out_resamp.connect(pll);

    // start blocks
    blocks.push_back(&src);
    blocks.push_back(&rrc_fir);
    blocks.push_back(&agc);
    blocks.push_back(&pll);
    blocks.push_back(&crec);
    //blocks.push_back(&out_resamp);

    for (auto &blk : blocks)
        blk->start();

    // simulate some delay
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // DEBUG -> Write to stdout
    auto stream = pll.getOutputStream();
    std::vector<CF32> inputSamples(FmRate/60, 0);

    //auto stream_2 = pll.getOutputStream();
    std::vector<CF32> inputSamples_2(4096, 0);
    std::vector<CF32> errors;
    while(1){
        stream->WaitCv();
        //pll.error_stream.WaitCv();
        size_t nread = stream->readFromBuffer(inputSamples);
        //nread = std::min(nread, pll.error_stream.readFromBuffer(inputSamples_2));
        // try demod
        for (size_t i = 0; i < nread; i++)
        {
            WriteCout(0.5f * inputSamples[i]);
            //WriteCout(0.5f * inputSamples_2[i]);
            //errors.push_back(inputSamples_2[i]);
        }
        //if(errors.size() > 1024 * 1024){
        //    break;
        //}
    }

    // simulate some delay
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    //WriteWav("PLL_Error.wav", ComplexInterleave(errors), BaudRate, 2, 1.0f);

    // stop
    for (auto &blk : blocks)
        blk->stop();

    return 0;
}