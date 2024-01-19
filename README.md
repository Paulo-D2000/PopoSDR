# PopoSDR

> [!WARNING]
>
> This code is still experimental and needs SoapySDR to compile, only tested on Windows x64 with msvc.

### C/C++ Based SDR library with multithreaded pipelined DSP blocks

- ModemGmsk example on main.cpp

#### // TODO
- [x] Upload code.
- [x] Get a modem example working.
- [x] Fix the modulator code to generate the GMSK waveform again - AFSK1200 right now...
- [x] Split the lib and modem code
- [ ] Make the demodulator class & cleanup main
- [ ] Move main code to examples
- [ ] Setup CMake to build the lib / modems / examples
- [ ] Add one build guide.
- [ ] Document some of the API.
- [ ] Add more blocks.
- [ ] Refactor modem code.

#### // Future plans / Goals
- [x] Implement AFSK modulator
- [ ] Implement one PSK modulator
- [ ] Implement one Burst PSK demodulator (FFT based)
- [ ] Implement one Decision directed synchronizer (a * conj(â))
- [ ] Implement more advanced synchronizers
- [ ] Complete GMSK modem
- [ ] Complete FSK / AFSK modems
- [ ] Complete M-PSK / QAM / APSK modems
- [ ] Add OFDM ?
- [ ] Add DSSS / CDMA ?
