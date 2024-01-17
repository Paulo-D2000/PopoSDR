#include <Block.h>

template <typename IT, typename OT>
SyncBlock<IT, OT>::SyncBlock(size_t BufferSize, size_t InputRate, size_t OutputRate): inputs(4096*InputRate), outputs(4096*OutputRate) {
    m_pOutput = new Stream<OT>(BufferSize);
}

template <typename IT, typename OT>
size_t SyncBlock<IT, OT>::work(const size_t& n_inputItems, std::vector<IT>&  input, std::vector<OT>& output)
{
    return 0;
}

template <typename IT, typename OT>
void SyncBlock<IT, OT>::start(){
    LOG_DEBUG("Starting Block [{}]",m_name);
    m_worker = std::thread(&SyncBlock<IT,OT>::run, this);
}

template <typename IT, typename OT>
void SyncBlock<IT, OT>::run(){
    bool terminate = false;
    while(!terminate){
        m_pInput->WaitCv();
        if(m_pInput->isEmpty() && !m_pInput->isOpen()){
            terminate = true;
            break;
        }

        size_t nread = m_pInput->readFromBuffer(inputs);

        size_t ret = this->work(nread,inputs,outputs);

        m_pOutput->writeToBuffer(outputs, ret);
    }
    LOG_DEBUG("Joined thread on Block [{}].",m_name);
}

template <typename IT, typename OT>
void SyncBlock<IT, OT>::stop(){
    if(m_worker.joinable()){
        m_worker.join();
        m_pInput->close();
        m_pOutput->close();
        LOG_DEBUG("Stopped Block [{}]",m_name);
    }
}

template <typename IT, typename OT>
SyncBlock<IT, OT>::~SyncBlock(){
    stop();
    if(m_pInput != nullptr){
        delete m_pInput;
    }
    if(m_pOutput != nullptr){
        delete m_pOutput;
    }
}

template class SyncBlock<F32>;
template class SyncBlock<F32, CF32>;

template class SyncBlock<CF32>;
template class SyncBlock<CF32, F32>;

template class SyncBlock<I8>;
template class SyncBlock<I16>;
template class SyncBlock<I32>;

template class SyncBlock<U8>;
template class SyncBlock<U16>;
template class SyncBlock<U32>;