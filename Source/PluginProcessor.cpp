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
    { "octAmt",    "Octaver Dose",     0,   1,  0, kFloat,  nullptr },
    { "power",     "Puissance",        0,   1,  0,kFloat,  nullptr },
    { "clip",      "Ecretage",         0,   1,  0,kFloat,  nullptr },
    { "unc",       "Incertitude",      0,   1,  0,kFloat,  nullptr },
    { "material",  "Materiel",         0,   2,  1,    kChoice, "Germanium|Lampe|Xenon" },
    { "lightning", "Foudre",           0,   1,  0,    kFloat,  nullptr },
    { "low",       "Low",            -12,  12,  0,    kFloat,  nullptr },
    { "mid",       "Mid",            -12,  12,  0,    kFloat,  nullptr },
    { "high",      "High",           -12,  12,  0,    kFloat,  nullptr },
    { "hpf",       "HPF Cutoff",       0,   1,  0,    kFloat,  nullptr },
    { "hpfPeak",   "HPF Peak",         0,   1,  0,kFloat,  nullptr },
    { "lpf",       "LPF Cutoff",       0,   1,  1,    kFloat,  nullptr },
    { "lpfPeak",   "LPF Peak",         0,   1,  0,kFloat,  nullptr },
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
    { "compType",  "Comp Type",        0,   2,  0,    kChoice, "Off|EAR 660|Fairchild" },
    { "compAmt",   "Comp Amount",      0,   1,  0,kFloat,  nullptr },
    { "compThresh","Comp Threshold",   0,   1,  0.35f,kFloat,  nullptr },
    { "compAtt",   "Comp Attack",      0,   1,  0.2f, kFloat,  nullptr },
    { "gateThresh","Gate Threshold",   0,   1,  0,    kFloat,  nullptr },
    { "gateRel",   "Gate Release",     0,   1,  0.4f, kFloat,  nullptr },
    { "gateRange", "Gate Range",       0,   1,  0.8f, kFloat,  nullptr },
    { "springMix", "Spring Mix",       0,   1,  0,kFloat,  nullptr },
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
    { "bbdTime",   "BBD Time",         0,   1,  0.4f, kFloat,  nullptr },
    { "bbdFb",     "BBD Feedback",     0,   1,  0, kFloat,  nullptr },
    { "bbdTone",   "BBD Tone",         0,   1,  0.5f, kFloat,  nullptr },
    { "bbdMod",    "BBD Mod",          0,   1,  0.3f, kFloat,  nullptr },
    { "bbdMix",    "BBD Mix",          0,   1,  0.f,  kFloat,  nullptr },
    { "bbdPing",   "BBD Ping-Pong",    0,   1,  1,    kBool,   nullptr },
    { "bbdSync",   "BBD Sync",         0,  15,  0,    kChoice, kSyncChoices },
    { "revMix",    "Reverb Mix",       0,   1,  0,    kFloat,  nullptr },
    { "revSize",   "Reverb Size",      0,   1,  0.5f, kFloat,  nullptr },
    { "revDecay",  "Reverb Decay",     0,   1,  0.6f, kFloat,  nullptr },
    { "revPre",    "Reverb Pre-Delay", 0,   1,  0.1f, kFloat,  nullptr },
    { "revDamp",   "Reverb Damping",   0,   1,  0.4f, kFloat,  nullptr },
    { "revMod",    "Reverb Mod",       0,   1,  0.3f, kFloat,  nullptr },
    { "revWidth",  "Reverb Width",     0,   1,  1,    kFloat,  nullptr },
    { "revShimmer","Reverb Shimmer",   0,   1,  0,    kFloat,  nullptr },
    { "revLowCut", "Reverb Low Cut",   0,   1,  0.1f, kFloat,  nullptr },
    { "revFreeze", "Reverb Freeze",    0,   1,  0,    kBool,   nullptr },
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
        for (int i = 1; i <= 4; ++i) F ("s_m" + juce::String (i), "Macro " + juce::String (i), 0, 1, 0);
        juce::StringArray srcs, dsts; for (auto* s : kModSrcNames) srcs.add (s); for (auto* s : kModDstNames) dsts.add (s);
        for (int i = 1; i <= kSlots; ++i)
        {
            const juce::String p ("s_slot" + juce::String (i)), n ("Mod " + juce::String (i));
            C (p + "Src", n + " Source", srcs.joinIntoString ("|"), 0);  C (p + "Dst", n + " Dest", dsts.joinIntoString ("|"), 0);
            F (p + "Amt", n + " Amount", -1, 1, 0);
        }
        F ("s_fold", "Wavefolder", 0, 1, 0);  F ("s_foldSym", "Fold Symmetry", -1, 1, 0);
        C ("s_arpOn", "Arp", "Off|On|Latch", 0);
        C ("s_arpMode", "Arp Direction", "Up|Down|Up-Down", 0);
        C ("s_arpOct", "Arp Range", "1 Oct|2 Oct|Full", 0);
        C ("s_ftype", "Filter Type", "MS-20|SEM|Ladder", 0);  F ("s_semMorph", "SEM Morph", 0, 1, 0);
        C ("s_assign", "Key Assign", "Poly|Mono|Unison|Share|Chord", 0);  B ("s_hold", "Hold", false);
        F ("s_uniDetune", "Unison Detune", 0, 1, 0.3f);  B ("s_chordLearn", "Chord Learn", false);
        for (int i = 5; i <= kMacros; ++i) F ("s_m" + juce::String (i), "Macro " + juce::String (i), 0, 1, 0);
        F ("s_arpRate", "Arp Rate", 0, 1, 0.55f);  C ("s_arpSync", "Arp Sync", divs.joinIntoString ("|"), 0);
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

struct Factory { const char* name; const char* category; void (*apply) (OrageAudioProcessor&); };
const char* const kCategories[] = { "Pad", "Bass", "Lead", "Pluck / Keys", "Arp / Seq", "FX / Texture", "Rack FX" };

void fInit (OrageAudioProcessor&) {}

// anciens reglages par defaut des modules FX : les presets d'usine ont ete ecrits avec ces valeurs
void legacyFxDefaults (OrageAudioProcessor& p)
{
    p.setParamReal ("octAmt", 0.5f); p.setParamReal ("power", 0.65f); p.setParamReal ("clip", 0.35f);
    p.setParamReal ("hpfPeak", 0.15f); p.setParamReal ("lpfPeak", 0.15f); p.setParamReal ("compType", 1); p.setParamReal ("compAmt", 0.35f);
    p.setParamReal ("springMix", 0.15f); p.setParamReal ("bbdFb", 0.4f);
}

