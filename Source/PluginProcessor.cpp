/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DubEchoAudioProcessor::DubEchoAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

DubEchoAudioProcessor::~DubEchoAudioProcessor()
{
}

//==============================================================================
const juce::String DubEchoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DubEchoAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DubEchoAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DubEchoAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DubEchoAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DubEchoAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DubEchoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DubEchoAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DubEchoAudioProcessor::getProgramName (int index)
{
    return {};
}

void DubEchoAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DubEchoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    updateFXChain();
    leftChain.prepare(spec);
    rightChain.prepare(spec);
}

void DubEchoAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DubEchoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void DubEchoAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    updateFXChain();
    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

//==============================================================================
bool DubEchoAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DubEchoAudioProcessor::createEditor()
{
   return new DubEchoAudioProcessorEditor (*this);
}

//==============================================================================
void DubEchoAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void DubEchoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DubEchoAudioProcessor();
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    settings.reverbSize = apvts.getRawParameterValue("Reverb Size")->load();
    settings.reverbDamping = apvts.getRawParameterValue("Reverb Damping")->load();
    settings.reverbWet = apvts.getRawParameterValue("Reverb Dry/Wet")->load();
    settings.delayTime = apvts.getRawParameterValue("Delay Time")->load();
    settings.delayFeedBack = apvts.getRawParameterValue("Delay Feedback")->load();
    settings.delayWet = apvts.getRawParameterValue("Delay Dry/Wet")->load();

    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout DubEchoAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("Reverb Size",
        "Reverb Size",juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Reverb Damping",
        "Reverb Damping", juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Reverb Dry/Wet",
        "Reverb Dry/Wet", juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Delay Time",
        "Delay Time", juce::NormalisableRange<float>(0.f, 2.f, 0.01f, 1.f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Delay Feedback",
        "DelayFeedback", juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Delay Dry/Wet",
        "Delay Dry/Wet", juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f), 0.5f));

    return layout;
}
void DubEchoAudioProcessor::updateFXChain()
{
    auto settings = getChainSettings(apvts);
    updateDelay(settings);
    updateReverb(settings);
}

void DubEchoAudioProcessor::updateDelay(ChainSettings& settings)
{

    auto& leftDelay = leftChain.get<ChainPositions::delay>();
    auto& rightDelay = rightChain.get<ChainPositions::delay>();

    leftDelay.setDelayTime(0, settings.delayTime);
    leftDelay.setDelayTime(1, settings.delayTime);
    leftDelay.setFeedback(settings.delayFeedBack);
    leftDelay.setWetLevel(settings.delayWet);

    rightDelay.setDelayTime(0, settings.delayTime);
    rightDelay.setDelayTime(1, settings.delayTime);
    rightDelay.setFeedback(settings.delayFeedBack);
    rightDelay.setWetLevel(settings.delayWet);

}

void DubEchoAudioProcessor::updateReverb(ChainSettings& settings)
{
    auto& leftReverb = leftChain.get<ChainPositions::reverb>();
    auto& rightReverb = rightChain.get<ChainPositions::reverb>();

    auto parameters = leftReverb.getParameters();
    parameters.wetLevel = settings.reverbWet;
    parameters.damping = settings.reverbDamping;
    parameters.dryLevel = 1.f - parameters.wetLevel;
    parameters.roomSize = settings.reverbSize;

    leftReverb.setParameters(parameters);
    rightReverb.setParameters(parameters);
}
