#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
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

    bool m_client;
    int m_sockfd;
    sockaddr_in m_addr;
    std::unique_ptr<U8[]> m_buffer;
    size_t m_pktsize;
};