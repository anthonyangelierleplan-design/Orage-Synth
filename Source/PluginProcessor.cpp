#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SynthEditor.h"

namespace
{
enum Kind { kFloat, kBool, kChoice };
struct PDef { const char* id; const char* name; float mn, mx, def; Kind kind; const char* choices; };

const char* const kSyncChoices = "Free|4 bar|2 bar|1 bar|1/2|1/4|1/8|1/16|1/32|1/2T|1/4T|1/8T|1/16T|1/4.|1/8.|1/16.";

// l'ordre ici = l'ordre de lecture dans readParams()
const PDef kDefs[] = {
    { "on",        "Footswitch",       0,   1,  1,    kBool,   nullptr },
    { "output",    "Output",         -24,  12, -6,    kFloat,  nullptr },
    { "mix",       "Dry/Wet",          0,   1,  1,    kFloat,  nullptr },
    { "chaos",     "Chaos",            0,   1,  0,    kFloat,  nullptr },
    { "octMode",   "Octaver",          0,   3,  0,    kChoice, "Off|Down|Up|Both" },
    { "octAmt",    "Octaver Dose",     0,   1,  0.5f, kFloat,  nullptr },
    { "power",     "Puissance",        0,   1,  0.65f,kFloat,  nullptr },
    { "clip",      "Ecretage",         0,   1,  0.35f,kFloat,  nullptr },
    { "unc",       "Incertitude",      0,   1,  0.35f,kFloat,  nullptr },
    { "material",  "Materiel",         0,   2,  1,    kChoice, "Germanium|Lampe|Xenon" },
    { "lightning", "Foudre",           0,   1,  0,    kFloat,  nullptr },
    { "low",       "Low",            -12,  12,  0,    kFloat,  nullptr },
    { "mid",       "Mid",            -12,  12,  0,    kFloat,  nullptr },
    { "high",      "High",           -12,  12,  0,    kFloat,  nullptr },
    { "hpf",       "HPF Cutoff",       0,   1,  0,    kFloat,  nullptr },
    { "hpfPeak",   "HPF Peak",         0,   1,  0.15f,kFloat,  nullptr },
    { "lpf",       "LPF Cutoff",       0,   1,  1,    kFloat,  nullptr },
    { "lpfPeak",   "LPF Peak",         0,   1,  0.15f,kFloat,  nullptr },
    { "destroy",   "Destroy Amount",   0,   1,  0,    kFloat,  nullptr },
    { "dRate",     "Destroy Rate",     0,   1,  0.4f, kFloat,  nullptr },
    { "dGrain",    "Destroy Grain",    0,   1,  0.4f, kFloat,  nullptr },
    { "dSync",     "Destroy Sync",     0,  15,  0,    kChoice, kSyncChoices },
    { "dStutter",  "Stutter",          0,   1,  1,    kBool,   nullptr },
    { "dReverse",  "Reverse",          0,   1,  1,    kBool,   nullptr },
    { "dTape",     "Tape Stop",        0,   1,  1,    kBool,   nullptr },
    { "dShred",    "Shred",            0,   1,  1,    kBool,   nullptr },
    { "dDrop",     "Dropout",          0,   1,  1,    kBool,   nullptr },
    { "dBuzz",     "Buzz",             0,   1,  1,    kBool,   nullptr },
    { "compType",  "Comp Type",        0,   2,  1,    kChoice, "Off|EAR 660|Fairchild" },
    { "compAmt",   "Comp Amount",      0,   1,  0.35f,kFloat,  nullptr },
    { "compThresh","Comp Threshold",   0,   1,  0.35f,kFloat,  nullptr },
    { "compAtt",   "Comp Attack",      0,   1,  0.2f, kFloat,  nullptr },
    { "gateThresh","Gate Threshold",   0,   1,  0,    kFloat,  nullptr },
    { "gateRel",   "Gate Release",     0,   1,  0.4f, kFloat,  nullptr },
    { "gateRange", "Gate Range",       0,   1,  0.8f, kFloat,  nullptr },
    { "springMix", "Spring Mix",       0,   1,  0.15f,kFloat,  nullptr },
    { "springDecay","Spring Decay",    0,   1,  0.5f, kFloat,  nullptr },
    { "springDrip","Spring Drip",      0,   1,  0.5f, kFloat,  nullptr },
    { "lfo1Rate",  "LFO1 Rate",        0,   1,  0.35f,kFloat,  nullptr },
    { "lfo1Shape", "LFO1 Shape",       0,   1,  0,    kFloat,  nullptr },
    { "lfo1Level", "LFO1 Level",       0,   1,  0.5f, kFloat,  nullptr },
    { "lfo1Sync",  "LFO1 Sync",        0,  15,  0,    kChoice, kSyncChoices },
    { "lfo2Rate",  "LFO2 Rate",        0,   1,  0.55f,kFloat,  nullptr },
    { "lfo2Shape", "LFO2 Shape",       0,   1,  0.34f,kFloat,  nullptr },
    { "lfo2Level", "LFO2 Level",       0,   1,  0.5f, kFloat,  nullptr },
    { "lfo2Sync",  "LFO2 Sync",        0,  15,  0,    kChoice, kSyncChoices },
    { "shRate",    "S&H Rate",         0,   1,  0.45f,kFloat,  nullptr },
    { "shSlew",    "S&H Slew",         0,   1,  0.15f,kFloat,  nullptr },
    { "shLevel",   "S&H Level",        0,   1,  0.5f, kFloat,  nullptr },
    { "shSync",    "S&H Sync",         0,  15,  0,    kChoice, kSyncChoices },
    { "envRel",    "Env Release",      0,   1,  0.4f, kFloat,  nullptr },
    { "envLevel",  "Env Level",        0,   1,  0.6f, kFloat,  nullptr },
};
constexpr int kNumDefs = (int) (sizeof (kDefs) / sizeof (kDefs[0]));

// ---- presets d'usine
using namespace orage;
#if ORAGE_SYNTH

// ---------------------------------------------------------------- parametres du synthe (version instrument)
struct SynDef { juce::String id, name; float mn, mx, def; Kind kind; juce::String choices; };
const std::vector<SynDef>& synDefs()
{
    static const std::vector<SynDef> defs = []
    {
        std::vector<SynDef> v;
        auto F = [&v] (const juce::String& id, const juce::String& nm, float mn, float mx, float d) { v.push_back ({ id, nm, mn, mx, d, kFloat, {} }); };
        auto B = [&v] (const juce::String& id, const juce::String& nm, bool d) { v.push_back ({ id, nm, 0, 1, d ? 1.f : 0.f, kBool, {} }); };
        auto C = [&v] (const juce::String& id, const juce::String& nm, const juce::String& ch, int d) { v.push_back ({ id, nm, 0, 0, (float) d, kChoice, ch }); };
        const juce::String waves ("Saw|Pulse|Triangle"), octs ("-2|-1|0|+1|+2");
        C ("s_wave1", "VCO1 Wave", waves, 0);  C ("s_oct1", "VCO1 Octave", octs, 2);
        C ("s_wave2", "VCO2 Wave", waves, 0);  C ("s_oct2", "VCO2 Octave", octs, 2);
        F ("s_tune2", "VCO2 Tune", -12, 12, 0);  F ("s_fine2", "VCO2 Fine", -100, 100, 0);
        F ("s_pw1", "VCO1 PW", 0, 1, 0.5f);  F ("s_pw2", "VCO2 PW", 0, 1, 0.5f);
        B ("s_sync", "Hard Sync", false);  F ("s_fm", "FM", 0, 1, 0);  F ("s_ring", "Ring Mod", 0, 1, 0);
        F ("s_lvl1", "VCO1 Level", 0, 1, 0.8f);  F ("s_lvl2", "VCO2 Level", 0, 1, 0);  F ("s_noise", "Noise", 0, 1, 0);
        F ("s_glide", "Glide", 0, 1, 0);
        F ("s_hpf", "Syn HPF", 0, 1, 0);  F ("s_hpfRes", "Syn HPF Peak", 0, 1, 0);
        F ("s_lpf", "Syn LPF", 0, 1, 0.7f);  F ("s_lpfRes", "Syn LPF Peak", 0, 1, 0.25f);
        F ("s_fltEnv", "Filter Env Amt", 0, 1, 0.3f);  F ("s_key", "Key Track", 0, 1, 0.5f);  F ("s_drive", "Drive", 0, 1, 0.2f);
        F ("s_aA", "Amp A", 0, 1, 0.1f);  F ("s_aD", "Amp D", 0, 1, 0.4f);  F ("s_aS", "Amp S", 0, 1, 0.8f);  F ("s_aR", "Amp R", 0, 1, 0.35f);
        F ("s_fA", "Flt A", 0, 1, 0.1f);  F ("s_fD", "Flt D", 0, 1, 0.45f);  F ("s_fS", "Flt S", 0, 1, 0.2f);  F ("s_fR", "Flt R", 0, 1, 0.35f);
        F ("s_master", "Synth Master", 0, 1, 0.7f);
        juce::StringArray divs; for (auto* d : kLfoDivNames) divs.add (d);
        for (int i = 1; i <= kLfos; ++i)
        {
            const juce::String p ("s_l" + juce::String (i));
            F (p + "Rate", "LFO" + juce::String (i) + " Rate", 0, 1, 0.4f);  C (p + "Wave", "LFO" + juce::String (i) + " Wave", "Sine|Triangle|Saw|Square|S&H", 0);
            F (p + "Slew", "LFO" + juce::String (i) + " Slew", 0, 1, 0);  F (p + "Fade", "LFO" + juce::String (i) + " Fade", 0, 1, 0);
            C (p + "Sync", "LFO" + juce::String (i) + " Sync", divs.joinIntoString ("|"), 0);  B (p + "Trig", "LFO" + juce::String (i) + " Retrig", false);
        }
        for (int i = 1; i <= kMacros; ++i) F ("s_m" + juce::String (i), "Macro " + juce::String (i), 0, 1, 0);
        juce::StringArray srcs, dsts; for (auto* s : kModSrcNames) srcs.add (s); for (auto* s : kModDstNames) dsts.add (s);
        for (int i = 1; i <= kSlots; ++i)
        {
            const juce::String p ("s_slot" + juce::String (i)), n ("Mod " + juce::String (i));
            C (p + "Src", n + " Source", srcs.joinIntoString ("|"), 0);  C (p + "Dst", n + " Dest", dsts.joinIntoString ("|"), 0);
            F (p + "Amt", n + " Amount", -1, 1, 0);
        }
        return v;
    }();
    return defs;
}
void addSynthParams (juce::AudioProcessorValueTreeState::ParameterLayout& l)
{
    for (const auto& d : synDefs())
    {
        const juce::ParameterID pid { d.id, 1 };
        if (d.kind == kBool)        l.add (std::make_unique<juce::AudioParameterBool> (pid, d.name, d.def > 0.5f));
        else if (d.kind == kChoice) l.add (std::make_unique<juce::AudioParameterChoice> (pid, d.name, juce::StringArray::fromTokens (d.choices, "|", ""), (int) d.def));
        else                        l.add (std::make_unique<juce::AudioParameterFloat> (pid, d.name, juce::NormalisableRange<float> (d.mn, d.mx, 0.001f), d.def));
    }
}
#endif

struct Factory { const char* name; void (*apply) (OrageAudioProcessor&); };

void fInit (OrageAudioProcessor&) {}

void fWaters (OrageAudioProcessor& p)
{
    p.setParamReal ("octMode", 1); p.setParamReal ("octAmt", 0.45f);
    p.setParamReal ("power", 0.9f); p.setParamReal ("material", 2); p.setParamReal ("clip", 0.55f);
    p.setParamReal ("unc", 0.5f); p.setParamReal ("lightning", 0.25f);
    p.setParamReal ("compType", 1); p.setParamReal ("compAmt", 0.5f); p.setParamReal ("compThresh", 0.4f);
    p.setParamReal ("lpf", 0.8f); p.setParamReal ("lpfPeak", 0.3f); p.setParamReal ("springMix", 0.f);
}
void fWah (OrageAudioProcessor& p)
{
    p.setParamReal ("power", 0.7f); p.setParamReal ("material", 1);
    p.setParamReal ("lpf", 0.25f); p.setParamReal ("lpfPeak", 0.85f);
    p.setParamReal ("envLevel", 1.f); p.setParamReal ("envRel", 0.35f);
    p.engine.setCvCable (SrcEnv, CvLpf, true);
}
void fSh (OrageAudioProcessor& p)
{
    p.setParamReal ("power", 0.6f); p.setParamReal ("lpf", 0.45f); p.setParamReal ("lpfPeak", 0.6f);
    p.setParamReal ("shSync", 6); p.setParamReal ("shLevel", 0.8f); p.setParamReal ("shSlew", 0.2f);
    p.engine.setCvCable (SrcSh, CvDrive, true); p.engine.setCvCable (SrcSh, CvLpf, true);
}
void fGlitch (OrageAudioProcessor& p)
{
    p.setParamReal ("power", 0.5f); p.setParamReal ("destroy", 0.7f); p.setParamReal ("dSync", 7); p.setParamReal ("dGrain", 0.5f);
    p.setParamReal ("compType", 2); p.setParamReal ("compAmt", 0.5f);
}
void fSpring (OrageAudioProcessor& p)
{
    p.setParamReal ("power", 0.5f); p.setParamReal ("springMix", 0.5f); p.setParamReal ("springDecay", 0.8f);
    p.engine.setAudioCable (NIn, NDist, true); p.engine.setAudioCable (NSpring, NDist, true);   // boucle de retour ressort -> disto
}
void fGated (OrageAudioProcessor& p)
{
    p.setParamReal ("power", 0.95f); p.setParamReal ("material", 2); p.setParamReal ("clip", 0.6f);
    p.setParamReal ("gateThresh", 0.45f); p.setParamReal ("gateRel", 0.2f); p.setParamReal ("gateRange", 1.f);
    p.setParamReal ("compType", 1); p.setParamReal ("compAmt", 0.7f); p.setParamReal ("compThresh", 0.55f); p.setParamReal ("compAtt", 0.3f);
}
#if ORAGE_SYNTH

void sLead (OrageAudioProcessor& p)
{
    p.setParamReal ("s_wave1", 0); p.setParamReal ("s_wave2", 0); p.setParamReal ("s_sync", 1); p.setParamReal ("s_tune2", 7);
    p.setParamReal ("s_lvl1", 0.7f); p.setParamReal ("s_lvl2", 0.5f); p.setParamReal ("s_lpf", 0.6f); p.setParamReal ("s_lpfRes", 0.55f);
    p.setParamReal ("s_slot1Src", MsLfo1); p.setParamReal ("s_slot1Dst", MdPitch2); p.setParamReal ("s_slot1Amt", 0.35f);
    p.setParamReal ("s_l1Rate", 0.35f); p.setParamReal ("s_l1Wave", 0);
    p.setParamReal ("power", 0.45f); p.setParamReal ("compType", 1);
}
void sBass (OrageAudioProcessor& p)
{
    p.setParamReal ("s_wave1", 1); p.setParamReal ("s_wave2", 0); p.setParamReal ("s_oct1", 1); p.setParamReal ("s_oct2", 1);
    p.setParamReal ("s_lvl2", 0.6f); p.setParamReal ("s_tune2", 0.15f); p.setParamReal ("s_lpf", 0.4f); p.setParamReal ("s_fltEnv", 0.7f);
    p.setParamReal ("s_lpfRes", 0.4f); p.setParamReal ("s_fD", 0.3f); p.setParamReal ("s_fS", 0.05f); p.setParamReal ("s_aS", 0.9f);
    p.setParamReal ("s_slot1Src", MsMac1); p.setParamReal ("s_slot1Dst", MdLpf); p.setParamReal ("s_slot1Amt", 0.8f);
    p.setParamReal ("power", 0.9f); p.setParamReal ("material", 2); p.setParamReal ("clip", 0.55f); p.setParamReal ("unc", 0.45f);
    p.setParamReal ("lightning", 0.3f); p.setParamReal ("compType", 1); p.setParamReal ("compAmt", 0.5f);
}
void sFm (OrageAudioProcessor& p)
{
    p.setParamReal ("s_wave1", 2); p.setParamReal ("s_wave2", 2); p.setParamReal ("s_fm", 0.55f); p.setParamReal ("s_tune2", 12);
    p.setParamReal ("s_lvl1", 0.8f); p.setParamReal ("s_lpf", 0.8f); p.setParamReal ("s_lpfRes", 0.1f);
    p.setParamReal ("s_slot1Src", MsEnv); p.setParamReal ("s_slot1Dst", MdFm); p.setParamReal ("s_slot1Amt", 0.6f);
    p.setParamReal ("s_aD", 0.35f); p.setParamReal ("s_aS", 0.4f);
}
void sRing (OrageAudioProcessor& p)
{
    p.setParamReal ("s_wave1", 0); p.setParamReal ("s_wave2", 1); p.setParamReal ("s_ring", 0.8f); p.setParamReal ("s_tune2", 5.5f);
    p.setParamReal ("s_lvl1", 0.3f); p.setParamReal ("s_lpf", 0.7f);
    p.setParamReal ("s_slot1Src", MsLfo2); p.setParamReal ("s_slot1Dst", MdPw2); p.setParamReal ("s_slot1Amt", 0.8f);
    p.setParamReal ("s_l2Rate", 0.3f); p.setParamReal ("s_l2Wave", 1);
    p.setParamReal ("destroy", 0.4f); p.setParamReal ("dSync", 7);
}
#endif
const Factory kFactory[] = {
    { "Init", fInit },
#if ORAGE_SYNTH
    { "Hard Sync Lead", sLead }, { "Justice Bass Synth", sBass }, { "FM Bell Pluck", sFm }, { "Ring Wobble", sRing },
#endif
    { "Waters Of Nazareth (depart)", fWaters },
    { "Gated Fuzz Wall", fGated },
    { "Envelope Wah Scream", fWah },
    { "Sample & Hold 1/8", fSh },
    { "Glitch Machine 1/16", fGlitch },
    { "Spring Feedback", fSpring },
};
constexpr int kNumFactory = (int) (sizeof (kFactory) / sizeof (kFactory[0]));
}

