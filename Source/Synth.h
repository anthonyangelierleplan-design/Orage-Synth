// Orage synth engine : 6 voix, style Korg MS20-M (2 VCO, hard sync, FM, PWM, ring, filtres HPF/LPF resonants)
// + 4 LFO parametrables (facon Serum), 4 macros, matrice de modulation. Aucune dependance JUCE.
#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>

namespace orage
{
constexpr int kVoices = 6, kLfos = 4, kMacros = 4, kSlots = 8;
enum ModSrc { MsNone, MsLfo1, MsLfo2, MsLfo3, MsLfo4, MsMac1, MsMac2, MsMac3, MsMac4, MsEnv, MsVel, MsWheel, kNumModSrc };
enum ModDst { MdNone, MdPitch, MdPitch2, MdPw1, MdPw2, MdFm, MdRing, MdMix, MdHpf, MdLpf, MdReso, MdLevel, MdPan, kNumModDst };
inline const char* const kModSrcNames[kNumModSrc] { "-", "LFO 1", "LFO 2", "LFO 3", "LFO 4", "MACRO 1", "MACRO 2", "MACRO 3", "MACRO 4", "ENV FLT", "VELOCITY", "MOD WHEEL" };
inline const char* const kModDstNames[kNumModDst] { "-", "PITCH", "PITCH VCO2", "PWM VCO1", "PWM VCO2", "FM", "RING", "MIX VCO2", "HPF", "LPF", "RESO", "LEVEL", "PAN" };
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
    float master = 0.7f;
    struct Lfo { float rate = 0.4f, slew = 0.f, fade = 0.f; int wave = 0, sync = 0, trig = 0; } lfo[kLfos];     // wave : 0 sin, 1 tri, 2 saw, 3 carre, 4 S&H
    float macro[kMacros] {};
    struct Slot { int src = 0, dst = 0; float amt = 0.f; } slot[kSlots];
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
    void setParams (const SynthParams& p) { P = p; }
    void setHost (double bpmIn) { bpm = bpmIn > 20.0 ? (float) bpmIn : 120.f; }
    void setWheel (float w) { wheel = w; }
    void setBend (float semis) { bend = semis; }

    void noteOn (int note, float vel)
    {
        Voice* v = nullptr;
        for (auto& c : voices) if (! c.active) { v = &c; break; }
        if (v == nullptr)
        {
            float best = 1e9f;
            for (auto& c : voices)
            {
                const float score = (c.aenv.st == sy::Env::Rel ? 0.f : 1000.f) + c.aenv.v * 100.f + (float) (counter - c.age) * -1e-6f;
                if (score < best) { best = score; v = &c; }
            }
        }
        const bool glide = P.glide > 0.001f && lastNote >= 0;
        *v = Voice();
        v->active = true; v->note = note; v->vel = vel; v->age = ++counter;
        v->pitch = glide ? (float) lastNote : (float) note;
        v->aenv.on(); v->fenv.on();
        for (int i = 0; i < kLfos; ++i) { v->lph[i] = 0.f; v->lfade[i] = 0.f; v->lsh[i] = rnd(); v->lcur[i] = v->lsh[i]; }
        lastNote = note;
    }
    void noteOff (int note)
    {
        for (auto& v : voices) if (v.active && v.note == note && v.aenv.st != sy::Env::Rel) { v.aenv.off(); v.fenv.off(); }
    }
    void allNotesOff() { for (auto& v : voices) if (v.active) { v.aenv.off(); v.fenv.off(); } }
    int activeVoices() const { int n = 0; for (auto& v : voices) n += v.active ? 1 : 0; return n; }

    void process (float* L, float* R, int n)
    {
        for (int i = 0; i < n; ++i) { L[i] = 0.f; R[i] = 0.f; }
        // LFO globaux (voix sans retrig)
        const float lfoHz[kLfos] = { hz (0), hz (1), hz (2), hz (3) };
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
            for (auto& v : voices)
                if (v.active) renderVoice (v, L + pos, R + pos, len, lfoHz, dt);
        }
        for (int i = 0; i < n; ++i) { L[i] = std::tanh (L[i] * P.master * 0.7f); R[i] = std::tanh (R[i] * P.master * 0.7f); }
    }

