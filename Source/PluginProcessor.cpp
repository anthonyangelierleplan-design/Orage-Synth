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
    { "stutOn",    "Stutter Engage",   0,   1,  0,    kBool,   nullptr },
    { "stutMode",  "Stutter Mode",     0,   3,  0,    kChoice, "Beat Repeat|Reverse|Gate|Offbeat" },
    { "stutDiv",   "Stutter Division", 0,   6,  4,    kChoice, "1 bar|1/2|1/4|1/8|1/16|1/32|1/64" },
    { "stutFeel",  "Stutter Feel",     0,   2,  0,    kChoice, "Straight|Triplet|Dotted" },
    { "stutLen",   "Stutter Length",   0,   1,  0.6f, kFloat,  nullptr },
    { "stutDecay", "Stutter Decay",    0,   1,  0,    kFloat,  nullptr },
    { "stutPitch", "Stutter Pitch",    0,   1,  0.5f, kFloat,  nullptr },
    { "stutMix",   "Stutter Mix",      0,   1,  1,    kFloat,  nullptr },
};
constexpr int kNumDefs = (int) (sizeof (kDefs) / sizeof (kDefs[0]));

// ---- presets d'usine
#include <map>
using namespace orage;
#if ORAGE_SYNTH
static_assert (orage::Engine::kExt <= orage::kFxMod && orage::kLanes == orage::kArpLanes, "entrees de modulation des effets et pistes : moteur, synthe et arpegiateur doivent correspondre");

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
        C ("s_arpMode", "Arp Direction", "Up|Down|Up-Down|Played|Random|Converge", 0);
        C ("s_arpOct", "Arp Range", "1 Oct|2 Oct|Full", 0);
        C ("s_ftype", "Filter Type", "MS-20|SEM|Ladder|MS-10 (circuit)|Ladder CEM3320 (circuit)|SEM (circuit)", 0);  F ("s_semMorph", "SEM Morph", 0, 1, 0);
        C ("s_assign", "Key Assign", "Poly|Mono|Unison|Share|Chord", 0);  B ("s_hold", "Hold", false);
        F ("s_uniDetune", "Unison Detune", 0, 1, 0.3f);  B ("s_chordLearn", "Chord Learn", false);
        for (int i = 5; i <= kMacros; ++i) F ("s_m" + juce::String (i), "Macro " + juce::String (i), 0, 1, 0);
        F ("s_arpRate", "Arp Rate", 0, 1, 0.55f);  C ("s_arpSync", "Arp Sync", divs.joinIntoString ("|"), 0);
        F ("s_arpGate", "Arp Gate", 0.05f, 1, 0.5f);  F ("s_arpSwing", "Arp Swing", 0, 1, 0);  F ("s_arpChance", "Arp Chance", 0, 1, 1);
        B ("s_arpFree", "Arp Free Run", false);
        { juce::StringArray lens; for (int k = 1; k <= 16; ++k) lens.add (juce::String (k) + (k == 1 ? " STEP" : " STEPS")); C ("s_arpLen", "Arp Steps", lens.joinIntoString ("|"), 7);
          for (int k = 1; k <= kLanes; ++k)
          {
              const juce::String q ("s_lane" + juce::String (k));
              C (q + "Len", "Lane " + juce::String (k) + " Steps", lens.joinIntoString ("|"), 7);
              C (q + "Dst", "Lane " + juce::String (k) + " Dest", dsts.joinIntoString ("|"), 0);
              F (q + "Amt", "Lane " + juce::String (k) + " Amount", -1, 1, 0.5f);
              F (q + "Glide", "Lane " + juce::String (k) + " Glide", 0, 1, 0);
          } }
        C ("s_fmDir", "FM Direction", "2>1|1>2|Both", 0);  C ("s_fmRatio", "FM Ratio", "Free|1:2|2:3|1:1|3:2|2:1|3:1|4:1|5:1|7:1|7:2|x1.41", 0);  C ("s_fmWave", "FM Wave", "Osc|Sine|Tri|Square", 0);
        F ("s_fmFine", "FM Fine", -1, 1, 0);  F ("s_fmAtt", "FM Attack", 0, 1, 0);  F ("s_fmDec", "FM Decay", 0, 1, 0.4f);
        F ("s_fmEnvAmt", "FM Env Amount", 0, 1, 0);  F ("s_fmVel", "FM Velocity", 0, 1, 0);  F ("s_fmAT", "FM Aftertouch", 0, 1, 0);
        C ("s_arpType", "Arp Type", "Arp|Seq", 0);  B ("s_seqRec", "Step Record", false);
        { juce::StringArray br; for (int k = 1; k <= 24; ++k) br.add (juce::String (k)); C ("s_bendRange", "Pitch Bend Range", br.joinIntoString ("|"), 1); }
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
// reglages par defaut d'origine des modules FX : seuls les presets "Rack FX" ont ete ecrits avec ces valeurs
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
    M (p, 13, MsWheel, MdLpf, 0.4f);                                    // molette de modulation : routage par defaut (filtre), reglable dans la matrice
#else
    juce::ignoreUnused (p, cat);
#endif
}