juce::AudioProcessorValueTreeState::ParameterLayout OrageAudioProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    for (const auto& d : kDefs)
    {
        const juce::ParameterID pid { d.id, 1 };
        if (d.kind == kBool)
            l.add (std::make_unique<juce::AudioParameterBool> (pid, d.name, d.def > 0.5f));
        else if (d.kind == kChoice)
            l.add (std::make_unique<juce::AudioParameterChoice> (pid, d.name, juce::StringArray::fromTokens (d.choices, "|", ""), (int) d.def));
        else
            l.add (std::make_unique<juce::AudioParameterFloat> (pid, d.name, juce::NormalisableRange<float> (d.mn, d.mx, 0.001f), d.def));
    }
#if ORAGE_SYNTH
    addSynthParams (l);
#endif
    return l;
}

OrageAudioProcessor::OrageAudioProcessor()
    : AudioProcessor (BusesProperties()
#if ! ORAGE_SYNTH
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, ORAGE_SYNTH ? "ORAGESYNTH" : "ORAGEFX", createLayout())
{
    for (const auto& d : kDefs) raw.push_back (apvts.getRawParameterValue (d.id));
#if ORAGE_SYNTH
    for (const auto& d : synDefs()) rawSyn.push_back (apvts.getRawParameterValue (d.id));
#endif
}