void fWaters (OrageAudioProcessor& p)
{
    p.setParamReal ("octMode", 1); p.setParamReal ("octAmt", 0.45f);
    p.setParamReal ("power", 0.9f); p.setParamReal ("material", 2); p.setParamReal ("clip", 0.55f);
    p.setParamReal ("unc", 0.2f); p.setParamReal ("lightning", 0.25f);
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
    p.setParamReal ("power", 0.9f); p.setParamReal ("material", 2); p.setParamReal ("clip", 0.55f); p.setParamReal ("unc", 0.2f);
    p.setParamReal ("lightning", 0.3f); p.setParamReal ("compType", 1); p.setParamReal ("compAmt", 0.5f);
}
void sFm (OrageAudioProcessor& p)
{
    p.setParamReal ("s_wave1", 2); p.setParamReal ("s_wave2", 2); p.setParamReal ("s_fm", 0.55f); p.setParamReal ("s_tune2", 12);
    p.setParamReal ("s_lvl1", 0.8f); p.setParamReal ("s_lpf", 0.8f); p.setParamReal ("s_lpfRes", 0.1f);
    p.setParamReal ("s_slot1Src", MsEnv); p.setParamReal ("s_slot1Dst", MdFm); p.setParamReal ("s_slot1Amt", 0.6f);
    p.setParamReal ("s_aD", 0.35f); p.setParamReal ("s_aS", 0.4f);
}
void sMs20Bass (OrageAudioProcessor& p)
{
    p.setParamReal ("s_wave1", 0); p.setParamReal ("s_oct1", 1); p.setParamReal ("s_wave2", 1); p.setParamReal ("s_oct2", 1); p.setParamReal ("s_lvl2", 0.5f);
    p.setParamReal ("s_hpf", 0.2f); p.setParamReal ("s_hpfRes", 0.8f);                      // le pic du HPF gonfle les basses
    p.setParamReal ("s_lpf", 0.45f); p.setParamReal ("s_lpfRes", 0.7f); p.setParamReal ("s_fltEnv", 0.55f); p.setParamReal ("s_drive", 0.45f);
    p.setParamReal ("s_fD", 0.3f); p.setParamReal ("s_fS", 0.1f); p.setParamReal ("s_aS", 0.9f);
    p.setParamReal ("power", 0.5f); p.setParamReal ("compType", 1); p.setParamReal ("compAmt", 0.45f);
}
void sFoldLead (OrageAudioProcessor& p)
{
    p.setParamReal ("s_wave1", 2); p.setParamReal ("s_wave2", 0); p.setParamReal ("s_lvl2", 0.35f); p.setParamReal ("s_tune2", 0.1f);
    p.setParamReal ("s_fold", 0.55f); p.setParamReal ("s_foldSym", 0.2f); p.setParamReal ("s_lpf", 0.75f); p.setParamReal ("s_lpfRes", 0.4f);
    p.setParamReal ("s_slot1Src", MsEnv); p.setParamReal ("s_slot1Dst", MdFold); p.setParamReal ("s_slot1Amt", 0.5f);
    p.setParamReal ("s_slot2Src", MsLfo1); p.setParamReal ("s_slot2Dst", MdFold); p.setParamReal ("s_slot2Amt", 0.25f);
    p.setParamReal ("s_l1Rate", 0.3f);
    p.setParamReal ("bbdMix", 0.3f); p.setParamReal ("bbdTime", 0.55f); p.setParamReal ("bbdFb", 0.45f);
}
void sArpBbd (OrageAudioProcessor& p)
{
    p.setParamReal ("s_wave1", 0); p.setParamReal ("s_wave2", 1); p.setParamReal ("s_lvl2", 0.4f); p.setParamReal ("s_tune2", 7);
    p.setParamReal ("s_lpf", 0.55f); p.setParamReal ("s_lpfRes", 0.5f); p.setParamReal ("s_fltEnv", 0.5f); p.setParamReal ("s_fD", 0.18f); p.setParamReal ("s_fS", 0.f);
    p.setParamReal ("s_aD", 0.25f); p.setParamReal ("s_aS", 0.2f);
    p.setParamReal ("s_arpOn", 1); p.setParamReal ("s_arpMode", 2); p.setParamReal ("s_arpOct", 1); p.setParamReal ("s_arpRate", 0.68f);
    p.setParamReal ("bbdMix", 0.45f); p.setParamReal ("bbdSync", 5); p.setParamReal ("bbdFb", 0.55f); p.setParamReal ("bbdPing", 1);
    p.setParamReal ("s_slot1Src", MsLfo1); p.setParamReal ("s_slot1Dst", MdBbdMix); p.setParamReal ("s_slot1Amt", 0.3f);
}
void sRing (OrageAudioProcessor& p)
{
    p.setParamReal ("s_wave1", 0); p.setParamReal ("s_wave2", 1); p.setParamReal ("s_ring", 0.8f); p.setParamReal ("s_tune2", 5.5f);
    p.setParamReal ("s_lvl1", 0.3f); p.setParamReal ("s_lpf", 0.7f);
    p.setParamReal ("s_slot1Src", MsLfo2); p.setParamReal ("s_slot1Dst", MdPw2); p.setParamReal ("s_slot1Amt", 0.8f);
    p.setParamReal ("s_l2Rate", 0.3f); p.setParamReal ("s_l2Wave", 1);
    p.setParamReal ("destroy", 0.4f); p.setParamReal ("dSync", 7);
}

