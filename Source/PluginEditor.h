/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"



class LookAndFeel : public juce::LookAndFeel_V4
{

public:
    LookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, juce::Colours::red);
    }

    void drawRotarySlider(juce::Graphics&,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override;

};
//==============================================================================
/**
*/
class DubEchoAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DubEchoAudioProcessorEditor (DubEchoAudioProcessor&);
    ~DubEchoAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DubEchoAudioProcessor& audioProcessor;

    juce::Slider delayTime;
    juce::Slider delayFeedBack;
    juce::Slider delayWet;
    juce::Slider reverbSize;
    juce::Slider reverbDamping;
    juce::Slider reverbWet;
    LookAndFeel lnf;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DubEchoAudioProcessorEditor)
};
