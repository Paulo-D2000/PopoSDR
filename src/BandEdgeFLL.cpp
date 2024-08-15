#include <BandEdgeFLL.h>
#include <Constants.h>
#include <Utils.h>
#include <Filters.h>

// Based on GnuRadio's FLL Implementation
// https://github.com/gnuradio/gnuradio/blob/main/gr-digital/lib/fll_band_edge_cc_impl.cc

BandEdgeFLL::BandEdgeFLL(size_t Sps, float Damping, float LoopBw, const size_t &BufferSize):
 SyncBlock(BufferSize), m_cloop(Damping, LoopBw, Sps, M_PI_F * M_PI_F), m_sps(Sps)
{
    this->m_name = "BandEdgeFLL";
    LOG_DEBUG("Created Band-Edge FLL");
    size_t filter_size = 11*m_sps+1;
    size_t samps_per_sym = m_sps;
    float rolloff = 0.5;

    const int M = rintf(filter_size / samps_per_sym);
    float power = 0;

    std::vector<float> bb_taps;
    for (int i = 0; i < filter_size; i++) {
        float k = -M + i * 2.0 / samps_per_sym;
        float tap = sinc(rolloff * k - 0.5) + sinc(rolloff * k + 0.5);
        power += tap;
        bb_taps.push_back(tap);
    }

    std::vector<CF32> taps_upper(filter_size);
    std::vector<CF32> taps_lower(filter_size);

    // Create the band edge filters by spinning the baseband
    // filter up and down to the right places in frequency.
    // Also, normalize the power in the filters
    int N = (bb_taps.size() - 1.0) / 2.0;
    for (int i = 0; i < filter_size; i++) {
        float tap = bb_taps[i] / power;

        float k = (-N + (int)i) / (2.0 * samps_per_sym);

        CF32 t1 = tap * std::exp(_1j * -M_TWOPI_F * (1 + rolloff) * k);
        CF32 t2 = tap * std::exp(_1j * M_TWOPI_F * (1 + rolloff) * k);

        taps_lower[filter_size - i - 1] = t1;
        taps_upper[filter_size - i - 1] = t2;
    }

    m_be_filter_n.loadTaps(taps_lower);
    m_be_filter_p.loadTaps(taps_upper);

    m_cloop.setMaxFreq(M_TWOPI * (2.0 / m_sps));
    m_cloop.setMinFreq(-M_TWOPI * (2.0 / m_sps));
}

size_t BandEdgeFLL::work(const size_t &n_inputItems, std::vector<CF32> &input, std::vector<CF32> &output)
{
    size_t OutputIdx = 0;
    auto ptaps = m_be_filter_p.getTaps();
    auto ntaps = m_be_filter_n.getTaps();
    for (size_t i = 0; i < n_inputItems; i++)
    {
        CF32 out = output[OutputIdx++] = input[i] * std::exp(_1j * m_cloop.getLast());
        CF32 out_lo = m_be_filter_p.filter(out);
        CF32 out_hi = m_be_filter_n.filter(out);
        float error = std::norm(out_lo) - std::norm(out_hi);
        m_cloop.update(error);
        m_cloop.phase_wrap();
        m_cloop.freqLimit();
    }
    return OutputIdx;
}

BandEdgeFLL::~BandEdgeFLL()
{
    LOG_DEBUG("Destroyed Band-Edge FLL");
}
