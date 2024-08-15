#pragma once


#ifdef _WIN32 // Include windows socket headers
#include <winsock2.h>
#define socklen_t int
#else // Include linux socket headers
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <unistd.h>
#include <assert.h>

#include <Block.h>

enum NetworkConnection {TCP=0, UDP};

template <typename OT, NetworkConnection net>
class SocketSource: public SourceBlock<OT>
{
public:
    SocketSource(uint16_t Port, size_t PacketSize = 1024, bool client = true, const size_t& BufferSize = 0);

    void userStart();

    void userStop();
    
    size_t work(std::vector<OT>& output);

    ~SocketSource();

private:

    size_t work_float(std::vector<F32>& output);
    size_t work_s16(std::vector<I16>& output);
    size_t work_complex(std::vector<CF32>& output);

#ifdef _WIN32
    WSADATA wsaData;
#endif

    bool m_client;
    int m_sockfd;
    sockaddr_in m_addr;
    std::unique_ptr<U8[]> m_buffer;
    size_t m_pktsize;
};

template <typename IT, NetworkConnection net>
class SocketSink: public SinkBlock<IT>
{
public:
    SocketSink(uint16_t Port, size_t PacketSize = 1024, bool client = true, const size_t& BufferSize = 0);

    void userStart();

    void userStop();
    
    size_t work(const size_t& n_inputItems, std::vector<IT>& input);

    ~SocketSink();

private:

    size_t work_float(const size_t& n_inputItems, const std::vector<F32>& input);
    size_t work_s16(const size_t& n_inputItems, const std::vector<I16>& input);
    size_t work_complex(const size_t& n_inputItems, const std::vector<CF32>& input);

#ifdef _WIN32
    WSADATA wsaData;
#endif

    bool m_client;
    int m_sockfd;
    sockaddr_in m_addr;
    std::unique_ptr<U8[]> m_buffer;
    size_t m_pktsize;
};

