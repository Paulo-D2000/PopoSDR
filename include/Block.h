#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <Buffer.h>

template <typename T>
class Stream {
private:
    size_t mSize;
    bool closed;
    Buffer<T> buffer;
    std::mutex m_mtx;
    std::condition_variable m_cv;
public:
    Stream(const size_t& Buffersize) : buffer(Buffersize), mSize(Buffersize) {open();}
    ~Stream(){close();}

    bool isOpen(){
        std::unique_lock<std::mutex> lock(m_mtx);
        bool ret = !closed;
        lock.unlock();
        return ret;
    }

    void WaitCv(){
        std::unique_lock<std::mutex> lock(m_mtx);
        m_cv.wait(lock, [this]() { return !buffer.isEmpty() || closed; });
        return;
    }

    bool isEmpty(){
        return buffer.isEmpty();
    }

    bool isFull(){
        return buffer.isFull();
    }

    void open(){
        std::unique_lock<std::mutex> lock(m_mtx);
        LOG_DEBUG("Open Stream {}",mSize);
        closed = false;
        lock.unlock();
        m_cv.notify_all();
    }

    void close(){
        std::unique_lock<std::mutex> lock(m_mtx);
        LOG_DEBUG("Close Stream {}",mSize);
        closed = true;
        lock.unlock();
        m_cv.notify_all();
    }

    void writeToBuffer(const std::vector<T>& data, size_t N) {
        std::unique_lock<std::mutex> lock(m_mtx);
        N = std::min(N, data.size());
        for (size_t i = 0; i < N; i++)
        {
            buffer.write(data.at(i));
        }
        m_cv.notify_all();
    }

    size_t readFromBuffer(std::vector<T>& data, size_t N=0) {
        size_t i = 0;
        std::unique_lock<std::mutex> lock(m_mtx);
        if(N == 0){
            N = data.size();
        }
        m_cv.wait(lock, [this]() { return !buffer.isEmpty() || closed; });
        for (i = 0; i < N; i++)
        {
            if(buffer.isEmpty()){
                break;
            }
            data[i] = buffer.read();
        }
        m_cv.notify_all();
        return i;
    }
};


template <typename IT, typename OT=IT>
class SyncBlock
{
private:
    std::vector<IT> inputs;
    std::vector<OT> outputs;
    Stream<IT>* m_pInput;
    Stream<OT>* m_pOutput;
    std::thread m_worker;
    void run();

protected:
    std::string m_name = "Generic Block";

public:
    SyncBlock(size_t BufferSize, size_t InputRate=1, size_t OutputRate=1);

    void addSuffix(const std::string& suffix){
        this->m_name += suffix;
    }

    void start();

    template <typename X>
    void connect(SyncBlock<X,IT>& Other)
    {
        m_pInput = Other.getOutputStream();
        LOG_DEBUG("Connected {} to {}",m_name,Other.getName());
    }

    void connect(Stream<IT>* stream){
        m_pInput = stream;
    }

    void stop();

    std::string getName(){ return m_name; }

    Stream<IT>* getInputStream(){ return m_pInput; }
    Stream<OT>* getOutputStream(){ return m_pOutput; }

    virtual size_t work(const size_t& n_inputItems, std::vector<IT>&  input, std::vector<OT>& output);

    ~SyncBlock();
};
