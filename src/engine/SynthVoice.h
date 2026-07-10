#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

namespace strom
{

// A "sound" in JUCE terms is just a tag describing which notes/channels a
// voice may respond to. Ström has one kind of sound that covers everything.
struct StromSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override    { return true; }
    bool appliesToChannel (int) override { return true; }
};

// One voice = one currently-sounding note. The Synthesiser owns 16 of these
// and hands each incoming MIDI note to a free one (stealing the oldest when
// all are busy). Milestone 1 keeps the voice minimal: a naive sawtooth
// oscillator shaped by an ADSR envelope.
//
// The oscillator is a *phase accumulator*: `phase` runs 0→1 over one cycle of
// the waveform, advancing by `phaseIncrement` (= frequency / sampleRate) every
// sample. A sawtooth is then simply phase scaled to the range -1..+1.
// This naive saw aliases (rings falsely at high notes) — fixed in milestone 2
// with PolyBLEP. See docs/LEARNING.md chapter 1.
class SynthVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound (juce::SynthesiserSound* sound) override;

    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                          int startSample, int numSamples) override;

    // Called by the processor once per block so envelope knob changes reach
    // every voice. Safe to call while a note is playing.
    void setEnvelopeParameters (const juce::ADSR::Parameters& params);

private:
    double phase          = 0.0;   // position inside the waveform cycle, 0..1
    double phaseIncrement = 0.0;   // cycle fraction to advance per sample
    float  level          = 0.0f;  // velocity-scaled loudness of this note

    juce::ADSR adsr;               // attack / decay / sustain / release shaper
};

} // namespace strom
