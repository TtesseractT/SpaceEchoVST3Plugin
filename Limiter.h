#pragma once

#include "AudioDevEffect.h"
#include "EnvelopeShaper.h"

#include <cmath>

class Limiter : public AudioDevEffect
{
public:
    // constructor
    Limiter()
    {
        m_EnvelopeShaper.setAttack(0.f);
    }

    // callback for initializing with samplerate
    void prepareForPlayback(double samplerate) override
    {
        m_EnvelopeShaper.prepareForPlayback(samplerate);
    }

    // callback for audio processing
    void processAudioSample(double& sample) override
    {
        // boost signal
        sample *= m_Boost;

        // copy signal in detection signal
        double detectionSignal = sample;

        // rectify detection signal
        detectionSignal = fabs(detectionSignal);

        // apply envelope shaping to detection signal
        m_EnvelopeShaper.processAudioSample(detectionSignal);

        // check if the detection signal exceeds 0dB
        if (detectionSignal > 1.f)
        {
            // compute gain
            double gain = 1.f / detectionSignal;

            // and apply limiting
            sample *= gain;
        }

        // apply ceiling
        sample *= m_Ceiling;
    }

    // setters for limiting parameters
    void setBoost(double boost)
    {
        m_Boost = decibelToAmplitude(boost);
    }

    void setRelease(double release)
    {
        m_EnvelopeShaper.setRelease(release);
    }

    void setCeiling(double ceiling)
    {
        m_Ceiling = decibelToAmplitude(ceiling);
    }

private:
    // limiter private variables
    double m_Boost = 1.f;
    double m_Ceiling = 1.f;
    EnvelopeShaper m_EnvelopeShaper;

    // transform decibel to amplitude
    double decibelToAmplitude(double decibel)
    {
        return pow(10.f, decibel / 20.f);
    }
};
