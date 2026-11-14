// Orage synth engine : 6 voix, style Korg MS20-M (2 VCO, hard sync, FM, PWM, ring, filtres HPF/LPF resonants)
// + 4 LFO parametrables (facon Serum), 4 macros, matrice de modulation. Aucune dependance JUCE.
#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>

namespace orage
{
constexpr int kVoices = 6, kLfos = 4, kMacros = 8, kSlots = 16, kLanes = 8;
enum Assign { AsPoly, AsMono, AsUnison, AsShare, AsChord };
enum ModSrc { MsNone, MsLfo1, MsLfo2, MsLfo3, MsLfo4, MsMac1, MsMac2, MsMac3, MsMac4, MsEnv, MsVel, MsWheel, MsAmpEnv, MsKey, MsRandom, MsChanAT, MsPolyAT, MsMac5, MsMac6, MsMac7, MsMac8, MsArp1, MsArp2, MsArp3, MsArp4, MsArp5, MsArp6, MsArp7, MsArp8, kNumModSrc };
inline const float kFmRatios[12] { 1.f, 0.5f, 2.f / 3.f, 1.f, 1.5f, 2.f, 3.f, 4.f, 5.f, 7.f, 3.5f, 1.41421356f };
inline int macroSrc (int i) { return i < 4 ? MsMac1 + i : MsMac5 + (i - 4); }
inline int laneSrc (int i)  { return i < 4 ? MsArp1 + i : MsArp5 + (i - 4); }
enum ModDst { MdNone, MdPitch, MdPitch2, MdPw1, MdPw2, MdFm, MdRing, MdMix, MdHpf, MdLpf, MdReso, MdLevel, MdPan,
              MdFold, MdFxPower, MdFxClip, MdFxUnc, MdFxHpf, MdFxLpf, MdFxPeak, MdFxDestroy, MdFxRate, MdFxSpring, MdFxMix, MdFxChaos, MdBbdTime, MdBbdFb, MdBbdMix,
              MdRevMix, MdRevDecay, MdRevDamp,
              MdNoise, MdHpfReso, MdDrive, MdMorph, MdFltEnv, MdFoldSym, MdLvl1,
              MdStutOn, MdStutMode, MdStutDiv, MdStutFeel, MdStutLen, MdStutDecay, MdStutPitch, MdStutMix,
              MdAmpA, MdAmpD, MdAmpS, MdAmpR, MdFltA, MdFltD, MdFltS, MdFltR, MdGlide, MdKeyTrack,                              // voix : enveloppes, portamento, key tracking
              MdRevSize, MdRevPre, MdRevShimmer, MdRevWidth, MdBbdTone, MdBbdWobble, MdSprDecay, MdCompAmt, MdGateThresh, MdOctAmt, MdColLow, MdColMid, MdColHigh,   // effets (entrees 25..37 du moteur)
              MdArpRate, MdArpGate, MdArpSwing, MdArpChance, MdLfo1Rate, MdLfo2Rate, MdLfo3Rate, MdLfo4Rate,                // globaux du synthe (entrees 38..45)
              kNumModDst };
constexpr int kFxMod = 46;                // destinations FX : MdFxPower .. MdRevDamp (les 10 premieres = les entrees CV du moteur d'effets)
inline const char* const kModSrcNames[kNumModSrc] { "-", "LFO 1", "LFO 2", "LFO 3", "LFO 4", "MACRO 1", "MACRO 2", "MACRO 3", "MACRO 4", "ENV FLT", "VELOCITY", "MOD WHEEL", "ENV AMP", "KEY TRACK", "RANDOM", "AFTERTOUCH", "POLY AT", "MACRO 5", "MACRO 6", "MACRO 7", "MACRO 8", "ARP LANE 1", "ARP LANE 2", "ARP LANE 3", "ARP LANE 4", "ARP LANE 5", "ARP LANE 6", "ARP LANE 7", "ARP LANE 8" };
inline const char* const kModDstNames[kNumModDst] { "-", "PITCH", "PITCH VCO2", "PWM VCO1", "PWM VCO2", "FM", "RING", "MIX VCO2", "HPF", "LPF", "RESO", "LEVEL", "PAN",
    "FOLD", "FX DRIVE", "FX CLIP", "FX UNCERT.", "FX HPF", "FX LPF", "FX PEAK", "FX DESTROY", "FX D.RATE", "FX SPRING", "FX DRY/WET", "FX CHAOS", "BBD TIME", "BBD FEEDBK", "BBD MIX",
    "REV MIX", "REV DECAY", "REV DAMP", "NOISE", "HPF RESO", "DRIVE", "SEM MORPH", "FLT ENV AMT", "FOLD SYM", "LEVEL VCO1",
    "STUT ENGAGE", "STUT MODE", "STUT DIV", "STUT FEEL", "STUT LENGTH", "STUT DECAY", "STUT PITCH", "STUT MIX",
    "AMP ATTACK", "AMP DECAY", "AMP SUSTAIN", "AMP RELEASE", "FLT ATTACK", "FLT DECAY", "FLT SUSTAIN", "FLT RELEASE", "PORTAMENTO", "KEY TRACK",
    "REV SIZE", "REV PRE-DELAY", "REV SHIMMER", "REV WIDTH", "BBD TONE", "BBD WOBBLE", "SPRING DECAY", "COMP AMOUNT", "GATE THRESH", "OCTAVER DOSE", "COLOR LOW", "COLOR MID", "COLOR HIGH",
    "ARP RATE", "ARP GATE", "ARP SWING", "ARP CHANCE", "LFO 1 RATE", "LFO 2 RATE", "LFO 3 RATE", "LFO 4 RATE" };
inline const char* const kLfoDivNames[14] { "Free", "4 bar", "2 bar", "1 bar", "1/2", "1/4", "1/8", "1/16", "1/4.", "1/8.", "1/16.", "1/4T", "1/8T", "1/16T" };
inline const float kLfoDivBeats[14] { 0.f, 16.f, 8.f, 4.f, 2.f, 1.f, 0.5f, 0.25f, 1.5f, 0.75f, 0.375f, 2.f / 3.f, 1.f / 3.f, 1.f / 6.f };

struct SynthParams
{
    int wave1 = 0, wave2 = 0;                 // 0 saw, 1 pulse, 2 triangle
    int oct1 = 0, oct2 = 0;                   // -2..+2
    float tune2 = 0.f, fine2 = 0.f;           // demi-tons, cents
    float pw1 = 0.5f, pw2 = 0.5f;
    int sync = 0;                             // hard sync : VCO2 recale par VCO1
    float fm = 0.f, ring = 0.f;               // 0..1
    float lvl1 = 0.8f, lvl2 = 0.f, noise = 0.f;
    float glide = 0.f;
    float hpf = 0.f, hpfRes = 0.f, lpf = 0.7f, lpfRes = 0.25f, fltEnv = 0.3f, keytrack = 0.5f, drive = 0.2f;
    float aA = 0.01f, aD = 0.3f, aS = 0.8f, aR = 0.25f;
    float fA = 0.01f, fD = 0.35f, fS = 0.2f, fR = 0.25f;
    int assign = 0, hold = 0;                 // affectation des touches (Poly, Mono, Unison, Share, Chord) et HOLD
    int fmDir = 0, fmRatio = 0, fmWave = 0;   // FM : sens (0 VCO2>VCO1, 1 VCO1>VCO2, 2 croisee), rapport de frequence (0 libre), onde du modulateur (0 = celle du VCO)
    float fmFine = 0.f, fmAtt = 0.f, fmDec = 0.4f, fmEnvAmt = 0.f, fmVel = 0.f, fmAT = 0.f;     // FM : desaccord fin, enveloppe de profondeur, vitesse, aftertouch
    float uniDetune = 0.3f;                   // ecart de desaccord des voix empilees
    int ftype = 0;                            // filtre passe-bas : 0 MS20, 1 SEM, 2 Ladder
    float semMorph = 0.f;                     // SEM : LP -> notch -> HP
    float fold = 0.f, foldSym = 0.f;           // wavefolder : quantite, symetrie
    float master = 0.7f;
    struct Lfo { float rate = 0.4f, slew = 0.f, fade = 0.f; int wave = 0, sync = 0, trig = 0; } lfo[kLfos];     // wave : 0 sin, 1 tri, 2 saw, 3 carre, 4 S&H
    float macro[kMacros] {};
    struct Slot { int src = 0, dst = 0; float amt = 0.f; } slot[kSlots];
    struct Lane { int dst = 0; float amt = 0.f, glide = 0.f; } lane[kLanes];      // pistes de modulation de l'arpegiateur : destination directe (sans passer par la matrice)
};

namespace sy
{
inline float blep (float t, float dt)
{
    if (t < dt) { t /= dt; return t + t - t * t - 1.f; }
    if (t > 1.f - dt) { t = (t - 1.f) / dt; return t * t + t + t + 1.f; }
    return 0.f;
}
inline float osc (int wave, float ph, float dt, float pw)
{
    switch (wave)
    {
        case 0: return 2.f * ph - 1.f - blep (ph, dt);
        case 1: { float t2 = ph - pw; if (t2 < 0.f) t2 += 1.f; return (ph < pw ? 1.f : -1.f) + blep (ph, dt) - blep (t2, dt); }
        default: return 4.f * std::fabs (ph - 0.5f) - 1.f;
    }
}
struct Env
{
    enum { Idle, Att, Dec, Sus, Rel } st = Idle;
    float v = 0.f;
    void on()  { st = Att; }
    void off() { if (st != Idle) st = Rel; }
    float tick (float a, float d, float s, float r, float fs)
    {
        auto coef = [fs] (float sec) { return 1.f - std::exp (-1.f / (juce_max (sec, 0.0005f) * fs)); };
        switch (st)
        {
            case Att: v += (1.4f - v) * coef (a * 0.45f + 0.0005f); if (v >= 1.f) { v = 1.f; st = Dec; } break;
            case Dec: v += (s - v) * coef (d * 0.5f); if (std::fabs (v - s) < 0.0005f) { v = s; st = Sus; } break;
            case Sus: v = s; break;
            case Rel: v += (0.f - v) * coef (r * 0.5f); if (v < 0.0003f) { v = 0.f; st = Idle; } break;
            default: break;
        }
        return v;
    }
    static float juce_max (float a, float b) { return a > b ? a : b; }
};
struct Svf
{
    float s1 = 0.f, s2 = 0.f;
    // renvoie lp, hp via references
    void run (float x, float g, float k, float& lp, float& hp)
    {
        const float a1 = 1.f / (1.f + g * (g + k)), a2 = g * a1, a3 = g * a2;
        const float v3 = x - s2, v1 = a1 * s1 + a2 * v3, v2 = s2 + a2 * s1 + a3 * v3;
        s1 = 2.f * v1 - s1;
        s1 = s1 / (1.f + 0.25f * s1 * s1 * 0.25f * 4.f);          // saturation douce de la bande passante : l'auto-oscillation se stabilise
        s2 = 2.f * v2 - s2;
        lp = v2; hp = x - k * v1 - v2;
    }
};
// ---- HPF MS20 : le pic de resonance reste libre (peu de saturation), donc il gonfle les basses quand le cutoff est bas
struct Hp
{
    float s1 = 0.f, s2 = 0.f;
    float run (float x, float g, float k)
    {
        const float a1 = 1.f / (1.f + g * (g + k)), a2 = g * a1, a3 = g * a2;
        const float v3 = x - s2, v1 = a1 * s1 + a2 * v3, v2 = s2 + a2 * s1 + a3 * v3;
        s1 = 2.f * v1 - s1;
        s1 = s1 / (1.f + 0.02f * s1 * s1);
        s2 = 2.f * v2 - s2;
        return x - k * v1 - v2;
    }
};
// ---- LPF MS20 : resonance qui crie (saturation asymetrique dans la boucle + emphase de la bande)
struct Lp
{
    float s1 = 0.f, s2 = 0.f;
    float run (float x, float g, float k, float res)
    {
        const float a1 = 1.f / (1.f + g * (g + k)), a2 = g * a1, a3 = g * a2;
        const float v3 = x - s2, v1 = a1 * s1 + a2 * v3, v2 = s2 + a2 * s1 + a3 * v3;
        s1 = 2.f * v1 - s1;
        const float wild = 0.10f + 0.9f * res * res;
        s1 = s1 > 0.f ? s1 / (1.f + 0.10f * s1 * s1) : s1 / (1.f + wild * 0.6f * s1 * s1);   // diode : les deux alternances ne saturent pas pareil
        s2 = 2.f * v2 - s2;
        return v2 + res * res * 1.1f * v1;                                                  // la bande passante est ajoutee a la sortie : le pic devient criard
    }
};
// ---- SEM : SVF 12 dB, resonance douce, morph LP -> notch -> HP
struct Sem
{
    float s1 = 0.f, s2 = 0.f;
    float run (float x, float g, float k, float morph)
    {
        const float a1 = 1.f / (1.f + g * (g + k)), a2 = g * a1, a3 = g * a2;
        const float v3 = x - s2, v1 = a1 * s1 + a2 * v3, v2 = s2 + a2 * s1 + a3 * v3;
        s1 = 2.f * v1 - s1;
        s1 = s1 / (1.f + 0.12f * s1 * s1);
        s2 = 2.f * v2 - s2;
        const float lp = v2, hp = x - k * v1 - v2;
        const float lg = std::min (1.f, std::max (0.f, 2.f * (1.f - morph))), hg = std::min (1.f, std::max (0.f, 2.f * morph));
        return lp * lg + hp * hg;
    }
};
// ---- Ladder 24 dB (4 poles TPT, retour instantane resolu, saturation a l'entree)
struct Ladder
{
    float s[4] {};
    float run (float x, float g, float k)
    {
        const float G = g / (1.f + g), H = 1.f - G;
        const float G2 = G * G, G3 = G2 * G, G4 = G2 * G2;
        const float S = G3 * H * s[0] + G2 * H * s[1] + G * H * s[2] + H * s[3];
        float u = (x - k * S) / (1.f + k * G4);
        u = std::tanh (u);
        float in = u;
        for (int i = 0; i < 4; ++i)
        {
            const float y = G * in + H * s[i];
            s[i] = 2.f * y - s[i];
            in = y;
        }
        return in;
    }
};
// ---- wavefolder (repli sinus) : 0 = transparent
inline float fold (float x, float amt, float sym)
{
    if (amt < 0.001f) return x;
    const float g = 1.f + 9.f * amt * amt;
    const float u = g * x + sym * 0.6f * amt;
    const float y = std::sin (u * 1.5707963f * 1.5f) * 0.9f;
    const float mixA = std::min (1.f, amt * 5.f);
    return x + (y - x) * mixA;
}
} // namespace sy

class Synth
{
public:
    void prepare (float sampleRate)
    {
        fs = sampleRate;
        for (auto& v : voices) v = Voice();
        for (int i = 0; i < kLfos; ++i) { gPhase[i] = 0.f; gSh[i] = 0.f; gCur[i] = 0.f; }
        counter = 0; seed = 12345u;
    }
    void setParams (const SynthParams& p)
    {
        const int oldAssign = P.assign; const bool oldHold = P.hold != 0;
        P = p;
        if (P.assign != oldAssign) allNotesOff();
        else if (oldHold && ! P.hold) dropLatched();
    }
    void setChord (const int* iv, int n) { chordN = std::max (0, std::min (6, n)); for (int i = 0; i < chordN; ++i) chord[i] = iv[i]; }
    float lfoHz (int i) const { return hz (i); }                       // cadence de l'arpegiateur = LFO 2
    void setHost (double bpmIn) { bpm = bpmIn > 20.0 ? (float) bpmIn : 120.f; }
    void setWheel (float w) { wheel = w; }
    void setBend (float semis) { bend = semis; }
    void setChanAT (float a) { chanAT = a; }
    void setPolyAT (int note, float a) { for (auto& v : voices) if (v.active && v.note == note) v.pat = a; }