// ---- PADS
// ---- BASS
// ---- LEADS
// ---- PLUCK / KEYS
// ---- ARP / SEQ
// ---- KEY ASSIGN / MONOPOLY
// ---- sequenceur : motif de pas ("0" silence, "1" note, "2" accent), ratchets ("1".."4") et pistes de modulation
void setSteps (OrageAudioProcessor& p, const char* st, const char* rt = nullptr)
{
    for (int i = 0; i < orage::kArpSteps; ++i)
    {
        const char c = st[i] != 0 ? st[i] : '1'; const char r = rt != nullptr && rt[i] != 0 ? rt[i] : '1';
        p.arpPat.set (i, c - '0', r - '0');
    }
}
void setLane (OrageAudioProcessor& p, int lane, int dst, float amt, float glide, std::initializer_list<float> vals)
{
    const juce::String q ("s_lane" + juce::String (lane + 1));
    p.setParamReal ((q + "Dst").toRawUTF8(), (float) dst); p.setParamReal ((q + "Amt").toRawUTF8(), amt); p.setParamReal ((q + "Glide").toRawUTF8(), glide);
    p.setParamReal ((q + "Len").toRawUTF8(), (float) (vals.size() - 1));
    int i = 0; for (float v : vals) if (i < orage::kArpSteps) p.arpPat.lane[lane][i++] = v;
}
void setSeq (OrageAudioProcessor& p, std::initializer_list<int> notes, const char* ties = nullptr)
{
    int i = 0;
    for (int n : notes) if (i < orage::kArpSteps) { p.arpPat.seqNote[i] = n; p.arpPat.tie[i] = ties != nullptr && ties[i] == '1' ? 1 : 0; ++i; }
    p.setParamReal ("s_arpLen", (float) (i - 1));
    p.setParamReal ("s_arpType", 1);
}
// ---- FM
// ---- FX / TEXTURE
// ---- presets v18 : refonte complete avec les filtres circuit (MS-10, Ladder CEM3320, SEM)
void v18_Velvet_Dawn (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_tune2",0.12f},{"s_lvl1",0.6f},{"s_lvl2",0.55f},{"s_ftype",5},{"s_lpf",0.58f},{"s_lpfRes",0.22f},{"s_fltEnv",0.1f},{"s_drive",0.05f},{"s_aA",0.72f},{"s_aD",0.6f},{"s_aS",0.9f},{"s_aR",0.85f},{"s_l1Rate",0.12f},{"power",0.1f},{"bbdTime",0.12f},{"bbdMod",1},{"bbdMix",0.3f},{"bbdFb",0.1f},{"bbdPing",0},{"revMix",0.4f},{"revDecay",0.78f},{"revSize",0.7f},{"revDamp",0.45f},{"output",-13} });
    M (p, 1, MsLfo1, MdLpf, 0.1f);
    M (p, 2, MsPolyAT, MdLpf, 0.25f);
}
void v18_Glass_Cathedral (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",1},{"s_oct2",3},{"s_lvl1",0.6f},{"s_lvl2",0.35f},{"s_tune2",0.1f},{"s_ftype",4},{"s_lpf",0.52f},{"s_lpfRes",0.3f},{"s_fltEnv",0.12f},{"s_aA",0.7f},{"s_aS",0.9f},{"s_aR",0.9f},{"revMix",0.55f},{"revDecay",0.92f},{"revSize",0.9f},{"revPre",0.25f},{"revDamp",0.5f},{"revShimmer",0.3f},{"revMod",0.4f},{"output",-6} });
    M (p, 1, MsWheel, MdRevMix, 0.3f);
    M (p, 2, MsPolyAT, MdLpf, 0.3f);
}
void v18_Ocean_Floor (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_wave2",1},{"s_lvl2",0.45f},{"s_tune2",0.1f},{"s_ftype",4},{"s_lpf",0.3f},{"s_lpfRes",0.4f},{"s_fltEnv",0.4f},{"s_fA",0.8f},{"s_fD",0.8f},{"s_fS",0.5f},{"s_aA",0.75f},{"s_aS",0.9f},{"s_aR",0.88f},{"s_l2Rate",0.1f},{"power",0.2f},{"revMix",0.45f},{"revDecay",0.88f},{"revDamp",0.7f},{"revSize",0.85f},{"output",-13.2f} });
    M (p, 1, MsLfo2, MdPw2, 0.35f);
    M (p, 2, MsPolyAT, MdLpf, 0.3f);
}
void v18_Tidal_Choir (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_tune2",0.08f},{"s_lvl1",0.55f},{"s_lvl2",0.5f},{"s_noise",0.06f},{"s_ftype",5},{"s_semMorph",0.3f},{"s_lpf",0.5f},{"s_lpfRes",0.55f},{"s_aA",0.7f},{"s_aS",0.9f},{"s_aR",0.85f},{"s_l1Rate",0.14f},{"s_l2Rate",0.09f},{"revMix",0.5f},{"revDecay",0.85f},{"revSize",0.85f},{"revShimmer",0.2f},{"output",-9.1f} });
    M (p, 1, MsLfo1, MdMorph, 0.25f);
    M (p, 2, MsLfo2, MdLpf, 0.1f);
}
void v18_Amber_Strings (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_tune2",0.2f},{"s_lvl1",0.6f},{"s_lvl2",0.6f},{"s_noise",0.02f},{"s_ftype",5},{"s_lpf",0.62f},{"s_lpfRes",0.18f},{"s_aA",0.6f},{"s_aS",0.9f},{"s_aR",0.75f},{"bbdTime",0.12f},{"bbdFb",0.1f},{"bbdMod",1},{"bbdMix",0.35f},{"bbdPing",0},{"revMix",0.3f},{"revDecay",0.65f},{"compType",1},{"compAmt",0.25f},{"power",0.15f},{"s_l2Rate",0.3f},{"output",-10.6f} });
    M (p, 1, MsVel, MdLpf, 0.1f);
    M (p, 2, MsLfo2, MdPitch, 0.008f);
}
void v18_Cold_Aurora (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",1},{"s_oct2",3},{"s_lvl1",0.7f},{"s_lvl2",0.3f},{"s_tune2",0.07f},{"s_ftype",3},{"s_lpf",0.6f},{"s_lpfRes",0.3f},{"s_fltEnv",0.08f},{"s_aA",0.8f},{"s_aS",1},{"s_aR",0.9f},{"s_l3Rate",0.1f},{"revMix",0.55f},{"revDecay",0.95f},{"revSize",1},{"revShimmer",0.55f},{"revDamp",0.35f},{"revPre",0.15f},{"output",-9.2f} });
    M (p, 1, MsLfo3, MdPan, 0.4f);
    M (p, 2, MsPolyAT, MdRevMix, 0.25f);
}
void v18_Dust_Bowl (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_oct2",1},{"s_tune2",0.15f},{"s_lvl2",0.6f},{"s_noise",0.12f},{"s_ftype",4},{"s_lpf",0.4f},{"s_lpfRes",0.3f},{"s_drive",0.35f},{"s_aA",0.55f},{"s_aS",0.9f},{"s_aR",0.7f},{"power",0.3f},{"bbdTime",0.4f},{"bbdFb",0.3f},{"bbdMix",0.25f},{"bbdTone",0.4f},{"bbdMod",0.5f},{"revMix",0.35f},{"revDecay",0.7f},{"compType",1},{"compAmt",0.3f},{"output",-10} });
    M (p, 1, MsLfo1, MdLpf, 0.06f);
}
void v18_Moon_Sync (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_sync",1},{"s_tune2",5},{"s_lvl1",0.5f},{"s_lvl2",0.5f},{"s_ftype",5},{"s_lpf",0.6f},{"s_lpfRes",0.3f},{"s_aA",0.65f},{"s_aS",0.9f},{"s_aR",0.85f},{"s_l1Rate",0.1f},{"revMix",0.45f},{"revDecay",0.78f},{"revShimmer",0.25f},{"output",-8.5f} });
    M (p, 1, MsLfo1, MdPitch2, 0.35f);
    M (p, 2, MsPolyAT, MdLpf, 0.3f);
}
void v18_Warm_Brass_Pad (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_tune2",0.1f},{"s_lvl1",0.65f},{"s_lvl2",0.55f},{"s_ftype",4},{"s_lpf",0.38f},{"s_lpfRes",0.25f},{"s_fltEnv",0.45f},{"s_fA",0.35f},{"s_fD",0.5f},{"s_fS",0.6f},{"s_aA",0.4f},{"s_aS",0.9f},{"s_aR",0.55f},{"power",0.2f},{"compType",1},{"compAmt",0.3f},{"revMix",0.25f},{"revDecay",0.6f},{"output",-9.5f} });
    M (p, 1, MsVel, MdFltEnv, 0.2f);
}
void v18_Hollow_Wind (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_lvl1",0.4f},{"s_noise",0.3f},{"s_ftype",5},{"s_semMorph",0.45f},{"s_lpf",0.55f},{"s_lpfRes",0.6f},{"s_aA",0.8f},{"s_aS",0.95f},{"s_aR",0.9f},{"s_l1Rate",0.15f},{"s_l2Rate",0.07f},{"revMix",0.5f},{"revDecay",0.88f},{"revSize",0.9f},{"output",-2} });
    M (p, 1, MsLfo1, MdMorph, 0.2f);
    M (p, 2, MsLfo2, MdLpf, 0.15f);
    M (p, 3, MsPolyAT, MdNoise, 0.3f);
}
void v18_Slow_Motion_Chord (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",0},{"s_tune2",0.12f},{"s_lvl2",0.5f},{"s_ftype",5},{"s_lpf",0.5f},{"s_lpfRes",0.2f},{"s_assign",4},{"s_uniDetune",0.5f},{"s_aA",0.55f},{"s_aS",0.9f},{"s_aR",0.75f},{"s_l1Rate",0.1f},{"revMix",0.35f},{"revDecay",0.72f},{"power",0.15f},{"output",-9.8f} });
    M (p, 1, MsLfo1, MdLpf, 0.1f);
}
void v18_Giant_Space (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_wave2",0},{"s_oct2",3},{"s_lvl2",0.4f},{"s_tune2",0.09f},{"s_ftype",4},{"s_lpf",0.45f},{"s_lpfRes",0.3f},{"s_aA",0.85f},{"s_aS",1},{"s_aR",0.95f},{"revMix",0.65f},{"revDecay",0.96f},{"revSize",1},{"revShimmer",0.4f},{"revDamp",0.4f},{"revPre",0.2f},{"power",0.1f},{"output",-13.7f} });
    M (p, 1, MsLfo1, MdPan, 0.3f);
}
void v18_Pulse_Silk (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",1},{"s_wave2",1},{"s_tune2",0.15f},{"s_lvl1",0.5f},{"s_lvl2",0.5f},{"s_ftype",5},{"s_lpf",0.62f},{"s_lpfRes",0.25f},{"s_l2Rate",0.22f},{"s_l2Wave",1},{"s_aA",0.5f},{"s_aS",0.9f},{"s_aR",0.7f},{"bbdTime",0.12f},{"bbdMod",1},{"bbdMix",0.3f},{"bbdFb",0.1f},{"bbdPing",0},{"revMix",0.35f},{"revDecay",0.7f},{"output",-7.9f} });
    M (p, 1, MsLfo2, MdPw1, 0.4f);
    M (p, 2, MsLfo2, MdPw2, -0.4f);
}
void v18_Blue_Hour (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_tune2",0.14f},{"s_lvl2",0.6f},{"s_ftype",3},{"s_lpf",0.5f},{"s_lpfRes",0.25f},{"s_fltEnv",0.15f},{"s_fA",0.6f},{"s_aA",0.6f},{"s_aS",0.9f},{"s_aR",0.8f},{"bbdMix",0.3f},{"bbdSync",6},{"bbdFb",0.35f},{"bbdPing",1},{"revMix",0.45f},{"revDecay",0.8f},{"power",0.1f},{"output",-11.9f} });
    M (p, 1, MsLfo1, MdLpf, 0.08f);
}
void v18_Ladder_Sub (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_oct1",1},{"s_wave2",1},{"s_oct2",1},{"s_lvl2",0.5f},{"s_ftype",4},{"s_lpf",0.33f},{"s_lpfRes",0.3f},{"s_fltEnv",0.4f},{"s_fD",0.3f},{"s_fS",0.1f},{"s_aS",0.9f},{"s_aR",0.15f},{"power",0.4f},{"compType",1},{"compAmt",0.35f},{"output",-3.6f} });
}
void v18_Pro_Pluck_Bass (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_ftype",4},{"s_lpf",0.3f},{"s_lpfRes",0.45f},{"s_fltEnv",0.75f},{"s_fD",0.18f},{"s_fS",0},{"s_aD",0.3f},{"s_aS",0.5f},{"s_aR",0.12f},{"power",0.35f},{"compType",1},{"compAmt",0.4f},{"output",-3} });
    M (p, 1, MsVel, MdFltEnv, 0.25f);
}
void v18_Acid_MS_10 (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_lvl1",0.8f},{"s_ftype",3},{"s_lpf",0.28f},{"s_lpfRes",0.85f},{"s_fltEnv",0.6f},{"s_fD",0.22f},{"s_fS",0},{"s_drive",0.5f},{"s_glide",0.15f},{"s_assign",1},{"s_aS",0.7f},{"s_aR",0.12f},{"power",0.35f},{"compType",1},{"compAmt",0.45f},{"output",-3.6f} });
    M (p, 1, MsVel, MdFltEnv, 0.3f);
}
void v18_Reese_Deep (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_wave2",0},{"s_oct2",1},{"s_tune2",0.18f},{"s_lvl2",0.85f},{"s_ftype",5},{"s_lpf",0.4f},{"s_lpfRes",0.2f},{"s_l1Rate",0.2f},{"s_aS",0.9f},{"s_aR",0.15f},{"power",0.5f},{"compType",1},{"compAmt",0.4f},{"s_drive",0.3f},{"output",-4.2f} });
    M (p, 1, MsLfo1, MdLpf, 0.12f);
}
void v18_Growl_FM (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_oct1",1},{"s_wave2",2},{"s_oct2",1},{"s_fm",0.5f},{"s_ring",0.15f},{"s_lvl2",0.5f},{"s_ftype",3},{"s_lpf",0.55f},{"s_lpfRes",0.6f},{"s_drive",0.5f},{"s_l1Rate",0.5f},{"s_l1Sync",6},{"s_aS",0.9f},{"s_aR",0.15f},{"power",0.55f},{"clip",0.3f},{"compType",1},{"compAmt",0.35f},{"output",-4.8f} });
    M (p, 1, MsLfo1, MdFm, 0.35f);
    M (p, 2, MsLfo1, MdLpf, 0.15f);
}
void v18_Fat_Moog (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_wave2",1},{"s_oct2",1},{"s_lvl2",0.5f},{"s_pw2",0.45f},{"s_ftype",4},{"s_lpf",0.38f},{"s_lpfRes",0.55f},{"s_fltEnv",0.55f},{"s_fD",0.25f},{"s_fS",0.15f},{"s_drive",0.35f},{"s_glide",0.1f},{"s_assign",1},{"s_aS",0.85f},{"s_aR",0.15f},{"power",0.3f},{"compType",1},{"compAmt",0.4f},{"output",-2.5f} });
    M (p, 1, MsVel, MdFltEnv, 0.2f);
}
void v18_Sync_Snarl (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_wave2",0},{"s_oct2",1},{"s_sync",1},{"s_tune2",8},{"s_lvl2",0.6f},{"s_ftype",3},{"s_lpf",0.5f},{"s_lpfRes",0.4f},{"s_fltEnv",0.35f},{"s_fD",0.25f},{"s_fS",0.2f},{"s_assign",1},{"s_aS",0.9f},{"s_aR",0.15f},{"power",0.5f},{"compType",1},{"compAmt",0.35f},{"output",-3.7f} });
    M (p, 1, MsEnv, MdPitch2, 0.5f);
}
void v18_Wobble_Dub (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_wave2",0},{"s_oct2",1},{"s_tune2",0.1f},{"s_lvl2",0.5f},{"s_ftype",4},{"s_lpf",0.32f},{"s_lpfRes",0.6f},{"s_l1Rate",0.5f},{"s_l1Sync",7},{"s_l1Wave",1},{"s_assign",1},{"s_aS",0.9f},{"s_aR",0.15f},{"power",0.4f},{"compType",1},{"compAmt",0.4f},{"output",-3.7f} });
    M (p, 1, MsLfo1, MdLpf, 0.16f);
}
void v18_Sub_Drop (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_oct1",0},{"s_wave2",1},{"s_oct2",1},{"s_lvl2",0.3f},{"s_ftype",4},{"s_lpf",0.3f},{"s_lpfRes",0.15f},{"s_fltEnv",0.1f},{"s_aD",0.7f},{"s_aS",0.6f},{"s_aR",0.2f},{"s_fD",0.3f},{"s_fS",0.2f},{"power",0.2f},{"compType",1},{"compAmt",0.3f},{"output",5.2f} });
    M (p, 1, MsAmpEnv, MdPitch, 0.09f);
}
void v18_Hoover_Low (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_wave2",0},{"s_oct2",1},{"s_tune2",0.3f},{"s_lvl2",0.8f},{"s_ftype",5},{"s_semMorph",0.05f},{"s_lpf",0.52f},{"s_lpfRes",0.35f},{"s_assign",2},{"s_uniDetune",0.6f},{"s_glide",0.08f},{"s_aS",0.9f},{"s_aR",0.2f},{"power",0.45f},{"compType",1},{"compAmt",0.35f},{"output",-5.1f} });
    M (p, 1, MsEnv, MdPitch, 0.06f);
}
void v18_Fold_Bass (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_oct1",1},{"s_wave2",2},{"s_oct2",1},{"s_lvl2",0.5f},{"s_fold",0.45f},{"s_foldSym",0.2f},{"s_ftype",4},{"s_lpf",0.5f},{"s_lpfRes",0.3f},{"s_fltEnv",0.3f},{"s_fD",0.3f},{"s_fS",0.2f},{"s_aS",0.9f},{"s_aR",0.15f},{"power",0.3f},{"compType",1},{"compAmt",0.35f},{"output",-6.3f} });
    M (p, 1, MsEnv, MdFold, 0.4f);
}
void v18_Square_Bass_78 (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",1},{"s_oct1",1},{"s_pw1",0.5f},{"s_lvl1",0.8f},{"s_ftype",3},{"s_lpf",0.38f},{"s_lpfRes",0.35f},{"s_fltEnv",0.5f},{"s_fD",0.22f},{"s_fS",0.1f},{"s_aD",0.35f},{"s_aS",0.6f},{"s_aR",0.1f},{"power",0.25f},{"compType",1},{"compAmt",0.4f},{"output",-3.6f} });
}
void v18_Dirty_Rubber (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_wave2",0},{"s_oct2",1},{"s_tune2",0.1f},{"s_lvl2",0.5f},{"s_ftype",5},{"s_lpf",0.3f},{"s_lpfRes",0.7f},{"s_fltEnv",0.7f},{"s_fD",0.25f},{"s_fS",0.05f},{"s_drive",0.6f},{"s_aS",0.8f},{"s_aR",0.12f},{"power",0.5f},{"clip",0.3f},{"compType",1},{"compAmt",0.45f},{"output",-4.8f} });
    M (p, 1, MsVel, MdFltEnv, 0.2f);
}
void v18_Ladder_Squelch (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_lvl1",0.85f},{"s_ftype",4},{"s_lpf",0.25f},{"s_lpfRes",0.82f},{"s_fltEnv",0.65f},{"s_fD",0.2f},{"s_fS",0},{"s_drive",0.3f},{"s_glide",0.12f},{"s_assign",1},{"s_aS",0.7f},{"s_aR",0.1f},{"power",0.3f},{"compType",1},{"compAmt",0.4f},{"output",-5} });
    M (p, 1, MsVel, MdFltEnv, 0.3f);
}
void v18_Seq_Bass_Line (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_wave2",0},{"s_oct2",1},{"s_tune2",0.06f},{"s_lvl2",0.5f},{"s_ftype",3},{"s_lpf",0.32f},{"s_lpfRes",0.6f},{"s_fltEnv",0.45f},{"s_fD",0.18f},{"s_fS",0.05f},{"s_aD",0.3f},{"s_aS",0.3f},{"s_aR",0.08f},{"s_drive",0.4f},{"s_arpOn",1},{"s_assign",1},{"s_arpRate",0.7f},{"s_arpGate",0.5f},{"s_arpSwing",0.2f},{"power",0.3f},{"compType",1},{"compAmt",0.4f},{"output",-5.4f} });
    setSteps (p, "2112112121121121");
    setSeq (p, { 36, 36, 48, 36, 39, 36, 51, 36, 34, 34, 46, 34, 41, 36, 48, 43 }, "0000100000001000");
    setLane (p, 0, MdLpf, 0.5f, 0.15f, { 0.2f, 0.4f, 0.9f, 0.3f, 0.5f, 0.2f, 1, 0.4f });
}
void v18_Solo_SEM (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_tune2",0.1f},{"s_lvl2",0.6f},{"s_ftype",5},{"s_lpf",0.65f},{"s_lpfRes",0.6f},{"s_glide",0.12f},{"s_l1Rate",0.55f},{"s_l1Fade",0.5f},{"s_l1Trig",1},{"s_aA",0.3f},{"s_aS",0.9f},{"s_aR",0.45f},{"power",0.2f},{"bbdMix",0.25f},{"bbdTime",0.5f},{"bbdFb",0.35f},{"revMix",0.2f},{"revDecay",0.5f},{"output",-8.6f} });
    M (p, 1, MsLfo1, MdPitch, 0.02f);
    M (p, 2, MsPolyAT, MdLpf, 0.3f);
}
void v18_Screaming_MS_10 (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_tune2",0.08f},{"s_lvl2",0.5f},{"s_ftype",3},{"s_lpf",0.6f},{"s_lpfRes",0.7f},{"s_drive",0.45f},{"s_glide",0.15f},{"s_fltEnv",0.2f},{"s_fD",0.4f},{"s_fS",0.4f},{"s_aS",0.9f},{"s_aR",0.3f},{"power",0.35f},{"bbdMix",0.25f},{"bbdTime",0.5f},{"bbdFb",0.35f},{"revMix",0.2f},{"output",-12.2f} });
    M (p, 1, MsPolyAT, MdLpf, 0.2f);
    M (p, 2, MsWheel, MdReso, 0.15f);
}
void v18_Ladder_Sync_Lead (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_sync",1},{"s_tune2",7},{"s_lvl2",0.5f},{"s_ftype",4},{"s_lpf",0.7f},{"s_lpfRes",0.55f},{"s_glide",0.18f},{"s_drive",0.4f},{"s_aS",0.9f},{"s_aR",0.3f},{"power",0.4f},{"revMix",0.15f},{"output",-10.5f} });
    M (p, 1, MsEnv, MdPitch2, 0.4f);
}
void v18_Soft_Sine_Lead (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_lvl1",0.85f},{"s_ftype",5},{"s_lpf",0.85f},{"s_lpfRes",0.1f},{"s_glide",0.1f},{"s_l1Rate",0.5f},{"s_l1Fade",0.6f},{"s_l1Trig",1},{"s_aA",0.25f},{"s_aS",0.95f},{"s_aR",0.4f},{"power",0.1f},{"revMix",0.3f},{"revDecay",0.6f},{"revShimmer",0.1f},{"output",-3.8f} });
    M (p, 1, MsLfo1, MdPitch, 0.015f);
}
void v18_Pulse_Width_Lead (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",1},{"s_wave2",1},{"s_tune2",0.15f},{"s_lvl2",0.5f},{"s_ftype",5},{"s_lpf",0.62f},{"s_lpfRes",0.4f},{"s_l2Rate",0.25f},{"s_l2Wave",1},{"s_aS",0.9f},{"s_aR",0.3f},{"power",0.3f},{"bbdMix",0.3f},{"bbdSync",6},{"bbdFb",0.4f},{"output",-10.6f} });
    M (p, 1, MsLfo2, MdPw1, 0.5f);
    M (p, 2, MsLfo2, MdPw2, -0.5f);
}
void v18_Wavefold_Lead (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",0},{"s_lvl2",0.35f},{"s_tune2",0.1f},{"s_fold",0.5f},{"s_foldSym",0.2f},{"s_ftype",3},{"s_lpf",0.75f},{"s_lpfRes",0.4f},{"s_aS",0.9f},{"s_aR",0.3f},{"s_l1Rate",0.3f},{"bbdMix",0.3f},{"bbdTime",0.55f},{"bbdFb",0.45f},{"revMix",0.15f},{"output",-2.3f} });
    M (p, 1, MsEnv, MdFold, 0.5f);
    M (p, 2, MsLfo1, MdFold, 0.25f);
}
void v18_Unison_Monster (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_tune2",0.1f},{"s_lvl2",0.7f},{"s_ftype",4},{"s_lpf",0.55f},{"s_lpfRes",0.3f},{"s_fltEnv",0.3f},{"s_fD",0.3f},{"s_fS",0.4f},{"s_assign",2},{"s_uniDetune",0.4f},{"s_glide",0.08f},{"s_aS",0.9f},{"s_aR",0.25f},{"power",0.3f},{"revMix",0.15f},{"output",-8.3f} });
    M (p, 1, MsPolyAT, MdLpf, 0.2f);
}
void v18_FM_Brass_Lead (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",2},{"s_fm",0.3f},{"s_lvl2",0.4f},{"s_tune2",0.05f},{"s_ftype",4},{"s_lpf",0.6f},{"s_lpfRes",0.2f},{"s_fltEnv",0.35f},{"s_fA",0.2f},{"s_fD",0.4f},{"s_fS",0.5f},{"s_aA",0.15f},{"s_aS",0.9f},{"s_aR",0.3f},{"power",0.25f},{"revMix",0.2f},{"output",-5.1f} });
    M (p, 1, MsEnv, MdFm, 0.25f);
    M (p, 2, MsVel, MdFm, 0.15f);
}
void v18_Whistle (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_lvl1",0.85f},{"s_ftype",5},{"s_semMorph",0.5f},{"s_lpf",0.72f},{"s_lpfRes",0.72f},{"s_glide",0.1f},{"s_l1Rate",0.5f},{"s_l1Fade",0.5f},{"s_l1Trig",1},{"s_aA",0.2f},{"s_aS",0.95f},{"s_aR",0.4f},{"revMix",0.3f},{"revDecay",0.65f},{"output",-0.1f} });
    M (p, 1, MsLfo1, MdPitch, 0.012f);
    M (p, 2, MsPolyAT, MdMorph, 0.25f);
}
void v18_Glide_Mono (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",1},{"s_lvl2",0.4f},{"s_tune2",0.1f},{"s_ftype",3},{"s_lpf",0.6f},{"s_lpfRes",0.45f},{"s_glide",0.3f},{"s_assign",1},{"s_fltEnv",0.3f},{"s_fD",0.4f},{"s_fS",0.4f},{"s_aS",0.9f},{"s_aR",0.3f},{"power",0.3f},{"bbdMix",0.2f},{"bbdTime",0.45f},{"bbdFb",0.3f},{"revMix",0.15f},{"output",-11.1f} });
    M (p, 1, MsVel, MdLpf, 0.1f);
}
void v18_Aftertouch_Wail (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_tune2",0.1f},{"s_lvl2",0.5f},{"s_ftype",3},{"s_lpf",0.5f},{"s_lpfRes",0.7f},{"s_drive",0.4f},{"s_glide",0.1f},{"s_aS",0.9f},{"s_aR",0.35f},{"power",0.35f},{"revMix",0.25f},{"output",-13} });
    M (p, 1, MsPolyAT, MdLpf, 0.3f);
    M (p, 2, MsPolyAT, MdReso, -0.1f);
}
void v18_Hard_Sync_Sweep (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_sync",1},{"s_tune2",5},{"s_lvl1",0.6f},{"s_lvl2",0.5f},{"s_ftype",3},{"s_lpf",0.65f},{"s_lpfRes",0.4f},{"s_l1Rate",0.4f},{"s_l1Wave",2},{"s_aS",0.9f},{"s_aR",0.3f},{"power",0.4f},{"bbdMix",0.2f},{"bbdTime",0.5f},{"revMix",0.15f},{"output",-11.3f} });
    M (p, 1, MsLfo1, MdPitch2, 0.4f);
}
void v18_Cry_Baby (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_lvl1",0.8f},{"s_ftype",5},{"s_semMorph",0},{"s_lpf",0.4f},{"s_lpfRes",0.7f},{"s_fltEnv",0.5f},{"s_fA",0.1f},{"s_fD",0.5f},{"s_fS",0.3f},{"s_aS",0.9f},{"s_aR",0.3f},{"power",0.2f},{"revMix",0.2f},{"output",-6.4f} });
    M (p, 1, MsWheel, MdLpf, 0.3f);
    M (p, 2, MsVel, MdFltEnv, 0.2f);
}
void v18_Ring_Lead (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",2},{"s_ring",0.5f},{"s_tune2",7},{"s_lvl1",0.6f},{"s_lvl2",0.4f},{"s_ftype",5},{"s_lpf",0.7f},{"s_lpfRes",0.35f},{"s_aS",0.9f},{"s_aR",0.3f},{"power",0.25f},{"bbdMix",0.2f},{"bbdTime",0.5f},{"revMix",0.2f},{"output",-9} });
    M (p, 1, MsLfo1, MdPitch, 0.01f);
}
void v18_Glass_Pluck (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",2},{"s_oct2",3},{"s_lvl2",0.4f},{"s_ftype",3},{"s_lpf",0.5f},{"s_lpfRes",0.3f},{"s_fltEnv",0.7f},{"s_fD",0.15f},{"s_fS",0},{"s_aD",0.35f},{"s_aS",0},{"s_aR",0.3f},{"revMix",0.3f},{"revDecay",0.65f},{"revShimmer",0.2f},{"output",1.9f} });
    M (p, 1, MsVel, MdFltEnv, 0.2f);
}
void v18_Ladder_Keys (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",1},{"s_lvl2",0.4f},{"s_tune2",0.1f},{"s_ftype",4},{"s_lpf",0.4f},{"s_lpfRes",0.35f},{"s_fltEnv",0.5f},{"s_fD",0.3f},{"s_fS",0.3f},{"s_aD",0.45f},{"s_aS",0.3f},{"s_aR",0.25f},{"power",0.2f},{"revMix",0.2f},{"compType",1},{"compAmt",0.25f},{"output",-5.1f} });
    M (p, 1, MsVel, MdFltEnv, 0.3f);
}
void v18_Marimba_Ring (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",2},{"s_ring",0.7f},{"s_tune2",12},{"s_lvl1",0.7f},{"s_lvl2",0.3f},{"s_ftype",5},{"s_lpf",0.6f},{"s_lpfRes",0.1f},{"s_aD",0.28f},{"s_aS",0},{"s_aR",0.2f},{"revMix",0.25f},{"revDecay",0.5f},{"output",0.7f} });
}
void v18_FM_E_Piano (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",2},{"s_fm",0.35f},{"s_fmRatio",3},{"s_lvl1",0.7f},{"s_fmDec",0.4f},{"s_fmEnvAmt",0.5f},{"s_ftype",5},{"s_lpf",0.75f},{"s_lpfRes",0.1f},{"s_aD",0.5f},{"s_aS",0.3f},{"s_aR",0.3f},{"revMix",0.2f},{"revDecay",0.55f},{"bbdMix",0.2f},{"bbdTime",0.12f},{"bbdMod",1},{"bbdFb",0.1f},{"bbdPing",0},{"output",0.6f} });
    M (p, 1, MsVel, MdFm, 0.2f);
}
void v18_FM_Bell (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",2},{"s_fm",0.5f},{"s_fmRatio",11},{"s_lvl1",0.7f},{"s_fmDec",0.55f},{"s_fmEnvAmt",0.6f},{"s_ftype",5},{"s_lpf",0.85f},{"s_lpfRes",0.05f},{"s_aD",0.65f},{"s_aS",0},{"s_aR",0.5f},{"revMix",0.4f},{"revDecay",0.8f},{"revShimmer",0.25f},{"output",0.4f} });
}
void v18_Electric_Clav (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",1},{"s_pw1",0.35f},{"s_lvl1",0.8f},{"s_ftype",3},{"s_lpf",0.4f},{"s_lpfRes",0.5f},{"s_fltEnv",0.8f},{"s_fD",0.12f},{"s_fS",0},{"s_aD",0.22f},{"s_aS",0},{"s_aR",0.12f},{"power",0.2f},{"output",-4.1f} });
    M (p, 1, MsVel, MdFltEnv, 0.3f);
}
void v18_Harpsichord (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",1},{"s_oct2",3},{"s_lvl2",0.3f},{"s_ftype",5},{"s_lpf",0.68f},{"s_lpfRes",0.2f},{"s_fltEnv",0.3f},{"s_fD",0.1f},{"s_fS",0.1f},{"s_aD",0.25f},{"s_aS",0},{"s_aR",0.15f},{"revMix",0.2f},{"revDecay",0.45f},{"output",1.2f} });
}
void v18_Wurly_Drive (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",0},{"s_lvl2",0.25f},{"s_ftype",5},{"s_lpf",0.62f},{"s_lpfRes",0.15f},{"s_drive",0.45f},{"s_l2Rate",0.5f},{"s_aD",0.55f},{"s_aS",0.45f},{"s_aR",0.25f},{"power",0.25f},{"bbdMix",0.2f},{"bbdTime",0.1f},{"bbdMod",1},{"bbdFb",0.1f},{"bbdPing",0},{"revMix",0.15f},{"output",-6.5f} });
    M (p, 1, MsLfo2, MdLevel, 0.12f);
    M (p, 2, MsVel, MdDrive, 0.25f);
}
void v18_Kalimba (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_oct1",3},{"s_lvl1",0.8f},{"s_wave2",2},{"s_oct2",2},{"s_lvl2",0.2f},{"s_ftype",5},{"s_lpf",0.7f},{"s_lpfRes",0.05f},{"s_aD",0.3f},{"s_aS",0},{"s_aR",0.35f},{"revMix",0.35f},{"revDecay",0.6f},{"output",1.7f} });
}
void v18_Brass_Stab (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_tune2",0.12f},{"s_lvl2",0.6f},{"s_ftype",4},{"s_lpf",0.38f},{"s_lpfRes",0.25f},{"s_fltEnv",0.6f},{"s_fA",0.15f},{"s_fD",0.25f},{"s_fS",0.5f},{"s_aA",0.1f},{"s_aD",0.4f},{"s_aS",0.6f},{"s_aR",0.2f},{"power",0.2f},{"compType",1},{"compAmt",0.3f},{"revMix",0.2f},{"output",-5.2f} });
    M (p, 1, MsVel, MdFltEnv, 0.25f);
}
void v18_Vibes (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",2},{"s_oct2",3},{"s_lvl2",0.3f},{"s_ftype",5},{"s_lpf",0.7f},{"s_lpfRes",0.1f},{"s_l2Rate",0.45f},{"s_aD",0.55f},{"s_aS",0},{"s_aR",0.5f},{"revMix",0.3f},{"revDecay",0.65f},{"output",-0.2f} });
    M (p, 1, MsLfo2, MdLevel, 0.2f);
}
void v18_Pluck_Pad_Hybrid (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",1},{"s_lvl2",0.5f},{"s_tune2",0.1f},{"s_ftype",4},{"s_lpf",0.35f},{"s_lpfRes",0.4f},{"s_fltEnv",0.55f},{"s_fD",0.4f},{"s_fS",0.2f},{"s_aA",0.1f},{"s_aD",0.6f},{"s_aS",0.4f},{"s_aR",0.5f},{"revMix",0.35f},{"revDecay",0.75f},{"bbdMix",0.25f},{"bbdSync",6},{"bbdFb",0.35f},{"bbdPing",1},{"output",-4.8f} });
}
void v18_Trance_Gate_Arp (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_tune2",0.12f},{"s_lvl2",0.6f},{"s_ftype",4},{"s_lpf",0.5f},{"s_lpfRes",0.5f},{"s_fltEnv",0.5f},{"s_fD",0.18f},{"s_fS",0},{"s_aD",0.25f},{"s_aS",0.5f},{"s_aR",0.2f},{"s_arpOn",1},{"s_arpMode",0},{"s_arpOct",0},{"s_arpRate",0.79f},{"s_assign",2},{"s_uniDetune",0.35f},{"revMix",0.2f},{"power",0.25f},{"output",-6.1f} });
}
void v18_Arp_Rain (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",2},{"s_oct2",3},{"s_lvl2",0.4f},{"s_ftype",5},{"s_lpf",0.6f},{"s_lpfRes",0.3f},{"s_fltEnv",0.4f},{"s_fD",0.2f},{"s_fS",0},{"s_aD",0.3f},{"s_aS",0.1f},{"s_aR",0.4f},{"s_arpOn",1},{"s_arpMode",4},{"s_arpOct",2},{"s_arpRate",0.7f},{"s_arpGate",0.4f},{"bbdMix",0.4f},{"bbdSync",6},{"bbdFb",0.5f},{"bbdPing",1},{"revMix",0.35f},{"revDecay",0.75f},{"output",4.8f} });
}
void v18_Acid_Lanes (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",0},{"s_tune2",0.08f},{"s_lvl2",0.5f},{"s_ftype",3},{"s_lpf",0.3f},{"s_lpfRes",0.55f},{"s_fltEnv",0.2f},{"s_fD",0.2f},{"s_fS",0.05f},{"s_aD",0.3f},{"s_aS",0.4f},{"s_aR",0.1f},{"s_drive",0.4f},{"s_arpOn",1},{"s_arpMode",0},{"s_arpOct",1},{"s_assign",1},{"s_arpRate",0.68f},{"s_arpGate",0.45f},{"s_arpSwing",0.25f},{"power",0.3f},{"revMix",0.1f},{"output",-4.8f} });
    setSteps (p, "1121101211211012");
    setLane (p, 0, MdLpf, 0.55f, 0.2f, { 0.1f, 0.5f, 0.3f, 0.9f, 0.2f, 0.7f, 0.4f, 1, 0.15f, 0.6f, 0.3f, 0.8f, 0.25f, 0.55f, 0.35f, 0.95f });
    setLane (p, 1, MdReso, 0.3f, 0, { 0, 0, 1, 0, 0, 1, 0, 0 });
}
void v18_Pulse_Climb (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",1},{"s_wave2",0},{"s_lvl2",0.4f},{"s_tune2",0.1f},{"s_ftype",4},{"s_lpf",0.45f},{"s_lpfRes",0.4f},{"s_fltEnv",0.5f},{"s_fD",0.2f},{"s_fS",0},{"s_aD",0.25f},{"s_aS",0.1f},{"s_aR",0.2f},{"s_arpOn",1},{"s_arpMode",2},{"s_arpOct",2},{"s_arpRate",0.64f},{"s_arpGate",0.5f},{"revMix",0.25f},{"revDecay",0.6f},{"power",0.2f},{"output",-1.8f} });
    M (p, 1, MsLfo1, MdPw1, 0.3f);
}
void v18_Random_Dreams (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",2},{"s_fm",0.4f},{"s_tune2",7},{"s_ftype",5},{"s_lpf",0.6f},{"s_fltEnv",0.4f},{"s_fD",0.25f},{"s_fS",0},{"s_aD",0.3f},{"s_aS",0.1f},{"s_aR",0.4f},{"s_arpOn",1},{"s_arpMode",2},{"s_arpOct",2},{"s_arpRate",0.61f},{"s_assign",3},{"bbdMix",0.4f},{"bbdPing",1},{"bbdSync",6},{"revMix",0.4f},{"revDecay",0.7f},{"power",0.2f},{"output",-0.6f} });
}
void v18_Seq_Pluck (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",1},{"s_wave2",0},{"s_tune2",0.1f},{"s_lvl2",0.4f},{"s_ftype",5},{"s_lpf",0.5f},{"s_lpfRes",0.25f},{"s_fltEnv",0.5f},{"s_fD",0.22f},{"s_fS",0},{"s_aD",0.2f},{"s_aS",0},{"s_aR",0.2f},{"s_arpOn",1},{"s_arpRate",0.66f},{"s_arpGate",0.4f},{"bbdMix",0.3f},{"bbdSync",6},{"revMix",0.25f},{"power",0.2f},{"output",-2.1f} });
    setSteps (p, "2101121021011210");
    setSeq (p, { 60, 67, 72, 64, 71, 67, 76, 62, 69, 74, 65, 72, 67, 79, 64, 71 });
    setLane (p, 0, MdRevMix, 0.4f, 0, { 0.1f, 0.1f, 0.8f, 0.1f });
}
void v18_Ratchet_Pluck (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",1},{"s_wave2",0},{"s_tune2",0.1f},{"s_lvl2",0.4f},{"s_ftype",3},{"s_lpf",0.5f},{"s_lpfRes",0.35f},{"s_fltEnv",0.5f},{"s_fD",0.2f},{"s_fS",0},{"s_aD",0.18f},{"s_aS",0},{"s_aR",0.15f},{"s_arpOn",1},{"s_arpMode",4},{"s_arpOct",2},{"s_assign",0},{"s_arpRate",0.6f},{"s_arpGate",0.35f},{"s_arpChance",0.8f},{"revMix",0.3f},{"revDecay",0.6f},{"power",0.2f},{"output",-1} });
    setSteps (p, "2101112110121011", "1121142111211421");
    setLane (p, 0, MdRevMix, 0.5f, 0, { 0.1f, 0.1f, 0.4f, 0.1f, 0.9f });
}
void v18_Polymeter_Drift (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",1},{"s_lvl2",0.5f},{"s_tune2",7},{"s_ftype",5},{"s_lpf",0.45f},{"s_lpfRes",0.25f},{"s_aA",0.2f},{"s_aD",0.5f},{"s_aS",0.5f},{"s_aR",0.6f},{"s_arpOn",2},{"s_arpMode",0},{"s_arpOct",2},{"s_arpRate",0.52f},{"s_arpGate",0.8f},{"revMix",0.35f},{"revDecay",0.7f},{"revShimmer",0.15f},{"power",0.2f},{"output",-8.5f} });
    setLane (p, 0, MdLpf, 0.3f, 0.2f, { 0.2f, 0.7f, 0.4f, 0.9f, 0.3f });
    setLane (p, 1, MdPan, 0.5f, 0, { 0, 1, 0.5f, 0.2f, 0.8f, 0.4f, 0.9f });
    setLane (p, 2, MdReso, 0.2f, 0, { 0, 0.5f, 0, 1, 0, 0.3f, 0, 0.8f, 0.2f });
}
void v18_Bass_Sequence (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_wave2",1},{"s_oct2",1},{"s_lvl2",0.4f},{"s_ftype",4},{"s_lpf",0.34f},{"s_lpfRes",0.5f},{"s_fltEnv",0.5f},{"s_fD",0.2f},{"s_fS",0.05f},{"s_aD",0.3f},{"s_aS",0.4f},{"s_aR",0.1f},{"s_arpOn",1},{"s_assign",1},{"s_arpRate",0.66f},{"s_arpGate",0.55f},{"power",0.3f},{"compType",1},{"compAmt",0.35f},{"output",-4.1f} });
    setSteps (p, "2112112121121121");
    setSeq (p, { 33, 33, 45, 33, 36, 33, 48, 33, 31, 31, 43, 31, 38, 33, 45, 40 }, "0000100000001000");
    setLane (p, 0, MdLpf, 0.45f, 0.15f, { 0.3f, 0.5f, 0.8f, 0.4f, 0.6f, 0.3f, 0.9f, 0.5f });
}
void v18_Stutter_Sequencer (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",1},{"s_tune2",0.1f},{"s_lvl2",0.4f},{"s_ftype",5},{"s_lpf",0.55f},{"s_lpfRes",0.35f},{"s_fltEnv",0.4f},{"s_fD",0.25f},{"s_fS",0.1f},{"s_aD",0.3f},{"s_aS",0.3f},{"s_aR",0.2f},{"s_arpOn",1},{"s_arpMode",2},{"s_arpOct",1},{"s_assign",0},{"s_arpRate",0.62f},{"s_arpGate",0.6f},{"revMix",0.2f},{"revDecay",0.55f},{"power",0.15f},{"stutDiv",0},{"stutMode",0},{"stutFeel",0},{"stutLen",0.85f},{"stutDecay",0.15f},{"stutMix",1},{"output",0.3f} });
    setSteps (p, "1111111111111111");
    setLane (p, 0, MdStutOn, 1, 0, { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0 });
    setLane (p, 1, MdStutDiv, 1, 0, { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.67f, 0.67f, 0.5f, 0.5f, 0.5f, 0.5f, 0.83f, 0.67f, 0.5f, 0.5f });
    setLane (p, 2, MdStutMode, 1, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.33f, 0.67f, 0, 0 });
}
void v18_Wind_Machine (OrageAudioProcessor& p)
{
    S (p, { {"s_lvl1",0},{"s_noise",1},{"s_ftype",4},{"s_lpf",0.55f},{"s_lpfRes",0.5f},{"s_l4Rate",0.3f},{"s_l4Wave",4},{"s_l4Slew",0.5f},{"s_aA",0.6f},{"s_aS",1},{"s_aR",0.8f},{"revMix",0.5f},{"revDecay",0.8f},{"output",5.9f} });
    M (p, 1, MsLfo4, MdLpf, 0.12f);
    M (p, 2, MsPolyAT, MdLpf, 0.15f);
}
void v18_Metal_Rain (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",1},{"s_tune2",5.5f},{"s_ring",0.9f},{"s_fm",0.4f},{"s_lvl1",0.3f},{"s_ftype",3},{"s_lpf",0.75f},{"s_lpfRes",0.4f},{"s_l3Rate",0.5f},{"s_aD",0.3f},{"s_aS",0.2f},{"s_aR",0.5f},{"revMix",0.4f},{"revDecay",0.7f},{"revShimmer",0.5f},{"output",6} });
    M (p, 1, MsLfo3, MdPitch2, 0.4f);
}
void v18_Alien_Choir (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",2},{"s_fm",0.4f},{"s_tune2",7},{"s_lvl2",0.5f},{"s_ftype",5},{"s_semMorph",0.4f},{"s_lpf",0.6f},{"s_lpfRes",0.6f},{"s_aA",0.6f},{"s_aS",0.9f},{"s_aR",0.8f},{"revMix",0.5f},{"revDecay",0.85f},{"revSize",0.9f},{"revShimmer",0.35f},{"output",-5.9f} });
    M (p, 1, MsLfo1, MdMorph, 0.25f);
    M (p, 2, MsLfo1, MdFm, 0.15f);
}
void v18_Thunder_Drone (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",0},{"s_wave2",1},{"s_oct2",0},{"s_lvl2",0.5f},{"s_noise",0.3f},{"s_ftype",4},{"s_lpf",0.25f},{"s_lpfRes",0.4f},{"s_aA",0.5f},{"s_aS",1},{"s_aR",0.8f},{"power",0.6f},{"material",2},{"lightning",0.2f},{"revMix",0.5f},{"revDecay",0.9f},{"revDamp",0.6f},{"output",-18.8f} });
    M (p, 1, MsMac1, MdDrive, 0.3f);
}
void v18_Ring_Wobble (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_wave2",1},{"s_ring",0.8f},{"s_tune2",5.5f},{"s_lvl1",0.3f},{"s_ftype",3},{"s_lpf",0.7f},{"s_lpfRes",0.3f},{"s_l2Rate",0.3f},{"s_l2Wave",1},{"destroy",0.3f},{"dSync",7},{"revMix",0.25f},{"output",-2.5f} });
    M (p, 1, MsLfo2, MdPw2, 0.6f);
}
void v18_Sample_And_Hold_Cloud (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",0},{"s_lvl2",0.3f},{"s_tune2",7},{"s_ftype",5},{"s_lpf",0.5f},{"s_lpfRes",0.55f},{"s_l1Wave",4},{"s_l1Rate",0.45f},{"s_aA",0.4f},{"s_aS",0.9f},{"s_aR",0.7f},{"revMix",0.45f},{"revDecay",0.8f},{"bbdMix",0.35f},{"bbdSync",6},{"bbdFb",0.5f},{"bbdPing",1},{"output",-6.7f} });
    M (p, 1, MsLfo1, MdLpf, 0.2f);
    M (p, 2, MsLfo1, MdPitch, 0.04f);
}
void v18_Noise_Riser (OrageAudioProcessor& p)
{
    S (p, { {"s_lvl1",0.1f},{"s_noise",0.7f},{"s_ftype",4},{"s_lpf",0.2f},{"s_lpfRes",0.7f},{"s_fltEnv",0.9f},{"s_fA",0.95f},{"s_fD",0.9f},{"s_fS",1},{"s_aA",0.5f},{"s_aS",1},{"s_aR",0.5f},{"revMix",0.4f},{"revDecay",0.7f},{"output",4.3f} });
    M (p, 1, MsEnv, MdReso, 0.05f);
}
void v18_Dark_Drone (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",0},{"s_oct1",1},{"s_wave2",0},{"s_oct2",0},{"s_tune2",0.07f},{"s_lvl2",0.7f},{"s_ftype",4},{"s_lpf",0.28f},{"s_lpfRes",0.5f},{"s_fltEnv",0.1f},{"s_aA",0.8f},{"s_aS",1},{"s_aR",0.9f},{"s_l1Rate",0.06f},{"power",0.3f},{"revMix",0.55f},{"revDecay",0.95f},{"revDamp",0.7f},{"revSize",1},{"output",-12.9f} });
    M (p, 1, MsLfo1, MdLpf, 0.1f);
    M (p, 2, MsLfo1, MdPan, 0.3f);
}
void v18_Cosmic_Radio (OrageAudioProcessor& p)
{
    S (p, { {"s_wave1",2},{"s_wave2",1},{"s_ring",0.4f},{"s_fm",0.3f},{"s_tune2",11},{"s_ftype",3},{"s_lpf",0.65f},{"s_lpfRes",0.5f},{"s_l1Wave",4},{"s_l1Rate",0.55f},{"s_aA",0.3f},{"s_aS",0.9f},{"s_aR",0.6f},{"destroy",0.35f},{"dSync",7},{"dGrain",0.5f},{"revMix",0.45f},{"revDecay",0.8f},{"revShimmer",0.3f},{"output",-6.5f} });
    M (p, 1, MsLfo1, MdFm, 0.2f);
    M (p, 2, MsLfo1, MdRing, 0.2f);
}
#endif
const Factory kFactory[] = {
    { "Init", "", fInit },
#if ORAGE_SYNTH
    { "Velvet Dawn", "Pad", v18_Velvet_Dawn }, { "Glass Cathedral", "Pad", v18_Glass_Cathedral }, { "Ocean Floor", "Pad", v18_Ocean_Floor }, { "Tidal Choir", "Pad", v18_Tidal_Choir },
    { "Amber Strings", "Pad", v18_Amber_Strings }, { "Cold Aurora", "Pad", v18_Cold_Aurora }, { "Dust Bowl", "Pad", v18_Dust_Bowl }, { "Moon Sync", "Pad", v18_Moon_Sync },
    { "Warm Brass Pad", "Pad", v18_Warm_Brass_Pad }, { "Hollow Wind", "Pad", v18_Hollow_Wind }, { "Slow Motion Chord", "Pad", v18_Slow_Motion_Chord }, { "Giant Space", "Pad", v18_Giant_Space },
    { "Pulse Silk", "Pad", v18_Pulse_Silk }, { "Blue Hour", "Pad", v18_Blue_Hour }, { "Ladder Sub", "Bass", v18_Ladder_Sub }, { "Pro Pluck Bass", "Bass", v18_Pro_Pluck_Bass },
    { "Acid MS-10", "Bass", v18_Acid_MS_10 }, { "Reese Deep", "Bass", v18_Reese_Deep }, { "Growl FM", "Bass", v18_Growl_FM }, { "Fat Moog", "Bass", v18_Fat_Moog },
    { "Sync Snarl", "Bass", v18_Sync_Snarl }, { "Wobble Dub", "Bass", v18_Wobble_Dub }, { "Sub Drop", "Bass", v18_Sub_Drop }, { "Hoover Low", "Bass", v18_Hoover_Low },
    { "Fold Bass", "Bass", v18_Fold_Bass }, { "Square Bass 78", "Bass", v18_Square_Bass_78 }, { "Dirty Rubber", "Bass", v18_Dirty_Rubber }, { "Ladder Squelch", "Bass", v18_Ladder_Squelch },
    { "Seq Bass Line", "Bass", v18_Seq_Bass_Line }, { "Solo SEM", "Lead", v18_Solo_SEM }, { "Screaming MS-10", "Lead", v18_Screaming_MS_10 }, { "Ladder Sync Lead", "Lead", v18_Ladder_Sync_Lead },
    { "Soft Sine Lead", "Lead", v18_Soft_Sine_Lead }, { "Pulse Width Lead", "Lead", v18_Pulse_Width_Lead }, { "Wavefold Lead", "Lead", v18_Wavefold_Lead }, { "Unison Monster", "Lead", v18_Unison_Monster },
    { "FM Brass Lead", "Lead", v18_FM_Brass_Lead }, { "Whistle", "Lead", v18_Whistle }, { "Glide Mono", "Lead", v18_Glide_Mono }, { "Aftertouch Wail", "Lead", v18_Aftertouch_Wail },
    { "Hard Sync Sweep", "Lead", v18_Hard_Sync_Sweep }, { "Cry Baby", "Lead", v18_Cry_Baby }, { "Ring Lead", "Lead", v18_Ring_Lead }, { "Glass Pluck", "Pluck / Keys", v18_Glass_Pluck },
    { "Ladder Keys", "Pluck / Keys", v18_Ladder_Keys }, { "Marimba Ring", "Pluck / Keys", v18_Marimba_Ring }, { "FM E-Piano", "Pluck / Keys", v18_FM_E_Piano }, { "FM Bell", "Pluck / Keys", v18_FM_Bell },
    { "Electric Clav", "Pluck / Keys", v18_Electric_Clav }, { "Harpsichord", "Pluck / Keys", v18_Harpsichord }, { "Wurly Drive", "Pluck / Keys", v18_Wurly_Drive }, { "Kalimba", "Pluck / Keys", v18_Kalimba },
    { "Brass Stab", "Pluck / Keys", v18_Brass_Stab }, { "Vibes", "Pluck / Keys", v18_Vibes }, { "Pluck Pad Hybrid", "Pluck / Keys", v18_Pluck_Pad_Hybrid }, { "Trance Gate Arp", "Arp / Seq", v18_Trance_Gate_Arp },
    { "Arp Rain", "Arp / Seq", v18_Arp_Rain }, { "Acid Lanes", "Arp / Seq", v18_Acid_Lanes }, { "Pulse Climb", "Arp / Seq", v18_Pulse_Climb }, { "Random Dreams", "Arp / Seq", v18_Random_Dreams },
    { "Seq Pluck", "Arp / Seq", v18_Seq_Pluck }, { "Ratchet Pluck", "Arp / Seq", v18_Ratchet_Pluck }, { "Polymeter Drift", "Arp / Seq", v18_Polymeter_Drift }, { "Bass Sequence", "Arp / Seq", v18_Bass_Sequence },
    { "Stutter Sequencer", "Arp / Seq", v18_Stutter_Sequencer }, { "Wind Machine", "FX / Texture", v18_Wind_Machine }, { "Metal Rain", "FX / Texture", v18_Metal_Rain }, { "Alien Choir", "FX / Texture", v18_Alien_Choir },
    { "Thunder Drone", "FX / Texture", v18_Thunder_Drone }, { "Ring Wobble", "FX / Texture", v18_Ring_Wobble }, { "Sample And Hold Cloud", "FX / Texture", v18_Sample_And_Hold_Cloud }, { "Noise Riser", "FX / Texture", v18_Noise_Riser },
    { "Dark Drone", "FX / Texture", v18_Dark_Drone }, { "Cosmic Radio", "FX / Texture", v18_Cosmic_Radio },
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
    for (auto* prm : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (prm))
            if (mapIds.size() < kMaxMap) { mapIds.add (rp->paramID); mapParams.push_back (rp); }
    for (auto& c : mapCc) c.store (-1);
    for (auto& v : hwPos) v = -1.f;
    for (int i = 0; i < kMaxMap; ++i) { lastSetV[i] = -1.f; aCur[i] = aHw[i] = lastIn[i] = 0.f; caught[i] = anchored[i] = false; }
    resetMidiMap();
    if (const auto f = midiMapFile(); f.existsAsFile()) midiMapFromString (f.loadFileAsString());
    mapDirty.store (false);
}

