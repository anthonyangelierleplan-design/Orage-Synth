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
#define private public
#include "PluginProcessor.h"
#undef private
static void run (OrageAudioProcessor& p, juce::MidiBuffer mb) { juce::AudioBuffer<float> b (2, 512); b.clear(); p.processBlock (b, mb); }

static float sweep (OrageAudioProcessor& p, int mode, float start, const std::vector<int>& path, bool smoothedUi)
{
    p.takeoverMode.store (mode);
    auto* q = p.apvts.getParameter ("s_arpGate"); q->setValueNotifyingHost (start);
    run (p, {});
    float mx = 0.f, mn = 1.f;
    for (int v : path) { juce::MidiBuffer mb; mb.addEvent (juce::MidiMessage::controllerEvent (1, 107, v), 0); run (p, mb); mx = std::max (mx, q->getValue()); mn = std::min (mn, q->getValue()); }
    std::printf ("  mode %d start %.2f -> fin %.3f (min %.3f max %.3f)\n", mode, start, q->getValue(), mn, mx);
    return q->getValue();
}
int main()
{
    juce::ScopedJuceInitialiser_GUI init; int fail = 0; auto chk = [&] (bool ok, const char* w) { std::printf ("%s %s\n", ok ? "ok  " : "FAIL", w); if (! ok) ++fail; };
    OrageAudioProcessor p; p.prepareToPlay (48000.0, 512);
    std::vector<int> up, upDown;
    for (int v = 40; v <= 127; v += 3) up.push_back (v); up.push_back (127);
    upDown = up; for (int v = 127; v >= 0; v -= 4) upDown.push_back (v); for (int v = 0; v <= 127; v += 5) upDown.push_back (v); upDown.push_back (127);
    for (int m = 0; m < 3; ++m)
    {
        std::printf ("mode %d\n", m);
        for (float st : { 0.2f, 0.9f })
        {
            for (int k = 0; k < 2; ++k) { p.hwPos[107] = (k == 0 ? 1.f : 0.f); const float r = sweep (p, m, st, upDown, false); if (m != 1) chk (r > 0.97f, "va a fond apres montee-descente-montee"); }
        }
    }
    // CHORD LEARN : les notes sonnent, accord = notes tenues ensemble, auto-fin
    {
        auto setB = [&] (const char* id, float v) { p.apvts.getParameter (id)->setValueNotifyingHost (v); };
        setB ("s_chordLearn", 1.f); run (p, {});
        auto live = [&] { int n = 0; for (auto& v : p.synth.voices) if (v.active && v.gated) ++n; return n; };
        { juce::MidiBuffer mb; mb.addEvent (juce::MidiMessage::noteOn (1, 64, 0.8f), 0); mb.addEvent (juce::MidiMessage::noteOn (1, 60, 0.8f), 1); mb.addEvent (juce::MidiMessage::noteOn (1, 67, 0.8f), 2); run (p, mb); }
        chk (live() == 3, "learn : les 3 notes sonnent");
        { juce::MidiBuffer mb; mb.addEvent (juce::MidiMessage::noteOff (1, 64), 0); mb.addEvent (juce::MidiMessage::noteOff (1, 60), 1); mb.addEvent (juce::MidiMessage::noteOff (1, 67), 2); run (p, mb); }
        chk (p.chordN == 2 && p.chordIv[0] == 4 && p.chordIv[1] == 7, "accord appris : fondamentale la plus basse, +4 +7");
        chk (p.apvts.getRawParameterValue ("s_chordLearn")->load() < 0.5f, "learn termine tout seul");
    }
    std::printf (fail ? "ECHEC\n" : "TOUT OK\n"); return fail;
}
