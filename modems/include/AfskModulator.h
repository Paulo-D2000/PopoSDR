#pragma once

#include <iostream>
#include <Buffer.h>
#include <FirFilter.h>
#include <FmModulator.h>

struct AfskModulatorConfig{
    U8 Preamble = 0x7E;
    size_t PreambleLen = 50;
    size_t PostambleLen = 7;
};

class AfskModulator
{
public:
    AfskModulator(size_t BaudRate=1200, size_t SampleRate=48000, size_t BufferSize=4096);

    void sendPacket(const std::vector<U8>& packet, AfskModulatorConfig cfg = {});
    void modulate(Stream<U8>& input_stream);
    void start(){
        m_fmmod->start();
        m_interpolator->start();
    }

    void stop(){
        m_inp_stream.close();
        m_fmmod->stop();
        m_interpolator->stop();
    }
    
    size_t GetSampleRate() { return m_samplerate;}
    size_t GetBaudRate() { return m_samplerate/(m_interp*m_sps);}
    Stream<CF32>* GetStream() { return m_out_stream;}

    ~AfskModulator();

private:
    size_t m_interp;
    size_t m_sps;
    size_t m_buffsize;
    size_t m_samplerate;

    U8 m_chunk[8];
    F32 m_nrz[8];
    U8 m_nrzi=1;
    
    Stream<F32> m_inp_stream;
    Stream<CF32>* m_out_stream;
    FirFilter<CF32>* m_interpolator;
    FmModulator* m_fmmod;
};
