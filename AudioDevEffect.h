#pragma once

// interface class all Audio Dev effects inherit from
class AudioDevEffect
{
public:
    // callback for initializing with samplerate
    virtual void prepareForPlayback(double samplerate) = 0;

    // callback for audio processing
    virtual void processAudioSample(double& sample) = 0;
};