private:
    static constexpr int kCtl = 16;
    struct Voice
    {
        bool active = false; int note = 60; float vel = 1.f; uint64_t age = 0;
        sy::Env aenv, fenv; float pitch = 60.f, ph1 = 0.f, ph2 = 0.f, prev2 = 0.f;
        sy::Svf hp, lp; float lph[kLfos] {}, lfade[kLfos] {}, lsh[kLfos] {}, lcur[kLfos] {};
    };
    Voice voices[kVoices];
    SynthParams P;
    float fs = 44100.f, bpm = 120.f, wheel = 0.f, bend = 0.f, gPhase[kLfos] {}, gSh[kLfos] {}, gCur[kLfos] {};
    uint64_t counter = 0; int lastNote = -1; uint32_t seed = 12345u;

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
        for (int i = 0; i < kMacros; ++i) src[MsMac1 + i] = P.macro[i];
        const float fe = v.fenv.tick (P.fA, P.fD, P.fS, P.fR, fs / (float) kCtl);   // enveloppes cadencees au bloc
        src[MsEnv] = fe; src[MsVel] = v.vel; src[MsWheel] = wheel;
        float md[kNumModDst] {};
        for (const auto& s : P.slot)
            if (s.src > 0 && s.dst > 0 && s.amt != 0.f) md[s.dst] += src[s.src] * s.amt;

        const float glideC = P.glide > 0.001f ? 1.f - std::exp (-dtCtl / (P.glide * 0.6f + 0.002f)) : 1.f;
        v.pitch += (((float) v.note) - v.pitch) * glideC;
        const float n1 = v.pitch + 12.f * (float) P.oct1 + bend + md[MdPitch] * 12.f;
        const float n2 = v.pitch + 12.f * (float) P.oct2 + P.tune2 + P.fine2 * 0.01f + bend + md[MdPitch] * 12.f + md[MdPitch2] * 12.f;
        const float f1 = 440.f * std::pow (2.f, (n1 - 69.f) / 12.f), f2 = 440.f * std::pow (2.f, (n2 - 69.f) / 12.f);
        const float pw1 = std::min (0.95f, std::max (0.05f, P.pw1 + md[MdPw1] * 0.45f));
        const float pw2 = std::min (0.95f, std::max (0.05f, P.pw2 + md[MdPw2] * 0.45f));
        const float fmA = std::min (1.f, std::max (0.f, P.fm + md[MdFm])), ringA = std::min (1.f, std::max (0.f, P.ring + md[MdRing]));
        const float l2 = std::min (1.f, std::max (0.f, P.lvl2 + md[MdMix]));

        // ---- filtres
        const float kt = (v.pitch - 60.f) / 12.f * P.keytrack;
        const float hpfHz = 20.f * std::pow (400.f, std::min (1.f, std::max (0.f, P.hpf + md[MdHpf] * 0.5f)));
        const float lpfOct = std::min (1.f, std::max (0.f, P.lpf + md[MdLpf] * 0.5f));
        float lpfHz = 25.f * std::pow (720.f, lpfOct) * std::pow (2.f, kt + fe * P.fltEnv * 5.f);
        lpfHz = std::min (lpfHz, 0.45f * fs);
        const float res = std::min (1.f, std::max (0.f, P.lpfRes + md[MdReso] * 0.5f));
        const float gL = std::tan (3.14159265f * lpfHz / fs), gH = std::tan (3.14159265f * std::min (hpfHz, 0.45f * fs) / fs);
        const float kL = 2.f - 2.04f * res, kH = 2.f - 2.04f * P.hpfRes;
        const float drive = 1.f + P.drive * 5.f;

        const float level = std::min (2.f, std::max (0.f, 1.f + md[MdLevel]));
        const float pan = std::min (1.f, std::max (-1.f, md[MdPan]));
        const float gl = std::sqrt (0.5f * (1.f - pan)), gr = std::sqrt (0.5f * (1.f + pan));
        const float dt1 = f1 / fs, dt2 = f2 / fs;

        for (int i = 0; i < len; ++i)
        {
            // VCO1 est module en frequence par VCO2 (FM audio), VCO2 est recale par VCO1 (hard sync)
            const float inc1 = dt1 * std::pow (2.f, fmA * 2.5f * v.prev2);
            v.ph1 += inc1;
            if (v.ph1 >= 1.f) { v.ph1 -= 1.f; if (P.sync) v.ph2 = v.ph1 * dt2 / std::max (inc1, 1e-6f); }
            v.ph2 += dt2; if (v.ph2 >= 1.f) v.ph2 -= 1.f;
            const float o1 = sy::osc (P.wave1, v.ph1, std::min (0.45f, inc1), pw1);
            const float o2 = sy::osc (P.wave2, v.ph2, std::min (0.45f, dt2), pw2);
            v.prev2 = o2;
            float x = o1 * P.lvl1 + o2 * l2 + o1 * o2 * ringA * 1.4f + rnd() * P.noise * 0.7f;
            float lp, hp, tmp;
            v.hp.run (x * 0.7f, gH, kH, tmp, hp);                          // HPF puis LPF, comme sur le MS20
            v.lp.run (std::tanh (hp * drive), gL, kL, lp, tmp);
            const float a = v.aenv.tick (P.aA, P.aD, P.aS, P.aR, fs);
            const float y = lp * a * level * (0.35f + 0.65f * v.vel);
            L[i] += y * gl; R[i] += y * gr;
        }
        if (v.aenv.st == sy::Env::Idle) v.active = false;
    }
};
} // namespace orage
