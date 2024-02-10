#include <FmDemodulator.h>
#include <FirFilter.h>
#include <Filters.h>
#include <Utils.h>
#include <WaveFile.h>
#include <Agc.h>
#include <Networking.h>

int main(){
    // Vars
    size_t SampleRate = 1000000;  // 1000kHz
    size_t FmRate     = 250000;   // 250kHz
    size_t AudioRate  = 50000;    //48kHz
    float Cutoff      = 15000.0f; // 15kHz

    // Bufer Sizes
    size_t BufferSize = 131072;
    size_t PacketSize = 16384;
    
    FirRate demodRate = {
        .Interpolation = 1,
        .Decimation = SampleRate / FmRate
    };

    FirRate finalRate = {
        .Interpolation = 1,
        .Decimation = FmRate / AudioRate  
    };

    // fir taps
    std::vector<F32> demodTaps = Generate_Generic_LPF(FmRate,
                                                    FmRate/2,
                                                    1.0f,
                                                    11,
                                                    Kaiser);
    // fir taps
    std::vector<F32> audioTaps = Generate_Generic_LPF(AudioRate,
                                                    Cutoff,
                                                    1.0f,
                                                    11,
                                                    Kaiser);

    // Blocks
    FmDemodulator fm(75e3f, FmRate);
    Agc<CF32> agc(1e-5f, 0.707f, BufferSize);
    SocketSource<CF32, UDP> src(1234, PacketSize, false, BufferSize);
    FirFilter<CF32> demod_lpf(demodTaps, demodRate, BufferSize);
    FirFilter<F32> audio_lpf(audioTaps, finalRate, BufferSize);

    // connect
    agc.connect(src);
    demod_lpf.connect(agc);
    fm.connect(demod_lpf);
    audio_lpf.connect(fm);

    // start
    audio_lpf.start();
    fm.start();
    demod_lpf.start();
    agc.start();
    src.start();

    // simulate some delay
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // DEBUG -> Write to stdout
    auto stream = audio_lpf.getOutputStream();
    std::vector<F32> inputSamples(256, 0);
    std::vector<F32> outputSamples;

    while(1){
        stream->WaitCv();
        if(outputSamples.size() > AudioRate * 500){
            break;
        }
        size_t nread = stream->readFromBuffer(inputSamples, inputSamples.size());
        std::copy(inputSamples.begin(), inputSamples.end(), std::back_inserter(outputSamples));
        // Write to stdout
        for (size_t i = 0; i < nread; i++)
        {
            I16 val = (short)(0.5f * inputSamples[i] * (float)INT16_MAX);
            std::cout << (U8)(val & 0xFF) << (U8)((val >> 8) & 0xFF);
        }
    }

    // simulate some delay
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // stop
    src.stop();
    agc.stop();
    demod_lpf.stop();
    fm.stop();
    audio_lpf.stop();

    // Dump samples to wave file
    if(0){
    WriteWav("Test-FM-Demod.wav", outputSamples, AudioRate, 1, 0.5);
        float spwr = Vec_AvgPwr(outputSamples);
        LOG_TEST("Avg Power: {:.3f} V(rms)",spwr);
    }
    // exit
    return 0;
}