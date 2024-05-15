// General includes
#include <iostream>
#include <thread>
#include <random>
#include <cstring>
#include <iterator>

// Demodulator includes
#include <TimingPLL.h>
#include <FirFilter.h>
#include <Filters.h>
#include <Utils.h>
#include <Agc.h>
#include <Networking.h>
#include <CarrierRecovery.h>
#include <TerminalSink.h>

int main(){
    size_t FmRate     = 48e3;      // 48kHz
    size_t BaudRate   = FmRate/8; //4800 Bd

    // Bufer Sizes
    size_t PacketSize = 8192*4; // Change to match socket input...
    size_t BufferSize = 131072;

    // Blocks
    std::vector<Block*> blocks;
    SocketSource<CF32, TCP> src(1234, PacketSize, true, 2*BufferSize);

    // Compensate for FIR peaks & input level
    Agc<CF32> agc(1e-5f, 0.707f, BufferSize);

    // 16-QAM Constellation from GnuRadio
    std::vector<CF32> const_points = {{0.316228, -0.316228}, {-0.316228, -0.316228}, {0.948683, -0.948683}, {-0.948683, -0.948683}, {-0.948683, -0.316228}, {0.948683, -0.316228}, {-0.316228, -0.948683}, {0.316228, -0.948683}, {-0.948683, 0.948683}, {0.948683, 0.948683}, {-0.316228, 0.316228}, {0.316228, 0.316228}, {0.316228, 0.948683}, {-0.316228, 0.948683}, {0.948683, 0.316228}, {-0.948683, 0.316228}};
    float avg = Vec_AvgPwr(const_points);
    LOG_INFO("AVG: %.3f", avg);
    for (size_t i = 0; i < const_points.size(); i++)
    {
        const_points[i] /= avg;
    }
    
    CarrierRecovery crec(const_points, 0.707f, 1.0/200.0f, BufferSize);

    size_t nfilts = 32;
    auto rrc_pfb_taps = Generate_Root_Raised_Cosine(FmRate*nfilts, BaudRate, 0.5f, nfilts, 11);
    
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
    
    // Timing recovery PLL with Polyphase Matched filter
    TimingPLL<CF32> pll(FmRate, BaudRate, PFB, rrc_pfb, BufferSize);

    // Output IQ samples via stdout;
    TerminalSink<CF32> samp_out(BaudRate, BufferSize);

    // connect
    agc.connect(src);
    pll.connect(agc);
    crec.connect(pll);
    samp_out.connect(crec);

    // start blocks
    blocks.push_back(&src);
    blocks.push_back(&agc);
    blocks.push_back(&pll);
    blocks.push_back(&crec);
    blocks.push_back(&samp_out);

    for (auto &blk : blocks)
        blk->start();

    // wait for user input
    char quit;
    std::cin >> quit;

    // stop
    for (auto &blk : blocks)
        blk->stop();

    return 0;
}