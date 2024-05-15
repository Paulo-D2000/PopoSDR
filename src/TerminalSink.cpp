#include <TerminalSink.h>

template <typename IT>
TerminalSink<IT>::TerminalSink(size_t SampleRate, size_t BufferSize):SinkBlock<IT>(BufferSize), m_SampleRate(SampleRate) {
    this->m_name = "TerminalSink";
    LOG_DEBUG("Created Terminal Sink (STDOUT)");
    this->resizeInput(m_SampleRate / 60);
}

template <typename IT>
size_t TerminalSink<IT>::work(const size_t &n_inputItems, std::vector<IT> &input)
{
    size_t outputIdx = 0;
    for (size_t i = 0; i < n_inputItems; i++)
    {
        WriteCout(input.at(i));
    }
    return outputIdx;
}

template <typename IT>
TerminalSink<IT>::~TerminalSink()
{
    LOG_DEBUG("Destroyed Terminal Sink (STDOUT)");
}

template class TerminalSink<F32>;
template class TerminalSink<CF32>;