using KV = std::pair<const char*, float>;
void S (OrageAudioProcessor& p, std::initializer_list<KV> l) { for (auto& kv : l) p.setParamReal (kv.first, kv.second); }
void M (OrageAudioProcessor& p, int slot, int src, int dst, float amt)
{
    const juce::String b ("s_slot" + juce::String (slot));
    p.setParamReal ((b + "Src").toRawUTF8(), (float) src); p.setParamReal ((b + "Dst").toRawUTF8(), (float) dst); p.setParamReal ((b + "Amt").toRawUTF8(), amt);
}
// ---- selection de macros preparee par categorie (slots 5..12 = macros 1..8). Les macros demarrent a 0 : le son du preset ne change pas tant qu'on n'y touche pas.
struct MacroDef { int dst; float amt; };
void applyMacroSet (OrageAudioProcessor& p, const juce::String& cat)
{
#if ORAGE_SYNTH
    static const MacroDef pad[8]   { { MdLpf, 0.5f }, { MdRevMix, 0.4f }, { MdRevDecay, 0.3f }, { MdFold, 0.4f }, { MdPitch2, 0.04f }, { MdNoise, 0.3f }, { MdFltEnv, 0.4f }, { MdBbdMix, 0.4f } };
    static const MacroDef bass[8]  { { MdLpf, 0.5f }, { MdReso, 0.4f }, { MdDrive, 0.5f }, { MdFltEnv, 0.5f }, { MdFold, 0.4f }, { MdPitch2, 0.04f }, { MdMix, 0.4f }, { MdFxPower, 0.4f } };
    static const MacroDef lead[8]  { { MdLpf, 0.5f }, { MdReso, 0.4f }, { MdFm, 0.4f }, { MdFold, 0.4f }, { MdRevMix, 0.4f }, { MdBbdMix, 0.4f }, { MdPw1, 0.5f }, { MdFxPower, 0.4f } };
    static const MacroDef keys[8]  { { MdLpf, 0.5f }, { MdFltEnv, 0.5f }, { MdRing, 0.4f }, { MdFm, 0.4f }, { MdRevMix, 0.4f }, { MdDrive, 0.4f }, { MdPw2, 0.5f }, { MdBbdMix, 0.4f } };
    static const MacroDef arp[8]   { { MdLpf, 0.5f }, { MdReso, 0.4f }, { MdBbdMix, 0.5f }, { MdBbdFb, 0.4f }, { MdRevMix, 0.4f }, { MdFltEnv, 0.5f }, { MdDrive, 0.4f }, { MdFm, 0.4f } };
    static const MacroDef fx[8]    { { MdLpf, 0.5f }, { MdNoise, 0.5f }, { MdFm, 0.5f }, { MdRing, 0.5f }, { MdRevMix, 0.5f }, { MdFxDestroy, 0.5f }, { MdFxUnc, 0.5f }, { MdFxChaos, 0.5f } };
    static const MacroDef rack[8]  { { MdLpf, 0.5f }, { MdRevMix, 0.4f }, { MdBbdMix, 0.4f }, { MdFxPower, 0.4f }, { MdFxUnc, 0.4f }, { MdFxDestroy, 0.4f }, { MdFxSpring, 0.4f }, { MdFxChaos, 0.4f } };
    const MacroDef* d = cat == "Pad" ? pad : cat == "Bass" ? bass : cat == "Lead" ? lead : cat == "Pluck / Keys" ? keys : cat == "Arp / Seq" ? arp : cat == "FX / Texture" ? fx : rack;
    for (int k = 0; k < 8; ++k) M (p, 5 + k, macroSrc (k), d[k].dst, d[k].amt);
#else
    juce::ignoreUnused (p, cat);
#endif
}

