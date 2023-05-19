/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "VerticalDiscreteMeter.h"


struct LookAndFeel : juce::LookAndFeel_V4
{ 
    void drawRotarySlider(juce::Graphics&,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override;

};

struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix) :
        juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::NoTextBox),
        param(&rap),
        suffix(unitSuffix)
    {
        setLookAndFeel(&lnf);
    }

    ~RotarySliderWithLabels()
    {
        setLookAndFeel(nullptr);
    }

    struct LabelPos
    {
        float pos;
        juce::String label;
    };

    juce::Array<LabelPos> labels;

    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;

private:
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
};
//==============================================================================
/**
*/
class DubEchoAudioProcessorEditor : public juce::AudioProcessorEditor
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

    RotarySliderWithLabels delayTimeSlider,
        delayFeedBackSlider,
        delayWetSlider,
        reverbSizeSlider,
        reverbDampingSlider,
        reverbWetSlider;

    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    Attachment delayTimeSliderAttachment,
        delayFeedBackSliderAttachment,
        delayWetSliderAttachment,
        reverbSizeSliderAttachment,
        reverbDampingSliderAttachment,
        reverbWetSliderAttachment;

    std::vector<juce::Component*> getComps();

    GUI::VerticalDiscreteMeter verticalDiscreteMeterL, verticalDiscreteMeterR;
    LookAndFeel lnf;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DubEchoAudioProcessorEditor)
};