// ---------------------------------------------------------------- MIDI learn universel
juce::File OrageAudioProcessor::midiMapFile() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("Foudre Audio").getChildFile (ORAGE_SYNTH ? "midimap-synth.txt" : "midimap-fx.txt");
}
int OrageAudioProcessor::defaultCcOf (const juce::String& id) const
{
    for (int k = 0; k < 8; ++k) if (id == "s_m" + juce::String (k + 1)) return 16 + k;      // macros 1..8 = CC 16..23 (macros de l'Hydrasynth)
    // Carte par defaut = table "Sorted by CC Number" de l'Hydrasynth (un CC = un seul potard ; l'apprentissage MIDI peut tout rediriger)
    static const std::map<juce::String, int> hydra = {
        { "s_noise", 3 }, { "s_glide", 5 }, { "s_master", 7 },
        { "bbdFb", 14 }, { "bbdTime", 15 },
        { "s_lvl1", 44 }, { "s_lvl2", 45 },
        { "s_drive", 50 }, { "s_key", 51 },
        { "bbdTone", 63 }, { "revDecay", 65 }, { "revDamp", 67 },
        { "s_lpfRes", 71 }, { "s_l1Rate", 72 }, { "s_l2Rate", 73 }, { "s_lpf", 74 }, { "s_l3Rate", 76 }, { "s_l4Rate", 78 },
        { "s_aA", 81 }, { "s_aD", 82 }, { "s_aS", 83 }, { "s_aR", 84 },       // ENV1 de l'Hydrasynth = enveloppe d'ampli
        { "s_fA", 85 }, { "s_fD", 86 }, { "s_fS", 87 }, { "s_fR", 88 },       // ENV2 = enveloppe de filtre
        { "revMix", 91 }, { "bbdMix", 92 }, { "s_uniDetune", 95 } };
    const auto it = hydra.find (id);
    return it != hydra.end() ? it->second : -1;
}
void OrageAudioProcessor::resetMidiMap()
{
    for (auto& c : mapCc) c.store (-1);
    for (int i = 0; i < mapIds.size(); ++i) mapCc[(size_t) i].store (defaultCcOf (mapIds[i]));
    for (int i = 0; i < kMaxMap; ++i) anchored[i] = caught[i] = false;
    learnIdx.store (-1); mapDirty.store (true);
}
void OrageAudioProcessor::clearMidiAssign (const juce::String& id)
{
    const int i = mapIds.indexOf (id); if (i < 0) return;
    mapCc[(size_t) i].store (-1); anchored[i] = caught[i] = false; learnIdx.store (-1); mapDirty.store (true);
}
void OrageAudioProcessor::defaultMidiAssign (const juce::String& id)
{
    const int i = mapIds.indexOf (id); if (i < 0) return;
    mapCc[(size_t) i].store (defaultCcOf (id)); anchored[i] = caught[i] = false; learnIdx.store (-1); mapDirty.store (true);
}
juce::String OrageAudioProcessor::midiMapToString() const
{
    juce::String s ("M" + juce::String (takeoverMode.load()));
    for (int i = 0; i < mapIds.size(); ++i)
    {
        const int cc = mapCc[(size_t) i].load();
        if (cc != defaultCcOf (mapIds[i])) s << ";" << mapIds[i] << "=" << cc;           // seulement les ecarts par rapport aux CC d'origine
    }
    return s;
}
void OrageAudioProcessor::midiMapFromString (const juce::String& str)
{
    resetMidiMap();
    juce::StringArray t; t.addTokens (str.trim(), ";", "");
    for (auto& tok : t)
    {
        if (tok.startsWith ("M") && ! tok.contains ("=")) { takeoverMode.store (juce::jlimit (0, 2, tok.substring (1).getIntValue())); continue; }
        const int eq = tok.indexOfChar ('=');
        if (eq <= 0) continue;
        const int i = mapIds.indexOf (tok.substring (0, eq));
        if (i >= 0) mapCc[(size_t) i].store (juce::jlimit (-1, 127, tok.substring (eq + 1).getIntValue()));
    }
    mapDirty.store (false);
}
void OrageAudioProcessor::flushMidiMapIfDirty()
{
    if (! mapDirty.exchange (false)) return;
    const auto f = midiMapFile();
    f.getParentDirectory().createDirectory();
    f.replaceWithText (midiMapToString());
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
    arp.prepare ((float) sampleRate); arp.setPattern (&arpPat);
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
    p.stutOn = b();  p.stutMode = n();  p.stutDiv = n();  p.stutFeel = n();  p.stutLen = f();  p.stutDecay = f();  p.stutPitch = f();  p.stutMix = f();
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
    arpParams.gate = f(); arpParams.swing = f(); arpParams.chance = f(); arpParams.free = b();
    arpParams.len = n() + 1;
    for (int k = 0; k < kLanes; ++k) { arpParams.laneLen[k] = n() + 1; s.lane[k].dst = n(); s.lane[k].amt = f(); s.lane[k].glide = f(); }
    s.fmDir = n(); s.fmRatio = n(); s.fmWave = n(); s.fmFine = f(); s.fmAtt = f(); s.fmDec = f(); s.fmEnvAmt = f(); s.fmVel = f(); s.fmAT = f();
    arpParams.seq = n() == 1; seqRec = b();
    bendRange = (float) (n() + 1);
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
        {   // controleurs MIDI : appliques AVANT la lecture des reglages, et une seule fois par bloc (derniere valeur) -> pas de bloc de retard, moins de messages vers l'hote
            int lastCcV[128]; bool seen[128] = {};
            for (const auto meta : midi)
            {
                const auto m = meta.getMessage();
                if (m.isController()) { const int c = m.getControllerNumber(); lastCcV[c] = m.getControllerValue(); seen[c] = true; }
            }
            for (int c = 0; c < 128; ++c) if (seen[c]) handleCC (c, lastCcV[c]);
        }
        synth.setParams (readSynth());
        {   // modulation globale (matrice, pistes) de l'arpegiateur : valeurs du bloc precedent
            const float* gm = synth.fxMod();
            arpParams.gate   = juce::jlimit (0.05f, 1.f, arpParams.gate + 0.5f * gm[39]);
            arpParams.swing  = juce::jlimit (0.f, 1.f, arpParams.swing + 0.5f * gm[40]);
            arpParams.chance = juce::jlimit (0.f, 1.f, arpParams.chance + 0.5f * gm[41]);
        }
        float* sl = buffer.getWritePointer (0);
        float* sr = nc > 1 ? buffer.getWritePointer (1) : buffer.getWritePointer (0);

        // ---- evenements du bloc : MIDI direct, ou arpegiateur (qui genere ses propres notes)
        evs.clear();
        const bool recActive = arpParams.seq && seqRec;                    // enregistrement pas a pas : les touches jouent normalement et remplissent les pas
        orage::ArpParams ap = arpParams; if (recActive) ap.on = 0;
        arp.setParams (ap);
        const bool arpOn = ap.on != 0;
        if (recActive != lastRec) { lastRec = recActive; synth.allNotesOff(); arp.allOff(); if (recActive) arpPat.recPos = 0; }
        if (ap.on != lastArpOn || ap.assign != lastAssign) { lastArpOn = ap.on; lastAssign = ap.assign; synth.allNotesOff(); arp.allOff(); }
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
            if (m.isProgramChange()) { pendingProgram.store (m.getProgramChangeNumber()); triggerAsyncUpdate(); continue; }
            if (m.isController() && m.getControllerNumber() == 64)           // pedale de sustain : les fins de note sont retenues tant qu'elle est enfoncee
            {
                const bool dn = m.getControllerValue() >= 64;
                if (pedal && ! dn)
                    for (int nt = 0; nt < 128; ++nt)
                        if (pedalHeld[nt]) { pedalHeld[nt] = false; keyDown[nt] = false; if (arpOn) arp.noteOff (nt); else evs.push_back ({ pos, 1, nt, 0.f }); }
                pedal = dn;
            }
            if (m.isNoteOn()) { keyDown[m.getNoteNumber()] = true; pedalHeld[m.getNoteNumber()] = false; }
            else if (m.isNoteOff() && pedal && keyDown[m.getNoteNumber()]) { pedalHeld[m.getNoteNumber()] = true; continue; }
            else if (m.isNoteOff()) keyDown[m.getNoteNumber()] = false;
            if (m.isController()) { lastMidiMsg.store ((2 << 16) | (m.getControllerNumber() << 8) | m.getControllerValue()); if (m.getControllerNumber() == 1) wheelShown.store (m.getControllerValue()); }
            else if (m.isNoteOn()) lastMidiMsg.store ((1 << 16) | (m.getNoteNumber() << 8) | m.getVelocity());
            else if (m.isPitchWheel()) lastMidiMsg.store ((3 << 16) | ((m.getPitchWheelValue() >> 7) << 8) | (m.getPitchWheelValue() & 127));
            else if (m.isChannelPressure() || m.isAftertouch()) lastMidiMsg.store ((4 << 16) | (m.getChannelPressureValue() & 127));
            if (m.isNoteOn())
            {
                if (chordLearn && learnN < 6) learnNotes[learnN++] = m.getNoteNumber();
                if (recActive)
                {
                    const int rp = juce::jlimit (0, orage::kArpSteps - 1, arpPat.recPos.load());
                    arpPat.seqNote[rp] = m.getNoteNumber(); arpPat.step[rp] = m.getFloatVelocity() >= 0.85f ? 2 : 1; arpPat.tie[rp] = 0;
                    arpPat.recPos = (rp + 1) % juce::jlimit (1, orage::kArpSteps, arpParams.len);
                }
                if (chordLearn) continue;                                    // en apprentissage, les touches ne jouent pas
                if (arpOn) arp.noteOn (m.getNoteNumber(), m.getFloatVelocity()); else evs.push_back ({ pos, 0, m.getNoteNumber(), m.getFloatVelocity() });
            }
            else if (m.isNoteOff())      { if (chordLearn) continue; if (arpOn) arp.noteOff (m.getNoteNumber()); else evs.push_back ({ pos, 1, m.getNoteNumber(), 0.f }); }
            else if (m.isAllNotesOff() || m.isAllSoundOff()) { pedal = false; for (int nt = 0; nt < 128; ++nt) keyDown[nt] = pedalHeld[nt] = false; arp.allOff(); evs.push_back ({ pos, 2, 0, 0.f }); }
            else if (m.isPitchWheel())   evs.push_back ({ pos, 3, 0, (float) (m.getPitchWheelValue() - 8192) / 8192.f * bendRange });
            else if (m.isAftertouch())   evs.push_back ({ pos, 5, m.getNoteNumber(), (float) m.getAfterTouchValue() / 127.f });
            else if (m.isChannelPressure()) evs.push_back ({ pos, 6, 0, (float) m.getChannelPressureValue() / 127.f });
            else if (m.isController() && m.getControllerNumber() != 1) {}                // deja traite en debut de bloc
            else if (m.isController() && m.getControllerNumber() == 1) { evs.push_back ({ pos, 4, 0, (float) m.getControllerValue() / 127.f }); }
        }
        {
            orage::Arp::Ev ae[256];
            const int ac = arp.process (total, arpStepHz (bpm) * std::pow (2.f, 2.f * synth.fxMod()[38]), ae, 256);
            arpStepShown.store (arp.currentStep());
            for (int k = 0; k < ac; ++k)
            {
                const int ps = juce::jlimit (0, total - 1, ae[k].pos);
                if (ae[k].lane >= 0) evs.push_back ({ ps, 10, ae[k].lane, ae[k].val });
                else evs.push_back ({ ps, ae[k].on ? (ae[k].stack ? 8 : 7) : 9, ae[k].note, ae[k].vel });
            }
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
                case 10: synth.setLane (e.a, e.v); break;
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
    state.setProperty ("bypass", (int) engine.bypassMask.load(), nullptr);
    state.setProperty ("presetName", presetName, nullptr);
    state.setProperty ("category", presetCategory, nullptr);
    state.setProperty ("hdr", headerScope.load(), nullptr);
    { juce::String c; for (int k = 0; k < chordN; ++k) c << chordIv[k] << ","; state.setProperty ("chord", c, nullptr); }
#if ORAGE_SYNTH
    {
        juce::String c;
        for (int k = 0; k < orage::kArpSteps; ++k) c << arpPat.step[k].load() << ":" << arpPat.ratchet[k].load() << ":" << arpPat.seqNote[k].load() << ":" << arpPat.tie[k].load() << ",";
        for (int l = 0; l < orage::kArpLanes; ++l) { c << "|"; for (int k = 0; k < orage::kArpSteps; ++k) c << juce::String (arpPat.lane[l][k].load(), 3) << ","; }
        state.setProperty ("arpPat", c, nullptr);
    }
#endif
    return state;
}

