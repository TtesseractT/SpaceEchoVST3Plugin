#pragma once

#include "AudioDevEffect.h"
#include <cmath>

class EnvelopeShaper : public AudioDevEffect
{
public:
    // callback for initializing with samplerate
    void prepareForPlayback(double samplerate) override
    {
        m_Samplerate = samplerate;
        update();
    }

    // callback for audio processing
    void processAudioSample(double& sample) override
    {
        if (sample > m_Envelope)
        {
            m_Envelope += m_Attack * (sample - m_Envelope);
        }
        else if (sample < m_Envelope)
        {
            m_Envelope += m_Release * (sample - m_Envelope);
        }
        sample = m_Envelope;
    }

    // setters for compression parameters
    void setAttack(double attack)
    {
        m_AttackInMilliseconds = attack;
        update();
    }

    void setRelease(double release)
    {
        m_ReleaseInMilliseconds = release;
        update();
    }

private:
    // envelope shaper private variables and functions
    double m_Envelope = 0.f;
    double m_Samplerate = 96000.f;
    double m_AttackInMilliseconds = 10.f;
    double m_ReleaseInMilliseconds = 100.f;
    double m_Attack = 0.f;
    double m_Release = 0.f;

    // update attack and release scaling factors
    void update()
    {
        m_Attack = calculate(m_AttackInMilliseconds);
        m_Release = calculate(m_ReleaseInMilliseconds);
    }

    // calculate scaling factor from a value in milliseconds
    double calculate(double time)
    {
        if (time <= 0.f || m_Samplerate <= 0.f)
        {
            return 1.f;
        }
        return 1.f - exp(-1.f / (time * 0.001f * m_Samplerate));
    }
};
