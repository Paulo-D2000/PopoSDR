#include <Types.h>
#include <Logging.h>

#include <vector>
#include <fstream>
#include <filesystem>

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

std::vector<F32>ComplexInterleave(std::vector<CF32> data){
    std::vector<F32> output;
    for (size_t i = 0; i < data.size(); i++)
    {
        output.push_back(data.at(i).real());
        output.push_back(data.at(i).imag());
    }
    
    return output;
}

void WriteWav(std::string filename, std::vector<F32> data, size_t SampleRate=48000, int Channels=1, float scale=1.0f){
    LOG_INFO("Writing Wav");

    std::string cwd = std::filesystem::current_path().generic_string();
    std::string file_path = cwd + "/" + filename;

    LOG_TEST("Path: {}",file_path);
    LOG_TEST("SampleRate: {}",SampleRate);
    LOG_TEST("Channels: {}",Channels);
    LOG_TEST("Data Size: {}",data.size());

    /* File gets closed when std::ofstream goes out of scope */ 
    std::ofstream outFile(file_path, std::ios::binary);

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
        if(Isample > (float)INT16_MAX){
            Isample = 1.0f;
            nclip++;
        }else if (Isample < (float)INT16_MIN)
        {
            Isample = -1.0f;
            nclip++;
        }
        
        I16 sample = (int)(Isample); // Convert FP32 -> S16
        outFile.write((char*)&sample, sizeof(I16)); // Write S16 as 2 char's
    }
    if(nclip > 0){
        LOG_ERROR("Clipping {} samples...",nclip);
    }
}