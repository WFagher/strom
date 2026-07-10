#include "engine/SynthVoice.h"

namespace strom
{

bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<StromSound*> (sound) != nullptr;
}

void SynthVoice::startNote (int midiNoteNumber, float velocity,
                            juce::SynthesiserSound*, int)
{
    // MIDI note → frequency in Hz (note 69 = A4 = 440 Hz, 12 notes per octave).
    const auto frequency = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);

    phaseIncrement = frequency / getSampleRate();
    phase          = 0.0;

    // Perceived loudness is roughly logarithmic, so squaring the 0..1 velocity
    // gives a more natural touch response than using it linearly.
    level = velocity * velocity;

    adsr.setSampleRate (getSampleRate());
    adsr.noteOn();
}

void SynthVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        // Let the release stage fade the note out; renderNextBlock frees the
        // voice once the envelope reaches silence.
        adsr.noteOff();
    }
    else
    {
        // Voice is being stolen for a new note — cut immediately.
        adsr.reset();
        clearCurrentNote();
    }
}

void SynthVoice::setEnvelopeParameters (const juce::ADSR::Parameters& params)
{
    adsr.setParameters (params);
}

void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                                  int startSample, int numSamples)
{
    if (! isVoiceActive())
        return;

    // Headroom so 16 simultaneous voices can't clip the master bus.
    constexpr float voiceGain = 0.12f;

    for (int i = 0; i < numSamples; ++i)
    {
        // Naive sawtooth: map phase 0..1 to -1..+1.
        const auto saw = static_cast<float> (2.0 * phase - 1.0);

        const auto sample = saw * adsr.getNextSample() * level * voiceGain;

        // Same signal to every output channel (mono voice, stereo comes later).
        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            outputBuffer.addSample (channel, startSample + i, sample);

        phase += phaseIncrement;
        if (phase >= 1.0)
            phase -= 1.0;
    }

    // Envelope finished its release → this voice is free for the next note.
    if (! adsr.isActive())
        clearCurrentNote();
}

} // namespace strom
