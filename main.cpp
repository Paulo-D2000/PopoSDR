#include <iostream>
#include <fstream>
#include <thread>
#include <random>
#include <Modulator.h>

#include <FmDemodulator.h>
#include <TimingPLL.h>
#include <Filters.h>
#include <FirFilter.h>

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>

/* Init random generator */
std::mt19937 rng(std::random_device{}());
// Random bytes distribuition
std::uniform_int_distribution<U32> dist(0, UINT32_MAX);
std::normal_distribution<F32> ndist(0.0f,1.0f);

U8 splitBytes(U32 input){
    static U32 chunk;
    static U8 counter=3;
    if(counter == 3){
        counter = 0;
        chunk = input;
    }else{
        counter++;
        chunk >>= 8;
    }
    return chunk & 0xFF;
}

struct WavHeader{
    #pragma pack(push, 1)
    /* RIFF CHUNK */
    U32 RIFF_ID = 0x46464952;  // "RIFF"
    U32 RIFF_SZ = 0x00000000;  // FileSize - 8
    U32 RIFF_TY = 0x45564157;  // "WAVE"
    /* FMT CHUNK*/
    U32 FMT_ID = 0x20746D66;   // "fmt"
    U32 FMT_SZ = 16;           // Chunk size
    U16 ComprCode = 1;         // Compression Code
    U16 NChans = 1;            // NChannels
    U32 SampleRate = 48000;    // Sample Rate
    U32 ByteRate = 96000;      // Byte Rate = SampleRate * BlockAlign
    U16 BlockAlign = 2;        // BlockAlign = NChannels * BitsDepth/8
    U16 BitDepth = 16;         // BitDepth
    /* DATA CHUNK */
    U32 DATA_ID = 0x61746164;  // "data"
    U32 DataSize = 0x00000000; // Data size
    /* PCM DATA */
    #pragma pack(pop)
};

void WriteWav(std::string filename, std::vector<F32> data, size_t SampleRate=48000, int Channels=1, float scale=1.0f){
    LOG_INFO("Writing Wav");
    LOG_TEST("Path: {}",filename);
    LOG_TEST("SampleRate: {}",SampleRate);
    LOG_TEST("Channels: {}",Channels);
    LOG_TEST("Data Size: {}",data.size());

    /* File gets closed when std::ofstream goes out of scope */ 
    std::ofstream outFile(filename, std::ios::binary);

    /* Wav header (16 Bit Signed PCM Samples) */
    WavHeader hdr;
    hdr.NChans = Channels;
    hdr.SampleRate = (U32)SampleRate;
    hdr.BitDepth = 8 * sizeof(I16);
    hdr.BlockAlign = Channels * hdr.BitDepth/8;
    hdr.ByteRate = hdr.SampleRate * hdr.BlockAlign;
    hdr.DataSize = (U32)data.size() * hdr.BitDepth/8;
    hdr.RIFF_SZ = hdr.DataSize + sizeof(hdr) - 8;

    // Write Header (44 bytes)
    outFile.write((char*)&hdr, sizeof(hdr)); 

    size_t nclip = 0;

    // Write Data
    for (size_t i = 0; i < data.size(); i++)
    {
        F32 Isample = data.at(i) * scale * (float)INT16_MAX;
        if(std::abs(Isample) > (float)INT16_MAX){
            Isample *= 1.0f/Isample;
            nclip++;
        }
        I16 sample = (int)(Isample); // Convert FP32 -> S16
        outFile.write((char*)&sample, sizeof(I16)); // Write S16 as 2 char's
    }
    if(nclip > 0){
        LOG_ERROR("Clipping {} samples...",nclip);
    }
}

