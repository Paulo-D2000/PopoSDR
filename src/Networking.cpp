#include <Networking.h>
#include <fcntl.h>

void CheckError(int code){
    assert(code != -1);
}

template <typename OT, NetworkConnection net>
SocketSource<OT, net>::SocketSource(uint16_t Port, size_t PacketSize, bool client, const size_t& BufferSize):
  SourceBlock<OT>(BufferSize), m_client(client), m_pktsize(PacketSize){
    this->m_name = "SocketSource";
    LOG_DEBUG("Created Socket Source.");

    // create socket
    m_sockfd = socket(AF_INET, net == TCP ? SOCK_STREAM : SOCK_DGRAM, 0);
    CheckError(m_sockfd);
    //fcntl(m_sockfd, F_SETFL, O_NONBLOCK);

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(Port);
    m_addr.sin_addr.s_addr = INADDR_ANY;
    memset(m_addr.sin_zero, 0, 8);

    m_buffer = std::make_unique<U8[]>(m_pktsize);
    if(sizeof(OT) == 2){ // I16
        this->resizeOutput(m_pktsize / 2);
    }
    else if(sizeof(OT) == 4){ // F32
        this->resizeOutput(m_pktsize / 2);
    }
    else if(sizeof(OT) == 8){ // CF32
        this->resizeOutput(m_pktsize / 4);
    }
    
}

template <typename OT, NetworkConnection net>
void SocketSource<OT, net>::userStart(){
    // connect or bind
    if(!m_client){
        CheckError( bind(m_sockfd, (sockaddr*)&m_addr, sizeof(m_addr)) );
    } else {
        CheckError( connect(m_sockfd, (sockaddr*)&m_addr, sizeof(m_addr)) );
    }
}

template <typename OT, NetworkConnection net>
void SocketSource<OT, net>::userStop(){
    // close socket
    close(m_sockfd);
}
    
template <>
size_t SocketSource<I16, TCP>::work(std::vector<I16>& output){
    return work_s16(output);
}

template <>
size_t SocketSource<I16, UDP>::work(std::vector<I16>& output){
    return work_s16(output);
}

template <>
size_t SocketSource<CF32, TCP>::work(std::vector<CF32>& output){
    return work_complex(output);
}

template <>
size_t SocketSource<CF32, UDP>::work(std::vector<CF32>& output){
    return work_complex(output);
}

template <>
size_t SocketSource<F32, TCP>::work(std::vector<F32>& output){
    return work_float(output);
}

template <>
size_t SocketSource<F32, UDP>::work(std::vector<F32>& output){
    return work_float(output);
}

template <typename OT, NetworkConnection net>
size_t SocketSource<OT, net>::work_complex(std::vector<CF32>& output){
    int nread = recv(m_sockfd, (char*)&m_buffer[0], std::min(4*output.size(), m_pktsize), 0);
    if(nread == -1){
        return 0;
    }
    for (size_t i = 0; i < nread; i+=4)
    {
        I16 real = *((I16*)&m_buffer[i]);
        I16 imag = *((I16*)&m_buffer[i+2]);
        output[i/4] = 1.0f*CF32((float)real / (float)INT16_MAX, (float)imag / (float)INT16_MAX);
    }
    return nread / 4;
}

template <typename OT, NetworkConnection net>
size_t SocketSource<OT, net>::work_float(std::vector<F32>& output){
    int nread = recv(m_sockfd, (char*)&m_buffer[0], std::min(2*output.size(), m_pktsize), 0);
    if(nread == -1){
        return 0;
    }
    for (size_t i = 0; i < nread; i+=2)
    {
        I16 sample = *((I16*)&m_buffer[i]);
        output[i/2] = (float)sample / (float)INT16_MAX;
    }
    return nread / 2;
}

template <typename OT, NetworkConnection net>
size_t SocketSource<OT, net>::work_s16(std::vector<I16>& output){
    int nread = recv(m_sockfd, (char*)&output[0], std::min(output.size(), m_pktsize), 0);
    if(nread == -1){
        return 0;
    }
    return nread / sizeof(I16);
}


template <typename OT, NetworkConnection net>
SocketSource<OT,net>::~SocketSource(){
    LOG_DEBUG("Destroyed Socket Source.");
}

template class SocketSource<I16, TCP>;
template class SocketSource<CF32, TCP>;
template class SocketSource<F32, TCP>;

template class SocketSource<I16, UDP>;
template class SocketSource<CF32, UDP>;
template class SocketSource<F32, UDP>;