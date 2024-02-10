#include <Agc.h>

template<typename T>
Agc<T>::Agc(float LoopGain, float LevelReference, const size_t& BufferSize):
 SyncBlock<T>(BufferSize), m_gain(LoopGain), m_reference(LevelReference){
    m_value = 1.0f;
    this->m_name = "Agc";
    LOG_DEBUG("Created Agc");
}

template<typename T>
size_t Agc<T>::work(const size_t& n_inputItems, std::vector<T>&  input, std::vector<T>& output){
    size_t outputIdx = 0;
    for (size_t i = 0; i < n_inputItems; i++)
    {
        T out = input[i] * m_value;
        float error = m_reference - std::abs(out);
        m_value += error * m_gain;
        // Clamp gain
        m_value = std::clamp(m_value, -65535.0f, 65535.0f);
        output[outputIdx++] = out;
    }
    return outputIdx;
}

template<typename T>
Agc<T>::~Agc(){
    LOG_DEBUG("Destroyed Agc")
}

template class Agc<F32>;
template class Agc<CF32>;