#include <juce_audio_utils/juce_audio_utils.h>
#include <sstream>
#include <set>
#include "Arp.h"
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
    chk (p.defaultCcOf ("s_lvl2") == 46 && p.defaultCcOf ("s_arpOct") == 120 && p.defaultCcOf ("s_ring") == 43, "OSC2 vol=46, arp oct=120, ring=43");
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
    // 6. LFO1 -> filtre : le potard LFO change le son (variation du niveau d'un bloc a l'autre) et est sur le CC 52
    chk (p.defaultCcOf ("s_fltLfo") == 52, "s_fltLfo sur CC52");
    auto var = [&] (float amt)
    {
        p.loadFactoryPreset (0); p.setParamReal ("s_lpf", 0.35f); p.setParamReal ("s_lpfRes", 0.5f); p.setParamReal ("s_fltEnv", 0.f); p.setParamReal ("s_aS", 1.f);
        p.setParamReal ("s_l1Rate", 0.55f); p.setParamReal ("s_fltLfo", amt);
        juce::MidiBuffer mb; mb.addEvent (juce::MidiMessage::noteOn (1, 48, 0.9f), 0); rms (p, mb, 40);
        double m = 0, m2 = 0; const int N = 300; for (int k = 0; k < N; ++k) { const double r = rms (p, {}, 1); m += r; m2 += r * r; }
        m /= N; return std::sqrt (std::max (0.0, m2 / N - m * m)) / (m + 1e-9);
    };
    const double v0 = var (0.f), v1 = var (1.f); std::printf ("  variation relative : LFO 0 = %.4f, LFO 1 = %.4f\n", v0, v1);
    chk (v1 > 3 * v0 + 0.01, "le potard LFO module le cutoff");
    // 7. v19e : spring sur les CC de la reverb, rack Init = delay + spring, arpegiateur (ratchet global, triolets, 4 octaves)
    chk (p.defaultCcOf ("springMix") == 91 && p.defaultCcOf ("springDecay") == 65 && p.defaultCcOf ("springDrip") == 67 && p.defaultCcOf ("revMix") == 93 && p.defaultCcOf ("s_arpRatchet") == 109, "CC spring 91/65/67, reverb 93/94, ratchet 109");
    { OrageAudioProcessor q; const auto rk = q.engine.getRack(); chk (rk.size() == 2 && rk[0] == orage::NBbd && rk[1] == orage::NSpring, "nouvelle instance : rack = delay + spring"); }
    { p.loadFactoryPreset (0); const auto rk = p.engine.getRack(); chk (rk.size() == 2, "preset Init : delay + spring"); chk (p.apvts.getRawParameterValue ("springMix")->load() == 0.f && p.apvts.getRawParameterValue ("bbdMix")->load() == 0.f, "mix delay/spring a zero"); }
    auto notes = [] (int ratchet, int range, float stepHz)
    {
        orage::Arp a; a.prepare (48000.f); orage::ArpPattern pat; a.setPattern (&pat);
        orage::ArpParams P; P.on = 1; P.ratchet = ratchet; P.range = range; P.len = 8; a.setParams (P);
        for (int nt : { 60, 64, 67 }) a.noteOn (nt, 0.8f);
        orage::Arp::Ev ev[256]; int on = 0, hi = 0; std::set<int> distinct;
        for (int k = 0; k < 940; ++k) { const int c = a.process (512, stepHz, ev, 256); for (int i = 0; i < c; ++i) if (ev[i].lane < 0 && ev[i].on) { ++on; hi = std::max (hi, ev[i].note); distinct.insert (ev[i].note); } }
        return std::array<int, 3> { on, hi, (int) distinct.size() };
    };
    const auto r1 = notes (1, 0, 4.f), r3 = notes (3, 0, 4.f), rT = notes (1, 0, 6.f), r4o = notes (1, 3, 4.f);
    std::printf ("  arp 10 s : ratchet 1 = %d notes, ratchet 3 = %d, triolet(x1.5) = %d, 4 oct : %d notes distinctes (max %d)\n", r1[0], r3[0], rT[0], r4o[2], r4o[1]);
    chk (r3[0] >= 2.7 * r1[0] && r3[0] <= 3.3 * r1[0], "ratchet global 3x = 3 fois plus de notes");
    chk (rT[0] >= 1.4 * r1[0] && rT[0] <= 1.6 * r1[0], "triolets : 1,5 fois plus de pas");
    chk (r4o[2] == 12 && r4o[1] == 67 + 36, "4 octaves : 12 notes distinctes jusqu'a +3 octaves");
    std::printf ("%s\n", fail ? "ECHEC" : "TOUT OK"); return fail;
}
