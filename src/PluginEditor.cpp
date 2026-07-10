#include "PluginEditor.h"

namespace strom
{

StromEditor::StromEditor (StromProcessor& p)
    : AudioProcessorEditor (p),
      processor (p),
      parameterPanel (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    addAndMakeVisible (parameterPanel);
    addAndMakeVisible (keyboard);

    keyboard.setKeyWidth (24.0f);
    keyboard.setLowestVisibleKey (36); // C2

    setSize (520, 320);
}

void StromEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void StromEditor::resized()
{
    auto area = getLocalBounds();
    keyboard.setBounds (area.removeFromBottom (80));
    parameterPanel.setBounds (area);
}

} // namespace strom