    // ---- touches : l'interpretation depend du mode d'affectation (POLY, MONO, UNISON, SHARE, CHORD MEMORY) et de HOLD
    void noteOn (int note, float vel)
    {
        if (P.hold && physCount() == 0 && nKeys > 0) dropLatched();
        int k = findKey (note);
        if (k < 0 && nKeys < kKeys) k = nKeys++;
        if (k >= 0) keys[k] = { note, vel, true, ++stamp };
        if (P.assign == AsPoly) startVoice (note, vel, 0.f, 0.f, 1.f, -1, false);
        else retarget();
    }
    void noteOff (int note)
    {
        const int k = findKey (note);
        if (k < 0) { if (P.assign == AsPoly) releaseNote (note, false); return; }
        if (P.hold) { keys[k].phys = false; return; }
        for (int j = k; j + 1 < nKeys; ++j) keys[j] = keys[j + 1];
        --nKeys;
        if (P.assign == AsPoly) releaseNote (note, false); else retarget();
    }
    void allNotesOff() { nKeys = 0; for (auto& v : voices) if (v.active && v.gated) release (v); }
    // notes venant de l'arpegiateur : une voix par appel, ou une pile de voix desaccordees (mode UNISON)
    void noteOnRaw (int note, float vel) { startVoice (note, vel, 0.f, 0.f, 1.f, -1, true); }
    void noteOnStack (int note, float vel)
    {
        for (int i = 0; i < kVoices; ++i)
        {
            const float sp = 2.f * (float) i / (float) (kVoices - 1) - 1.f;
            startVoice (note, vel, sp * maxCents(), sp * 0.5f * std::min (1.f, P.uniDetune * 1.5f), 0.5f, -1, true);
        }
    }
    void noteOffRaw (int note) { releaseNote (note, true); }
    void setLane (int i, float v) { laneT[i] = v; }
    const float* fxMod() const { return fxOut; }                  // modulations globales des effets (matrice : destinations FX/BBD)
    int activeVoices() const { int n = 0; for (auto& v : voices) n += v.active ? 1 : 0; return n; }