bool OrageAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto out = l.getMainOutputChannelSet();
#if ORAGE_SYNTH
    return out == juce::AudioChannelSet::stereo();
#endif
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo())
           && out == l.getMainInputChannelSet();
}

void OrageAudioProcessor::prepareToPlay (double sampleRate, int)
{
    setLatencySamples (0);
    engine.prepare ((float) sampleRate);
#if ORAGE_SYNTH
    synth.prepare ((float) sampleRate);
#endif
}

orage::Params OrageAudioProcessor::readParams() const
{
    int i = 0;
    auto f = [this, &i] { return raw[(size_t) i++]->load(); };
    auto b = [&f] { return f() > 0.5f; };
    auto n = [&f] { return juce::roundToInt (f()); };

    orage::Params p;
    p.on = b();  p.output = f();  p.mix = f();  p.chaos = f();
    p.octMode = n();  p.octAmt = f();
    p.power = f();  p.clip = f();  p.unc = f();  p.material = n();  p.lightning = f();
    p.low = f();  p.mid = f();  p.high = f();
    p.hpf = f();  p.hpfPeak = f();  p.lpf = f();  p.lpfPeak = f();
    p.destroy = f();  p.dRate = f();  p.dGrain = f();  p.dSync = n();
    p.dStutter = b();  p.dReverse = b();  p.dTape = b();  p.dShred = b();  p.dDrop = b();  p.dBuzz = b();
    p.compType = n();  p.compAmt = f();  p.compThresh = f();  p.compAtt = f();
    p.gateThresh = f();  p.gateRel = f();  p.gateRange = f();
    p.springMix = f();  p.springDecay = f();  p.springDrip = f();
    p.lfo1Rate = f();  p.lfo1Shape = f();  p.lfo1Level = f();  p.lfo1Sync = n();
    p.lfo2Rate = f();  p.lfo2Shape = f();  p.lfo2Level = f();  p.lfo2Sync = n();
    p.shRate = f();  p.shSlew = f();  p.shLevel = f();  p.shSync = n();
    p.envRel = f();  p.envLevel = f();
    jassert (i == kNumDefs);
    return p;
}


