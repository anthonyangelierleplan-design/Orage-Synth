// Editeur de la version instrument : deux onglets (SYNTH / EFFECTS) + clavier a l'ecran.
// L'onglet EFFECTS est l'editeur d'Orage FX tel quel ; l'onglet SYNTH se pose par-dessus son corps (l'en-tete et les presets restent visibles).
#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginEditor.h"

class LfoView : public juce::Component
{
public:
    LfoView (juce::AudioProcessorValueTreeState& a, const juce::String& waveId, const juce::String& slewId) : apvts (a), wave (waveId), slew (slewId) {}
    void paint (juce::Graphics&) override;
private:
    float value (const juce::String& id) const;
    juce::AudioProcessorValueTreeState& apvts;
    juce::String wave, slew;
};

class SynthPanel : public juce::Component, private juce::Timer
{
public:
    explicit SynthPanel (OrageAudioProcessor&);
    ~SynthPanel() override { stopTimer(); }
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    KnobUnit* knob (juce::Component& parent, const char* id, const juce::String& text, const char* lo = "0", const char* hi = "10");
    ModuleUnit* unit (const juce::String& title);

    OrageAudioProcessor& proc;
    std::vector<std::unique_ptr<ModuleUnit>> units;
    std::vector<std::unique_ptr<KnobUnit>> knobs;
    std::vector<std::unique_ptr<juce::Component>> owned;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAtt;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAtt;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAtt;
    std::vector<LfoView*> lfoViews;
    juce::Image tex;
};

// Fenetre redimensionnable : tout le contenu est dessine a la taille de conception puis mis a l'echelle.
class SynthEditor : public juce::AudioProcessorEditor
{
public:
    explicit SynthEditor (OrageAudioProcessor&);
    ~SynthEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void showTab (int t);
    int designHeight() const;
    int designWidth() const;
    OrageLookAndFeel laf;
    OrageAudioProcessor& proc;
    juce::Component content;
    OrageAudioProcessorEditor fx;
    std::unique_ptr<SynthPanel> synthPanel;
    std::unique_ptr<juce::MidiKeyboardComponent> keys;
    juce::TextButton synthTab { "SYNTH" }, fxTab { "EFFECTS" };
    juce::ComponentBoundsConstrainer constrainer;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthEditor)
};