    void process (float* L, float* R, int n)
    {
        for (int i = 0; i < n; ++i) { L[i] = 0.f; R[i] = 0.f; }
        // LFO globaux (voix sans retrig)
        const float lfoHz[kLfos] = { hz (0) * std::pow (2.f, 3.f * fxOut[42]), hz (1) * std::pow (2.f, 3.f * fxOut[43]), hz (2) * std::pow (2.f, 3.f * fxOut[44]), hz (3) * std::pow (2.f, 3.f * fxOut[45]) };   // vitesse des LFO modulable (matrice, +/-3 octaves)
        for (int pos = 0; pos < n; pos += kCtl)
        {
            const int len = std::min (kCtl, n - pos);
            const float dt = (float) len / fs;
            for (int i = 0; i < kLfos; ++i)
            {
                gPhase[i] += lfoHz[i] * dt;
                if (gPhase[i] >= 1.f) { gPhase[i] -= std::floor (gPhase[i]); gSh[i] = rnd(); }
                gCur[i] += (gSh[i] - gCur[i]) * slewCoef (P.lfo[i].slew, lfoHz[i], dt);          // S&H : marches lissees par SLEW
            }
            for (int i = 0; i < kLanes; ++i)                                      // pistes de l'arpegiateur : marches ou glissando selon GLIDE
            {
                const float sec = P.lane[i].glide * 0.6f;
                laneC[i] += (laneT[i] - laneC[i]) * (sec < 0.002f ? 1.f : 1.f - std::exp (-dt / (sec * 0.35f)));
            }
            for (auto& v : voices)
                if (v.active) renderVoice (v, L + pos, R + pos, len, lfoHz, dt);
        }
        for (int i = 0; i < n; ++i) { L[i] = std::tanh (L[i] * P.master * 0.7f); R[i] = std::tanh (R[i] * P.master * 0.7f); }
        computeFxMod();
    }

private:
    static constexpr int kCtl = 16;
    struct Voice
    {
        bool active = false; int note = 60; float vel = 1.f; uint64_t age = 0;
        bool gated = false, raw = false; int slot = -1; float uniCents = 0.f, uniPan = 0.f, gainV = 1.f;
        sy::Env aenv, fenv; float pitch = 60.f, ph1 = 0.f, ph2 = 0.f, prev1 = 0.f, prev2 = 0.f, fmT = 0.f;
        sy::Hp hp; sy::Lp lp; sy::Sem sem; sy::Ladder lad; float pat = 0.f, patS = 0.f, rndv = 0.f; float xPrev = 0.f, drvLp = 0.f; float lph[kLfos] {}, lfade[kLfos] {}, lsh[kLfos] {}, lcur[kLfos] {}; float envM[8] {};     // envM : modulation des enveloppes (bloc precedent)
    };
    static constexpr int kKeys = 16;
    struct Key { int note; float vel; bool phys; uint64_t t; };
    Key keys[kKeys];
    int nKeys = 0, chordN = 4, chord[6] { 0, 4, 7, 11, 0, 0 };
    uint64_t stamp = 0;
    Voice voices[kVoices];
    SynthParams P;
    float fs = 44100.f, bpm = 120.f, wheel = 0.f, bend = 0.f, gPhase[kLfos] {}, gSh[kLfos] {}, gCur[kLfos] {};
    float laneT[kLanes] {}, laneC[kLanes] {};
    uint64_t counter = 0; int lastNote = -1; uint32_t seed = 12345u;
    float lastVel = 0.8f, fxOut[kFxMod] {}, chanAT = 0.f, lastRnd = 0.f, lastKey = 0.f;