void OrageAudioProcessor::applyState (juce::ValueTree state)
{
    engine.loadPatch (state.getProperty ("patch", "").toString().toStdString());
    const auto rk = state.getProperty ("rack", "").toString();
    if (rk.isNotEmpty()) engine.loadRack (rk.toStdString()); else engine.setDefaultRack();
    engine.bypassMask.store ((uint32_t) (int) state.getProperty ("bypass", 0));
    presetName = state.getProperty ("presetName", "Init").toString();
    presetCategory = state.getProperty ("category", "").toString();
    headerScope.store ((int) state.getProperty ("hdr", 0));
    {
        const auto cs = state.getProperty ("chord", "").toString();
        if (cs.isNotEmpty()) { juce::StringArray t; t.addTokens (cs, ",", ""); t.removeEmptyStrings(); chordN = 0; for (auto& x : t) if (chordN < 6) chordIv[chordN++] = x.getIntValue(); }
        else { const int d[4] { 4, 7, 11, 0 }; chordN = 3; for (int k = 0; k < 3; ++k) chordIv[k] = d[k]; }
    }
#if ORAGE_SYNTH
    {
        arpPat.reset();
        juce::StringArray parts; parts.addTokens (state.getProperty ("arpPat", "").toString(), "|", "");
        if (parts.size() > 0)
        {
            juce::StringArray t; t.addTokens (parts[0], ",", ""); t.removeEmptyStrings();
            for (int k = 0; k < orage::kArpSteps && k < t.size(); ++k)
            {
                juce::StringArray f; f.addTokens (t[k], ":", "");
                arpPat.step[k] = juce::jlimit (0, 2, f[0].getIntValue()); arpPat.ratchet[k] = juce::jlimit (1, 4, f[1].getIntValue());
                arpPat.seqNote[k] = f.size() > 2 ? juce::jlimit (0, 127, f[2].getIntValue()) : 60; arpPat.tie[k] = f.size() > 3 ? juce::jlimit (0, 1, f[3].getIntValue()) : 0;
            }
            for (int l = 0; l < orage::kArpLanes && l + 1 < parts.size(); ++l)
            {
                juce::StringArray v; v.addTokens (parts[l + 1], ",", ""); v.removeEmptyStrings();
                for (int k = 0; k < orage::kArpSteps && k < v.size(); ++k) arpPat.lane[l][k] = juce::jlimit (0.f, 1.f, v[k].getFloatValue());
            }
        }
    }
#endif
    apvts.replaceState (state);
}

void OrageAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto st = captureState();
    st.setProperty ("midiMap", midiMapToString(), nullptr);                // assignations MIDI : avec le projet, pas avec les presets
    if (auto xml = st.createXml()) copyXmlToBinary (*xml, dest);
}

void OrageAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            const auto vt = juce::ValueTree::fromXml (*xml);
            applyState (vt);
            if (vt.hasProperty ("midiMap")) midiMapFromString (vt.getProperty ("midiMap").toString());
            else if (vt.hasProperty ("macroCC"))                                // projets d'une ancienne version : CC des macros sauves a part
            {
                juce::StringArray t; t.addTokens (vt.getProperty ("macroCC").toString(), ",", ""); t.removeEmptyStrings();
                resetMidiMap();
                const bool oldDefault = vt.getProperty ("macroCC").toString().startsWith ("21,22,23,24,25,26,27,28");     // ancien reglage d'usine : on prend les CC 16..23
                for (int k = 0; k < 8 && k < t.size() && ! oldDefault; ++k)
                    if (const int i = mapIds.indexOf ("s_m" + juce::String (k + 1)); i >= 0) mapCc[(size_t) i].store (t[k].getIntValue());
            }
        }
}

void OrageAudioProcessor::setParamReal (const char* id, float real)
{
    if (auto* prm = apvts.getParameter (id)) prm->setValueNotifyingHost (prm->convertTo0to1 (real));
}

