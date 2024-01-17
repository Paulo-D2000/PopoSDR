#include <FirFilter.h>

template <typename T>
FirFilter<T>::FirFilter(const std::vector<F32>& taps, const size_t& BufferSize, const FirRate& rate): SyncBlock<T>(BufferSize, 1, rate.Interpolation) {
    m_rate = rate;
    this->m_name = "FirFilter";
    m_taps = std::move(taps);
    m_buffer.resize(m_taps.size());
    m_ptr = 0;
    LOG_DEBUG("Created FirFilter.");
}

template <typename T>
void FirFilter<T>::loadTaps(const std::vector<F32>& taps){
    m_taps = std::move(taps);
    m_buffer.resize(m_taps.size());
    m_ptr = 0;
    LOG_DEBUG("Loaded {} Taps on FirFilter.",m_taps.size());
}

template <typename T>
T FirFilter<T>::filter(const T& input){
    T output = T(0);
    m_buffer[m_ptr++] = input;
    size_t m_sumidx = m_ptr;
    if(m_ptr == m_buffer.size())
        m_ptr = 0;
    for (size_t i = 0; i < m_taps.size(); i++)
    {
        if(m_sumidx > 0){
            m_sumidx--;
        }else{
            m_sumidx = m_taps.size() - 1;
        }
        output += m_buffer[m_sumidx] * m_taps[i];
    }
    return output;
}

template <typename T>
size_t FirFilter<T>::work(const size_t& n_inputItems, std::vector<T>&  input, std::vector<T>& output){
    size_t outputIdx = 0;
    if(m_rate.Interpolation >= 1){
        for (size_t i = 0; i < n_inputItems; i++)
        {
            T sample = input.at(i);
            for (size_t j = 0; j < m_rate.Interpolation; j++)
            {
                output.at(outputIdx++) = filter(sample);
                sample *= 0.0f;
            }
        }
        return outputIdx;
    }else{
        LOG_ERROR("FirFilter Interpolation must be >= 1 or Decimation > 1!");
        return 0;
    }
}

template <typename T>
FirFilter<T>::~FirFilter(){
    LOG_DEBUG("Destroyed FirFilter.");
}

template class FirFilter<F32>;
template class FirFilter<CF32>;