    int findKey (int note) const { for (int i = 0; i < nKeys; ++i) if (keys[i].note == note) return i; return -1; }
    int physCount() const { int c = 0; for (int i = 0; i < nKeys; ++i) c += keys[i].phys ? 1 : 0; return c; }
    float maxCents() const { return 45.f * std::pow (std::max (0.f, std::min (1.f, P.uniDetune)), 1.2f); }
    static void release (Voice& v) { v.aenv.off(); v.fenv.off(); v.gated = false; }
    void releaseNote (int note, bool raw) { for (auto& v : voices) if (v.active && v.gated && v.note == note && v.raw == raw && v.slot < 0) release (v); }
    void dropLatched()                                                      // touches verrouillees (HOLD) : on les lache
    {
        int w = 0;
        for (int i = 0; i < nKeys; ++i)
        {
            if (keys[i].phys) keys[w++] = keys[i];
            else if (P.assign == AsPoly) releaseNote (keys[i].note, false);
        }
        nKeys = w;
        if (P.assign != AsPoly) retarget();
    }
    Voice* startVoice (int note, float vel, float cents, float pan, float gain, int slot, bool raw)
    {
        Voice* v = nullptr;
        if (slot >= 0) v = &voices[slot];
        else
        {
            for (auto& c : voices) if (! c.active) { v = &c; break; }
            if (v == nullptr)
            {
                float best = 1e9f;
                for (auto& c : voices)
                {
                    const float score = (c.gated ? 1000.f : 0.f) + c.aenv.v * 100.f + (float) (counter - c.age) * -1e-6f;
                    if (score < best) { best = score; v = &c; }
                }
            }
        }
        const bool glide = P.glide > 0.001f && lastNote >= 0;
        *v = Voice();
        v->active = true; v->gated = true; v->raw = raw; v->slot = slot; v->note = note; v->vel = vel; v->age = ++counter;
        v->uniCents = cents; v->uniPan = pan; v->gainV = gain;
        v->pitch = glide ? (float) lastNote : (float) note;
        v->aenv.on(); v->fenv.on(); lastVel = vel;
        v->rndv = rnd(); lastRnd = v->rndv; lastKey = std::max (-1.f, std::min (1.f, ((float) note - 60.f) / 48.f));
        for (int i = 0; i < kLfos; ++i) { v->lph[i] = 0.f; v->lfade[i] = 0.f; v->lsh[i] = rnd(); v->lcur[i] = v->lsh[i]; }
        lastNote = note;
        return v;
    }
    // MONO / UNISON / SHARE / CHORD : chaque voix a une note cible deduite des touches tenues
    void retarget()
    {
        int tgt[kVoices]; float cents[kVoices] {}, pan[kVoices] {};
        for (auto& t : tgt) t = -1;
        float gain = 1.f;
        int latest = -1; uint64_t lt = 0;
        for (int i = 0; i < nKeys; ++i) if (keys[i].t >= lt) { lt = keys[i].t; latest = i; }
        if (latest >= 0)
        {
            const int ln = keys[latest].note; const float mc = maxCents();
            if (P.assign == AsMono) tgt[0] = ln;
            else if (P.assign == AsUnison)
            {
                gain = 0.5f;
                for (int i = 0; i < kVoices; ++i) { const float sp = 2.f * (float) i / (float) (kVoices - 1) - 1.f; tgt[i] = ln; cents[i] = sp * mc; pan[i] = sp * 0.5f * std::min (1.f, P.uniDetune * 1.5f); }
            }
            else if (P.assign == AsShare)
            {
                gain = 0.6f;
                Key byTime[kKeys]; for (int i = 0; i < nKeys; ++i) byTime[i] = keys[i];
                std::sort (byTime, byTime + nKeys, [] (const Key& a, const Key& b) { return a.t < b.t; });
                const int k = nKeys;
                for (int i = 0; i < kVoices; ++i)
                {
                    tgt[i] = byTime[i % k].note;
                    const int group = (kVoices + k - 1 - (i % k)) / k;                  // nombre de voix sur cette note
                    const int j = i / k;
                    const float sp = group > 1 ? 2.f * (float) j / (float) (group - 1) - 1.f : 0.f;
                    cents[i] = sp * mc * 0.6f; pan[i] = sp * 0.3f;
                }
            }
            else if (P.assign == AsChord)
            {
                gain = 0.62f;
                const int n = std::max (1, chordN);
                for (int i = 0; i < n; ++i)
                {
                    tgt[i] = ln + (chordN > 0 ? chord[i] : 0);
                    cents[i] = ((i & 1) ? 1.f : -1.f) * mc * 0.15f;
                    pan[i] = n > 1 ? (2.f * (float) i / (float) (n - 1) - 1.f) * 0.35f : 0.f;
                }
            }
        }
        const bool legato = P.assign != AsShare;
        for (int i = 0; i < kVoices; ++i)
        {
            auto& v = voices[i];
            if (tgt[i] < 0) { if (v.active && v.gated && v.slot == i) release (v); continue; }
            if (v.active && v.gated && v.slot == i)
            {
                v.uniCents = cents[i]; v.uniPan = pan[i]; v.gainV = gain;
                if (v.note != tgt[i])
                {
                    if (legato) v.note = tgt[i];                                         // legato : pas de retrig, la voix glisse (portamento)
                    else { startVoice (tgt[i], keys[latest].vel, cents[i], pan[i], gain, i, false); }
                }
            }
            else startVoice (tgt[i], keys[latest].vel, cents[i], pan[i], gain, i, false);
        }
    }

