/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DubEchoAudioProcessorEditor::DubEchoAudioProcessorEditor (DubEchoAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    delayTimeSlider(*audioProcessor.apvts.getParameter("Delay Time"), ""),
    delayFeedBackSlider(*audioProcessor.apvts.getParameter("Delay Feedback"), ""),
    delayWetSlider(*audioProcessor.apvts.getParameter("Delay Dry/Wet"), ""),
    reverbSizeSlider(*audioProcessor.apvts.getParameter("Reverb Damping"), ""),
    reverbDampingSlider(*audioProcessor.apvts.getParameter("Reverb Size"), ""),
    reverbWetSlider(*audioProcessor.apvts.getParameter("Reverb Dry/Wet"), ""),

    delayTimeSliderAttachment(audioProcessor.apvts, "Delay Time", delayTimeSlider),
    delayFeedBackSliderAttachment(audioProcessor.apvts, "Delay Feedback", delayFeedBackSlider),
    delayWetSliderAttachment(audioProcessor.apvts, "Delay Dry/Wet", delayWetSlider),
    reverbSizeSliderAttachment(audioProcessor.apvts, "Reverb Size", reverbSizeSlider),
    reverbDampingSliderAttachment(audioProcessor.apvts, "Reverb Damping", reverbDampingSlider),
    reverbWetSliderAttachment(audioProcessor.apvts, "Reverb Dry/Wet", reverbWetSlider),

    verticalDiscreteMeterL([&]() { return audioProcessor.getRmsValue(0); }),
    verticalDiscreteMeterR([&]() { return audioProcessor.getRmsValue(1); })
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    //delayTimeSlider.labels.add({ 0.f }, "10 mS");
    //delayTimeSlider.labels.add({ 1.f }, "2000 mS");

    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }
    setSize (400, 300);
}

DubEchoAudioProcessorEditor::~DubEchoAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void DubEchoAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;
    g.fillAll(Colours::ivory);

    g.drawFittedText("Time", delayTimeSlider.getBounds(), juce::Justification::centredBottom, 1);
    g.drawFittedText("FeedBack", delayFeedBackSlider.getBounds(), juce::Justification::centredBottom, 1);
    g.drawFittedText("Dry/Wet", delayWetSlider.getBounds(), juce::Justification::centredBottom, 1);

    g.drawFittedText("Size", reverbSizeSlider.getBounds(), juce::Justification::centredBottom, 1);
    g.drawFittedText("Damping", reverbDampingSlider.getBounds(), juce::Justification::centredBottom, 1);
    g.drawFittedText("Dry/Wet", reverbWetSlider.getBounds(), juce::Justification::centredBottom, 1);

}

void DubEchoAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto border = 4;

    auto area = getLocalBounds();

    auto meterBounds = area.removeFromRight(area.getWidth() / 6);
    verticalDiscreteMeterL.setBounds(meterBounds.removeFromRight(meterBounds.getWidth() / 2).reduced(border));
    verticalDiscreteMeterR.setBounds(meterBounds.reduced(border));

    auto delayArea = area.removeFromTop(area.getHeight() / 2);
    delayTimeSlider.setBounds(delayArea.removeFromLeft(delayArea.getWidth() / 3).reduced(border));
    delayFeedBackSlider.setBounds(delayArea.removeFromLeft(delayArea.getWidth() / 2).reduced(border));
    delayWetSlider.setBounds(delayArea.reduced(border));

    auto reverbArea = area;
    reverbSizeSlider.setBounds(reverbArea.removeFromLeft(reverbArea.getWidth() / 3).reduced(border));
    reverbDampingSlider.setBounds(reverbArea.removeFromLeft(reverbArea.getWidth() / 2).reduced(border));
    reverbWetSlider.setBounds(reverbArea.reduced(border));
}

std::vector<juce::Component*> DubEchoAudioProcessorEditor::getComps()
{
    return
    {
        &delayTimeSlider,
        &delayFeedBackSlider,
        &delayWetSlider,
        &reverbSizeSlider,
        &reverbDampingSlider,
        &reverbWetSlider,

        &verticalDiscreteMeterL,
        &verticalDiscreteMeterR
    };
}


void LookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    auto enabled = slider.isEnabled();

    g.setColour(enabled ? Colours::black : Colours::darkgrey);
    g.fillEllipse(bounds);

    g.setColour(enabled ? Colours::lightgrey : Colours::grey);
    g.drawEllipse(bounds, 2.f);

    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();
        Path p;

        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);

        p.addRoundedRectangle(r, 2.f);

        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

        g.fillPath(p);

        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());

        g.setColour(enabled ? Colours::black : Colours::darkgrey);
        g.fillRect(r);

        g.setColour(enabled ? Colours::white : Colours::lightgrey);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    getLookAndFeel().drawRotarySlider(g,
        sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight(),
        jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
        startAng,
        endAng,
        *this);

    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;

    g.setColour(Colour(0u, 172u, 1u));
    g.setFont(getTextHeight());

    auto numChoices = labels.size();
    for (int i = 0; i < numChoices; ++i)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);

        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);

        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);

        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());

        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);

    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        return choiceParam->getCurrentChoiceName();

    juce::String str;

    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = getValue();
        str = juce::String(val);
    }
    return str;
}