std::vector<U8> String2Packet(const char* str){
    std::vector<U8> pkt;
     for (size_t i = 0; i < strlen(str); i++)
    {
        pkt.push_back(str[i]);
    }
    return pkt;
}
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
int main(){

    LOG_DEBUG("START\n");

	//SoapySink sink("driver=plutosdr", {.SampleRate=1e6, .Frequency=433e6, .BufferSize=4096});

    std::vector<F32> samples; // Wav Output Buffer

    LOG_INFO("Starting GMSK Modulator");
    
    Modulator mod(1200, (int)48e3);

    Stream<CF32>* stream = mod.GetStream();
    
    //sink.connect(stream);

    //sink.start();
    
    for (size_t j = 0; j < 5; j++)
    {
        /* Send packet */
        std::vector<U8> pkt({0x7E,0x05,0x55,0x16,0x15,0x09,0x2D,0x07,0x05,0x55,0x16,0x15,0x09,0x2D,0x86,0xC0,0x00,0x2A,0xA2,0xCA,0x2A,0xA2,0x04,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x04,0x2A,0xA2,0xCA,0x2A,0xA2,0x04,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xA5,0x72,0x7E,0x7E,0x7E,0X7E,0X7E,0X7E,0X7E,0X7E,0X7E});// = String2Packet("AAAAAAA COUNTER: 0\n");
        //pkt.push_back(32);
        //pkt.insert(pkt.end(), '\n');
        //pkt.insert(pkt.end(), std::to_string(j).c_str()[0]);
        mod.sendPacket(pkt, { .Preamble = 0x7E, .PreambleLen = 50, .Scrambler = false });
    }

    mod.start();

    mod.stop();
    //sink.stop();

    std::vector<CF32> inputSamples;
    std::vector<CF32> item(1);
    while(!stream->isEmpty()){
        stream->readFromBuffer(item,1);
        samples.push_back(item.at(0).real());
        samples.push_back(item.at(0).imag());
        inputSamples.push_back(item.at(0));
    }

    WriteWav("C:/Users/Vitor/source/repos/ModemGMSK/build/Test.wav", samples, mod.GetSampleRate(), 2);
    float spwr = Vec_AvgPwr(inputSamples);
    LOG_TEST("Avg Power: {:.3f} Vrms",spwr);

    /* Add AWGN */
    float SNRdB = 20.0f;

    float npwr = powf(10.0f,-SNRdB/10.0f) * spwr / sqrtf(2.0f);
    LOG_INFO("SNR: {:.1f}dB = {} Vrms",SNRdB,npwr);
    for (size_t i = 0; i < inputSamples.size(); i++)
    {
        inputSamples[i] = inputSamples[i] + CF32(npwr*ndist(rng),npwr*ndist(rng));
    }

    /* Matched Filter */
    FirFilter<CF32> lpfilt(Generate_Generic_LPF(mod.GetSampleRate(), mod.GetBaudRate(), 5),131072);
    lpfilt.addSuffix("[Gaussian] Matched");

    std::vector<CF32> filtSamples(inputSamples.size());
    lpfilt.work(inputSamples.size(), inputSamples, filtSamples);

    /* FM Demodulator */
    FmDemodulator demod(0.5f*(float)mod.GetBaudRate(), mod.GetSampleRate()/4);

    std::vector<F32> demodSamples(inputSamples.size());
    demod.work(inputSamples.size(), inputSamples, demodSamples);
    
    WriteWav("C:/Users/Vitor/source/repos/ModemGMSK/build/Demod.wav", demodSamples,  mod.GetSampleRate(), 1, 0.707f);
    spwr = Vec_AvgPwr(demodSamples);
    LOG_TEST("Avg Power: {:.3f} Vrms",spwr);

    /* Test Timing Recovery PLL */
    TimmingPLL pll(mod.GetSampleRate(), mod.GetBaudRate(), 0.9f);

    std::vector<F32> pllSamples(demodSamples.size()*mod.GetBaudRate()/mod.GetSampleRate());
    pll.work(demodSamples.size(), demodSamples, pllSamples);
    
    std::string syncword = "00011010110011111111110000011101";
    std::vector<int> sw;
    for (size_t i = 0; i < syncword.length(); i++)
    {
        sw.push_back(syncword.at(i)-'0');
    }
    std::vector<int> sbuf(sw.size(),0);
    LOG_DEBUG("SW SIZE: {}", sw.size());

    
    int maxcorr = 0;
    for (size_t i = 0; i < pllSamples.size(); i++)
    {
        sbuf.erase(sbuf.begin());
        int bit = Sign(pllSamples.at(i));
        sbuf.push_back(bit > 0.5);
        //pllSamples.at(i) = (F32)bit;
        int newcorr = 0;
        for (size_t j = 0; j < sw.size(); j++)
        {
            newcorr += !(sbuf.at(j) ^ sw.at(j));
        }
        maxcorr = std::max<int>(maxcorr, newcorr);
        if(newcorr > 27){
            std::stringstream result;
            std::copy(sbuf.begin(), sbuf.end(), std::ostream_iterator<int>(result, ""));
            std::stringstream sz;
            for (size_t k = 0; k < 8; k++)
            {
                sz << std::to_string(Sign(pllSamples.at(i+k)) > 0.01f);
            }
            
            LOG_TEST("New Corr: {}, New data: {}, SW: {}, PktSz: {}",newcorr,result.str(),syncword,sz.str());
        }
    }

    LOG_INFO("Max Correlation: {}",maxcorr);
    
    WriteWav("C:/Users/Vitor/source/repos/ModemGMSK/build/Timing.wav", pllSamples, mod.GetSampleRate(), 1, 0.707f);
    spwr = Vec_AvgPwr(pllSamples);
    LOG_TEST("Avg Power: {:.3f} Vrms",spwr);

    LOG_INFO("Terminating GMSK Modulator...");

    return 0;
}