// ---- PADS
void pAurora (OrageAudioProcessor& p)
{
    S (p, { {"output",-8},{"s_wave1",0},{"s_wave2",0},{"s_tune2",0.14f},{"s_lvl1",0.6f},{"s_lvl2",0.5f},{"s_ftype",1},{"s_lpf",0.58f},{"s_lpfRes",0.25f},{"s_fltEnv",0.1f},
            {"s_aA",0.72f},{"s_aD",0.6f},{"s_aS",0.9f},{"s_aR",0.85f},{"s_l1Rate",0.12f},{"s_drive",0.1f},{"power",0.2f},{"clip",0.1f},{"unc",0.1f},{"compAmt",0.2f},
            {"revMix",0.45f},{"revDecay",0.75f},{"revSize",0.7f},{"revShimmer",0.45f},{"revDamp",0.45f},{"revPre",0.2f} });
    M (p, 1, MsLfo1, MdLpf, 0.2f); M (p, 2, MsPolyAT, MdLpf, 0.5f); M (p, 3, MsWheel, MdRevMix, 0.3f);
}
void pCathedral (OrageAudioProcessor& p)
{
    S (p, { {"output",-8},{"s_wave1",0},{"s_wave2",1},{"s_oct2",1},{"s_lvl1",0.6f},{"s_lvl2",0.4f},{"s_tune2",0.1f},{"s_ftype",2},{"s_lpf",0.5f},{"s_lpfRes",0.3f},{"s_fltEnv",0.15f},
            {"s_aA",0.7f},{"s_aS",0.9f},{"s_aR",0.9f},{"power",0.2f},{"clip",0.1f},{"compAmt",0.2f},
            {"revMix",0.5f},{"revDecay",0.9f},{"revSize",0.9f},{"revPre",0.3f},{"revDamp",0.5f},{"revShimmer",0.15f},{"revMod",0.4f} });
    M (p, 1, MsWheel, MdRevMix, 0.35f); M (p, 2, MsPolyAT, MdLpf, 0.4f);
}
void pFrozen (OrageAudioProcessor& p)
{
    S (p, { {"output",-12},{"s_wave1",2},{"s_wave2",2},{"s_tune2",0.08f},{"s_lvl1",0.7f},{"s_lvl2",0.5f},{"s_oct2",3},{"s_ftype",1},{"s_lpf",0.45f},{"s_lpfRes",0.2f},{"s_fltEnv",0.05f},
            {"s_aA",0.8f},{"s_aS",1.f},{"s_aR",0.92f},{"s_l3Rate",0.1f},{"power",0.15f},{"clip",0.05f},
            {"revMix",0.6f},{"revDecay",1.f},{"revSize",1.f},{"revShimmer",0.6f},{"revDamp",0.35f},{"revPre",0.15f} });
    M (p, 1, MsLfo3, MdPan, 0.5f); M (p, 2, MsPolyAT, MdRevMix, 0.3f);
}
void pStrings (OrageAudioProcessor& p)
{
    S (p, { {"output",-8},{"s_wave1",0},{"s_wave2",0},{"s_tune2",0.2f},{"s_lvl1",0.6f},{"s_lvl2",0.6f},{"s_noise",0.02f},{"s_ftype",1},{"s_semMorph",0.f},{"s_lpf",0.62f},{"s_lpfRes",0.2f},
            {"s_aA",0.62f},{"s_aS",0.9f},{"s_aR",0.78f},{"power",0.25f},{"clip",0.1f},{"compAmt",0.3f},
            {"bbdTime",0.12f},{"bbdFb",0.1f},{"bbdMod",1.f},{"bbdMix",0.3f},{"bbdPing",0},{"revMix",0.3f},{"revDecay",0.65f} });
    M (p, 1, MsVel, MdLpf, 0.2f); M (p, 2, MsLfo2, MdPitch, 0.01f); S (p, { {"s_l2Rate",0.3f} });
}
void pDark (OrageAudioProcessor& p)
{
    S (p, { {"output",-9},{"s_wave1",0},{"s_oct1",1},{"s_wave2",1},{"s_lvl2",0.5f},{"s_tune2",0.12f},{"s_ftype",2},{"s_lpf",0.3f},{"s_lpfRes",0.5f},{"s_fltEnv",0.35f},{"s_fA",0.8f},{"s_fD",0.8f},{"s_fS",0.5f},
            {"s_aA",0.7f},{"s_aS",0.9f},{"s_aR",0.85f},{"s_l2Rate",0.1f},{"power",0.35f},{"clip",0.2f},
            {"revMix",0.4f},{"revDecay",0.85f},{"revDamp",0.7f},{"revSize",0.8f} });
    M (p, 1, MsLfo2, MdPw2, 0.6f); M (p, 2, MsPolyAT, MdLpf, 0.5f);
}
void pHydra (OrageAudioProcessor& p)
{
    S (p, { {"output",-8},{"s_wave1",2},{"s_wave2",0},{"s_lvl2",0.3f},{"s_tune2",0.1f},{"s_fold",0.1f},{"s_ftype",2},{"s_lpf",0.5f},{"s_lpfRes",0.25f},
            {"s_aA",0.6f},{"s_aS",0.9f},{"s_aR",0.8f},{"power",0.25f},{"revMix",0.4f},{"revDecay",0.7f},{"revShimmer",0.3f} });
    M (p, 1, MsPolyAT, MdFold, 0.6f); M (p, 2, MsPolyAT, MdLpf, 0.4f); M (p, 3, MsMac1, MdRevDecay, 0.3f);
}
void pMoonSync (OrageAudioProcessor& p)
{
    S (p, { {"output",-8},{"s_wave1",0},{"s_wave2",0},{"s_sync",1},{"s_tune2",5},{"s_lvl1",0.5f},{"s_lvl2",0.5f},{"s_ftype",1},{"s_semMorph",0.f},{"s_lpf",0.6f},{"s_lpfRes",0.3f},
            {"s_aA",0.65f},{"s_aS",0.9f},{"s_aR",0.85f},{"s_l1Rate",0.1f},{"power",0.25f},{"revMix",0.45f},{"revDecay",0.75f},{"revShimmer",0.25f} });
    M (p, 1, MsLfo1, MdPitch2, 0.35f); M (p, 2, MsPolyAT, MdLpf, 0.4f);
}
void pBreath (OrageAudioProcessor& p)
{
    S (p, { {"output",-8},{"s_wave1",0},{"s_lvl1",0.6f},{"s_noise",0.15f},{"s_ftype",1},{"s_semMorph",0.3f},{"s_lpf",0.55f},{"s_lpfRes",0.5f},
            {"s_aA",0.8f},{"s_aS",0.9f},{"s_aR",0.85f},{"s_l1Rate",0.15f},{"power",0.2f},{"revMix",0.5f},{"revDecay",0.8f},{"revSize",0.8f} });
    M (p, 1, MsLfo1, MdMorph, 0.7f); M (p, 2, MsPolyAT, MdNoise, 0.3f);
}
// ---- BASS
void bSub (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",1},{"s_oct1",1},{"s_pw1",0.5f},{"s_wave2",2},{"s_oct2",0},{"s_lvl2",0.6f},{"s_ftype",2},{"s_lpf",0.35f},{"s_lpfRes",0.3f},{"s_fltEnv",0.4f},{"s_fD",0.3f},{"s_fS",0.1f},
            {"s_aS",0.9f},{"s_aR",0.2f},{"power",0.5f},{"compAmt",0.4f} });
}
void bAcid (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_lvl1",0.8f},{"s_ftype",2},{"s_lpf",0.28f},{"s_lpfRes",0.85f},{"s_fltEnv",0.6f},{"s_fD",0.22f},{"s_fS",0.f},{"s_drive",0.5f},{"s_glide",0.15f},
            {"s_aS",0.7f},{"s_aR",0.15f},{"power",0.5f},{"compAmt",0.5f} });
    M (p, 1, MsVel, MdFltEnv, 0.5f);
}
void bReese (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_wave2",0},{"s_oct2",1},{"s_tune2",0.18f},{"s_lvl2",0.8f},{"s_ftype",1},{"s_lpf",0.4f},{"s_lpfRes",0.2f},{"s_l1Rate",0.2f},{"s_aS",0.9f},{"s_aR",0.2f},
            {"power",0.6f},{"compAmt",0.4f},{"s_drive",0.3f} });
    M (p, 1, MsLfo1, MdLpf, 0.25f);
}
void bGrowl (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_oct1",1},{"s_wave2",2},{"s_oct2",1},{"s_fm",0.5f},{"s_ring",0.2f},{"s_lvl2",0.5f},{"s_lpf",0.55f},{"s_lpfRes",0.6f},{"s_drive",0.5f},
            {"s_l1Rate",0.5f},{"s_l1Sync",6},{"s_aS",0.9f},{"s_aR",0.2f},{"power",0.65f},{"clip",0.4f} });
    M (p, 1, MsLfo1, MdFm, 0.5f); M (p, 2, MsLfo1, MdLpf, 0.3f);
}
void bPluck (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_ftype",0},{"s_lpf",0.28f},{"s_lpfRes",0.4f},{"s_fltEnv",0.8f},{"s_fD",0.15f},{"s_fS",0.f},{"s_aD",0.3f},{"s_aS",0.4f},{"s_aR",0.2f},
            {"power",0.4f},{"compAmt",0.4f} });
}
// ---- LEADS
void lSem (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_tune2",0.1f},{"s_lvl2",0.6f},{"s_ftype",1},{"s_lpf",0.65f},{"s_lpfRes",0.6f},{"s_glide",0.12f},{"s_l1Rate",0.55f},{"s_l1Fade",0.5f},{"s_l1Trig",1},
            {"s_aA",0.3f},{"s_aS",0.9f},{"s_aR",0.45f},{"power",0.35f},{"bbdMix",0.25f},{"bbdTime",0.5f},{"bbdFb",0.35f},{"revMix",0.2f},{"revDecay",0.5f} });
    M (p, 1, MsLfo1, MdPitch, 0.02f); M (p, 2, MsPolyAT, MdLpf, 0.4f);
}
void lLadder (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_sync",1},{"s_tune2",7},{"s_lvl2",0.5f},{"s_ftype",2},{"s_lpf",0.7f},{"s_lpfRes",0.75f},{"s_glide",0.2f},{"s_drive",0.6f},{"s_aS",0.9f},{"s_aR",0.3f},
            {"power",0.45f},{"revMix",0.15f} });
    M (p, 1, MsEnv, MdPitch2, 0.4f);
}
void lSine (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_lvl1",0.8f},{"s_ftype",1},{"s_lpf",0.85f},{"s_lpfRes",0.1f},{"s_glide",0.1f},{"s_l1Rate",0.5f},{"s_l1Fade",0.6f},{"s_l1Trig",1},{"s_aA",0.25f},{"s_aS",0.95f},{"s_aR",0.4f},
            {"power",0.2f},{"revMix",0.3f},{"revDecay",0.6f},{"revShimmer",0.1f} });
    M (p, 1, MsLfo1, MdPitch, 0.015f); M (p, 2, MsPolyAT, MdFm, 0.0f);
}
void lPwm (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",1},{"s_wave2",1},{"s_tune2",0.15f},{"s_lvl2",0.5f},{"s_lpf",0.6f},{"s_lpfRes",0.4f},{"s_l2Rate",0.25f},{"s_l2Wave",1},{"s_aS",0.9f},{"s_aR",0.3f},{"power",0.4f},
            {"bbdMix",0.3f},{"bbdSync",5},{"bbdFb",0.4f} });
    M (p, 1, MsLfo2, MdPw1, 0.7f); M (p, 2, MsLfo2, MdPw2, -0.7f);
}
void lWail (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_tune2",0.1f},{"s_lvl2",0.5f},{"s_ftype",0},{"s_lpf",0.5f},{"s_lpfRes",0.7f},{"s_drive",0.4f},{"s_glide",0.1f},{"s_aS",0.9f},{"s_aR",0.35f},{"power",0.4f},{"revMix",0.25f} });
    M (p, 1, MsPolyAT, MdLpf, 0.6f); M (p, 2, MsPolyAT, MdPitch, 0.02f); M (p, 3, MsVel, MdLevel, 0.2f);
}
// ---- PLUCK / KEYS
void kGlass (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",2},{"s_oct2",3},{"s_lvl2",0.3f},{"s_ftype",1},{"s_lpf",0.5f},{"s_fltEnv",0.5f},{"s_fD",0.2f},{"s_fS",0.f},{"s_aD",0.3f},{"s_aS",0.f},{"s_aR",0.5f},
            {"power",0.2f},{"revMix",0.3f},{"revDecay",0.6f},{"revShimmer",0.15f} });
}
void kLadder (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",1},{"s_lvl2",0.3f},{"s_tune2",0.1f},{"s_ftype",2},{"s_lpf",0.4f},{"s_lpfRes",0.3f},{"s_fltEnv",0.5f},{"s_fD",0.35f},{"s_fS",0.2f},{"s_aD",0.5f},{"s_aS",0.4f},{"s_aR",0.35f},
            {"power",0.3f},{"revMix",0.2f} });
    M (p, 1, MsVel, MdFltEnv, 0.6f);
}
void kMarimba (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",2},{"s_oct2",3},{"s_ring",0.5f},{"s_lvl1",0.7f},{"s_lvl2",0.3f},{"s_lpf",0.6f},{"s_fltEnv",0.3f},{"s_fD",0.15f},{"s_fS",0.f},{"s_aD",0.22f},{"s_aS",0.f},{"s_aR",0.3f},
            {"power",0.2f},{"revMix",0.25f},{"revDecay",0.5f} });
}
void kClav (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",1},{"s_pw1",0.3f},{"s_hpf",0.3f},{"s_hpfRes",0.5f},{"s_lpf",0.6f},{"s_lpfRes",0.4f},{"s_fltEnv",0.6f},{"s_fD",0.12f},{"s_fS",0.f},{"s_drive",0.4f},{"s_aD",0.25f},{"s_aS",0.f},{"s_aR",0.2f},
            {"power",0.4f} });
    M (p, 1, MsVel, MdLevel, 0.3f);
}
// ---- ARP / SEQ
void aRain (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",0},{"s_lvl2",0.3f},{"s_tune2",12},{"s_ftype",1},{"s_lpf",0.55f},{"s_lpfRes",0.3f},{"s_fltEnv",0.5f},{"s_fD",0.2f},{"s_fS",0.f},{"s_aD",0.25f},{"s_aS",0.f},{"s_aR",0.4f},
            {"s_arpOn",1},{"s_arpMode",0},{"s_arpOct",1},{"s_arpRate",0.68f},{"bbdMix",0.35f},{"bbdSync",5},{"bbdFb",0.5f},{"revMix",0.35f},{"revDecay",0.7f},{"revShimmer",0.2f},{"power",0.25f} });
}
void aTrance (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_tune2",0.12f},{"s_lvl2",0.6f},{"s_ftype",2},{"s_lpf",0.5f},{"s_lpfRes",0.5f},{"s_fltEnv",0.5f},{"s_fD",0.18f},{"s_fS",0.f},{"s_aD",0.25f},{"s_aS",0.5f},{"s_aR",0.2f},
            {"s_arpOn",1},{"s_arpMode",0},{"s_arpOct",0},{"s_arpRate",0.79f},{"s_assign",2},{"s_uniDetune",0.35f},{"revMix",0.2f},{"power",0.4f} });
}
void aRandom (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",2},{"s_fm",0.4f},{"s_tune2",7},{"s_ftype",1},{"s_lpf",0.6f},{"s_fltEnv",0.4f},{"s_fD",0.25f},{"s_fS",0.f},{"s_aD",0.3f},{"s_aS",0.1f},{"s_aR",0.4f},
            {"s_arpOn",1},{"s_arpMode",2},{"s_arpOct",2},{"s_arpRate",0.61f},{"s_assign",3},{"bbdMix",0.4f},{"bbdPing",1},{"bbdSync",5},{"revMix",0.4f},{"revDecay",0.7f},{"power",0.25f} });
    M (p, 1, MsRandom, MdLpf, 0.3f);
}
void aBassSeq (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_wave2",1},{"s_oct2",1},{"s_lvl2",0.4f},{"s_ftype",2},{"s_lpf",0.3f},{"s_lpfRes",0.6f},{"s_fltEnv",0.6f},{"s_fD",0.2f},{"s_fS",0.f},{"s_aD",0.25f},{"s_aS",0.4f},{"s_aR",0.15f},
            {"s_arpOn",1},{"s_arpMode",2},{"s_arpOct",1},{"s_arpRate",0.72f},{"s_assign",1},{"power",0.5f},{"compAmt",0.4f} });
}
// ---- KEY ASSIGN / MONOPOLY
void lUnison (OrageAudioProcessor& p)
{
    S (p, { {"output",-9},{"s_wave1",0},{"s_wave2",0},{"s_tune2",0.1f},{"s_lvl2",0.7f},{"s_ftype",2},{"s_lpf",0.5f},{"s_lpfRes",0.3f},{"s_fltEnv",0.3f},{"s_fD",0.3f},{"s_fS",0.4f},
            {"s_assign",2},{"s_uniDetune",0.4f},{"s_glide",0.08f},{"s_aS",0.9f},{"s_aR",0.25f},{"power",0.35f},{"revMix",0.15f} });
    M (p, 1, MsPolyAT, MdLpf, 0.3f);
}
void pChordMem (OrageAudioProcessor& p)
{
    S (p, { {"output",-9},{"s_wave1",2},{"s_wave2",0},{"s_tune2",0.12f},{"s_lvl2",0.5f},{"s_ftype",1},{"s_lpf",0.5f},{"s_lpfRes",0.2f},{"s_aA",0.4f},{"s_aS",0.9f},{"s_aR",0.7f},
            {"s_assign",4},{"s_uniDetune",0.5f},{"s_l1Rate",0.1f},{"revMix",0.3f},{"revDecay",0.7f},{"power",0.2f} });
    M (p, 1, MsLfo1, MdLpf, 0.2f);
}
void pShare (OrageAudioProcessor& p)
{
    S (p, { {"output",-9},{"s_wave1",0},{"s_wave2",1},{"s_lvl2",0.5f},{"s_tune2",0.1f},{"s_ftype",1},{"s_lpf",0.55f},{"s_lpfRes",0.2f},{"s_aA",0.5f},{"s_aS",0.9f},{"s_aR",0.8f},
            {"s_assign",3},{"s_uniDetune",0.4f},{"revMix",0.3f},{"revDecay",0.7f},{"power",0.2f} });
}
void aMonopoly (OrageAudioProcessor& p)
{
    S (p, { {"output",-9},{"s_wave1",0},{"s_wave2",0},{"s_tune2",0.12f},{"s_lvl2",0.6f},{"s_ftype",2},{"s_lpf",0.4f},{"s_lpfRes",0.5f},{"s_fltEnv",0.5f},{"s_fD",0.2f},{"s_fS",0.1f},{"s_aD",0.25f},{"s_aS",0.5f},{"s_aR",0.2f},
            {"s_arpOn",2},{"s_arpMode",2},{"s_arpOct",2},{"s_assign",3},{"s_arpRate",0.65f},{"bbdMix",0.3f},{"bbdSync",5},{"revMix",0.2f},{"power",0.3f} });
}
// ---- FX / TEXTURE
void xWind (OrageAudioProcessor& p)
{
    S (p, { {"s_lvl1",0.f},{"s_noise",0.8f},{"s_ftype",2},{"s_lpf",0.4f},{"s_lpfRes",0.6f},{"s_l4Rate",0.3f},{"s_l4Wave",4},{"s_l4Slew",0.5f},{"s_aA",0.7f},{"s_aS",1.f},{"s_aR",0.8f},{"power",0.2f},
            {"revMix",0.5f},{"revDecay",0.8f} });
    M (p, 1, MsLfo4, MdLpf, 0.5f); M (p, 2, MsPolyAT, MdLpf, 0.3f);
}
void xMetal (OrageAudioProcessor& p)
{
    S (p, { {"output",-8},{"s_wave1",0},{"s_wave2",1},{"s_tune2",5.5f},{"s_ring",0.9f},{"s_fm",0.4f},{"s_lvl1",0.3f},{"s_lpf",0.75f},{"s_l3Rate",0.4f},{"s_l3Wave",4},{"s_aD",0.4f},{"s_aS",0.2f},{"s_aR",0.5f},{"power",0.3f},
            {"revMix",0.4f},{"revDecay",0.7f},{"revShimmer",0.5f} });
    M (p, 1, MsLfo3, MdPitch2, 0.4f);
}
void xAlien (OrageAudioProcessor& p)
{
    S (p, { {"output",-10},{"s_wave1",2},{"s_wave2",2},{"s_fm",0.4f},{"s_tune2",7},{"s_lvl2",0.5f},{"s_ftype",1},{"s_lpf",0.6f},{"s_lpfRes",0.5f},{"s_l1Rate",0.2f},{"s_aA",0.6f},{"s_aS",0.9f},{"s_aR",0.8f},{"power",0.3f},
            {"revMix",0.5f},{"revDecay",0.85f},{"revSize",0.9f},{"revShimmer",0.35f} });
    M (p, 1, MsLfo1, MdMorph, 0.8f); M (p, 2, MsLfo1, MdFm, 0.3f);
}
void xThunder (OrageAudioProcessor& p)
{
    S (p, { {"output",-9},{"s_wave1",0},{"s_oct1",0},{"s_wave2",1},{"s_oct2",0},{"s_lvl2",0.5f},{"s_noise",0.3f},{"s_ftype",2},{"s_lpf",0.25f},{"s_lpfRes",0.4f},{"s_aA",0.5f},{"s_aS",1.f},{"s_aR",0.85f},
            {"power",0.8f},{"material",2},{"lightning",0.3f},{"revMix",0.55f},{"revDecay",0.9f},{"revDamp",0.6f} });
    M (p, 1, MsMac1, MdDrive, 0.6f); M (p, 2, MsMac1, MdLpf, 0.5f);
}
#endif
const Factory kFactory[] = {
    { "Init", "", fInit },
#if ORAGE_SYNTH
    { "Aurora Glass", "Pad", pAurora }, { "Cathedral Choir", "Pad", pCathedral }, { "Frozen Lake", "Pad", pFrozen }, { "Warm Strings", "Pad", pStrings },
    { "Dark Matter", "Pad", pDark }, { "Hydra Swell", "Pad", pHydra }, { "Moonlit Sync", "Pad", pMoonSync }, { "Breath Of Orage", "Pad", pBreath },
    { "Sub Ladder", "Bass", bSub }, { "Acid Squelch", "Bass", bAcid }, { "Reese Detune", "Bass", bReese }, { "Growl FM", "Bass", bGrowl }, { "Pluck Bass", "Bass", bPluck },
    { "Justice Bass Synth", "Bass", sBass }, { "MS20 Bass Boost", "Bass", sMs20Bass },
    { "Solo SEM", "Lead", lSem }, { "Ladder Screamer", "Lead", lLadder }, { "Soft Sine Lead", "Lead", lSine }, { "Pulse Width Lead", "Lead", lPwm }, { "Aftertouch Wail", "Lead", lWail },
    { "Hard Sync Lead", "Lead", sLead }, { "Wavefold Lead", "Lead", sFoldLead },
    { "Glass Pluck", "Pluck / Keys", kGlass }, { "Ladder Keys", "Pluck / Keys", kLadder }, { "Marimba Ring", "Pluck / Keys", kMarimba }, { "Electric Clav", "Pluck / Keys", kClav }, { "FM Bell Pluck", "Pluck / Keys", sFm },
    { "Arp Rain", "Arp / Seq", aRain }, { "Trance Gate Arp", "Arp / Seq", aTrance }, { "Random Dreams", "Arp / Seq", aRandom }, { "Bass Sequence", "Arp / Seq", aBassSeq }, { "Arp Echo BBD", "Arp / Seq", sArpBbd },
    { "Unison Monster", "Lead", lUnison }, { "Chord Memory Pad", "Pad", pChordMem }, { "Share Strings", "Pad", pShare }, { "Monopoly Arp", "Arp / Seq", aMonopoly },
    { "Wind Machine", "FX / Texture", xWind }, { "Metal Rain", "FX / Texture", xMetal }, { "Alien Choir", "FX / Texture", xAlien }, { "Thunder Drone", "FX / Texture", xThunder }, { "Ring Wobble", "FX / Texture", sRing },
#endif
    { "Waters Of Nazareth (depart)", "Rack FX", fWaters },
    { "Gated Fuzz Wall", "Rack FX", fGated },
    { "Envelope Wah Scream", "Rack FX", fWah },
    { "Sample & Hold 1/8", "Rack FX", fSh },
    { "Glitch Machine 1/16", "Rack FX", fGlitch },
    { "Spring Feedback", "Rack FX", fSpring },
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
    arp.prepare ((float) sampleRate);
    evs.clear(); evs.reserve (2048);
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
    p.bbdTime = f();  p.bbdFb = f();  p.bbdTone = f();  p.bbdMod = f();  p.bbdMix = f();  p.bbdPing = b();  p.bbdSync = n();
    p.revMix = f();  p.revSize = f();  p.revDecay = f();  p.revPre = f();  p.revDamp = f();  p.revMod = f();  p.revWidth = f();  p.revShimmer = f();  p.revLowCut = f();  p.revFreeze = b();
    jassert (i == kNumDefs);
    return p;
}