#if ORAGE_SYNTH
int OrageAudioProcessor::modDstOfParam (const juce::String& id)
{
    struct E { const char* id; int dst; };
    static const E tab[] = {
        { "s_pw1", MdPw1 }, { "s_pw2", MdPw2 }, { "s_fm", MdFm }, { "s_ring", MdRing }, { "s_lvl2", MdMix }, { "s_lvl1", MdLvl1 }, { "s_noise", MdNoise },
        { "s_hpf", MdHpf }, { "s_hpfRes", MdHpfReso }, { "s_lpf", MdLpf }, { "s_lpfRes", MdReso }, { "s_fltEnv", MdFltEnv }, { "s_drive", MdDrive }, { "s_semMorph", MdMorph },
        { "s_fold", MdFold }, { "s_foldSym", MdFoldSym }, { "s_glide", MdGlide }, { "s_key", MdKeyTrack },
        { "s_aA", MdAmpA }, { "s_aD", MdAmpD }, { "s_aS", MdAmpS }, { "s_aR", MdAmpR }, { "s_fA", MdFltA }, { "s_fD", MdFltD }, { "s_fS", MdFltS }, { "s_fR", MdFltR },
        { "s_arpRate", MdArpRate }, { "s_arpGate", MdArpGate }, { "s_arpSwing", MdArpSwing }, { "s_arpChance", MdArpChance },
        { "s_l1Rate", MdLfo1Rate }, { "s_l2Rate", MdLfo2Rate }, { "s_l3Rate", MdLfo3Rate }, { "s_l4Rate", MdLfo4Rate },
        { "power", MdFxPower }, { "clip", MdFxClip }, { "unc", MdFxUnc }, { "hpf", MdFxHpf }, { "lpf", MdFxLpf }, { "destroy", MdFxDestroy }, { "dRate", MdFxRate },
        { "springMix", MdFxSpring }, { "springDecay", MdSprDecay }, { "mix", MdFxMix }, { "chaos", MdFxChaos }, { "compAmt", MdCompAmt }, { "gateThresh", MdGateThresh }, { "octAmt", MdOctAmt },
        { "low", MdColLow }, { "mid", MdColMid }, { "high", MdColHigh },
        { "bbdTime", MdBbdTime }, { "bbdFb", MdBbdFb }, { "bbdMix", MdBbdMix }, { "bbdTone", MdBbdTone }, { "bbdMod", MdBbdWobble },
        { "revMix", MdRevMix }, { "revDecay", MdRevDecay }, { "revDamp", MdRevDamp }, { "revSize", MdRevSize }, { "revPre", MdRevPre }, { "revShimmer", MdRevShimmer }, { "revWidth", MdRevWidth },
        { "stutLen", MdStutLen }, { "stutDecay", MdStutDecay }, { "stutPitch", MdStutPitch }, { "stutMix", MdStutMix } };
    for (const auto& e : tab) if (id == e.id) return e.dst;
    return 0;
}

