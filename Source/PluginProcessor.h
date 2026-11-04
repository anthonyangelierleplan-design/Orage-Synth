#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Engine.h"
#ifndef ORAGE_SYNTH
 #define ORAGE_SYNTH 0
#endif
#include "Synth.h"

class OrageAudioProcessor : public juce::AudioProcessor
{
public:
    OrageAudioProcessor();
    ~OrageAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    #if ORAGE_SYNTH
    const juce::String getName() const override { return "Orage"; }
    bool acceptsMidi() const override  { return true; }
#else
    const juce::String getName() const override { return "Orage FX"; }
    bool acceptsMidi() const override  { return false; }
#endif
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ---- presets et remises a zero (thread de l'interface)
    static int getNumFactoryPresets();
    static juce::String getFactoryPresetName (int index);
    void loadFactoryPreset (int index);
    juce::File getPresetDir() const;
    juce::StringArray getUserPresetNames() const;
    bool saveUserPreset (const juce::String& name);
    bool loadUserPreset (const juce::String& name);
    bool deleteUserPreset (const juce::String& name);
    void resetModules();        // remet tous les reglages par defaut (rack et patchbay intacts)
    void clearRack();           // vide le rack : INPUT est relie directement a OUTPUT
    void defaultRack();         // remet les modules dans l'ordre d'origine
    void clearPatchbay();       // retire tous les cables
    void setParamReal (const char* id, float realValue);

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;  // clavier a l'ecran (version instrument)
    orage::Synth synth;                     // utilise seulement par la version instrument (ORAGE_SYNTH)
    orage::Engine engine;                   // rack + patchbay vivent ici ; l'editeur les lit et les modifie
    juce::String presetName { "Init" };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    orage::Params readParams() const;
    orage::SynthParams readSynth() const;
    std::vector<std::atomic<float>*> rawSyn;
    juce::ValueTree captureState();
    void applyState (juce::ValueTree state);

    std::vector<std::atomic<float>*> raw;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OrageAudioProcessor)
};
