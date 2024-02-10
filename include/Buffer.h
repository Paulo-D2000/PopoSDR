#pragma once

#include <atomic>
#include <memory>
#include <chrono>
#include <Logging.h>
#include <Types.h>

template <typename T>
class Buffer
{
public:
    Buffer();

    Buffer(const size_t& BufferSize);

    void resize(const size_t& BufferSize);

    void write(const T& sample);

    size_t GetOccupancy(){ return m_occupancy; }

    T read();

    bool isEmpty(){
        return m_head == m_tail;
    }

    bool isFull(){
        return ((m_head+1) & m_mask) == m_tail;
    }

    double getThroughput(){
        return m_Thput;
    }

    ~Buffer();

private:
    size_t m_head;
    size_t m_tail;

    size_t m_size;
    size_t m_mask;

    std::unique_ptr<T> m_buffer;
    size_t m_occupancy;

    size_t m_NumTransfers;
    std::chrono::time_point<std::chrono::system_clock> m_start;
    double m_Thput;
};
