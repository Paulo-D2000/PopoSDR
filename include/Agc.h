#pragma once
#include <Block.h>
#include <algorithm>

template <typename T>
class Agc: public SyncBlock<T>
{
private:
    float m_gain;
    float m_value;
    float m_reference;

public:
    Agc(float LoopGain=1e-4f, float LevelReference=1.0f, const size_t& BufferSize=0);

    size_t work(const size_t& n_inputItems, std::vector<T>&  input, std::vector<T>& output);

    ~Agc();
};
