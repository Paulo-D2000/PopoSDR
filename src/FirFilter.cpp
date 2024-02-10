#include <FirFilter.h>

template <typename T>
FirFilter<T>::FirFilter(const std::vector<F32>& taps, const FirRate& rate, const size_t& BufferSize):
 SyncBlock<T>(BufferSize, rate.Decimation, rate.Interpolation), m_rate(rate),
  m_taps(std::move(taps)), m_ptr(0), m_buffer(taps.size()) {
    this->m_name = "FirFilter";
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
    if(m_rate.Interpolation >= 1 and m_rate.Decimation == 1){
        for (size_t i = 0; i < n_inputItems; i++)
        {
            T sample = input.at(i);
            for (size_t j = 0; j < m_rate.Interpolation; j++)
            {
                output.at(outputIdx++) = filter(sample);
                sample *= 0.0f;
            }
        }
    }else if(m_rate.Decimation > 1 and m_rate.Interpolation == 1){
        for (size_t i = 0; i < n_inputItems; i+=m_rate.Decimation)
        {
            T sample = input.at(i);
            output.at(outputIdx++) = filter(sample);
        }
    }
    else{
        LOG_ERROR("FirFilter Interpolation must be >= 1 or Decimation > 1!");
    }
    return outputIdx;
}

template <typename T>
FirFilter<T>::~FirFilter(){
    LOG_DEBUG("Destroyed FirFilter.");
}

template <typename T>
PolyPhaseFIR<T>::PolyPhaseFIR(const std::vector<F32> &taps, const FirRate &rate, const size_t &BufferSize):
SyncBlock<T>(BufferSize, rate.Decimation, rate.Interpolation), m_rate(rate)
{
    this->m_name = "PolyPhaseFIR";

    if(m_rate.Interpolation >1 && m_rate.Decimation != 1){
        LOG_ERROR("Interpolating FIR! Decimation Must be 1");
        exit(-1);
    }

    if(m_rate.Decimation > 1 && m_rate.Interpolation != 1){
        LOG_ERROR("Decimating FIR! Interpolation Must be 1");
        exit(-1);
    }
    
    size_t newsize = taps.size();
    size_t nfilts = std::max(m_rate.Decimation, m_rate.Interpolation);

    if(newsize % nfilts != 0){
        newsize += nfilts - (newsize % nfilts);
    }
    std::vector<F32> finalTaps(newsize, 0);
    std::copy(taps.begin(), taps.end(), finalTaps.begin());

    LOG_DEBUG("[{}] Allocated {} Taps in {}x {}-Tap Banks", this->m_name, newsize, nfilts, newsize/nfilts);

    std::vector <F32> tapseg(newsize / nfilts);
    for (size_t i = 0; i < nfilts; i++)
    { 
        for (size_t j = 0; j < tapseg.size(); j++)
        {
            tapseg[j] = finalTaps[i+j*nfilts];
        }
        m_filterbank.push_back(new FirFilter<T>(tapseg, {1,1}, 0));
    }
    
    LOG_DEBUG("Created Polyphase FIR");
}

template <typename T>
size_t PolyPhaseFIR<T>::work(const size_t &n_inputItems, std::vector<T> &input, std::vector<T> &output)
{
    size_t outputIdx = 0;
    if(m_rate.Interpolation >= 1 && m_rate.Decimation == 1){
        for (size_t i = 0; i < n_inputItems; i++)
        {
            T sample = input[i];
            for (size_t nf = 0; nf < m_filterbank.size(); nf++)
            {
                output[outputIdx++] = m_filterbank[nf]->filter(sample);
            }
        }
    }else if(m_rate.Decimation > 1 && m_rate.Interpolation == 1){
        size_t nf = 0;
        std::fill(output.begin(), output.end(), T());
        for (size_t i = 0; i < n_inputItems; i++)
        {   
            if(nf == m_filterbank.size()){
                nf = 0;
            }
            output[i/m_rate.Decimation] += m_filterbank[nf++]->filter(input[i]);
        }
        return n_inputItems/m_rate.Decimation;
    }
    else{
        LOG_ERROR("PolyhaseFIR Interpolation must be >= 1 or Decimation > 1!");
    }
    return outputIdx;
}

template <typename T>
PolyPhaseFIR<T>::~PolyPhaseFIR()
{
    for (auto &fir : m_filterbank)
    {
        if(fir != nullptr){
            delete fir;
        }
    }
    
    
    LOG_DEBUG("Destroyed Polyphase FIR");
}

template class FirFilter<F32>;
template class FirFilter<CF32>;

template class PolyPhaseFIR<F32>;
template class PolyPhaseFIR<CF32>;