float OrageAudioProcessor::modScaleOfDst (int)          // 100 % = tour complet du potard (la matrice compense l'echelle native du moteur, cf. modNative)
{
    return 1.f;
}

int OrageAudioProcessor::knobMods (int dst, KnobMod out[2]) const
{
    int n = 0;
    if (dst <= 0) return 0;
    for (int i = 1; i <= kSlots && n < 2; ++i)
    {
        const juce::String q ("s_slot" + juce::String (i));
        const int d = juce::roundToInt (apvts.getRawParameterValue (q + "Dst")->load()), s = juce::roundToInt (apvts.getRawParameterValue (q + "Src")->load());
        if (d == dst && s > 0) out[n++] = { i, s, apvts.getRawParameterValue (q + "Amt")->load() };
    }
    return n;
}

int OrageAudioProcessor::assignKnobMod (int dst, int src)
{
    KnobMod cur[2]; const int n = knobMods (dst, cur);
    for (int k = 0; k < n; ++k) if (cur[k].src == src)                                // deja routee : on la reutilise (et on lui donne une quantite si elle etait a zero, sinon rien ne s'entend)
    {
        if (std::fabs (cur[k].amt) < 0.01f) setKnobModAmt (cur[k].slot, 0.5f);
        return cur[k].slot;
    }
    if (n >= 2) return -2;
    for (int i = 1; i <= kSlots; ++i)                                                 // premier slot libre
    {
        const juce::String q ("s_slot" + juce::String (i));
        if (juce::roundToInt (apvts.getRawParameterValue (q + "Src")->load()) == 0 && juce::roundToInt (apvts.getRawParameterValue (q + "Dst")->load()) == 0)
        {
            setParamReal ((q + "Src").toRawUTF8(), (float) src); setParamReal ((q + "Dst").toRawUTF8(), (float) dst); setParamReal ((q + "Amt").toRawUTF8(), 0.5f);
            return i;
        }
    }
    return -1;
}