#if ORAGE_SYNTH
orage::SynthParams OrageAudioProcessor::readSynth() const
{
    size_t i = 0;
    auto f = [this, &i] { return rawSyn[i++]->load(); };
    auto b = [&f] { return f() > 0.5f; };
    auto n = [&f] { return juce::roundToInt (f()); };
    auto sec = [&f] { return 0.001f * std::pow (5000.f, f()); };      // 1 ms .. 5 s
    SynthParams s;
    s.wave1 = n(); s.oct1 = n() - 2; s.wave2 = n(); s.oct2 = n() - 2;
    s.tune2 = f(); s.fine2 = f(); s.pw1 = f(); s.pw2 = f();
    s.sync = b() ? 1 : 0; s.fm = f(); s.ring = f();
    s.lvl1 = f(); s.lvl2 = f(); s.noise = f(); s.glide = f();
    s.hpf = f(); s.hpfRes = f(); s.lpf = f(); s.lpfRes = f(); s.fltEnv = f(); s.keytrack = f(); s.drive = f();
    s.aA = sec(); s.aD = sec(); s.aS = f(); s.aR = sec();
    s.fA = sec(); s.fD = sec(); s.fS = f(); s.fR = sec();
    s.master = f();
    for (auto& l : s.lfo) { l.rate = f(); l.wave = n(); l.slew = f(); l.fade = f(); l.sync = n(); l.trig = b() ? 1 : 0; }
    for (auto& m : s.macro) m = f();
    for (auto& sl : s.slot) { sl.src = n(); sl.dst = n(); sl.amt = f(); }
    jassert (i == rawSyn.size());
    return s;
}
#endif

void OrageAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int nc = juce::jmin (2, buffer.getNumChannels());
    const int total = buffer.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear (c, 0, total);
    if (nc == 0 || total == 0) return;

    // tempo et position d'Ableton (synchro des LFO, du S&H et de la destroy machine)
    double bpm = 120.0, ppq = 0.0;
    bool playing = false, hasPpq = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto bp = pos->getBpm()) bpm = *bp;
            if (auto q = pos->getPpqPosition()) { ppq = *q; hasPpq = true; }
            playing = pos->getIsPlaying();
        }
    engine.setHost (bpm, ppq, playing, hasPpq);

#if ORAGE_SYNTH
    {
        buffer.clear();
        keyboardState.processNextMidiBuffer (midi, 0, total, true);     // clavier a l'ecran -> notes MIDI
        synth.setHost (bpm);
        synth.setParams (readSynth());
        float* sl = buffer.getWritePointer (0);
        float* sr = nc > 1 ? buffer.getWritePointer (1) : buffer.getWritePointer (0);
        int pos = 0;
        auto render = [&] (int upTo)
        {
            if (upTo > pos) { synth.process (sl + pos, sr + pos, upTo - pos); pos = upTo; }
        };
        for (const auto meta : midi)
        {
            render (juce::jlimit (0, total, meta.samplePosition));
            const auto m = meta.getMessage();
            if (m.isNoteOn())                  synth.noteOn (m.getNoteNumber(), m.getFloatVelocity());
            else if (m.isNoteOff())            synth.noteOff (m.getNoteNumber());
            else if (m.isAllNotesOff() || m.isAllSoundOff()) synth.allNotesOff();
            else if (m.isPitchWheel())         synth.setBend ((float) (m.getPitchWheelValue() - 8192) / 8192.f * 2.f);
            else if (m.isController() && m.getControllerNumber() == 1) synth.setWheel ((float) m.getControllerValue() / 127.f);
        }
        render (total);
    }