    static int fxIndex (int dst)                                           // destination de matrice -> entree du moteur d'effets (-1 : pas une destination FX)
    {
        if (dst >= MdFxPower && dst <= MdRevDamp) return dst - MdFxPower;
        if (dst >= MdStutOn && dst <= MdStutMix) return 17 + (dst - MdStutOn);
        if (dst >= MdRevSize && dst <= MdColHigh) return 25 + (dst - MdRevSize);
        if (dst >= MdArpRate && dst <= MdLfo4Rate) return 38 + (dst - MdArpRate);
        return -1;
    }
    void computeFxMod()                                                    // sources globales : LFO libres, macros, molette, enveloppe de filtre (voix la plus forte), velocite
    {
        float src[kNumModSrc] {};
        for (int i = 0; i < kLfos; ++i) src[MsLfo1 + i] = lfoWave (gPhase[i], P.lfo[i].wave, gCur[i]);
        for (int i = 0; i < kMacros; ++i) src[macroSrc (i)] = P.macro[i];
        for (int i = 0; i < kLanes; ++i) src[laneSrc (i)] = laneC[i];
        float e = 0.f; for (auto& v : voices) if (v.active) e = std::max (e, v.fenv.v);
        src[MsEnv] = e; src[MsVel] = lastVel; src[MsWheel] = wheel;
        float ae = 0.f, pa = 0.f; for (auto& v : voices) if (v.active) { ae = std::max (ae, v.aenv.v); pa = std::max (pa, v.patS); }
        src[MsAmpEnv] = ae; src[MsKey] = lastKey; src[MsRandom] = lastRnd; src[MsChanAT] = chanAT; src[MsPolyAT] = std::max (pa, chanAT);
        float acc[kFxMod] {};
        for (const auto& sl : P.slot)
            if (sl.src > 0 && sl.amt != 0.f && fxIndex (sl.dst) >= 0) acc[fxIndex (sl.dst)] += src[sl.src] * sl.amt;
        for (int i = 0; i < kLanes; ++i)
            if (P.lane[i].amt != 0.f && fxIndex (P.lane[i].dst) >= 0) acc[fxIndex (P.lane[i].dst)] += laneC[i] * P.lane[i].amt;
        for (int i = 0; i < kFxMod; ++i) fxOut[i] = std::min (2.f, std::max (-2.f, acc[i]));
    }

