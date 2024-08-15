#pragma once

#include <Constants.h>
#include <Logging.h>
#include <limits>

class ControlLoop
{
private:
    float m_Kp;
    float m_Ki;

    float m_integrator_0;
    float m_integrator_1;

    float m_min_freq = std::numeric_limits<float>::lowest();
    float m_max_freq = std::numeric_limits<float>::infinity();
public:
    struct LoopGains{float Kp; float Ki;};

    /// @brief Generate a PI + (NCO optional) Control Loop
    /// @param Damping (float) Loop Damping, 0.707 usually
    /// @param LoopBandwidth (float) Loop Bandwith, usually 1/100 of Fs
    /// @param Fs (float) Sampling Frequency
    /// @param KD (float) Detector Gain 
    ControlLoop(float Damping, float LoopBandwidth, float Fs=1.0f, float KD=1.0f): m_integrator_0(0.0f), m_integrator_1(0.0f)
    {
        float denom = Fs*(Damping + 1.0f / (4.0f * Damping));
        m_Kp = (4.0f * Damping * LoopBandwidth) / (KD*denom);
        m_Ki = (4.0f * LoopBandwidth * LoopBandwidth) / (KD*denom*denom);
        LOG_DEBUG("ControlLoop Gains:\n  Kp: %f\n  Ki: %f",m_Kp, m_Ki);
    }

    /// @brief Generate a PI + (NCO optional) Control Loop
    /// @param gains LoopGains struct
    ControlLoop(LoopGains gains): m_integrator_0(0.0f), m_integrator_1(0.0f){
        m_Kp = gains.Kp;
        m_Ki = gains.Ki;
        LOG_DEBUG("ControlLoop Gains:\n  Kp: %f\n  Ki: %f",m_Kp, m_Ki);
    }

    /// @brief Generate a PI + (NCO optional) Control Loop
    ControlLoop(): m_integrator_0(0.0f), m_integrator_1(0.0f){
        m_Kp = 0.0f;
        m_Ki = 0.0f;
        LOG_DEBUG("ControlLoop Gains:\n  Kp: %f\n  Ki: %f",m_Kp, m_Ki);
    }
    
    /// @brief Sets the Loop gains 
    /// @param Kp (float) Proportional Gain
    /// @param Ki (float) Integral Gain
    void setGains(float Kp, float Ki){
        m_Kp = Kp;
        m_Ki = Ki;
        m_integrator_0 = 0;
        m_integrator_1 = 0;
    }

    
    /// @brief Updates the control loop (PI + NCO)
    /// @param input input value (float)
    /// @return (float) 1st order loop result
    float update(float input){
        m_integrator_0 = m_integrator_0 + m_Ki * input;
        float pi_result = m_integrator_0 + m_Kp * input;
        m_integrator_1 += pi_result;
        return pi_result;
    }

    /// @brief Returns the current loop gains
    /// @return LoopGains struct
    LoopGains getGains(){
        return LoopGains{m_Kp, m_Ki};
    }
    
    /// @brief Returns the 1st order loop result (PI Filter)
    /// @return float 
    float getIntegrator0(){
        return m_integrator_0;
    }

    /// @brief Returns the 2nd order loop result (PI Filter + NCO)
    /// @return float
    float getLast(){
        return m_integrator_1;
    }

    /// @brief Sets the min freq
    void setMinFreq(float minFreq){
        m_min_freq = minFreq;
    }

    /// @brief Sets the max freq
    void setMaxFreq(float maxFreq){
        m_max_freq = maxFreq;
    }

    /// @brief Keeps the Freq between -m_min_freq, m_max_freq
    void freqLimit(){
        if (m_integrator_0 > m_max_freq)
            m_integrator_0 = m_max_freq;
        else if (m_integrator_0 < m_min_freq)
            m_integrator_0 = m_min_freq;
    }

    void phase_wrap()
    {
        while (m_integrator_1 > (M_TWOPI_F))
            m_integrator_1 -= M_TWOPI_F;
        while (m_integrator_1 < (-M_TWOPI_F))
            m_integrator_1 += M_TWOPI_F;
    }

};
