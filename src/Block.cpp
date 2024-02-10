#include <Block.h>


/* Base Block */
void Block::run(){
    return;
}

void Block::start(){
    LOG_DEBUG("Starting Block [{}]",m_name);
    this->userStart();
    m_worker = std::thread(&Block::run, this);
}

void Block::stop(){
    if(m_worker.joinable()){
        this->close_terminate();
        m_worker.join();
        this->close();
        this->userStop();
        LOG_DEBUG("Stopped Block [{}]",m_name);
    }
}

void Block::userStart(){
    return;
}

void Block::close(){
    return;
}

void Block::close_terminate(){
    return;
}

void Block::userStop(){
    return;
}

/* Source Block */


template <typename OT>
SourceBlock<OT>::SourceBlock(size_t BufferSize): outputs(4096) {
    m_pOutput = new Stream<OT>(BufferSize);
}

template <typename OT>
size_t SourceBlock<OT>::work(std::vector<OT>& output)
{
    return 0;
}

template <typename OT>
void SourceBlock<OT>::run(){
    bool terminate = false;
    while(!terminate){
        m_pOutput->WaitCv(Stream<OT>::Full);

        size_t ret = 0;
        if(!m_pOutput->isFull()){
            ret = this->work(outputs);
        }

        if(ret == 0 || !m_pOutput->isOpen()){
           terminate = true; 
           break;
        }
        
        if(ret != 0){
            m_pOutput->writeToBuffer(outputs, ret);
        }
    }
    LOG_DEBUG("Joined thread on Block [{}].",m_name);
}


template <typename OT>
void SourceBlock<OT>::close_terminate(){
    m_pOutput->close();
}

template <typename OT>
SourceBlock<OT>::~SourceBlock(){
    stop();
    if(m_pOutput != nullptr){
        delete m_pOutput;
    }
}


/* Sync Block */


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
void SyncBlock<IT, OT>::run(){
    bool terminate = false;
    while(!terminate){
        m_pInput->WaitCv();
        if(m_pInput->isEmpty() && !m_pInput->isOpen()){
            terminate = true;
            break;
        }

        size_t nread = m_pInput->readFromBuffer(inputs);

        size_t ret = this->work(nread, inputs, outputs);

        m_pOutput->writeToBuffer(outputs, ret);
    }
    LOG_DEBUG("Joined thread on Block [{}].",m_name);
}

template <typename IT, typename OT>
void SyncBlock<IT, OT>::close(){
    m_pInput->close();
    m_pOutput->close();
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


/* Sink Block */


template <typename IT>
SinkBlock<IT>::SinkBlock(size_t BufferSize): inputs(4096) {}

template <typename IT>
size_t SinkBlock<IT>::work(const size_t& n_inputItems, std::vector<IT>&  input)
{
    return 0;
}

template <typename IT>
void SinkBlock<IT>::run(){
    bool terminate = false;
    while(!terminate){
        m_pInput->WaitCv();
        if(m_pInput->isEmpty() && !m_pInput->isOpen()){
            terminate = true;
            break;
        }

        size_t nread = m_pInput->readFromBuffer(inputs);

        size_t ret = this->work(nread, inputs);
    }
    LOG_DEBUG("Joined thread on Block [{}].",m_name);
}

template <typename IT>
void SinkBlock<IT>::close(){
    m_pInput->close();
}

template <typename IT>
SinkBlock<IT>::~SinkBlock(){
    stop();
    if(m_pInput != nullptr){
        delete m_pInput;
    }
}


/* Instantiations */


// Sources //
// Basic types (Float & Complex)
template class SourceBlock<F32>;
template class SourceBlock<CF32>;
// Signed
template class SourceBlock<I8>;
template class SourceBlock<I16>;
template class SourceBlock<I32>;
// Unsigned
template class SourceBlock<U8>;
template class SourceBlock<U16>;
template class SourceBlock<U32>;


// Syncs (Synchronous) //
// Basic types (Float & Complex)
template class SyncBlock<F32>;
template class SyncBlock<CF32>;
// Signed
template class SyncBlock<I8>;
template class SyncBlock<I16>;
template class SyncBlock<I32>;
// Unsigned
template class SyncBlock<U8>;
template class SyncBlock<U16>;
template class SyncBlock<U32>;
// Cross types (Float & Complex)
template class SyncBlock<F32, CF32>;
template class SyncBlock<CF32, F32>;
// Cross types (Unsigned & Complex)
template class SyncBlock<U8, CF32>;
template class SyncBlock<CF32, U8>;


// Sinks //
// Basic types (Float & Complex)
template class SinkBlock<F32>;
template class SinkBlock<CF32>;
// Signed
template class SinkBlock<I8>;
template class SinkBlock<I16>;
template class SinkBlock<I32>;
// Unsigned
template class SinkBlock<U8>;
template class SinkBlock<U16>;
template class SinkBlock<U32>;