#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "engine/SynthVoice.h"

namespace strom
{

namespace
{
    constexpr int numVoices = 16;

    // Seconds-range time knobs feel wrong when linear (all the action lives in
    // the first tenth of the range), so they get a logarithmic-ish skew.
    juce::NormalisableRange<float> timeRange (float min, float max)
    {
        juce::NormalisableRange<float> range { min, max, 0.0001f };
        range.setSkewForCentre (min + (max - min) * 0.25f);
        return range;
    }
}

StromProcessor::StromProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    for (int i = 0; i < numVoices; ++i)
        synth.addVoice (new SynthVoice());

    synth.addSound (new StromSound());
}

juce::AudioProcessorValueTreeState::ParameterLayout StromProcessor::createParameterLayout()
{
    using P = juce::AudioParameterFloat;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // ParameterID version (the ", 1") lets future Ström versions add or
    // reorder parameters without breaking automation saved in old projects.
    layout.add (std::make_unique<P> (juce::ParameterID { "master", 1 }, "Master",
                                     juce::NormalisableRange<float> { -60.0f, 0.0f, 0.1f }, -12.0f,
                                     juce::AudioParameterFloatAttributes{}.withLabel ("dB")));

    layout.add (std::make_unique<P> (juce::ParameterID { "ampAttack", 1 }, "Attack",
                                     timeRange (0.001f, 5.0f), 0.005f,
                                     juce::AudioParameterFloatAttributes{}.withLabel ("s")));
    layout.add (std::make_unique<P> (juce::ParameterID { "ampDecay", 1 }, "Decay",
                                     timeRange (0.001f, 5.0f), 0.12f,
                                     juce::AudioParameterFloatAttributes{}.withLabel ("s")));
    layout.add (std::make_unique<P> (juce::ParameterID { "ampSustain", 1 }, "Sustain",
                                     juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f }, 0.8f));
    layout.add (std::make_unique<P> (juce::ParameterID { "ampRelease", 1 }, "Release",
                                     timeRange (0.001f, 8.0f), 0.25f,
                                     juce::AudioParameterFloatAttributes{}.withLabel ("s")));

    return layout;
}

void StromProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);

    masterGain.prepare ({ sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 });
    masterGain.setRampDurationSeconds (0.02); // smooth knob moves, no zipper noise
}

bool StromProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void StromProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Merge notes clicked on the editor's keyboard into the host's MIDI.
    keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

    // Push current envelope knob values to every voice (atomic reads, cheap).
    juce::ADSR::Parameters envParams {
        apvts.getRawParameterValue ("ampAttack")->load(),
        apvts.getRawParameterValue ("ampDecay")->load(),
        apvts.getRawParameterValue ("ampSustain")->load(),
        apvts.getRawParameterValue ("ampRelease")->load()
    };

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            voice->setEnvelopeParameters (envParams);

    buffer.clear();
    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    masterGain.setGainDecibels (apvts.getRawParameterValue ("master")->load());
    auto block = juce::dsp::AudioBlock<float> (buffer);
    masterGain.process (juce::dsp::ProcessContextReplacing<float> (block));
}

void StromProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void StromProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* StromProcessor::createEditor()
{
    return new StromEditor (*this);
}

} // namespace strom

// JUCE's entry point: the host calls this to create the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new strom::StromProcessor();
}
