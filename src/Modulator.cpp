#include <Modulator.h>
#include <Filters.h>
#include <random>

Modulator::Modulator(size_t BaudRate, size_t SampleRate, size_t BufferSize) :  m_sps(SampleRate/BaudRate), m_samplerate(SampleRate), m_buffsize(BufferSize), m_inp_stream(m_buffsize*32)
{
    LOG_DEBUG("Created Modulator.");
    LOG_DEBUG("  Params:");
    LOG_DEBUG("    BaudRate:   {}",BaudRate);
    LOG_DEBUG("    SampleRate: {}",m_samplerate);
    LOG_DEBUG("    Sps(Final): {}",m_sps);

    m_interp = 1;
    std::vector<F32> interp_taps({1.0f});
    
    if(m_sps > 10){
        m_interp = m_sps / 10;
        m_sps = 10;
        interp_taps = Generate_Generic_LPF((float)SampleRate, 0.5f * (float)SampleRate / (float)m_interp, 6, Kaiser);
    }

    LOG_TEST("Modulator Interpolation: {}", m_interp);
    LOG_TEST("Modulator Final SR: {}, Desired SR: {}", m_interp * m_sps * BaudRate, SampleRate);

    std::vector<F32> taps = Generate_Gaussian_LPF(3.0f, 0.5f, (float)m_sps);
    auto ptr = new FirFilter<F32>(taps,m_buffsize*32);
    m_Gfilter = ptr;
    m_Gfilter->addSuffix("(Gaussian)");
    m_interpolator = new FirFilter<CF32>(interp_taps, m_buffsize*32*m_interp, {.Interpolation=m_interp, .Decimation=1});
    m_interpolator->addSuffix("(Interpolator)");

    // Deviation for GMSK = BitRate / 4
    m_fmmod = new FmModulator(500.f, SampleRate / m_interp);

    //m_Gfilter->connect(&m_inp_stream);
    m_fmmod->connect(&m_inp_stream);
    m_interpolator->connect(*m_fmmod);

    m_out_stream = m_interpolator->getOutputStream();
}

int popcount(unsigned x)
{
    int c = 0;
    for (; x != 0; x &= x - 1)
        c++;
    return c;
}

uint8_t ccsds_scramble(uint8_t input){
    static uint8_t shift_reg = 0xFF;
    uint8_t out = shift_reg & 1;
    uint8_t new_bit = (popcount(shift_reg & 0b10101001) ^ input) & 1;
    shift_reg = ((shift_reg >> 1) | (new_bit << 7));
    return out;
}

F32 m_phase = 0.0f;
void Modulator::sendPacket(const std::vector<U8> &packet, ModulatorConfig cfg)
{
    static Stream<U8> bytes(1024); // Tx Bytes FIFO

    /* Push [Preamble] bytes on the Tx FIFO */
    std::vector<U8> buf(cfg.PreambleLen, cfg.Preamble);

    /* Push [Syncword] bytes on the Tx FIFO */
    //for (size_t i = 0; i < 32; i+=8){ 
    //    buf.push_back((cfg.Syncword >> (24-i)) & 0xFF);
    //}
    bytes.writeToBuffer(buf, buf.size());
    //modulate(bytes);
    
    /* Push [Data] bytes on the Tx FIFO */
    bytes.writeToBuffer(packet,packet.size());
    modulate(bytes,cfg.Scrambler);
}
U8 nrzi = 1;
void Modulator::modulate(Stream<U8> &input_stream, bool scramble)
{
    if(!input_stream.isOpen()){
        LOG_ERROR("Input stream was closed!");
        return;
    }

    std::vector<U8>buf(8);
    std::vector<F32> obuf(8*m_sps);
    while(!input_stream.isEmpty()){
        input_stream.readFromBuffer(buf, buf.size());

        for (size_t k = 0; k < buf.size(); k++)
        {
            /* Get Byte */
            U8 byte = buf.at(k);

            /* Split into Bits  */
            memset(m_chunk,0,8);
            if(scramble){
                /* CCSDS Scrambe  */
                for (size_t i = 0; i < 8; i++)
                    m_chunk[i] = ccsds_scramble((byte >> (7-i)) & 0x01);
            }else{
                for (size_t i = 0; i < 8; i++)
                {   
                    if(!((byte >> (7-i)) & 0x01)){
                        nrzi = !nrzi;
                    }
                    m_chunk[i] = nrzi;
                }
            }


            /* NRZI Encode & Send to FIR Filter (Gaussian)*/
            for (size_t i = 0; i < 8; i++){
                F32 nrz_sample = 2.0f * (F32)m_chunk[i] - 1.0f; // [0,1] -> [-1,1] 
                /* Interp */
                for (size_t j = 0; j < m_sps; j++)
                {
                   obuf.at(j+i*m_sps) = nrz_sample;
                }
            }
            m_inp_stream.writeToBuffer(obuf, obuf.size());
        }
    }
}

Modulator::~Modulator()
{
    stop();
    LOG_DEBUG("Destroyed Modulator.");
}