#endif
    engine.setParams (readParams());
    float* io[2] = { buffer.getWritePointer (0), nc > 1 ? buffer.getWritePointer (1) : nullptr };
    engine.process (io, total, nc);
}

juce::AudioProcessorEditor* OrageAudioProcessor::createEditor()
{
    return new SynthEditor (*this);
}

// ---------------------------------------------------------------- etat (projet Ableton) et presets
juce::ValueTree OrageAudioProcessor::captureState()
{
    auto state = apvts.copyState();
    state.setProperty ("patch", juce::String (engine.serializePatch()), nullptr);
    state.setProperty ("rack", juce::String (engine.serializeRack()), nullptr);
    state.setProperty ("presetName", presetName, nullptr);
    return state;
}

void OrageAudioProcessor::applyState (juce::ValueTree state)
{
    engine.loadPatch (state.getProperty ("patch", "").toString().toStdString());
    const auto rk = state.getProperty ("rack", "").toString();
    if (rk.isNotEmpty()) engine.loadRack (rk.toStdString()); else engine.setDefaultRack();
    presetName = state.getProperty ("presetName", "Init").toString();
    apvts.replaceState (state);
}

void OrageAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = captureState().createXml()) copyXmlToBinary (*xml, dest);
}

void OrageAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            applyState (juce::ValueTree::fromXml (*xml));
}

