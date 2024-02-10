# PopoSDR

> [!WARNING]
>
> This code is still experimental and was only tested on very specific conditions.

### C/C++ Based SDR library with multithreaded pipelined DSP blocks

#### Available Blocks:
###### Sources:
- Network Source (UDP // TCP Server or Client)
###### Filters:
- FIR Filter (Decimating or Interpolating) :white_check_mark:
- FIR Taps (Gaussian, Lowpass, Root Raised Cosine) :white_check_mark:
- CIC Filter (Decimating or Interpolating) :warning:
###### General:
- AGC (Automatic Gain Control) :white_check_mark:
- Carrier Recovery (Decision-Directed) :white_check_mark:
- Timing PLL (Decision-Directed Timing Recovery) :warning:
- FM Modulator :white_check_mark:
- FM Demodulator (Differentiate & Delay) :warning:
- Constellation Mapper (Converts Bytes to Symbols) :white_check_mark:
- Constellation Demapper (Not implemented) :x:
###### Utils:
- Wave File Writing (.wav) for debugging tests


Some blocks are [EXPERIMENTAL] and marked with the :warning: this means they have some strange behaivour in some cases or just haven't been tested enough.

#### Modems (STILL IN DEV)
- Afsk Modulator (BELL 202 Like 1200Bd FSK Modulator)

- Gmsk Modulator (GMSK 4800bd Modulator)

#### Examples (EXPERIMENTAL)
##### Simulation:
- **AFSK Test** :white_check_mark: - Generates AFSK Packets and demodulates them writing some debug Wave files.

- **GMSK Test** :white_check_mark: - Generates GMSK Packets with CCSDS Syncword and demodulates them printing the detected syncwords, also writes debugging Wave files.
- **Main Test** :warning: - This is just some general testing code, it should write one S16-LE IQ stream on stdout with 32-APSK symbols and write some Wave Files testing the CIC filters...

##### Network sources:
- **WFM Mono UDP** :white_check_mark: - Reads S16-LE IQ samples at 1 MHz SampleRate from one local UDP client on port 1234 feeds them to one WFM (Wide FM Station) demodulates the Mono chanel and writes one 48KHz S16-LE audio stream on stdout.

- **GMSK Rx** :warning: - Reads S16-LE IQ samples at 1MHz SampleRate from one local TCP server on port 1234 feeds them to a 1200Bd GMSK BT=0.5 demodulator that searches for the `0x1ACFFC1D` Syncword and prints their data to stderr. It also outputs one S16LE 4800Hz stream on stdout for debugging.

- **QAM RX** :warning: - Reads S16-LE IQ samples at 48KHz SampleRate from one local TCP and feeds them to the experimental 16-QAM Demodulator outputting the synchronized IQ symbols via stdout as one 4800Hz IQ S16-LE stream... This is still on the testing phase!

#### Info: 
- S16-LE - 16 bit signed integer
- IQ - Quadrature signal, 2 channels, Real & Imaginary

#### // TODO

- :white_check_mark: Upload code.
- :white_check_mark: Get a modem example working.
- :white_check_mark: Fix the modulator code to generate the GMSK waveform again - AFSK1200 right now...
- :white_check_mark: Split the lib and modem code
- :white_large_square: Refactor modulator code.
- :white_large_square: Make the demodulator class & cleanup main
- :white_large_square: Make a modem class.
- :white_check_mark: Move main code to examples
- :white_check_mark: Setup CMake to build the lib / modems / examples
- :white_large_square: Add one build guide.
- :white_large_square: Document some of the API.
- :white_large_square: Add more blocks.

#### // Future plans / Goals

- :white_check_mark: Implement AFSK modulator
- :white_check_mark: Implement one PSK modulator
- :white_large_square: Implement one Burst PSK demodulator (FFT-based)
- :white_check_mark: Implement one Decision directed synchronizer (a * conj(â))
- :white_large_square: Implement more advanced synchronizers
- :white_large_square: Complete GMSK modem
- :white_large_square: Complete FSK / AFSK modems
- :white_large_square: Complete M-PSK / QAM / APSK modems
- :white_large_square: Add OFDM ?
- :white_large_square: Add DSSS / CDMA ?
