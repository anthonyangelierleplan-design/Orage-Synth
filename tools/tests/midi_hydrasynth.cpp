#include <juce_audio_utils/juce_audio_utils.h>
#include <sstream>
#include <map>
#include <array>
#include <atomic>
#include <functional>
#include <algorithm>
#include <cmath>
#include <random>
#include <thread>
#include <mutex>

#include "PluginProcessor.h"
#undef private
// MIDI : carte Hydrasynth par defaut, Program Change, sustain, pitch bend
static double rms (OrageAudioProcessor& p, juce::MidiBuffer mb, int blocks = 1)
{
    juce::AudioBuffer<float> b (2, 512); double s = 0;
    for (int k = 0; k < blocks; ++k) { b.clear(); p.processBlock (b, mb); mb.clear(); for (int i = 0; i < 512; ++i) s += b.getSample (0, i) * b.getSample (0, i); }
    return std::sqrt (s / (512.0 * blocks));
}
int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    int fail = 0; auto chk = [&] (bool ok, const char* w) { std::printf ("%s %s\n", ok ? "ok  " : "FAIL", w); if (! ok) ++fail; };
    OrageAudioProcessor p; p.prepareToPlay (48000.0, 512);
    // 1. aucune collision de CC par defaut
    std::map<int, juce::String> seen; bool dup = false; int n = 0;
    for (int i = 0; i < p.mapIds.size(); ++i) { const int c = p.defaultCcOf (p.mapIds[i]); if (c < 0) continue; ++n; if (seen.count (c)) { dup = true; std::printf ("  collision CC%d: %s / %s\n", c, seen[c].toRawUTF8(), p.mapIds[i].toRawUTF8()); } seen[c] = p.mapIds[i]; }
    chk (! dup, "pas de collision de CC"); std::printf ("  %d parametres sur CC par defaut\n", n);
    for (auto id : { "s_lpf", "s_lpfRes", "s_aA", "s_fR", "revMix", "bbdFb", "s_l4Rate", "s_uniDetune" }) chk (p.defaultCcOf (id) >= 0, id);
    // 2. CC74 -> cutoff (mode direct)
    p.takeoverMode.store (2);
    { juce::MidiBuffer mb; mb.addEvent (juce::MidiMessage::controllerEvent (1, 74, 127), 0); rms (p, mb); }
    auto* lp = p.apvts.getParameter ("s_lpf"); chk (lp->getValue() > 0.99f, "CC74=127 -> cutoff 100 %");
    { juce::MidiBuffer mb; mb.addEvent (juce::MidiMessage::controllerEvent (1, 74, 0), 0); rms (p, mb); } chk (lp->getValue() < 0.01f, "CC74=0 -> cutoff 0 %");
    // 3. Program Change
    { juce::MidiBuffer mb; mb.addEvent (juce::MidiMessage::programChange (1, 5), 0); rms (p, mb); }
    for (int k = 0; k < 50; ++k) juce::MessageManager::getInstance()->runDispatchLoopUntil (10);
    chk (p.presetName == OrageAudioProcessor::getFactoryPresetName (5), "Program Change 5 -> preset 5");
    // 4. sustain : note relachee pedale enfoncee continue de sonner, puis s'arrete au relachement
    p.loadFactoryPreset (0); p.setParamReal ("s_arpOn", 0.f); p.setParamReal ("s_hold", 0.f); p.setParamReal ("s_aR", 0.1f);
    { juce::MidiBuffer mb; mb.addEvent (juce::MidiMessage::controllerEvent (1, 64, 127), 0); mb.addEvent (juce::MidiMessage::noteOn (1, 60, 0.9f), 1); rms (p, mb, 20); }
    { juce::MidiBuffer mb; mb.addEvent (juce::MidiMessage::noteOff (1, 60), 0); rms (p, mb, 1); }
    const double heldA = rms (p, {}, 20);
    { juce::MidiBuffer mb; mb.addEvent (juce::MidiMessage::controllerEvent (1, 64, 0), 0); rms (p, mb, 1); }
    const double after = rms (p, {}, 400);
    std::printf ("  rms note tenue %.4f, apres relachement pedale %.6f\n", heldA, after);
    chk (heldA > 0.002 && after < heldA * 0.2, "pedale sustain");
    // 5. pitch bend : etendue
    p.setParamReal ("s_bendRange", 11.f);
    juce::MidiBuffer mb; mb.addEvent (juce::MidiMessage::pitchWheel (1, 16383), 0); rms (p, mb);
    chk (std::fabs (p.bendRange - 12.f) < 0.5f, "bend range lu");
    std::printf ("  bendRange = %.1f\n", p.bendRange);
    std::printf ("%s\n", fail ? "ECHEC" : "TOUT OK"); return fail;
}
