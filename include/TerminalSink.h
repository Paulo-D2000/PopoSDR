#pragma once

#include <Block.h>

template <typename IT>
class TerminalSink: public SinkBlock<IT>
{
private:
    size_t m_SampleRate;

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

public:
    TerminalSink(size_t SampleRate, size_t BufferSize);

    virtual size_t work(const size_t& n_inputItems, std::vector<IT>&  input);

    ~TerminalSink();
};