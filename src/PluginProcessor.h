#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

namespace strom
{

// The processor is the plugin's engine room: FL Studio (or the standalone
// app) calls processBlock() a few hundred times per second, each time handing
// us a small buffer to fill with audio and the MIDI that arrived meanwhile.
//
// Everything inside processBlock runs on the *audio thread*, which must never
// wait: no memory allocation, no locks, no file access. Break that rule and
// the result is audible crackle.
class StromProcessor : public juce::AudioProcessor
{
public:
    StromProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Strom"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // FL saves/restores plugin state with the project through these.
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Programs (old-style preset slots) — unused, but hosts expect sane stubs.
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    // Single source of truth for every knob. The DAW automates these, the UI
    // attaches to them, and state save/load serialises them.
    juce::AudioProcessorValueTreeState apvts;

    // Shared with the editor's on-screen keyboard so clicked notes reach the
    // synth exactly like real MIDI.
    juce::MidiKeyboardState keyboardState;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::Synthesiser synth;
    juce::dsp::Gain<float> masterGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StromProcessor)
};

} // namespace strom