#if ORAGE_SYNTH
orage::SynthParams OrageAudioProcessor::readSynth()
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
    for (int k = 0; k < 4; ++k) s.macro[k] = f();
    for (auto& sl : s.slot) { sl.src = n(); sl.dst = n(); sl.amt = f(); }
    s.fold = f(); s.foldSym = f();
    arpParams.on = n(); arpParams.dir = n(); arpParams.range = n();
    s.ftype = n(); s.semMorph = f();
    s.assign = n(); s.hold = b() ? 1 : 0; s.uniDetune = f(); chordLearn = b();
    arpParams.assign = s.assign; arpParams.hold = s.hold != 0;
    for (int k = 4; k < kMacros; ++k) s.macro[k] = f();
    arpRateKnob = f(); arpSyncDiv = n();
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

        // ---- evenements du bloc : MIDI direct, ou arpegiateur (qui genere ses propres notes)
        evs.clear();
        arp.setParams (arpParams);
        const bool arpOn = arpParams.on != 0;
        if (arpParams.on != lastArpOn || arpParams.assign != lastAssign) { lastArpOn = arpParams.on; lastAssign = arpParams.assign; synth.allNotesOff(); arp.allOff(); }
        if (chordLearn != lastLearn)
        {
            if (chordLearn) { learnN = 0; }
            else if (learnN > 0)
            {
                int iv[6], n = 0; const int root = learnNotes[0];
                for (int k = 1; k < learnN && n < 6; ++k) iv[n++] = learnNotes[k] - root;
                if (n == 0) { iv[0] = 0; n = 1; }
                for (int k = 0; k < n; ++k) chordIv[k] = iv[k];
                chordN = n;
            }
            lastLearn = chordLearn;
        }
        synth.setChord (chordIv, chordN); arp.setChord (chordIv, chordN);
        for (const auto meta : midi)
        {
            const auto m = meta.getMessage();
            const int pos = juce::jlimit (0, total - 1, meta.samplePosition);
            if (m.isNoteOn())
            {
                if (chordLearn && learnN < 6) learnNotes[learnN++] = m.getNoteNumber();
                if (chordLearn) continue;                                    // en apprentissage, les touches ne jouent pas
                if (arpOn) arp.noteOn (m.getNoteNumber(), m.getFloatVelocity()); else evs.push_back ({ pos, 0, m.getNoteNumber(), m.getFloatVelocity() });
            }
            else if (m.isNoteOff())      { if (chordLearn) continue; if (arpOn) arp.noteOff (m.getNoteNumber()); else evs.push_back ({ pos, 1, m.getNoteNumber(), 0.f }); }
            else if (m.isAllNotesOff() || m.isAllSoundOff()) { arp.allOff(); evs.push_back ({ pos, 2, 0, 0.f }); }
            else if (m.isPitchWheel())   evs.push_back ({ pos, 3, 0, (float) (m.getPitchWheelValue() - 8192) / 8192.f * 2.f });
            else if (m.isAftertouch())   evs.push_back ({ pos, 5, m.getNoteNumber(), (float) m.getAfterTouchValue() / 127.f });
            else if (m.isChannelPressure()) evs.push_back ({ pos, 6, 0, (float) m.getChannelPressureValue() / 127.f });
            else if (m.isController() && m.getControllerNumber() != 1 && handleMacroCC (m.getControllerNumber(), m.getControllerValue())) {}
            else if (m.isController() && m.getControllerNumber() == 1) evs.push_back ({ pos, 4, 0, (float) m.getControllerValue() / 127.f });
        }
        {
            orage::Arp::Ev ae[256];
            const int ac = arp.process (total, arpStepHz (bpm), ae, 256);
            for (int k = 0; k < ac; ++k) evs.push_back ({ juce::jlimit (0, total - 1, ae[k].pos), ae[k].on ? (ae[k].stack ? 8 : 7) : 9, ae[k].note, ae[k].vel });
        }
        std::stable_sort (evs.begin(), evs.end(), [] (const MidiEv& a, const MidiEv& b) { return a.pos < b.pos; });

        int pos = 0;
        auto render = [&] (int upTo)
        {
            if (upTo > pos) { synth.process (sl + pos, sr + pos, upTo - pos); pos = upTo; }
        };
        for (const auto& e : evs)
        {
            render (e.pos);
            switch (e.type)
            {
                case 0: synth.noteOn (e.a, e.v); break;
                case 1: synth.noteOff (e.a); break;
                case 2: synth.allNotesOff(); break;
                case 3: synth.setBend (e.v); break;
                case 7: synth.noteOnRaw (e.a, e.v); break;
                case 8: synth.noteOnStack (e.a, e.v); break;
                case 9: synth.noteOffRaw (e.a); break;
                case 5: synth.setPolyAT (e.a, e.v); break;
                case 6: synth.setChanAT (e.v); break;
                default: synth.setWheel (e.v); break;
            }
        }
        render (total);
        engine.setExtMod (synth.fxMod());
    }
