/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DubEchoAudioProcessorEditor::DubEchoAudioProcessorEditor (DubEchoAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setLookAndFeel(&lnf);
    
    delayTime.setSliderStyle(juce::Slider::Rotary);
    delayTime.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(delayTime);

    delayWet.setSliderStyle(juce::Slider::Rotary);
    delayWet.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(delayWet);

    delayFeedBack.setSliderStyle(juce::Slider::Rotary);
    delayFeedBack.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(delayFeedBack);

    reverbSize.setSliderStyle(juce::Slider::Rotary);
    reverbSize.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(reverbSize);

    reverbDamping.setSliderStyle(juce::Slider::Rotary);
    reverbDamping.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(reverbDamping);

    reverbWet.setSliderStyle(juce::Slider::Rotary);
    reverbWet.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(reverbWet);
    setSize (400, 300);
}

DubEchoAudioProcessorEditor::~DubEchoAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void DubEchoAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::lightgrey);
}

void DubEchoAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto border = 4;

    auto area = getLocalBounds();

    auto delayArea = area.removeFromTop(area.getHeight() / 2);
    delayTime.setBounds(delayArea.removeFromLeft(delayArea.getWidth() / 3).reduced(border));
    delayFeedBack.setBounds(delayArea.removeFromLeft(delayArea.getWidth() / 2).reduced(border));
    delayWet.setBounds(delayArea.reduced(border));

    auto reverbArea = area;
    reverbSize.setBounds(reverbArea.removeFromLeft(reverbArea.getWidth() / 3).reduced(border));
    reverbDamping.setBounds(reverbArea.removeFromLeft(reverbArea.getWidth() / 2).reduced(border));
    reverbWet.setBounds(reverbArea.reduced(border));
}

void LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider&)
{
    using namespace juce;

    auto radius = (float) jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float) width * 0.5f;
    auto centreY = (float)y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour(Colours::orange);
    g.fillEllipse(rx, ry, rw, rw);

    g.setColour(Colours::red);
    g.drawEllipse(rx, ry, rw, rw, 1.0f);

    juce::Path p;
    auto pointerLength = radius * 0.33f;
    auto pointerThickness = 2.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(AffineTransform::rotation(angle).translated (centreX, centreY));

    g.setColour(Colours::yellow);
    g.fillPath(p);
}
