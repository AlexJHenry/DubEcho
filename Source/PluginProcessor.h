/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

//==============================================================================
template <typename Type> 
class DelayLine
{
public:
    void clear() noexcept
    {
        std::fill(rawData.begin, rawData.end(), Type(0));
    }

    size_t size() const noexcept
    {
        return rawData.size();
    }

    void resize(size_t newValue)
    {
        rawData.resize(newValue);
        leastRecentIndex = 0;
    }

    Type back() const noexcept
    {
        return rawData[leastRecentIndex];
    }

    Type get(size_t delayInSamples) const noexcept
    {
        jassert(delayInSamples >= 0 && delayInSamples < size());
        return rawData[(leastRecentIndex + 1 + delayInSamples) % size()];
    }

    Type set(size_t delayInSamples, Type newValue) noexcept
    {
        jassert(delayInSamples >= 0 && delayInSamples < size());
        rawData[(leastRecentIndex + 1 + delayInSamples) % size()] = newValue;
    }

    void push(Type valueToAdd) noexcept
    {
        rawData[leastRecentIndex] = valueToAdd;
        leastRecentIndex = leastRecentIndex == 0 ? size() - 1 : leastRecentIndex - 1;
    }

private:
    std::vector<Type> rawData;
    size_t leastRecentIndex = 0;
};

//==============================================================================
template <typename Type, size_t maxNumChannels = 2>
class Delay
{
public:
    //==============================================================================
    Delay()
    {
        setMaxDelayTime(2.1f);
        setDelayTime(0, 0.7f);
        setDelayTime(1, 0.5f);
        setWetLevel(0.5f);
        setFeedback(0.5f);
    }

    //==============================================================================
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        jassert(spec.numChannels <= maxNumChannels);
        sampleRate = (Type)spec.sampleRate;
        updateDelayLineSize();
        updateDelayTime();

        filterCoefs = juce::dsp::IIR::Coefficients<Type>::makeFirstOrderHighPass(sampleRate, Type(1e3));

        for (auto& f : filters)
        {
            f.prepare(spec);
            f.coefficients = filterCoefs;
        }
    }

    //==============================================================================
    void reset() noexcept
    {
        for (auto& f : filters)
            filters.reset();

        for (auto& dline : delayLines)
            dline.clear();
    }

    //==============================================================================
    size_t getNumChannels() const noexcept
    {
        return delayLines.size();
    }

    //==============================================================================
    void setMaxDelayTime(Type newValue)
    {
        jassert(newValue > Type(0));
        maxDelayTime = newValue;
        updateDelayLineSize();
    }

    //==============================================================================
    void setFeedback(Type newValue) noexcept
    {
        jassert(newValue >= Type(0) && newValue <= Type(1));
        feedback = newValue;
    }

    //==============================================================================
    void setWetLevel(Type newValue) noexcept
    {
        jassert(newValue >= Type(0) && newValue <= Type(1));
        wetLevel = newValue;
    }

    //==============================================================================
    void setDelayTime(size_t channel, Type newValue)
    {
        if (channel >= getNumChannels())
        {
            jassertfalse;
            return;
        }

        jassert(newValue >= Type(0));
        delayTimes[channel] = newValue;

        updateDelayTime();
    }

    //==============================================================================
    template <typename ProcessContext>
    void process(const ProcessContext& context) noexcept
    {
        auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        auto numSamples = outputBlock.getNumSamples();
        auto numChannels = outputBlock.getNumChannels();

        jassert(inputBlock.getNumSamples() == numSamples);
        jassert(inputBlock.getNumChannels() == numChannels);

        juce::ignoreUnused(numSamples);

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* input = inputBlock.getChannelPointer(ch);
            auto* output = outputBlock.getChannelPointer(ch);
            auto& dline = delayLines[ch];
            auto delayTime = delayTimesSample[ch];
            auto& filter = filters[ch];

            for (size_t i = 0; i < numSamples; ++i)
            {
                auto delayedSample = filter.processSample(dline.get(delayTime));
                auto inputSample = input[i];
                auto dlineInputSample = std::tanh(inputSample + feedback * delayedSample);
                dline.push(dlineInputSample);
                auto outputSample = inputSample + wetLevel * delayedSample;
                output[i] = outputSample;
            }
        }
    }

private:
    std::array<DelayLine<Type>, maxNumChannels> delayLines;
    std::array<size_t, maxNumChannels> delayTimesSample;
    std::array<Type, maxNumChannels> delayTimes;
    Type feedback{ Type(0) };
    Type wetLevel{ Type(0) };

    std::array<juce::dsp::IIR::Filter<Type>, maxNumChannels> filters;
    typename juce::dsp::IIR::Coefficients<Type>::Ptr filterCoefs;

    Type sampleRate{ Type(44.1e3) };
    Type maxDelayTime{ Type(2) };

    //==============================================================================
    void updateDelayLineSize()
    {
        auto delayLineSizeSamples = (size_t)std::ceil(maxDelayTime * sampleRate);

        for (auto& dline : delayLines)
        {
            dline.resize(delayLineSizeSamples);
        }
    }

    //==============================================================================
    void updateDelayTime() noexcept
    {
        for (size_t ch = 0; ch < maxNumChannels; ++ch)
            delayTimesSample[ch] = (size_t)juce::roundToInt(delayTimes[ch] * sampleRate);
    }
};
enum ChainPositions
{
    reverb,
    delay,
};

struct ChainSettings
{
    float reverbSize{ 0.5f }, reverbDamping{ 0.5f }, reverbWet{ 0.5f };
    float delayTime{ 0.5f }, delayFeedBack{ 0.5f }, delayWet{ 0 };
};

using MonoChain = juce::dsp::ProcessorChain<juce::dsp::Reverb, Delay<float>>;

//==============================================================================
class DubEchoAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    DubEchoAudioProcessor();
    ~DubEchoAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Creates a layout containing all the delay and reverb parameters
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };

    float getRmsValue(const int channel) const;
    
private:
    MonoChain leftChain, rightChain;
    juce::LinearSmoothedValue<float> rmsLevelLeft, rmsLevelRight;
    //==============================================================================
    void updateRmsVal(juce::AudioBuffer<float>& buffer);
    void updateFXChain();
    void updateDelay(ChainSettings& settings);
    void updateReverb(ChainSettings& settings);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DubEchoAudioProcessor)
};