#endif
    engine.setParams (readParams());
    float* io[2] = { buffer.getWritePointer (0), nc > 1 ? buffer.getWritePointer (1) : nullptr };
    engine.process (io, total, nc);
    {
        int w = scopeW.load (std::memory_order_relaxed);
        for (int i = 0; i < total; ++i)
        {
            const float l = io[0][i], r = io[1] != nullptr ? io[1][i] : l;
            scopeBuf[(w + i) & (kScope - 1)] = 0.5f * (l + r);
        }
        scopeW.store ((w + total) & (kScope - 1), std::memory_order_release);
    }
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
    state.setProperty ("category", presetCategory, nullptr);
    state.setProperty ("hdr", headerScope.load(), nullptr);
    { juce::String c; for (int k = 0; k < chordN; ++k) c << chordIv[k] << ","; state.setProperty ("chord", c, nullptr); }
    { juce::String c; for (int k = 0; k < 8; ++k) c << macroCC[k].load() << ","; state.setProperty ("macroCC", c, nullptr); }
    return state;
}

void OrageAudioProcessor::applyState (juce::ValueTree state)
{
    engine.loadPatch (state.getProperty ("patch", "").toString().toStdString());
    const auto rk = state.getProperty ("rack", "").toString();
    if (rk.isNotEmpty()) engine.loadRack (rk.toStdString()); else engine.setDefaultRack();
    presetName = state.getProperty ("presetName", "Init").toString();
    presetCategory = state.getProperty ("category", "").toString();
    headerScope.store ((int) state.getProperty ("hdr", 0));
    {
        juce::StringArray t; t.addTokens (state.getProperty ("macroCC", "").toString(), ",", ""); t.removeEmptyStrings();
        for (int k = 0; k < 8; ++k) macroCC[k].store (k < t.size() ? t[k].getIntValue() : 21 + k);
    }
    {
        const auto cs = state.getProperty ("chord", "").toString();
        if (cs.isNotEmpty()) { juce::StringArray t; t.addTokens (cs, ",", ""); t.removeEmptyStrings(); chordN = 0; for (auto& x : t) if (chordN < 6) chordIv[chordN++] = x.getIntValue(); }
        else { const int d[4] { 4, 7, 11, 0 }; chordN = 3; for (int k = 0; k < 3; ++k) chordIv[k] = d[k]; }
    }
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
juce::String OrageAudioProcessor::getFactoryPresetCategory (int i) { return kFactory[juce::jlimit (0, kNumFactory - 1, i)].category; }
juce::StringArray OrageAudioProcessor::getCategories() { juce::StringArray a; for (auto* c : kCategories) a.add (c); return a; }

void OrageAudioProcessor::loadFactoryPreset (int i)
{
    i = juce::jlimit (0, kNumFactory - 1, i);
    resetModules();
    engine.clearPatch();
    if (i == 0) engine.setRack ({}); else engine.setDefaultRack();      // Init = rack vide ; les autres presets utilisent tous les modules
    if (i != 0) legacyFxDefaults (*this);
    kFactory[i].apply (*this);
#if ORAGE_SYNTH
    if (i != 0) applyMacroSet (*this, kFactory[i].category);
#endif
    presetName = kFactory[i].name;
    presetCategory = kFactory[i].category;
}

// CC MIDI -> macros (ex. macros de l'Hydrasynth). Retourne true si le CC a ete consomme.
bool OrageAudioProcessor::handleMacroCC (int cc, int value)
{
#if ORAGE_SYNTH
    const int ln = learnMacro.load();
    if (ln >= 0 && ln < 8 && cc != 1 && cc < 120) { macroCC[ln].store (cc); learnMacro.store (-1); }
    for (int k = 0; k < 8; ++k)
        if (macroCC[k].load() == cc)
        {
            if (auto* prm = apvts.getParameter ("s_m" + juce::String (k + 1))) prm->setValueNotifyingHost ((float) value / 127.f);
            return true;
        }
#else
    juce::ignoreUnused (cc, value);
#endif
    return false;
}

// cadence propre de l'arpegiateur : libre (0,5 a 20 pas/s) ou calee sur le tempo (memes divisions que les LFO)
float OrageAudioProcessor::arpStepHz (double bpm) const
{
#if ORAGE_SYNTH
    if (arpSyncDiv > 0 && arpSyncDiv < 14) return (float) bpm / 60.f / kLfoDivBeats[arpSyncDiv];
    return 0.5f * std::pow (40.f, arpRateKnob);
#else
    juce::ignoreUnused (bpm); return 1.f;
#endif
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

juce::String OrageAudioProcessor::getUserPresetCategory (const juce::String& name) const
{
    if (auto xml = juce::parseXML (getPresetDir().getChildFile (juce::File::createLegalFileName (name) + ".orfx")))
        return xml->getStringAttribute ("category", "");
    return {};
}

bool OrageAudioProcessor::saveUserPreset (const juce::String& name, const juce::String& category)
{
    const auto clean = name.trim();
    if (clean.isEmpty()) return false;
    const auto dir = getPresetDir();
    if (! dir.createDirectory().wasOk()) return false;
    presetName = clean; presetCategory = category;
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
