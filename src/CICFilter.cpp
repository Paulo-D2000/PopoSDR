#include <CICFilter.h>


CICFilter::CICFilter(const size_t& Order, const CICRate& rate, const size_t& BufferSize): SyncBlock<CF32>(BufferSize, 1, rate.Interpolation) {
    m_rate = rate;
    this->m_name = "CICFilter";
    for (size_t i = 0; i < Order; i++)
    {
        m_integ.push_back({CI64(0, 0)});
        m_comb.push_back({std::vector<CI64>(std::max(m_rate.Interpolation, m_rate.Decimation), CI64{0,0})});
    }
    LOG_DEBUG("Created CICFilter.");
}



CF32 CICFilter::filter(const CF32& input){
    CI64 output = {0,0};
    output.fromComplex(input);

    // Integrator
    for (size_t i = 0; i < m_integ.size(); i++)
    {
        output = m_integ[i].update(output);
    }

    // Comb
    for (size_t i = 0; i < m_comb.size(); i++)
    {
        output = m_comb[i].update(output);
    }
    
    return output.toComplex(1 << (16 - m_comb.size()));
}


size_t CICFilter::work(const size_t& n_inputItems, std::vector<CF32>&  input, std::vector<CF32>& output){
    size_t outputIdx = 0;
    if(m_rate.Interpolation > 1 && m_rate.Decimation == 1){
        for (size_t i = 0; i < n_inputItems; i++)
        {
            CF32 sample = input.at(i) / (float)m_rate.Interpolation;
            for (size_t j = 0; j < m_rate.Interpolation; j++)
            {
                output.at(outputIdx++) = filter(sample);
                sample *= 0.0f;
            }
        }
        return outputIdx;
    }
    else if(m_rate.Decimation >= 1 && m_rate.Interpolation == 1){
        for (size_t i = 0; i < n_inputItems; i+=m_rate.Decimation)
        {
            for (size_t j = 0; j < m_rate.Decimation; j++)
            {
                CF32 sample = input.at(i);
                output.at(outputIdx) = filter(sample);
            }
            outputIdx++;
        }
        return outputIdx;
    }
    else{
        LOG_ERROR("CICFilter Interpolation must be >= 1 or Decimation > 1!");
        return 0;
    }
}


CICFilter::~CICFilter(){
    LOG_DEBUG("Destroyed CICFilter.");
}