#include <Buffer.h>
#include <stdlib.h>

template <typename T>
Buffer<T>::Buffer() : m_size(0), m_buffer(nullptr), m_head(0), m_tail(0), m_mask(0) {}

template <typename T>
Buffer<T>::Buffer(const size_t& BufferSize)
{
    m_head = 0;
    m_tail = 0;
    m_occupancy = 0;
    m_size = BufferSize;
    if(m_size % 2 != 0){
        LOG_ERROR("Buffer<%s>(%d) Size must be a power of 2!.",typeid(T).name(),m_size);
        m_size = 1ull << (long long)(log2(BufferSize)+1);
        LOG_INFO("New BufferSize: %d", m_size);
    }
    m_mask = m_size - 1;
    m_buffer = std::unique_ptr<T>(new T[m_size]);
    LOG_DEBUG("Created Buffer<%s>(%d).",typeid(T).name(),m_size);
}

template <typename T>
void Buffer<T>::resize(const size_t &BufferSize)
{
    m_head = 0;
    m_tail = 0;
    m_occupancy = 0;
    m_buffer.reset();
    size_t psz = m_size;
    m_size = BufferSize;
    if(m_size % 2 != 0){
        LOG_ERROR("Buffer<%s>(%d) Size must be a power of 2!.",typeid(T).name(),m_size);
        m_size = 1ll << (long long)(log2(BufferSize)+1);
        LOG_INFO("New BufferSize: %d", m_size);
    }
    m_mask = m_size - 1;
    m_buffer = std::unique_ptr<T>(new T[m_size]);
    LOG_DEBUG("Resized Buffer<%s>(%d) to (%d) elements.", typeid(T).name(), psz, m_size);
}

template <typename T>
void Buffer<T>::write(const T &sample)
{
    if(isFull()){
        LOG_ERROR("[BUFFER<%s>(%d)] IS FULL!! Overflowing...",typeid(T).name(), m_size);
    }
    T* ptr = m_buffer.get();
    ptr[m_head] = sample;
    m_head = (m_head + 1) & m_mask;
    m_occupancy++;
}

template <typename T>
T Buffer<T>::read(){
    if(isEmpty()){
        LOG_ERROR("[BUFFER<%s>(%d)] IS EMPTY!! Underflowing...",typeid(T).name(), m_size);
        return T();
    }
    T* ptr = m_buffer.get();
    T sample = ptr[m_tail];
    m_tail = (m_tail + 1) & m_mask;
    m_occupancy--;
    auto now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = now - m_start;
    if(elapsed.count() > 0.5){
        m_Thput = static_cast<F32>(m_NumTransfers) / elapsed.count();
        m_NumTransfers = 0;
        LOG_DEBUG("[BUFFER<%s>(%d)] Throughtput: %.1f KT/s", typeid(T).name(), m_size, m_Thput / 1000);
        m_start = std::chrono::system_clock::now();
    }
    m_NumTransfers++;
    return sample;
}

template <typename T>
Buffer<T>::~Buffer()
{
    LOG_DEBUG("Destroyed Buffer<%s>(%d).", typeid(T).name(), m_size);
}

template class Buffer<F32>;
template class Buffer<CF32>;

template class Buffer<I8>;
template class Buffer<I16>;
template class Buffer<I32>;

template class Buffer<U8>;
template class Buffer<U16>;
template class Buffer<U32>;
