#include <FmDemodulator.h>
#include <Utils.h>
#include <complex>

FmDemodulator::FmDemodulator(float DeviationHz, size_t SampleRate) : SyncBlock(131072, 1, 1), mNumCoeffs(5),
mDelayLine(2),  mFifo(5), mCoeffs({-3.0f/16.0f, 31.0f/32.0f, 0.0f, -31.0f/32.0f, 3.0f/16.0f}) // Lyons-2 Coeffs
{
    m_name = "FmDemodulator";
    LOG_DEBUG("Created Fm Demodulator.");
    m_inv_deviation = 1.0f / (M_TWOPI_F * (DeviationHz / (float)SampleRate));
}

size_t FmDemodulator::work(const size_t& n_inputItems, std::vector<CF32>&  input, std::vector<F32>& output){
    size_t outputIdx = 0;
    for (size_t i = 0; i < n_inputItems; i++)
    {
        mDelayLine.pop_back();          // remove last
        mDelayLine.insert(mDelayLine.begin(),input.at(i)); // push new 

        mFifo.pop_back();          // remove last
        mFifo.insert(mFifo.begin(),input.at(i)); // push new

        CF32 z_dlay = mDelayLine.back(); // get last
        // Apply FIR differentiator (scale by tap length)
        CF32 z_hat = std::inner_product(mFifo.begin(), mFifo.end(), mCoeffs.begin(), CF32(0.0f));

        // Fm demod via Differentiator
        F32 demod = z_dlay.real() * z_hat.imag()  - z_dlay.imag() * z_hat.real();
        demod *= m_inv_deviation; // Normalize
        
        output.at(outputIdx++) = demod; // write output
    }
    return outputIdx;
}

size_t FmDemodulator::work_slow(const size_t& n_inputItems, std::vector<CF32>&  input, std::vector<F32>& output){
    size_t outputIdx = 0;
    CF32 delay = 0.0;
    for (size_t i = 0; i < n_inputItems; i++)
    {
        F32 demod = std::arg(input[i] * std::conj(delay)); // slow demod
        demod *= m_inv_deviation; // Normalize
        delay = input[i]; // update delay
        output.at(outputIdx++) = demod; // write output
    }
    return outputIdx;
}

void FmDemodulator::Reset(){
    std::fill(mDelayLine.begin(), mDelayLine.end(), 0.0f);
    std::fill(mFifo.begin(), mFifo.end(), 0.0f);
}

FmDemodulator::~FmDemodulator(){
    LOG_DEBUG("Destroyed Fm Demodulator.");
}