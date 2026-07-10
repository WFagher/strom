#pragma once

#include "PluginProcessor.h"

namespace strom
{

// Milestone-1 interim UI: an auto-generated panel of sliders (one per
// parameter) plus a playable on-screen keyboard. The real vintage faceplate
// arrives in milestone 5 — this exists so the synth is usable from day one.
class StromEditor : public juce::AudioProcessorEditor
{
public:
    explicit StromEditor (StromProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    StromProcessor& processor;

    juce::GenericAudioProcessorEditor parameterPanel;
    juce::MidiKeyboardComponent keyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StromEditor)
};

} // namespace strom