void OrageAudioProcessor::setParamReal (const char* id, float real)
{
    if (auto* prm = apvts.getParameter (id)) prm->setValueNotifyingHost (prm->convertTo0to1 (real));
}

void OrageAudioProcessor::resetModules()
{
    for (const auto& d : kDefs)
        if (juce::String (d.id) != "on")
            if (auto* prm = apvts.getParameter (d.id)) prm->setValueNotifyingHost (prm->getDefaultValue());
#if ORAGE_SYNTH
    for (const auto& d : synDefs())
        if (auto* prm = apvts.getParameter (d.id)) prm->setValueNotifyingHost (prm->getDefaultValue());
#endif
}
void OrageAudioProcessor::clearRack()     { engine.setRack ({}); }
void OrageAudioProcessor::defaultRack()   { engine.setDefaultRack(); }
void OrageAudioProcessor::clearPatchbay() { engine.clearPatch(); }

int OrageAudioProcessor::getNumFactoryPresets() { return kNumFactory; }
juce::String OrageAudioProcessor::getFactoryPresetName (int i) { return kFactory[juce::jlimit (0, kNumFactory - 1, i)].name; }

void OrageAudioProcessor::loadFactoryPreset (int i)
{
    i = juce::jlimit (0, kNumFactory - 1, i);
    resetModules();
    engine.clearPatch();
    if (i == 0) engine.setRack ({}); else engine.setDefaultRack();      // Init = rack vide ; les autres presets utilisent tous les modules
    kFactory[i].apply (*this);
    presetName = kFactory[i].name;
}

juce::File OrageAudioProcessor::getPresetDir() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("Foudre Audio").getChildFile (ORAGE_SYNTH ? "Orage" : "Orage FX").getChildFile ("Presets");
}

juce::StringArray OrageAudioProcessor::getUserPresetNames() const
{
    juce::StringArray names;
    for (const auto& f : getPresetDir().findChildFiles (juce::File::findFiles, false, "*.orfx"))
        names.add (f.getFileNameWithoutExtension());
    names.sort (true);
    return names;
}

bool OrageAudioProcessor::saveUserPreset (const juce::String& name)
{
    const auto clean = name.trim();
    if (clean.isEmpty()) return false;
    const auto dir = getPresetDir();
    if (! dir.createDirectory().wasOk()) return false;
    presetName = clean;
    if (auto xml = captureState().createXml())
        return xml->writeTo (dir.getChildFile (juce::File::createLegalFileName (clean) + ".orfx"));
    return false;
}

bool OrageAudioProcessor::loadUserPreset (const juce::String& name)
{
    const auto file = getPresetDir().getChildFile (juce::File::createLegalFileName (name) + ".orfx");
    if (auto xml = juce::parseXML (file))
        if (xml->hasTagName (apvts.state.getType()))
        {
            applyState (juce::ValueTree::fromXml (*xml));
            presetName = name;
            return true;
        }
    return false;
}

bool OrageAudioProcessor::deleteUserPreset (const juce::String& name)
{
    return getPresetDir().getChildFile (juce::File::createLegalFileName (name) + ".orfx").deleteFile();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new OrageAudioProcessor(); }
