#pragma once

#include <Logging.h>

class ControlLoop
{
private:
    float m_Kp;
    float m_Ki;

    float m_integrator_0;
    float m_integrator_1;
public:
    struct Gains{float Kp; float Ki;};

    ControlLoop(float Damping, float LoopBandwidth): m_integrator_0(0.0f), m_integrator_1(0.0f)
    {
        float denom = 1.0f + 2.0f * Damping * LoopBandwidth + LoopBandwidth * LoopBandwidth;
        m_Kp = (4.0f * Damping * LoopBandwidth) / denom;
        m_Ki = (4.0f * LoopBandwidth * LoopBandwidth) / denom;
        LOG_DEBUG("ControlLoop Gains:\n  Kp: {}\n  Ki:{}",m_Kp, m_Ki);
    }

    ControlLoop(Gains gains): m_integrator_0(0.0f), m_integrator_1(0.0f){
        m_Kp = gains.Kp;
        m_Ki = gains.Ki;
        LOG_DEBUG("ControlLoop Gains:\n  Kp: {}\n  Ki:{}",m_Kp, m_Ki);
    }

    void setGains(float Kp, float Ki){
        m_Kp = Kp;
        m_Ki = Ki;
        m_integrator_0 = 0;
        m_integrator_1 = 0;
    }

    float update(float input){
        m_integrator_0 += m_Ki * input;
        float pi_result = m_Kp * input + m_integrator_0;
        m_integrator_1 += pi_result;
        return pi_result;
    }
    
    float getIntegrator0(){
        return m_integrator_0;
    }

    float getLast(){
        return m_integrator_1;
    }
};
