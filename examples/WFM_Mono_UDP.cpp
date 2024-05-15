#include <FmDemodulator.h>
#include <FirFilter.h>
#include <Filters.h>
#include <Utils.h>
#include <WaveFile.h>
#include <Agc.h>
#include <Networking.h>
#include <TerminalSink.h>

int main(){

    // Vars
    size_t SampleRate  = 1000000;  // 1.0 MHz
    size_t FmSRate     = 250000;   // 250 kHz
    size_t DemodSRate  = 50000;    // 50 kHz
    size_t AudioSRate  = 48000;    // 48 kHz
    float Cutoff       = 15000.0f; // 15 kHz

    // Bufer Sizes
    size_t BufferSize = 131072;
    size_t PacketSize = 1024;
    
    FirRate demodRate = {
        .Interpolation = 1,
        .Decimation = SampleRate / FmSRate
    };

    FirRate finalRate = {
        .Interpolation = 1,
        .Decimation = FmSRate / AudioSRate  
    };

    // fir taps
    std::vector<F32> demodTaps = Generate_Generic_LPF(FmSRate,
                                                    FmSRate/2,
                                                    1.0f,
                                                    5,
                                                    Kaiser);
    // fir taps
    std::vector<F32> audioTaps = Generate_Generic_LPF(DemodSRate,
                                                    Cutoff,
                                                    1.0f,
                                                    5,
                                                    Kaiser);

    // Blocks
    FmDemodulator fm(75e3f, FmSRate);
    Agc<CF32> agc(1e-5f, 0.707f, BufferSize);
    SocketSource<CF32, UDP> src(1234, PacketSize, false, BufferSize);
    FirFilter<CF32> demod_lpf(demodTaps, demodRate, BufferSize);
    FirFilter<F32> audio_lpf(audioTaps, finalRate, BufferSize);
    TerminalSink<F32> samp_out(AudioSRate, BufferSize);

    // Resample
    size_t mul = std::gcd(DemodSRate, AudioSRate);
    size_t up = AudioSRate / mul, down = DemodSRate / mul;

    FirRate up_rate = {up, 1};
    auto up_taps = Generate_Generic_LPF(DemodSRate*up, AudioSRate/2, up, 3, Kaiser);
    PolyPhaseFIR<F32> up_resamp(up_taps, up_rate, BufferSize);

    FirRate down_rate = {1, down};
    auto down_taps = Generate_Generic_LPF(DemodSRate*up, AudioSRate/2, 1.0f, 3, Kaiser);
    PolyPhaseFIR<F32> down_resamp(down_taps, down_rate, BufferSize);

    // connect
    agc.connect(src);
    demod_lpf.connect(agc);
    fm.connect(demod_lpf);
    audio_lpf.connect(fm);
    up_resamp.connect(audio_lpf);
    down_resamp.connect(up_resamp);
    samp_out.connect(down_resamp);

    // start
    samp_out.start();
    down_resamp.start();
    up_resamp.start();
    audio_lpf.start();
    fm.start();
    demod_lpf.start();
    agc.start();
    src.start();

    // Wait for User input -> quit
    char quit;
    std::cin >> quit;

    // stop
    src.stop();
    agc.stop();
    demod_lpf.stop();
    fm.stop();
    audio_lpf.stop();
    up_resamp.stop();
    down_resamp.stop();

    // Dump samples to wave file
    //if(0){
    //WriteWav("Test-FM-Demod.wav", outputSamples, AudioRate, 1, 0.5);
    //    float spwr = Vec_AvgPwr(outputSamples);
    //    LOG_TEST("Avg Power: %.3f V(rms)",spwr);
    //}

    // exit
    return 0;
}
