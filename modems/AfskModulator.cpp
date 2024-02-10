#include <AfskModulator.h>
#include <Filters.h>
#include <random>
#include <cstring>

AfskModulator::AfskModulator(size_t BaudRate, size_t SampleRate, size_t BufferSize) :  m_sps(SampleRate/BaudRate), m_samplerate(SampleRate), m_buffsize(BufferSize), m_inp_stream(m_buffsize*32)
{
    LOG_DEBUG("Created AfskModulator.");
    LOG_DEBUG("  Params:");
    LOG_DEBUG("    BaudRate:   {}",BaudRate);
    LOG_DEBUG("    SampleRate: {}",m_samplerate);
    LOG_DEBUG("    Sps(Final): {}",m_sps);

    m_interp = 1;
    std::vector<F32> interp_taps({1.0f});
    
    if(m_sps > 10){
        m_interp = m_sps / 10;
        m_sps = 10;
        interp_taps = Generate_Generic_LPF((float)SampleRate, 0.5f * (float)SampleRate / (float)m_interp, (float)m_interp, 6, Kaiser);
    }

    LOG_TEST("AfskModulator Interpolation: {}", m_interp);
    LOG_TEST("AfskModulator Final SR: {}, Desired SR: {}", m_interp * m_sps * BaudRate, SampleRate);

    m_interpolator = new FirFilter<CF32>(interp_taps, {.Interpolation=m_interp, .Decimation=1}, m_buffsize*32*m_interp);
    m_interpolator->addSuffix("(Interpolator)");

    // Deviation for AFSK = 500Hz * baudrate/1200 ?
    m_fmmod = new FmModulator(500.f * (float)BaudRate/1200.0f, SampleRate / m_interp);

    //m_Gfilter->connect(&m_inp_stream);
    m_fmmod->connect(&m_inp_stream);
    m_interpolator->connect(*m_fmmod);

    m_out_stream = m_interpolator->getOutputStream();
}


void AfskModulator::sendPacket(const std::vector<U8> &packet, AfskModulatorConfig cfg)
{
    static Stream<U8> bytes(1024); // Tx Bytes FIFO

    /* Push [Preamble] bytes on the Tx FIFO */
    std::vector<U8> buf(cfg.PreambleLen, cfg.Preamble);
    bytes.writeToBuffer(buf, buf.size());
    
    /* Push [Data] bytes on the Tx FIFO */
    bytes.writeToBuffer(packet,packet.size());

    /* Push [Postamble] bytes on the Tx FIFO */
    buf = std::vector<U8>(cfg.PostambleLen, cfg.Preamble);
    bytes.writeToBuffer(buf, buf.size());

    modulate(bytes);
}

void AfskModulator::modulate(Stream<U8> &input_stream)
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

            /* Split into Bits & NRZI Encode */
            memset(m_chunk,0,8);
            for (size_t i = 0; i < 8; i++)
            {   
                if(!((byte >> (7-i)) & 0x01)){
                    m_nrzi = !m_nrzi;
                }
                m_chunk[i] = m_nrzi;
            }

            /* Send to FM mod */
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

AfskModulator::~AfskModulator()
{
    stop();
    LOG_DEBUG("Destroyed AfskModulator.");
}