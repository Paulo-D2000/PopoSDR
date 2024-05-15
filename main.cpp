#include <iostream>
#include <fstream>
#include <thread>
#include <random>

#include <WaveFile.h>
#include <Buffer.h>
#include <Utils.h>
#include <CICFilter.h>
#include <Constellation.h>
#include <Constants.h>


/* This file is only for testing */

/* Init random generator */
std::mt19937 rng(std::random_device{}());
// Random bytes distribuition
std::uniform_int_distribution<U32> dist(0, UINT32_MAX);
std::normal_distribution<F32> ndist(0.0f,1.0f);

/*
class SoapySink: public SyncBlock<CF32>
{
public:
    struct Settings{
        double SampleRate;
        double Frequency;
        size_t BufferSize=131072;
    };
    SoapySink(std::string KwArgs, Settings config={.SampleRate=1e6,.Frequency=433e6}): SyncBlock(config.BufferSize, Sink) {
        m_name = "SoapySink";
        LOG_DEBUG("Created Soapy Sink.");

        SoapySDR::Kwargs args = SoapySDR::KwargsFromString(KwArgs);
        m_sdr = SoapySDR::Device::make(args);

        if(m_sdr == NULL){
            LOG_ERROR("SoapySDR::Device::make failed");
            exit(EXIT_FAILURE);
        }

        // 3. apply settings
        m_sdr->setSampleRate( SOAPY_SDR_TX, 0, config.SampleRate);
        m_sdr->setFrequency( SOAPY_SDR_TX, 0, config.Frequency);

        // 4. setup a stream (complex floats)
        m_tx_stream = m_sdr->setupStream( SOAPY_SDR_TX, SOAPY_SDR_CF32);
        if( m_tx_stream == NULL)
        {
            LOG_ERROR("Failed");
            SoapySDR::Device::unmake( m_sdr );
            exit(EXIT_FAILURE);
        }
        m_sdr->activateStream( m_tx_stream, 0, 0, 0);
    }
    size_t work(const size_t& n_inputItems, std::vector<CF32>&  input, std::vector<CF32>& output){
        void *buffs[] = {input.data()};
	    int flags;
	    long long time_ns;
	    int ret = m_sdr->writeStream(m_tx_stream, buffs, n_inputItems, flags, time_ns, 1e5);
	    LOG_DEBUG("ret = {}, flags = {}, time_ns = {}", ret, flags, time_ns);
        return ret;
    }
    ~SoapySink(){
        m_sdr->deactivateStream( m_tx_stream, 0, 0);	//stop streaming
        m_sdr->closeStream( m_tx_stream );

        // 8. cleanup device handle
        SoapySDR::Device::unmake( m_sdr );
        LOG_DEBUG("Destroyed Soapy Sink.");
    }
private:
    SoapySDR::Device *m_sdr;
    SoapySDR::Stream *m_tx_stream;
};
*/

void _writeFloat(F32 data){
    I16 val = (short)(0.5f * data * (float)INT16_MAX);
    std::cout << (U8)(val & 0xFF) << (U8)((val >> 8) & 0xFF);
}

void WriteCout(F32 data){
    _writeFloat(data);
}

void WriteCout(CF32 data){
    _writeFloat(data.real());
    _writeFloat(data.imag());
}

int main(){
	//SoapySink sink("driver=plutosdr", {.SampleRate=1e6, .Frequency=433e6, .BufferSize=4096});
    
    //sink.connect(stream);

    //sink.start();
    
    // CIC Converter
    CICRate Rate = {24, 15};
    CICFilter cic_interp(5, {Rate.Interpolation, 1}, 1024);
    CICFilter cic_decim(5, {1, Rate.Decimation}, 1024);

    CICFilter resampler(1, Rate, 1024);

    // Samples
    size_t Nsamples = 8192;
    std::vector<CF32> input_data(Nsamples);
    std::vector<CF32> temp_data(Nsamples * Rate.Interpolation);
    std::vector<CF32> output_data(Nsamples * Rate.Interpolation);

    // AWGN Noise
    F32 npwr = 1.0f / sqrtf(2.0f);
    for (size_t i = 0; i < input_data.size(); i++)
    {
        input_data[i] =  CF32(npwr*ndist(rng),npwr*ndist(rng));
    }
    
    // Filter
    cic_interp.work(input_data.size(), input_data, temp_data);
    cic_decim.work(temp_data.size(), temp_data, output_data);

    float spwr = Vec_AvgPwr(output_data);
    LOG_TEST("Avg Power: {:.3f} V(rms)",spwr);

    WriteWav("test_cic.wav", ComplexInterleave(output_data), 48000, 2, 500.0f);

    std::vector <CF32> points;

    for (size_t i = 0; i < 4; i++)
    {
        float angle = (float)i / 4.0f + 0.125f;
        points.push_back(0.4f * 0.707f * std::exp(_1j * M_TWOPI_F * angle));
    }

    for (size_t i = 0; i < 12; i++)
    {
        float angle = (float)i / 12.0f;
        points.push_back(0.67f * std::exp(_1j * M_TWOPI_F * angle));
    }

    for (size_t i = 0; i < 16; i++)
    {
        float angle = (float)i / 16.0f + 0.5f / 16.0f;
        points.push_back(std::exp(_1j * M_TWOPI_F * angle));
    }
    

    ConstellationMapper qam_mapper(points, 131072);
    ConstellationDemapper qam_demapper(points, 131072);

    std::vector<U8> bitsin(256, 0);
    std::vector<U8> bitsout(256, 0);

    // Test the Map -> Demap
    while(1){
        for (size_t i = 0; i < 256; i++)
        {
            bitsin[i] = dist(rng) & 1;
        }
        size_t n = qam_mapper.work(bitsin.size(), bitsin, output_data);
        size_t n2 = qam_demapper.work(n, output_data, bitsout);
        for (size_t i = 0; i < std::min(bitsin.size(),n2); i++)
        {
            LOG_INFO("%d == %d ? %d", bitsin[i], bitsout[i], bitsin[i] == bitsout[i]);
            //CF32 awgn = CF32(ndist(rng), ndist(rng)) / sqrtf(2.0f);
            //WriteCout(awgn * 0.1f + output_data[i] * 1.4142f);
        }
        break;
        //LOG_DEBUG("Wrote {} Samples", n);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000*n/48000));
    }
    
    return 0;
}