    float rnd() { seed = seed * 1664525u + 1013904223u; return (float) (seed >> 8) / 8388608.f - 1.f; }

    float hz (int i) const
    {
        const auto& l = P.lfo[i];
        if (l.sync > 0 && l.sync < 14) return bpm / 60.f / kLfoDivBeats[l.sync];
        return 0.05f * std::pow (800.f, l.rate);                         // 0.05 .. 40 Hz
    }
public:
    static float lfoWave (float ph, int wave, float sh)
    {
        switch (wave)
        {
            case 0:  return std::sin (6.2831853f * ph);
            case 1:  return 4.f * std::fabs (ph - 0.5f) - 1.f;
            case 2:  return 2.f * ph - 1.f;
            case 3:  return ph < 0.5f ? 1.f : -1.f;
            default: return sh;
        }
    }
    static float slewCoef (float slew, float hzv, float dt)
    {
        if (slew < 0.001f) return 1.f;
        const float tau = std::max (1e-4f, slew * 0.5f / std::max (hzv, 0.01f));
        return 1.f - std::exp (-dt / tau);
    }

private:
    void renderVoice (Voice& v, float* L, float* R, int len, const float* lfoHzv, float dtCtl)
    {
        // ---- sources de modulation (une fois par bloc de controle)
        float src[kNumModSrc] {};
        for (int i = 0; i < kLfos; ++i)
        {
            const auto& l = P.lfo[i];
            float ph = gPhase[i], sh = gCur[i], fadeG = 1.f;
            if (l.trig)
            {
                v.lph[i] += lfoHzv[i] * dtCtl;
                if (v.lph[i] >= 1.f) { v.lph[i] -= std::floor (v.lph[i]); v.lsh[i] = rnd(); }
                v.lcur[i] += (v.lsh[i] - v.lcur[i]) * slewCoef (l.slew, lfoHzv[i], dtCtl);
                ph = v.lph[i]; sh = v.lcur[i];
                v.lfade[i] += dtCtl;
                const float ft = l.fade * 4.f;
                fadeG = ft > 0.001f ? std::min (1.f, v.lfade[i] / ft) : 1.f;
            }
            src[MsLfo1 + i] = lfoWave (ph, l.wave, sh) * fadeG;
        }
        for (int i = 0; i < kMacros; ++i) src[macroSrc (i)] = P.macro[i];
        for (int i = 0; i < kLanes; ++i) src[laneSrc (i)] = laneC[i];
        auto tm = [] (float t, float m) { return t * std::pow (4.f, m); };               // temps d'enveloppe : x4 / :4 a pleine modulation
        const float fe = v.fenv.tick (tm (P.fA, v.envM[4]), tm (P.fD, v.envM[5]), std::min (1.f, std::max (0.f, P.fS + v.envM[6])), tm (P.fR, v.envM[7]), fs / (float) kCtl);   // enveloppes cadencees au bloc
        src[MsEnv] = fe; src[MsVel] = v.vel; src[MsWheel] = wheel;
        src[MsAmpEnv] = v.aenv.v; src[MsKey] = std::max (-1.f, std::min (1.f, (v.pitch - 60.f) / 48.f)); src[MsRandom] = v.rndv;
        v.patS += (v.pat - v.patS) * 0.4f;
        src[MsChanAT] = chanAT; src[MsPolyAT] = std::max (v.patS, chanAT);
        float md[kNumModDst] {};
        for (const auto& s : P.slot)
            if (s.src > 0 && s.dst > 0 && s.amt != 0.f) md[s.dst] += src[s.src] * s.amt;
        for (int i = 0; i < kLanes; ++i)
            if (P.lane[i].dst > 0 && P.lane[i].amt != 0.f) md[P.lane[i].dst] += laneC[i] * P.lane[i].amt;

        for (int k = 0; k < 8; ++k) v.envM[k] = md[MdAmpA + k];
        const float aA2 = tm (P.aA, md[MdAmpA]), aD2 = tm (P.aD, md[MdAmpD]), aS2 = std::min (1.f, std::max (0.f, P.aS + md[MdAmpS])), aR2 = tm (P.aR, md[MdAmpR]);
        const float glideV = std::min (1.f, std::max (0.f, P.glide + md[MdGlide]));
        const float glideC = glideV > 0.001f ? 1.f - std::exp (-dtCtl / (glideV * 0.6f + 0.002f)) : 1.f;
        v.pitch += (((float) v.note) - v.pitch) * glideC;
        const float n1 = v.pitch + 12.f * (float) P.oct1 + bend + md[MdPitch] * 12.f + v.uniCents * 0.01f;
        const float n2 = v.pitch + 12.f * (float) P.oct2 + P.tune2 + P.fine2 * 0.01f + bend + md[MdPitch] * 12.f + md[MdPitch2] * 12.f + v.uniCents * 0.01f;
        // rapport de frequence entre les deux VCO (FM) : VCO2 suit VCO1 x rapport, la hauteur propre de VCO2 est ignoree
        const float n2r = P.fmRatio > 0 ? n1 + 12.f * std::log2 (kFmRatios[std::min (11, P.fmRatio)]) + P.fmFine * 0.5f + md[MdPitch2] * 12.f : n2 + P.fmFine * 0.5f;
        const float f1 = 440.f * std::pow (2.f, (n1 - 69.f) / 12.f), f2 = 440.f * std::pow (2.f, (n2r - 69.f) / 12.f);
        const float pw1 = std::min (0.95f, std::max (0.05f, P.pw1 + md[MdPw1] * 0.45f));
        const float pw2 = std::min (0.95f, std::max (0.05f, P.pw2 + md[MdPw2] * 0.45f));
        const float noiseA = std::min (1.f, std::max (0.f, P.noise + md[MdNoise])), lv1 = std::min (1.f, std::max (0.f, P.lvl1 + md[MdLvl1]));
        // enveloppe de profondeur FM : montee (ATT) puis descente exponentielle (DEC) depuis le debut de la note
        v.fmT += dtCtl;
        const float fAtt = 0.001f * std::pow (1000.f, P.fmAtt), fDec = 0.03f * std::pow (100.f, P.fmDec);
        const float fEnv = v.fmT < fAtt ? v.fmT / fAtt : std::exp (-(v.fmT - fAtt) / fDec * 3.f);
        const float fmA = std::min (1.f, std::max (0.f, P.fm + md[MdFm] + P.fmEnvAmt * fEnv + P.fmVel * v.vel + P.fmAT * v.patS)), ringA = std::min (1.f, std::max (0.f, P.ring + md[MdRing]));
        const float l2 = std::min (1.f, std::max (0.f, P.lvl2 + md[MdMix]));

        // ---- filtres
        const float kt = (v.pitch - 60.f) / 12.f * std::min (1.5f, std::max (0.f, P.keytrack + md[MdKeyTrack]));
        const float hpfHz = 25.f * std::pow (600.f, std::min (1.f, std::max (0.f, P.hpf + md[MdHpf] * 0.5f)));
        const float lpfOct = std::min (1.f, std::max (0.f, P.lpf + md[MdLpf] * 0.5f));
        float lpfHz = 25.f * std::pow (720.f, lpfOct) * std::pow (2.f, kt + fe * std::min (1.f, std::max (0.f, P.fltEnv + md[MdFltEnv] * 0.5f)) * 5.f);
        lpfHz = std::min (lpfHz, 0.45f * fs);
        const float res = std::min (1.f, std::max (0.f, P.lpfRes + md[MdReso] * 0.5f));
        const float hRes = std::min (1.f, std::max (0.f, P.hpfRes + md[MdHpfReso] * 0.5f));
        const float morph = std::min (1.f, std::max (0.f, P.semMorph + md[MdMorph] * 0.5f));
        const float gL = std::tan (3.14159265f * lpfHz / fs), gH = std::tan (3.14159265f * std::min (hpfHz, 0.45f * fs) / fs);
        const float kL = 2.f * std::pow (1.f - res, 1.35f) - 0.12f * res, kH = 2.f * std::pow (1.f - hRes, 1.5f) + 0.02f;
        const float kSem = 1.75f * std::pow (1.f - res, 1.3f) + 0.06f, kLad = 4.05f * res;
        const float foldA = std::min (1.f, std::max (0.f, P.fold + md[MdFold])), foldS = std::min (1.f, std::max (-1.f, P.foldSym + md[MdFoldSym]));
        const float drive = 1.f + std::min (1.f, std::max (0.f, P.drive + md[MdDrive])) * 5.f;
        const float drvC = 1.f - std::exp (-6.2831853f * 1200.f / fs), drvEmph = 4.5f * (drive - 1.f) / 5.f;      // emphase au-dessus de ~1,2 kHz, proportionnelle au drive

        const float level = std::min (2.f, std::max (0.f, 1.f + md[MdLevel]));
        const float pan = std::min (1.f, std::max (-1.f, md[MdPan] + v.uniPan));
        const float gl = std::sqrt (0.5f * (1.f - pan)), gr = std::sqrt (0.5f * (1.f + pan));
        const float dt1 = f1 / fs, dt2 = f2 / fs;

        for (int i = 0; i < len; ++i)
        {
            // FM audio (sens au choix) ; VCO2 est recale par VCO1 si le hard sync est actif
            const float inc1 = P.fmDir != 1 ? dt1 * std::pow (2.f, fmA * 2.5f * v.prev2) : dt1;
            const float inc2 = P.fmDir != 0 ? dt2 * std::pow (2.f, fmA * 2.5f * v.prev1) : dt2;
            v.ph1 += inc1;
            if (v.ph1 >= 1.f) { v.ph1 -= 1.f; if (P.sync) v.ph2 = v.ph1 * inc2 / std::max (inc1, 1e-6f); }
            v.ph2 += inc2; if (v.ph2 >= 1.f) v.ph2 -= 1.f;
            const float o1 = sy::osc (P.wave1, v.ph1, std::min (0.45f, inc1), pw1);
            const float o2 = sy::osc (P.wave2, v.ph2, std::min (0.45f, inc2), pw2);
            if (P.fmWave == 0) { v.prev1 = o1; v.prev2 = o2; }
            else                                                                  // modulateur dedie : sinus, triangle ou carre
            {
                auto m = [this] (float ph, float inc) { return P.fmWave == 1 ? std::sin (6.2831853f * ph) : sy::osc (P.fmWave == 2 ? 2 : 1, ph, std::min (0.45f, inc), 0.5f); };
                v.prev1 = m (v.ph1, inc1); v.prev2 = m (v.ph2, inc2);
            }
            float x = o1 * lv1 + o2 * l2 + o1 * o2 * ringA * 1.4f + rnd() * noiseA * 0.7f;
            if (foldA > 0.001f) { const float xm = 0.5f * (x + v.xPrev); v.xPrev = x; x = 0.5f * (sy::fold (xm, foldA, foldS) + sy::fold (x, foldA, foldS)); }   // wavefolder (2 points : un peu moins de repliement)
            else v.xPrev = x;
            const float hp = v.hp.run (x * 0.7f, gH, kH);                  // HPF puis LPF, comme sur le MS20
            // DRIVE "shiny" : on accentue le haut du spectre AVANT la saturation (les graves gonflent moins, les harmoniques brillent), puis on rend le niveau
            v.drvLp += drvC * (hp - v.drvLp);
            const float hiPart = hp - v.drvLp;
            const float dIn = hp + drvEmph * hiPart;
            const float dx = (std::tanh (dIn * drive + 0.06f * (drive - 1.f)) - std::tanh (0.06f * (drive - 1.f))) * (1.f / (1.f + 0.12f * (drive - 1.f)));
            float lp;
            if (P.ftype == 1)      lp = v.sem.run (dx, gL, kSem, morph) * 1.15f;
            else if (P.ftype == 2) lp = v.lad.run (dx * 1.2f, gL, kLad) * (1.f + 1.0f * res);
            else                   lp = v.lp.run (dx, gL, kL, res);
            const float a = v.aenv.tick (aA2, aD2, aS2, aR2, fs);
            const float y = lp * a * level * v.gainV * (0.35f + 0.65f * v.vel);
            L[i] += y * gl; R[i] += y * gr;
        }
        if (v.aenv.st == sy::Env::Idle) v.active = false;
    }
};
} // namespace orage
