# PopoSDR

> [!WARNING]
>
> This code is still experimental and was only tested on Windows x64 with msvc.

### C/C++ Based SDR library with multithreaded pipelined DSP blocks

- ModemGmsk example on main.cpp

#### // TODO

- :white_check_mark: Upload code.
- :white_check_mark: Get a modem example working.
- :white_check_mark: Fix the modulator code to generate the GMSK waveform again - AFSK1200 right now...
- :white_check_mark: Split the lib and modem code
- :white_large_square: Make the demodulator class & cleanup main
- :white_large_square: Move main code to examples
- :white_large_square: Setup CMake to build the lib / modems / examples
- :white_large_square: Add one build guide.
- :white_large_square: Document some of the API.
- :white_large_square: Add more blocks.
- :white_large_square: Refactor modem code.

#### // Future plans / Goals

- :white_check_mark: Implement AFSK modulator
- :white_large_square: Implement one PSK modulator
- :white_large_square: Implement one Burst PSK demodulator (FFT based)
- :white_large_square: Implement one Decision directed synchronizer (a * conj(â))
- :white_large_square: Implement more advanced synchronizers
- :white_large_square: Complete GMSK modem
- :white_large_square: Complete FSK / AFSK modems
- :white_large_square: Complete M-PSK / QAM / APSK modems
- :white_large_square: Add OFDM ?
- :white_large_square: Add DSSS / CDMA ?
