#pragma once
#include <JuceHeader.h>
namespace GUI
{
    class Bulb : public juce::Component
    {
    public:
        Bulb(const juce::Colour& c) : colour(c) {}

        void paint(juce::Graphics& g) override
        {
            const auto delta = 4.f;
            const auto bounds = getLocalBounds().toFloat().reduced(delta);
            const auto side = juce::jmin(bounds.getWidth(), bounds.getHeight());
            const auto bulbFillBounds = juce::Rectangle<float>(bounds.getX(), bounds.getY(), side, side);

            if (isOn)
                g.setColour(colour);
            else
                g.setColour(juce::Colours::black);

            g.fillEllipse(bulbFillBounds);
            g.setColour(juce::Colours::black);
            g.drawEllipse(bulbFillBounds, 1.f);

            if (isOn)
            {
                g.setGradientFill(
                    juce::ColourGradient{
                        colour.withAlpha(0.3f),
                        bulbFillBounds.getCentre(),
                        colour.withLightness(1.5f).withAlpha(0.f),
                        {},
                        true
                    });
                g.fillEllipse(bulbFillBounds.expanded(delta));
            };
        }

        void setState(const bool state) { isOn = state; }

    private:
        bool isOn = false;
        juce::Colour colour{};
    };
    class VerticalDiscreteMeter : public juce::Component, juce::Timer
    {
    public:
        VerticalDiscreteMeter(std::function<float()>&& valueFunction) : valueSupplier(std::move(valueFunction))
        {
            startTimerHz(24);
        }

        void paint(juce::Graphics& g) override
        {
            const auto level = juce::jmap(valueSupplier(), -60.f, 6.f, 0.f, 1.f);
            for (auto i = 0; i < totalNoBulbs; i++)
            {
                if (level >= static_cast<float>(i + 1) / totalNoBulbs)
                    bulbs[i]->setState(true);
                else
                    bulbs[i]->setState(false);

            }
        }

        void resized() override
        {
            const auto bounds = getLocalBounds().toFloat();
            gradient = juce::ColourGradient{
                            juce::Colours::green,
                            bounds.getBottomLeft(),
                            juce::Colours::red,
                            bounds.getTopLeft(),
                            false
            };
            gradient.addColour(0.5, juce::Colours::yellow);

            auto bulbBounds = getLocalBounds();
            const auto bulbHeight = bulbBounds.getHeight() / totalNoBulbs;
            bulbs.clear();
    
            for (auto i = 0; i < totalNoBulbs; i++)
            {
                auto bulb = std::make_unique<Bulb>(gradient.getColourAtPosition(static_cast<double>(i) / totalNoBulbs));
                addAndMakeVisible(bulb.get());
                bulb->setBounds(bulbBounds.removeFromBottom(bulbHeight));
                bulbs.push_back(std::move(bulb));
            }

        }

        void timerCallback() override
        {
            repaint();
        }
    private:
        std::function<float()> valueSupplier;
        std::vector<std::unique_ptr<Bulb>> bulbs;
        juce::ColourGradient gradient{};
        const int totalNoBulbs = 10;

    };
}