void OrageAudioProcessor::setKnobModAmt (int slot, float amt) { setParamReal (("s_slot" + juce::String (slot) + "Amt").toRawUTF8(), juce::jlimit (-1.f, 1.f, amt)); }

void OrageAudioProcessor::clearKnobMod (int slot)
{
    const juce::String q ("s_slot" + juce::String (slot));
    setParamReal ((q + "Src").toRawUTF8(), 0.f); setParamReal ((q + "Dst").toRawUTF8(), 0.f); setParamReal ((q + "Amt").toRawUTF8(), 0.f);
}

float OrageAudioProcessor::modSourceLive (int src) const
{
    for (int i = 0; i < kMacros; ++i) if (macroSrc (i) == src) return apvts.getRawParameterValue ("s_m" + juce::String (i + 1))->load();
    if (src == MsWheel) return (float) wheelShown.load() / 127.f;
    for (int i = 0; i < kLfos; ++i) if (src == MsLfo1 + i) return synth.lfoLive (i);
    return -99.f;                                                              // inconnue : l'anneau reste fixe
}
#endif

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
#if ORAGE_SYNTH
    arpPat.reset();
#endif
    engine.clearPatch();
    engine.bypassMask.store (0);
    if (i == 0) engine.setRack ({}); else engine.setDefaultRack();      // Init = rack vide ; les autres presets utilisent tous les modules
    if (i != 0 && juce::String (kFactory[i].category) == "Rack FX") legacyFxDefaults (*this);
    kFactory[i].apply (*this);
#if ORAGE_SYNTH
    if (i != 0) applyMacroSet (*this, kFactory[i].category);
#endif
    presetName = kFactory[i].name;
    presetCategory = kFactory[i].category;
}

// CC MIDI -> parametres assignes (macros de l'Hydrasynth, ou tout potard appris par clic droit). Retourne true si le CC a ete consomme.
bool OrageAudioProcessor::handleCC (int cc, int value)
{
    if (cc < 0 || cc > 127) return false;
    const float v = (float) value / 127.f;
    const int ln = learnIdx.load();
    if (ln >= 0 && ln < mapIds.size() && cc != 0 && cc != 32 && cc != 64 && cc < 120)
    { mapCc[(size_t) ln].store (cc); learnIdx.store (-1); mapDirty.store (true); }
    const float prevHw = hwPos[cc];
    hwPos[cc] = v;
    bool used = false;
    const int n = mapIds.size();
    for (int i = 0; i < n; ++i)
        if (mapCc[(size_t) i].load() == cc) { used = true; applyMapped (i, v, prevHw); }
    return used;
}

// Reprise de valeur : l'Hydrasynth n'envoie sa position qu'en bougeant. SCALE : la valeur de depart du plugin est conservee et rejoint
// progressivement celle du potard (pas de saut, pas de course a la valeur) ; PICKUP : rien ne bouge tant que le potard n'a pas croise la valeur ; DIRECT : saute.
void OrageAudioProcessor::applyMapped (int i, float v, float prevHw)
{
    auto* prm = mapParams[(size_t) i];
    const float cur = prm->getValue();
    const int mode = takeoverMode.load();
    float out = v;
    if (prm->isDiscrete() || mode == 2) { anchored[i] = caught[i] = false; }
    else
    {
        const bool external = lastSetV[i] < 0.f || std::fabs (cur - lastSetV[i]) > 0.004f;      // valeur changee ailleurs (preset, souris, automation)
        if (mode == 1)
        {
            if (external) { caught[i] = false; lastIn[i] = prevHw >= 0.f ? prevHw : v; }
            if (! caught[i])
            {
                if (std::fabs (v - cur) < 0.02f || (lastIn[i] - cur) * (v - cur) <= 0.f) caught[i] = true;
                lastIn[i] = v;
                if (! caught[i]) return;
            }
        }
        else
        {
            if (external || ! anchored[i]) { anchored[i] = true; aCur[i] = cur; aHw[i] = prevHw >= 0.f ? prevHw : v; }
            if (v >= aHw[i]) out = aCur[i] + (v - aHw[i]) * (1.f - aCur[i]) / std::max (1.f - aHw[i], 1e-4f);
            else             out = aCur[i] - (aHw[i] - v) * aCur[i] / std::max (aHw[i], 1e-4f);
            out = juce::jlimit (0.f, 1.f, out);
        }
    }
    prm->setValueNotifyingHost (out);
    lastSetV[i] = prm->getValue();
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
