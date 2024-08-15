#pragma once

#include <Block.h>
#include <ControlLoop.h>

enum TLL_Type {MMSE, PFB};

template <typename T>
class TimingPLL: public SyncBlock<T>
{
private:
    F32 m_sps;
    I32 m_step;
    I32 m_counter;
    I32 m_pcounter;
    
    F32 m_alpha;
    T m_prevInput;
    T m_prevprevInput;
    std::vector<std::vector<F32>> m_userTaps;
    TLL_Type m_type;
    ControlLoop m_cloop;
    Constellation m_const_obj;
public:
    struct TLL_Config{
        F32 Damping = 0.707f;
        F32 LoopBw = 1.0f/100.0f;
        TLL_Type Type=MMSE;
        std::vector<std::vector<F32>> PFB_Taps={};
        Constellation Constellation_Obj = {{-1,-1},{-1,1},{1,-1},{1,1}};
    };

    TimingPLL(const size_t& SampleRate, const size_t& SymbolRate, const float& Alpha=0.75f, const size_t& BufferSize=131072);

    TimingPLL(const size_t& SampleRate, const size_t& SymbolRate,
     TLL_Config Config = {
        0.707f,
        1.0f/100.0f,
        MMSE,
        {},
        {{-1,-1},{-1,1},{1,-1},{1,1}}
      },
      const size_t& BufferSize=131072
    );

    size_t work(const size_t& n_inputItems, std::vector<T>&  input, std::vector<T>& output);

    Stream<CF32> error_stream;

    std::vector<CF32> error_vec;

    void Reset();

    ~TimingPLL();
};
