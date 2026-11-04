// ORAGE v2 - moteur DSP semi-modulaire (aucune dependance a JUCE, donc testable hors plugin)
#pragma once
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

namespace orage
{
constexpr float kPi = 3.14159265358979f;
constexpr int kSub = 32;                      // taille du bloc de controle (modulations)

// ------------------------------------------------------------------ topologie du patch
enum Node { NIn, NOct, NDist, NColor, NFilter, NDestroy, NComp, NSpring, NGate, NOut, kNumNodes };
enum Src  { SrcLfo1, SrcLfo2, SrcSh, SrcEnv, kNumSrc };
enum Cv   { CvDrive, CvClip, CvUnc, CvHpf, CvLpf, CvPeak, CvDestroy, CvDRate, CvSpring, CvMix, CvShIn, CvShClk, kNumCv };

constexpr int kMaxRack = 8;                   // nombre de modules audio posables sur le rack

// durees des divisions de la synchro (en noires) ; 0 = libre
inline float divBeats (int idx)
{
    static const float t[16] { 0.f, 16.f, 8.f, 4.f, 2.f, 1.f, 0.5f, 0.25f, 0.125f, 4.f / 3.f, 2.f / 3.f, 1.f / 3.f, 1.f / 6.f, 1.5f, 0.75f, 0.375f };
    return t[idx < 0 ? 0 : (idx > 15 ? 15 : idx)];
}

// ------------------------------------------------------------------ helpers
inline float clamp01 (float x)                { return std::min (1.f, std::max (0.f, x)); }
inline float lerp (float a, float b, float t) { return a + (b - a) * t; }
inline float dbToLin (float db)               { return std::pow (10.f, db * 0.05f); }
inline float linToDb (float x)                { return 20.f * std::log10 (std::max (x, 1.0e-9f)); }
inline float onePoleCoef (float fc, float fs) { return 1.f - std::exp (-2.f * kPi * fc / fs); }
inline float timeCoef (float sec, float fs)   { return 1.f - std::exp (-1.f / (std::max (sec, 1.0e-5f) * fs)); }

inline float safety (float x)                 // transparent sous 0.8, ecretage doux au-dessus
{
    const float a = std::fabs (x);
    if (a < 0.8f) return x;
    const float s = x < 0.f ? -1.f : 1.f;
    return s * (0.8f + 0.2f * std::tanh ((a - 0.8f) / 0.2f));
}

struct Rng
{
    uint32_t s = 0x9E3779B9u;
    void seed (uint32_t v) { s = v ? v : 0x9E3779B9u; }
    uint32_t nextU() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }
    float bi()  { return (float) (nextU() & 0xFFFFFF) * (1.f / 8388608.f) - 1.f; }
    float uni() { return (float) (nextU() & 0xFFFFFF) * (1.f / 16777216.f); }
};

struct Buf { float ch[2][kSub]; };

// ------------------------------------------------------------------ biquad (RBJ)
struct Biquad
{
    float b0 = 1.f, b1 = 0.f, b2 = 0.f, a1 = 0.f, a2 = 0.f;
    float z1[2] { 0.f, 0.f }, z2[2] { 0.f, 0.f };

    void reset() { z1[0] = z1[1] = z2[0] = z2[1] = 0.f; }
    float process (float x, int c)
    {
        const float y = b0 * x + z1[c];
        z1[c] = b1 * x - a1 * y + z2[c];
        z2[c] = b2 * x - a2 * y;
        return y;
    }
    void setRaw (double B0, double B1, double B2, double A0, double A1, double A2)
    {
        b0 = (float) (B0 / A0); b1 = (float) (B1 / A0); b2 = (float) (B2 / A0);
        a1 = (float) (A1 / A0); a2 = (float) (A2 / A0);
    }
    void makeLowPass (double fs, double f, double q)
    {
        const double w = 2.0 * kPi * f / fs, al = std::sin (w) / (2.0 * q), cs = std::cos (w);
        setRaw ((1 - cs) / 2, 1 - cs, (1 - cs) / 2, 1 + al, -2 * cs, 1 - al);
    }
    void makePeak (double fs, double f, double q, double gainDb)
    {
        const double A = std::pow (10.0, gainDb / 40.0), w = 2.0 * kPi * f / fs;
        const double al = std::sin (w) / (2.0 * q), cs = std::cos (w);
        setRaw (1 + al * A, -2 * cs, 1 - al * A, 1 + al / A, -2 * cs, 1 - al / A);
    }
    void makeLowShelf (double fs, double f, double gainDb)
    {
        const double A = std::pow (10.0, gainDb / 40.0), w = 2.0 * kPi * f / fs;
        const double cs = std::cos (w), al = std::sin (w) / 2.0 * std::sqrt (2.0), t = 2.0 * std::sqrt (A) * al;
        setRaw (A * ((A + 1) - (A - 1) * cs + t), 2 * A * ((A - 1) - (A + 1) * cs), A * ((A + 1) - (A - 1) * cs - t),
                (A + 1) + (A - 1) * cs + t, -2 * ((A - 1) + (A + 1) * cs), (A + 1) + (A - 1) * cs - t);
    }
    void makeHighShelf (double fs, double f, double gainDb)
    {
        const double A = std::pow (10.0, gainDb / 40.0), w = 2.0 * kPi * f / fs;
        const double cs = std::cos (w), al = std::sin (w) / 2.0 * std::sqrt (2.0), t = 2.0 * std::sqrt (A) * al;
        setRaw (A * ((A + 1) + (A - 1) * cs + t), -2 * A * ((A - 1) + (A + 1) * cs), A * ((A + 1) + (A - 1) * cs - t),
                (A + 1) - (A - 1) * cs + t, 2 * ((A - 1) - (A + 1) * cs), (A + 1) - (A - 1) * cs - t);
    }
};

// ------------------------------------------------------------------ parametres
struct Params
{
    bool  on = true;
    float output = -6.f, mix = 1.f, chaos = 0.f;
    int   octMode = 0;  float octAmt = 0.5f;
    float power = 0.65f, clip = 0.35f, unc = 0.35f;  int material = 1;  float lightning = 0.f;
    float low = 0.f, mid = 0.f, high = 0.f;
    float hpf = 0.f, hpfPeak = 0.15f, lpf = 1.f, lpfPeak = 0.15f;
    float destroy = 0.f, dRate = 0.4f, dGrain = 0.4f;
    bool  dStutter = true, dReverse = true, dTape = true, dShred = true, dDrop = true, dBuzz = true;
    int   compType = 1;  float compAmt = 0.35f;
    float springMix = 0.15f, springDecay = 0.5f, springDrip = 0.5f;
    float lfo1Rate = 0.35f, lfo1Shape = 0.f,   lfo1Level = 0.5f;
    float lfo2Rate = 0.55f, lfo2Shape = 0.34f, lfo2Level = 0.5f;
    float shRate = 0.45f, shSlew = 0.15f, shLevel = 0.5f;
    float envRel = 0.4f, envLevel = 0.6f;
    int   lfo1Sync = 0, lfo2Sync = 0, shSync = 0, dSync = 0;      // 0 = libre, sinon index de division (divBeats)
    float compThresh = 0.35f, compAtt = 0.2f;
    float gateThresh = 0.f, gateRel = 0.4f, gateRange = 0.8f;
};

// ------------------------------------------------------------------ octaver (bascule /2 + redresseur)
struct Octaver
{
    struct Ch { float lp1 = 0, lp2 = 0, env = 0, sub = 0, upLp = 0, absLp = 0; bool pos = false, flip = false; };
    Ch ch[2];
    float lpC = 0, envA = 0, envR = 0, subC = 0, upC = 0, dcC = 0;

    void prepare (float fs)
    {
        lpC = onePoleCoef (380.f, fs);  envA = timeCoef (0.004f, fs);  envR = timeCoef (0.060f, fs);
        subC = onePoleCoef (500.f, fs); upC = onePoleCoef (2200.f, fs); dcC = onePoleCoef (25.f, fs);
        ch[0] = ch[1] = Ch();
    }
    void process (int c, float x, float& down, float& up)
    {
        auto& s = ch[c];
        s.lp1 += lpC * (x - s.lp1);
        s.lp2 += lpC * (s.lp1 - s.lp2);
        const float a = std::fabs (s.lp2);
        s.env += (a > s.env ? envA : envR) * (a - s.env);
        const float th = std::max (0.0015f, 0.25f * s.env);
        if (! s.pos && s.lp2 > th)       { s.pos = true; s.flip = ! s.flip; }
        else if (s.pos && s.lp2 < -th)     s.pos = false;
        s.sub += subC * ((s.flip ? s.env : -s.env) - s.sub);
        down = s.sub * 2.4f;
        s.upLp += upC * (x - s.upLp);
        const float r = std::fabs (s.upLp);
        s.absLp += dcC * (r - s.absLp);
        up = (r - s.absLp) * 1.6f;
    }
};

// ------------------------------------------------------------------ bruit lisse / orage
struct SmoothNoise
{
    float y = 0.f, coef = 0.01f, norm = 1.f;
    void setRate (float fc, float fs)
    {
        coef = onePoleCoef (fc, fs);
        const float sd = std::sqrt ((1.f / 3.f) * coef / (2.f - coef));
        norm = 0.55f / std::max (sd, 1.0e-4f);
    }
    float tick (Rng& r) { y += coef * (r.bi() - y); return std::max (-1.f, std::min (1.f, y * norm)); }
};

struct Storm
{
    struct Out { float u1, u2, u3, light, crackle, collapse, kick, latch; };
    Rng rng; SmoothNoise n1, n2, n3;
    float fs = 44100.f, lightEnv = 0.f, lightDecay = 0.999f, crackLp = 0.f;
    float collapseEnv = 0.f, collapseDecay = 0.99f, kickEnv = 0.f, latchEnv = 0.f, latchDecay = 0.9999f;

    void prepare (float sr)
    {
        fs = sr; rng.seed (0xC0FFEEu); n1.y = n2.y = n3.y = 0.f; lightEnv = collapseEnv = kickEnv = latchEnv = 0.f; crackLp = 0.f;
        collapseDecay = std::exp (-1.f / (0.007f * fs));
    }
    void setUnc (float u)
    {
        n1.setRate (8.f + 90.f * u, fs);
        n2.setRate (2.f + 20.f * u, fs);
        n3.setRate (20.f + 180.f * u, fs);
    }
    // Un coup de foudre change vraiment la chaine : le rail s'effondre (coupure), la polarisation saute (rafale d'harmoniques paires),
    // le gain explose, puis l'etage reste "verrouille" en arc quelques dizaines de ms.
    Out tick (float strikesPerSec)
    {
        Out o;
        o.u1 = n1.tick (rng); o.u2 = n2.tick (rng); o.u3 = n3.tick (rng);
        if (strikesPerSec > 0.f && rng.uni() < strikesPerSec / fs)
        {
            lightEnv = 0.55f + 0.45f * rng.uni();
            lightDecay = std::exp (-1.f / ((0.006f + 0.05f * rng.uni()) * fs));
            collapseEnv = 1.f;
            kickEnv = (rng.bi() > 0.f ? 1.f : -1.f) * (0.6f + 0.8f * rng.uni());
            latchEnv = 1.f;
            latchDecay = std::exp (-1.f / ((0.08f + 0.20f * rng.uni()) * fs));
        }
        lightEnv *= lightDecay; kickEnv *= lightDecay; collapseEnv *= collapseDecay; latchEnv *= latchDecay;
        if (lightEnv < 1.0e-4f) lightEnv = 0.f;
        if (collapseEnv < 1.0e-4f) collapseEnv = 0.f;
        if (latchEnv < 1.0e-3f) latchEnv = 0.f;
        o.light = lightEnv * (0.7f + 0.3f * rng.uni());
        o.collapse = collapseEnv; o.kick = kickEnv; o.latch = latchEnv;
        const float n = rng.bi();
        crackLp += 0.15f * (n - crackLp);
        o.crackle = n - crackLp;
        return o;
    }
};

struct ChaosMod
{
    Rng rng;
    float v[4] { 0, 0, 0, 0 }, tgt[4] { 0, 0, 0, 0 }, timer = 0.f;
    void tick (float dt, float amount)
    {
        timer -= dt;
        if (timer <= 0.f)
        {
            for (auto& t : tgt) t = rng.bi();
            timer = 0.08f + 0.9f * rng.uni() * (1.1f - 0.7f * amount);
        }
        const float k = 1.f - std::exp (-dt / 0.12f);
        for (int i = 0; i < 4; ++i) v[i] += k * (tgt[i] - v[i]);
    }
};

// ------------------------------------------------------------------ materiaux de la disto
inline float shapeGermanium (float v, float bias, float starve)
{
    const float b = 0.30f + bias;
    float a = std::tanh (v + b) - std::tanh (b);
    if (a < 0.f) a *= 0.78f;
    const float aa = std::fabs (a);
    return a * (aa / (aa + starve));
}
inline float tubeStage (float x) { return x >= 0.f ? std::tanh (x) : -0.95f * (1.f - std::exp (1.3f * x)); }
inline float shapeTube (float v, float bias) { return tubeStage (tubeStage (v + bias * 0.6f) * 2.2f) * 0.9f; }

struct DistChannel { float hp = 0.f, env = 0.f, arc = 0.f; bool arcOn = false; };

inline float shapeXenon (float v, DistChannel& cs, float strike, float upC, float downC, float fold)
{
    const float a = std::fabs (v);
    if (! cs.arcOn && a > strike)            cs.arcOn = true;
    else if (cs.arcOn && a < strike * 0.4f)  cs.arcOn = false;
    const float target = cs.arcOn ? 1.f : 0.f;
    cs.arc += (target > cs.arc ? upC : downC) * (target - cs.arc);
    const float glow  = std::tanh (v * 0.7f) * 0.45f;
    const float flash = std::tanh (v * 7.f) * 0.85f
                      + 0.3f * fold * std::sin (std::max (-8.f, std::min (8.f, v)) * 2.4f);
    return glow + (flash - glow) * cs.arc;
}

// ------------------------------------------------------------------ sur-echantillonnage x4 (IIR Butterworth ordre 6)
struct Oversampler4
{
    Biquad up1[3], up2[3], dn1[3], dn2[3];

    static float filt (Biquad* f, float x, int c) { return f[2].process (f[1].process (f[0].process (x, c), c), c); }

    void prepare (float fs)
    {
        static const double Q[3] { 0.5176, 0.7071, 1.9319 };
        for (int i = 0; i < 3; ++i)
        {
            up1[i].makeLowPass (fs * 2.0, fs * 0.45, Q[i]);  dn1[i] = up1[i];
            up2[i].makeLowPass (fs * 4.0, fs * 0.90, Q[i]);  dn2[i] = up2[i];
            up1[i].reset(); up2[i].reset(); dn1[i].reset(); dn2[i].reset();
        }
    }
    void up (float x, int c, float* o4)
    {
        const float a[2] { filt (up1, x * 2.f, c), filt (up1, 0.f, c) };
        for (int j = 0; j < 2; ++j) { o4[2 * j] = filt (up2, a[j] * 2.f, c); o4[2 * j + 1] = filt (up2, 0.f, c); }
    }
    float down (const float* i4, int c)
    {
        float h[2];
        for (int j = 0; j < 2; ++j) { filt (dn2, i4[2 * j], c); h[j] = filt (dn2, i4[2 * j + 1], c); }
        filt (dn1, h[0], c);
        return filt (dn1, h[1], c);
    }
};

// ------------------------------------------------------------------ filtre MS20 : SVF 2 poles, resonance qui s'auto-oscille, etat sature
struct Svf
{
    float ic1[2] { 0, 0 }, ic2[2] { 0, 0 }, a1 = 0, a2 = 0, a3 = 0, k = 1.4f;

    void set (float fc, float res, float fs)
    {
        const float g = std::tan (kPi * std::min (fc, 0.45f * fs) / fs);
        k = 1.4f * std::pow (1.f - res, 1.5f) + 0.02f;
        a1 = 1.f / (1.f + g * (g + k)); a2 = g * a1; a3 = g * a2;
    }
    void tick (float x, int c, float& lp, float& hp)
    {
        const float v3 = x - ic2[c];
        const float v1 = a1 * ic1[c] + a2 * v3;
        const float v2 = ic2[c] + a2 * ic1[c] + a3 * v3;
        ic1[c] = 1.6f * std::tanh ((2.f * v1 - ic1[c]) / 1.6f);     // le "pincement" du MS20 : la resonance sature
        ic2[c] = 2.f * v2 - ic2[c];
        lp = v2;
        hp = x - k * v1 - v2;
    }
    void reset() { ic1[0] = ic1[1] = ic2[0] = ic2[1] = 0.f; }
};

// ------------------------------------------------------------------ compresseur "a lampe" agressif (vari-mu, EAR 660 / Fairchild) + gain auto
struct TubeComp
{
    int type = 0;
    float amt = 0.f, fs = 44100.f, env = 0.f, grDb = 0.f;
    float aC = 0, rFast = 0, rSlow = 0, thr = -3.f, knee = 8.f, rmax = 12.f, sat = 0.1f, makeup = 1.f;

    void prepare (float sr) { fs = sr; env = 0.f; grDb = 0.f; }

    float grCurve (float over) const
    {
        if (over <= 0.f) return 0.f;
        const float ratio = 1.f + (rmax - 1.f) * (over / (over + knee));   // vari-mu : le ratio monte avec la reduction
        return over * (1.f - 1.f / ratio);
    }

    void set (int t, float a, float th, float att)
    {
        type = t; amt = a;
        const float atk = 0.00005f * std::pow (600.f, att);                 // 0,05 ms ... 30 ms
        if (t == 1)      { aC = timeCoef (atk, fs);        rFast = timeCoef (0.04f, fs); rSlow = timeCoef (0.45f, fs); knee = 6.f;  rmax = std::max (3.f, 30.f * (0.25f + 0.75f * a)); sat = 0.12f + 0.20f * a; }
        else if (t == 2) { aC = timeCoef (atk * 2.5f, fs); rFast = timeCoef (0.08f, fs); rSlow = timeCoef (0.5f, fs);  knee = 10.f; rmax = std::max (3.f, 18.f * (0.25f + 0.75f * a)); sat = 0.18f + 0.25f * a; }
        thr = -2.f - 38.f * th;                                              // seuil : -2 dB ... -40 dB
        makeup = dbToLin (0.92f * grCurve (-8.f - thr));                     // compensation de gain automatique
    }

    void process (float& l, float& r)
    {
        if (type == 0) return;
        const float det = std::max (std::fabs (l), std::fabs (r));
        const float rel = lerp (rFast, rSlow, clamp01 (grDb / 12.f));
        env += (det > env ? aC : rel) * (det - env);
        grDb = grCurve (linToDb (env) - thr);
        const float g = dbToLin (-grDb) * makeup;
        l *= g; r *= g;
        l = std::tanh (l + sat * amt * l * std::fabs (l));
        r = std::tanh (r + sat * amt * r * std::fabs (r));
    }
};

// ------------------------------------------------------------------ destroy machine : le signal lui-meme est decoupe, repete, inverse, ralenti, ecrase
struct Destroyer
{
    static constexpr int N = 1 << 17, M = N - 1;
    std::vector<float> ring[2];
    Rng rng;
    float fs = 48000.f, evG = 0.f, fadeC = 0.01f;
    long long lastIdx = -1;
    int wp = 0, type = -1, remain = 0, tickCount = 1, pos = 0, sliceLen = 1, sliceStart = 0, holdCount = 0, holdN = 4;
    float readPos = 0.f, speed = 1.f, tapeDecay = 0.9999f, hold[2] { 0.f, 0.f }, levels = 16.f, dropGain = 0.f, phase = 0.f, phInc = 0.f;

    void prepare (float sr)
    {
        fs = sr; for (auto& r : ring) r.assign ((size_t) N, 0.f);
        wp = 0; type = -1; evG = 0.f; fadeC = timeCoef (0.002f, fs); tickCount = 1; rng.seed (0xDE57u);
    }

    void start (float amount, float grainMs, int mask)
    {
        int types[6], nt = 0;
        for (int b = 0; b < 6; ++b) if (mask & (1 << b)) types[nt++] = b;
        if (nt == 0) return;
        type = types[(int) (rng.uni() * (float) nt) % nt];
        const int grain = std::max (64, (int) (grainMs * 0.001f * fs));
        pos = 0;
        switch (type)
        {
            case 0: { static const int div[3] { 1, 2, 4 };
                      sliceLen = std::max (48, grain / div[(int) (rng.uni() * 3.f) % 3]);
                      sliceStart = wp - sliceLen;
                      remain = sliceLen * (2 + (int) (rng.uni() * (2.f + 6.f * amount))); break; }
            case 1: sliceLen = std::max (64, (int) ((float) grain * (0.5f + rng.uni())));
                    sliceStart = wp - sliceLen;
                    remain = sliceLen * (rng.uni() < 0.3f ? 2 : 1); break;
            case 2: remain = std::max (64, (int) ((float) grain * (1.5f + 1.5f * rng.uni())));
                    readPos = (float) wp; speed = 1.f;
                    tapeDecay = std::exp (std::log (0.03f) / (float) remain); break;
            case 3: remain = std::max (64, (int) ((float) grain * (0.3f + 0.7f * rng.uni())));
                    holdN = 2 + (int) (rng.uni() * (8.f + 40.f * amount));
                    levels = std::exp2 (2.f + (float) (int) (rng.uni() * 5.f)); holdCount = 0; break;
            case 4: remain = std::max (64, (int) ((float) grain * (0.1f + 0.5f * rng.uni())));
                    dropGain = rng.uni() < 0.2f ? 0.25f : 0.f; break;
            default: { static const float f[4] { 50.f, 100.f, 150.f, 230.f };
                       remain = std::max (64, (int) ((float) grain * (0.5f + 1.5f * rng.uni())));
                       phInc = f[(int) (rng.uni() * 4.f) % 4] / fs; phase = 0.f; break; }
        }
    }

    float readRing (int c, int idx) const { return ring[c][(size_t) (idx & M)]; }

    void process (const Buf& in, Buf& out, int n, int nc, float amount, float rateHz, float grainMs, int mask,
                  float syncBeats, double ppq, double ppqInc)
    {
        for (int i = 0; i < n; ++i)
        {
            for (int c = 0; c < nc; ++c) ring[c][(size_t) (wp & M)] = in.ch[c][i];

            bool tickNow = false;
            if (syncBeats > 0.f)                                   // evenements cales sur la grille du tempo
            {
                ppq += ppqInc;
                const long long idx = (long long) std::floor (ppq / (double) syncBeats);
                if (idx != lastIdx) { lastIdx = idx; tickNow = true; }
            }
            else if (--tickCount <= 0)
            {
                tickCount = std::max (1, (int) (fs / std::max (rateHz, 0.1f) * (0.4f + 1.2f * rng.uni())));
                tickNow = true;
            }
            if (tickNow && type < 0 && mask != 0 && amount > 0.001f && rng.uni() < amount * 1.15f) start (amount, grainMs, mask);

            float g[2] { 0.f, 0.f }, target = 0.f;
            if (type >= 0)
            {
                if (remain > 0) { target = 1.f; --remain; }
                if (type == 3 && --holdCount <= 0) { for (int c = 0; c < nc; ++c) hold[c] = in.ch[c][i]; holdCount = holdN; }
                const float am = 0.5f + 0.5f * std::cos (2.f * kPi * phase);
                for (int c = 0; c < nc; ++c)
                {
                    const float x = in.ch[c][i];
                    switch (type)
                    {
                        case 0: case 1:
                        {
                            const int p = pos % sliceLen;
                            const int idx = type == 0 ? sliceStart + p : sliceStart + sliceLen - 1 - p;
                            const float fade = std::min (1.f, (float) std::min (p, sliceLen - 1 - p) / 16.f);
                            g[c] = readRing (c, idx) * fade; break;
                        }
                        case 2:
                        {
                            const int i0 = (int) std::floor (readPos); const float fr = readPos - (float) i0;
                            g[c] = lerp (readRing (c, i0), readRing (c, i0 + 1), fr); break;
                        }
                        case 3: g[c] = std::round (hold[c] * levels) / levels; break;
                        case 4: g[c] = x * dropGain; break;
                        default: g[c] = x * am; break;
                    }
                }
                ++pos;
                if (type == 2) { readPos += speed; speed *= tapeDecay; }
                if (type == 5) { phase += phInc; if (phase >= 1.f) phase -= 1.f; }
            }
            evG += fadeC * (target - evG);
            if (type >= 0 && remain <= 0 && evG < 0.002f) { type = -1; evG = 0.f; }
            for (int c = 0; c < nc; ++c) out.ch[c][i] = in.ch[c][i] + (g[c] - in.ch[c][i]) * evG;
            ++wp;
        }
    }
};

// ------------------------------------------------------------------ reverb a ressort : 3 ressorts, chaine d'allpass (le "boing"), boucle avec amortissement
struct Spring
{
    static constexpr int kStages = 26;
    struct One { std::vector<float> buf; int pos = 0, len = 1000, size = 1; float lp = 0.f; float ax[kStages] {}, ay[kStages] {}; };
    One s[3];
    float fs = 48000.f, dampC = 0.3f, a = -0.6f, fb = 0.7f, outGain = 0.5f;

    void prepare (float sr)
    {
        fs = sr;
        static const float ms[3] { 33.1f, 41.7f, 47.9f };
        for (int i = 0; i < 3; ++i)
        {
            auto& o = s[i];
            o.size = 1; while (o.size < (int) (0.08f * fs) + 8) o.size <<= 1;
            o.buf.assign ((size_t) o.size, 0.f);
            o.pos = 0; o.len = std::max (1, (int) (ms[i] * 0.001f * fs)); o.lp = 0.f;
            for (int k = 0; k < kStages; ++k) o.ax[k] = o.ay[k] = 0.f;
        }
    }
    void set (float decay, float drip)
    {
        fb = 0.30f + 0.665f * decay;
        a = -(0.35f + 0.45f * drip);
        dampC = onePoleCoef (6000.f - 3200.f * decay, fs);
        outGain = 2.6f * std::sqrt (1.f - fb);
    }
    void tick (float in, float& wl, float& wr)
    {
        float out[3];
        for (int i = 0; i < 3; ++i)
        {
            auto& o = s[i]; const int mask = o.size - 1;
            float v = o.buf[(size_t) ((o.pos - o.len) & mask)];
            for (int k = 0; k < kStages; ++k)
            {
                const float y = a * (v - o.ay[k]) + o.ax[k];
                o.ax[k] = v; o.ay[k] = y; v = y;
            }
            o.lp += dampC * (v - o.lp);
            v = o.lp;
            o.buf[(size_t) o.pos] = in * 0.8f + fb * v;
            o.pos = (o.pos + 1) & mask;
            out[i] = v;
        }
        wl = outGain * (0.7f * out[0] + 0.5f * out[1] - 0.3f * out[2]);
        wr = outGain * (0.7f * out[2] + 0.5f * out[1] - 0.3f * out[0]);
    }
};

// ------------------------------------------------------------------ modulations
struct Lfo
{
    float ph = 0.f;
    static float wave (float p, float shape)
    {
        const float w[4] { std::sin (2.f * kPi * p), 4.f * std::fabs (p - 0.5f) - 1.f, 2.f * p - 1.f, p < 0.5f ? 1.f : -1.f };
        const float s = clamp01 (shape) * 3.f;
        const int i = std::min (2, (int) s);
        return lerp (w[i], w[i + 1], s - (float) i);
    }
    float tick (float hz, float shape, int n, float fs)
    {
        ph += hz * (float) n / fs; ph -= std::floor (ph);
        return wave (ph, shape);
    }
    float tickSync (double ppq, float beats, float shape)          // phase verrouillee sur la position dans la mesure
    {
        const double c = ppq / (double) beats;
        ph = (float) (c - std::floor (c));
        return wave (ph, shape);
    }
};

struct SampleHold
{
    float ph = 0.f, held = 0.f, out = 0.f; bool clkHigh = false;
    // mode : 0 horloge interne, 1 entree CLK, 2 synchro tempo (trigSync)
    float tick (float hz, int mode, bool trigSync, float clk, float in, float slew, int n, float fs)
    {
        bool trig = false;
        if (mode == 1)      { trig = clk > 0.2f && ! clkHigh; clkHigh = clk > 0.2f; }
        else if (mode == 2) trig = trigSync;
        else { ph += hz * (float) n / fs; if (ph >= 1.f) { ph -= std::floor (ph); trig = true; } }
        if (trig) held = in;
        const float tau = slew * slew * 0.4f;
        const float k = tau < 0.0005f ? 1.f : 1.f - std::exp (-((float) n / fs) / tau);
        out += k * (held - out);
        return out;
    }
};

// ------------------------------------------------------------------ noise gate : seuil, release, profondeur (hysteresis + maintien)
struct Gate
{
    float fs = 48000.f, env = 0.f, g = 1.f, aC = 0.f, rC = 0.f, atkC = 0.f, relC = 0.f, thrLin = 0.f, closeLin = 0.f, floorG = 1.f;
    int holdN = 0, holdCount = 0;
    bool open = true;

    void prepare (float sr)
    {
        fs = sr; env = 0.f; g = 1.f; open = true; holdCount = 0;
        aC = timeCoef (0.0001f, fs); rC = timeCoef (0.004f, fs); atkC = timeCoef (0.0008f, fs);
        holdN = (int) (0.015f * fs);
    }
    void set (float thresh, float rel, float range)
    {
        thrLin = thresh < 0.005f ? 0.f : dbToLin (-70.f + 62.f * thresh);        // -70 dB ... -8 dB ; a zero : ouvert en permanence
        closeLin = thrLin * 0.8f;                                                 // hysteresis de 2 dB
        relC = timeCoef (0.01f * std::pow (60.f, rel), fs);                       // 10 ms ... 600 ms
        floorG = dbToLin (-90.f * clamp01 (range));
    }
    void process (float& l, float& r)
    {
        if (thrLin <= 0.f) { open = true; g += atkC * (1.f - g); l *= g; r *= g; return; }
        const float det = std::max (std::fabs (l), std::fabs (r));
        env += (det > env ? aC : rC) * (det - env);
        if (! open && env > thrLin) open = true;
        if (open)
        {
            if (env > closeLin) holdCount = holdN;
            else if (--holdCount <= 0) open = false;
        }
        const float target = open ? 1.f : floorG;
        g += (target > g ? atkC : relC) * (target - g);
        l *= g; r *= g;
    }
};

// ================================================================== moteur
class Engine
{
public:
    std::atomic<float> heat { 0.f }, arc { 0.f };
    std::atomic<float> compGr { 0.f }, gateLed { 1.f };            // vumeter du comp (dB de reduction) et voyant du gate
    std::atomic<float> modLed[kNumSrc];
    std::atomic<uint32_t> audioIn[kNumNodes], cvIn[kNumCv];     // le patch : masques de bits des sources branchees

    std::atomic<int> rackSlot[kMaxRack];                         // ordre des modules sur le rack (-1 = vide)

    Engine()
    {
        for (auto& a : audioIn) a.store (0);
        for (auto& c : cvIn) c.store (0);
        for (auto& m : modLed) m.store (0.f);
        setRack ({});                                   // au lancement : rack vide (INPUT -> OUTPUT direct)
    }

    // ---- rack
    void setDefaultRack() { setRack ({ NGate, NOct, NDist, NColor, NFilter, NDestroy, NComp, NSpring }); }
    void setRack (const std::vector<int>& order)
    {
        int k = 0;
        bool used[kNumNodes] {};
        for (int v : order)
            if (k < kMaxRack && v >= NOct && v < NOut && ! used[v]) { used[v] = true; rackSlot[k++].store (v); }
        for (; k < kMaxRack; ++k) rackSlot[k].store (-1);
    }
    std::vector<int> getRack() const
    {
        std::vector<int> r;
        for (auto& sl : rackSlot) { const int v = sl.load(); if (v >= NOct && v < NOut) r.push_back (v); }
        return r;
    }
    bool inRack (int node) const { for (auto& sl : rackSlot) if (sl.load() == node) return true; return false; }
    void addToRack (int node)    { if (! inRack (node)) { auto r = getRack(); r.push_back (node); setRack (r); } }
    void removeFromRack (int node)
    {
        auto r = getRack();
        r.erase (std::remove (r.begin(), r.end(), node), r.end());
        setRack (r);
    }
    std::string serializeRack() const
    {
        std::string str;
        for (int v : getRack()) str += std::to_string (v) + ",";
        return str;
    }
    void loadRack (const std::string& str)
    {
        std::vector<int> r;
        const char* q = str.c_str();
        while (*q)
        {
            char* end = nullptr;
            const long v = std::strtol (q, &end, 10);
            if (end == q) break;
            r.push_back ((int) v);
            q = (*end == ',') ? end + 1 : end;
        }
        setRack (r);
    }

    // ---- transport de l'hote (appele a chaque bloc)
    void setHost (double bpm, double ppq, bool playing, bool hasPpq)
    {
        hostBpm = bpm > 1.0 ? bpm : 120.0; hostPpq = ppq; hostPlaying = playing; hostHasPpq = hasPpq;
    }

    // ---- patch
    void setAudioCable (int src, int dst, bool on)
    {
        if (src < 0 || src >= NOut || dst < NOct || dst >= kNumNodes) return;
        if (on) audioIn[dst].fetch_or (1u << src); else audioIn[dst].fetch_and (~(1u << src));
    }
    void setCvCable (int src, int dst, bool on)
    {
        if (src < 0 || src >= kNumSrc || dst < 0 || dst >= kNumCv) return;
        if (on) cvIn[dst].fetch_or (1u << src); else cvIn[dst].fetch_and (~(1u << src));
    }
    void clearPatch() { for (auto& a : audioIn) a.store (0); for (auto& c : cvIn) c.store (0); }
    std::string serializePatch() const
    {
        std::string s;
        for (auto& a : audioIn) s += std::to_string (a.load()) + ",";
        for (auto& c : cvIn)    s += std::to_string (c.load()) + ",";
        return s;
    }
    void loadPatch (const std::string& s)
    {
        clearPatch();
        const char* p = s.c_str();
        for (int i = 0; i < kNumNodes + kNumCv; ++i)
        {
            char* end = nullptr;
            const unsigned long v = std::strtoul (p, &end, 10);
            if (end == p) break;
            p = (*end == ',') ? end + 1 : end;
            if (i < kNumNodes) audioIn[i].store ((uint32_t) v & 0x3FFu);
            else               cvIn[i - kNumNodes].store ((uint32_t) v & 0xFu);
        }
    }

    void prepare (float sampleRate)
    {
        fs = sampleRate; fsOS = fs * 4.f;
        oct.prepare (fs); os.prepare (fs); storm.prepare (fsOS); comp.prepare (fs); gate.prepare (fs);
        destroyer.prepare (fs); spring.prepare (fs);
        hpC = onePoleCoef (60.f, fsOS);
        sagA = timeCoef (0.03f, fsOS); sagR = timeCoef (0.25f, fsOS);
        xeUp = 0.45f; xeDown = timeCoef (0.0008f, fsOS);
        gainSm = timeCoef (0.003f, fsOS);
        dcC = onePoleCoef (16.f, fs);
        smoothC = timeCoef (0.02f, fs);
        for (auto& d : dch) d = DistChannel();
        for (int c = 0; c < 2; ++c) dcLp[c] = 0.f;
        lowB.reset(); midB.reset(); hiB.reset(); hpfSvf.reset(); lpfSvf.reset();
        lowDbLast = midDbLast = hiDbLast = -999.f;
        for (auto& b : out) for (auto& ch : b.ch) for (auto& v : ch) v = 0.f;
        mixCur = outCur = -1.f;
        gCur = 0.f; blockPeak = distPeak = lightPeak = presence = 0.f;
        lfo1 = Lfo(); lfo2 = Lfo(); sh = SampleHold(); envV = 0.f; ppqNow = 0.0; shLastIdx = -1; grPeak = 0.f;
        heat.store (0.f); arc.store (0.f);
        chaos.rng.seed (0xFACADEu); shRng.seed (0x5EEDu);
    }

    void setParams (const Params& in) { p = in; }

    void process (float** io, int n, int nc)
    {
        if (hostPlaying && hostHasPpq) ppqNow = hostPpq;           // en lecture : on se cale sur la position d'Ableton
        for (int pos = 0; pos < n; pos += kSub)
        {
            const int len = std::min (kSub, n - pos);
            float* sub[2] { io[0] + pos, nc > 1 ? io[1] + pos : nullptr };
            processSub (sub, len, nc);
        }
        finishBlock (n);
    }

private:
    // ------------------------------------------------ un bloc de controle
    void processSub (float** io, int n, int nc)
    {
        Buf& in0 = out[NIn];
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < kSub; ++i) in0.ch[c][i] = (c < nc && i < n) ? io[c][i] : 0.f;

        curBpm = hostBpm;
        curPpqStart = ppqNow;
        curPpqInc = curBpm / (60.0 * (double) fs);
        ppqNow += (double) n * curPpqInc;

        chaos.tick ((float) n / fs, p.chaos);
        updateModulation (n, in0);
        computeEffective();

        // modules presents sur le rack, dans l'ordre : le cablage par defaut (normalise) suit cet ordre
        int rackOrder[kMaxRack], rn = 0, defSrc[kNumNodes] {};
        bool active[kNumNodes] {};
        active[NIn] = true; active[NOut] = true;
        for (auto& sl : rackSlot)
        {
            const int v = sl.load();
            if (v >= NOct && v < NOut && ! active[v]) { active[v] = true; rackOrder[rn++] = v; }
        }
        uint32_t activeSrc = 1u << NIn;
        int prev = NIn;
        for (int k = 0; k < rn; ++k) { defSrc[rackOrder[k]] = prev; prev = rackOrder[k]; activeSrc |= 1u << rackOrder[k]; }
        defSrc[NOut] = prev;

        uint32_t srcMask[kNumNodes] {};
        for (int k = 0; k <= rn; ++k)
        {
            const int nd = k < rn ? rackOrder[k] : (int) NOut;
            const uint32_t m = audioIn[nd].load() & activeSrc;
            srcMask[nd] = m ? m : (1u << defSrc[nd]);          // sinon : cablage par defaut
        }
        int order[kNumNodes], cnt = 0, state[kNumNodes] {};
        state[NIn] = 2;
        for (int k = 0; k <= rn; ++k) visit (k < rn ? rackOrder[k] : (int) NOut, srcMask, state, order, cnt);

        bool done[kNumNodes] {};
        done[NIn] = true;
        for (int k = 0; k < cnt; ++k)
        {
            const int nd = order[k];
            Buf in {}; bool fed = false;
            for (int s = 0; s < NOut; ++s)
                if (srcMask[nd] & (1u << s))
                {
                    if (! done[s]) fed = true;              // boucle de retour : on lit le bloc precedent
                    for (int c = 0; c < nc; ++c) for (int i = 0; i < n; ++i) in.ch[c][i] += out[s].ch[c][i];
                }
            if (fed) for (int c = 0; c < nc; ++c) for (int i = 0; i < n; ++i) in.ch[c][i] = 1.25f * std::tanh (0.8f * in.ch[c][i]);

            switch (nd)
            {
                case NOct:     modOct (in, out[nd], n, nc); break;
                case NDist:    modDist (in, out[nd], n, nc); break;
                case NColor:   modColor (in, out[nd], n, nc); break;
                case NFilter:  modFilter (in, out[nd], n, nc); break;
                case NDestroy: destroyer.process (in, out[nd], n, nc, eDestroy, eDRateHz, eGrainMs, destroyMask(), eSyncBeats, curPpqStart, curPpqInc); break;
                case NGate:    modGate (in, out[nd], n, nc); break;
                case NComp:    modComp (in, out[nd], n, nc); break;
                case NSpring:  modSpring (in, out[nd], n, nc); break;
                default:       modOut (in, io, n, nc); break;
            }
            done[nd] = true;
        }
    }

    void visit (int nd, const uint32_t* srcMask, int* state, int* order, int& cnt)
    {
        if (state[nd] != 0) return;                           // deja traite, ou en cours (boucle de retour)
        state[nd] = 1;
        for (int s = NOct; s < NOut; ++s)
            if (srcMask[nd] & (1u << s)) visit (s, srcMask, state, order, cnt);
        state[nd] = 2;
        order[cnt++] = nd;
    }

    int destroyMask() const
    {
        return (p.dStutter ? 1 : 0) | (p.dReverse ? 2 : 0) | (p.dTape ? 4 : 0) | (p.dShred ? 8 : 0) | (p.dDrop ? 16 : 0) | (p.dBuzz ? 32 : 0);
    }

    // ------------------------------------------------ LFO, S&H, enveloppe -> tensions de commande
    void updateModulation (int n, const Buf& in)
    {
        const float b1 = divBeats (p.lfo1Sync), b2 = divBeats (p.lfo2Sync), bs = divBeats (p.shSync);
        const float v1 = (b1 > 0.f ? lfo1.tickSync (ppqNow, b1, p.lfo1Shape)
                                   : lfo1.tick (0.05f * std::pow (800.f, p.lfo1Rate), p.lfo1Shape, n, fs)) * p.lfo1Level;
        const float v2 = (b2 > 0.f ? lfo2.tickSync (ppqNow, b2, p.lfo2Shape)
                                   : lfo2.tick (0.05f * std::pow (800.f, p.lfo2Rate), p.lfo2Shape, n, fs)) * p.lfo2Level;
        float pk = 0.f;
        for (int c = 0; c < 2; ++c) for (int i = 0; i < n; ++i) pk = std::max (pk, std::fabs (in.ch[c][i]));
        const float dt = (float) n / fs;
        const float rel = 0.02f * std::pow (75.f, p.envRel);
        envV += (1.f - std::exp (-dt / (pk > envV ? 0.003f : rel))) * (pk - envV);
        const float ve = clamp01 (envV * 2.2f) * p.envLevel;

        const float base[3] { v1, v2, ve };
        const uint32_t inMask  = cvIn[CvShIn].load()  & ~(1u << SrcSh);
        const uint32_t clkMask = cvIn[CvShClk].load() & ~(1u << SrcSh);
        float inVal = 0.f, clkVal = 0.f;
        for (int s = 0; s < 3; ++s)
        {
            const float bs = base[s == 2 ? 2 : s];
            const int srcId = s == 2 ? SrcEnv : s;
            if (inMask & (1u << srcId))  inVal += bs;
            if (clkMask & (1u << srcId)) clkVal += bs;
        }
        if (! inMask) inVal = shRng.bi();
        bool syncTrig = false;
        if (bs > 0.f)
        {
            const long long idx = (long long) std::floor (ppqNow / (double) bs);
            syncTrig = idx != shLastIdx; shLastIdx = idx;
        }
        const int shMode = bs > 0.f ? 2 : (clkMask != 0 ? 1 : 0);
        const float vs = sh.tick (0.3f * std::pow (100.f, p.shRate), shMode, syncTrig, clkVal, inVal, p.shSlew, n, fs) * p.shLevel * 2.f;

        srcVal[SrcLfo1] = v1; srcVal[SrcLfo2] = v2; srcVal[SrcSh] = vs; srcVal[SrcEnv] = ve;
        for (int d = 0; d < kNumCv; ++d)
        {
            const uint32_t m = cvIn[d].load();
            float sum = 0.f;
            for (int s = 0; s < kNumSrc; ++s) if (m & (1u << s)) sum += srcVal[s];
            cv[d] = std::max (-2.f, std::min (2.f, sum));
        }
        for (int s = 0; s < kNumSrc; ++s) modLed[s].store (std::min (1.f, std::fabs (srcVal[s])));
    }

    float U (float base, int cvi) const { return clamp01 (base + 0.6f * cv[cvi]); }

    void computeEffective()
    {
        const float c = p.chaos;
        ePower = clamp01 (U (p.power, CvDrive) + 0.35f * c * chaos.v[0]);
        eClip  = clamp01 (U (p.clip,  CvClip)  + 0.45f * c * chaos.v[1]);
        eUnc   = clamp01 (U (p.unc,   CvUnc)   + 0.30f * c * std::fabs (chaos.v[2]));

        const float lowDb = p.low + 9.f * c * chaos.v[2], midDb = p.mid + 9.f * c * chaos.v[3], hiDb = p.high + 9.f * c * chaos.v[1];
        if (std::fabs (lowDb - lowDbLast) > 0.01f) { lowB.makeLowShelf (fs, 110.0, lowDb);       lowDbLast = lowDb; }
        if (std::fabs (midDb - midDbLast) > 0.01f) { midB.makePeak (fs, 900.0, 0.8, midDb);       midDbLast = midDb; }
        if (std::fabs (hiDb  - hiDbLast)  > 0.01f) { hiB.makeHighShelf (fs, 4500.0, hiDb);        hiDbLast  = hiDb; }

        static const float trim[3] { 0.85f, 0.80f, 0.65f };
        const int m = std::max (0, std::min (2, p.material));
        dGain   = dbToLin (-6.f + 62.f * std::pow (ePower, 1.15f)) * (m == 0 ? 0.7f : 1.f);
        dRail   = lerp (1.f, 0.10f, std::pow (eClip, 0.8f));
        dHard   = eClip * eClip;
        dSag    = (m == 1 ? 0.42f : m == 0 ? 0.22f : 0.12f) * (0.3f + 0.7f * ePower);
        dStarve = 0.015f + 0.12f * eUnc;
        dFold   = 0.4f + 0.6f * eUnc;
        dMakeup = std::pow (std::max (dRail, 0.1f), -0.7f) * trim[m];
        storm.setUnc (eUnc);
        if (gCur <= 0.f) gCur = dGain;

        // filtre MS20
        const float hU = U (p.hpf, CvHpf), lU = U (p.lpf, CvLpf);
        const float hRes = clamp01 (p.hpfPeak + 0.6f * cv[CvPeak]), lRes = clamp01 (p.lpfPeak + 0.6f * cv[CvPeak]);
        filterActive = hU > 0.003f || lU < 0.997f;
        hpfSvf.set (20.f * std::pow (250.f, hU), hRes, fs);
        lpfSvf.set (40.f * std::pow (450.f, lU), lRes, fs);

        eDestroy  = U (p.destroy, CvDestroy);
        eDRateHz  = 0.5f * std::pow (48.f, U (p.dRate, CvDRate));
        eSyncBeats = divBeats (p.dSync);
        if (eSyncBeats > 0.f) eGrainMs = eSyncBeats * 60000.f / (float) curBpm * 0.25f * std::pow (8.f, p.dGrain);
        else                  eGrainMs = 10.f * std::pow (35.f, p.dGrain);
        comp.set (p.compType, p.compAmt, p.compThresh, p.compAtt);
        gate.set (p.gateThresh, p.gateRel, p.gateRange);
        eSpringMix = U (p.springMix, CvSpring);
        spring.set (p.springDecay, p.springDrip);
        outTarget = dbToLin (p.output);
        mixTarget = p.on ? U (p.mix, CvMix) : 0.f;
        if (mixCur < 0.f) { mixCur = mixTarget; outCur = outTarget; }
    }

    // ------------------------------------------------ modules
    void modOct (const Buf& in, Buf& o, int n, int nc)
    {
        for (int i = 0; i < n; ++i)
            for (int c = 0; c < nc; ++c)
            {
                float x = in.ch[c][i];
                if (p.octMode != 0)
                {
                    float down = 0.f, up = 0.f, w = 0.f;
                    oct.process (c, x, down, up);
                    if (p.octMode == 1 || p.octMode == 3) w += down;
                    if (p.octMode == 2 || p.octMode == 3) w += up;
                    x = x * (1.f - 0.3f * p.octAmt) + w * p.octAmt;
                }
                o.ch[c][i] = x;
            }
    }

    void modDist (const Buf& in, Buf& o, int n, int nc)
    {
        const float strikeRate = p.lightning * 8.f + p.chaos * 1.5f;
        float pk = 0.f;
        for (int i = 0; i < n; ++i)
        {
            float u4[2][4];
            for (int c = 0; c < nc; ++c) os.up (in.ch[c][i], c, u4[c]);

            for (int k = 0; k < 4; ++k)
            {
                const auto st = storm.tick (strikeRate);
                lightPeak = std::max (lightPeak, st.light);
                gCur += gainSm * (dGain - gCur);
                const float gMod   = std::max (0.15f, 1.f + 0.75f * eUnc * st.u1);
                const float bias   = 0.55f * eUnc * st.u2 + 1.2f * st.kick;                       // la polarisation saute au coup de foudre
                const float railM  = (1.f - 0.55f * eUnc * (0.5f + 0.5f * st.u3)) * (1.f - 0.92f * st.collapse);   // le rail s'effondre
                const float gain   = gCur * gMod * (1.f + 7.f * st.light);
                const float strike = 0.5f * (1.f + 0.5f * eUnc * st.u3) * (1.f - 0.75f * st.latch);  // l'arc reste amorce
                const float starve = dStarve + 0.25f * st.latch;
                const float fold   = dFold + 0.6f * st.latch;

                for (int c = 0; c < nc; ++c)
                {
                    auto& cs = dch[c];
                    float x = u4[c][k];
                    cs.hp += hpC * (x - cs.hp);
                    x -= cs.hp;
                    const float R = std::max (0.04f, dRail * railM * (1.f - dSag * cs.env) * (1.f + 1.4f * st.light));
                    const float v = x * gain / R;
                    float y;
                    switch (p.material)
                    {
                        case 0:  y = shapeGermanium (v, bias, starve); break;
                        case 2:  y = shapeXenon (v, cs, strike, xeUp, xeDown, fold); break;
                        default: y = shapeTube (v, bias); break;
                    }
                    if (dHard > 0.f) y += (std::max (-1.f, std::min (1.f, v)) - y) * dHard;
                    y *= R;
                    const float ay = std::fabs (y) / std::max (dRail, 0.04f);
                    cs.env += (ay > cs.env ? sagA : sagR) * (ay - cs.env);
                    u4[c][k] = y * dMakeup + st.crackle * st.light * 0.12f * (0.25f + std::fabs (y));
                }
            }
            for (int c = 0; c < nc; ++c)
            {
                float y = os.down (u4[c], c);
                dcLp[c] += dcC * (y - dcLp[c]); y -= dcLp[c];
                o.ch[c][i] = y;
                pk = std::max (pk, std::fabs (y));
            }
        }
        distPeak = std::max (distPeak, pk);
    }

    void modGate (const Buf& in, Buf& o, int n, int nc)
    {
        for (int i = 0; i < n; ++i)
        {
            float l = in.ch[0][i], r = nc > 1 ? in.ch[1][i] : l;
            gate.process (l, r);
            o.ch[0][i] = l; if (nc > 1) o.ch[1][i] = r;
        }
        gateLed.store (gate.open ? 1.f : 0.f);
    }

    void modColor (const Buf& in, Buf& o, int n, int nc)
    {
        for (int i = 0; i < n; ++i)
            for (int c = 0; c < nc; ++c)
                o.ch[c][i] = hiB.process (midB.process (lowB.process (in.ch[c][i], c), c), c);
    }

    void modFilter (const Buf& in, Buf& o, int n, int nc)
    {
        for (int i = 0; i < n; ++i)
            for (int c = 0; c < nc; ++c)
            {
                if (! filterActive) { o.ch[c][i] = in.ch[c][i]; continue; }
                float lp, hp, lp2, hp2;
                hpfSvf.tick (in.ch[c][i], c, lp, hp);
                lpfSvf.tick (hp, c, lp2, hp2);
                o.ch[c][i] = lp2;
            }
    }

    void modComp (const Buf& in, Buf& o, int n, int nc)
    {
        for (int i = 0; i < n; ++i)
        {
            float l = in.ch[0][i], r = nc > 1 ? in.ch[1][i] : l;
            comp.process (l, r);
            grPeak = std::max (grPeak, comp.grDb);
            o.ch[0][i] = l; if (nc > 1) o.ch[1][i] = r;
        }
    }

    void modSpring (const Buf& in, Buf& o, int n, int nc)
    {
        const float a = eSpringMix * 0.5f * kPi, dryG = std::cos (a), wetG = std::sin (a) * 1.3f;
        for (int i = 0; i < n; ++i)
        {
            float wl = 0.f, wr = 0.f;
            if (eSpringMix > 0.002f)
            {
                const float m = nc > 1 ? 0.5f * (in.ch[0][i] + in.ch[1][i]) : in.ch[0][i];
                spring.tick (std::tanh (0.7f * m), wl, wr);
            }
            o.ch[0][i] = in.ch[0][i] * dryG + wl * wetG;
            if (nc > 1) o.ch[1][i] = in.ch[1][i] * dryG + wr * wetG;
        }
    }

    void modOut (const Buf& wetIn, float** io, int n, int nc)
    {
        const Buf& dry = out[NIn];
        for (int i = 0; i < n; ++i)
        {
            mixCur += smoothC * (mixTarget - mixCur);
            outCur += smoothC * (outTarget - outCur);
            for (int c = 0; c < nc; ++c)
            {
                const float wet = safety (wetIn.ch[c][i] * outCur);
                blockPeak = std::max (blockPeak, std::fabs (wet));
                io[c][i] = dry.ch[c][i] * (1.f - mixCur) + wet * mixCur;
            }
        }
    }

    // ------------------------------------------------ chaleur des lampes
    void finishBlock (int n)
    {
        const float dt = (float) n / fs;
        presence += (1.f - std::exp (-dt / 0.4f)) * (clamp01 (distPeak * 1.6f) - presence);
        const float target = clamp01 (ePower * (0.30f + 0.70f * presence) + 0.25f * eClip * presence
                                      + 0.15f * lightPeak + 0.10f * eUnc * presence);
        float h = heat.load();
        h += (1.f - std::exp (-dt / (target > h ? 1.4f : 3.5f))) * (target - h);
        heat.store (h);
        arc.store (std::max (arc.load() * 0.7f, lightPeak));
        compGr.store (std::max (grPeak, compGr.load() * 0.8f));
        grPeak = 0.f;
        blockPeak = 0.f; distPeak = 0.f; lightPeak = 0.f;
    }

    // ------------------------------------------------ etat
    Params p;
    float fs = 44100.f, fsOS = 176400.f;
    Buf out[kNumNodes];
    Octaver oct; Oversampler4 os; Storm storm; TubeComp comp; Gate gate; Destroyer destroyer; Spring spring; ChaosMod chaos;
    Biquad lowB, midB, hiB; Svf hpfSvf, lpfSvf;
    DistChannel dch[2];
    Lfo lfo1, lfo2; SampleHold sh; Rng shRng; float envV = 0.f;
    float srcVal[kNumSrc] {}, cv[kNumCv] {};
    float hpC = 0, sagA = 0, sagR = 0, xeUp = 0, xeDown = 0, gainSm = 0, dcC = 0, smoothC = 0;
    float ePower = 0, eClip = 0, eUnc = 0, eDestroy = 0, eDRateHz = 1, eGrainMs = 100, eSpringMix = 0;
    float dGain = 1, gCur = 0, dRail = 1, dHard = 0, dSag = 0, dStarve = 0.05f, dFold = 0.5f, dMakeup = 1;
    float outTarget = 1, mixTarget = 1, outCur = -1, mixCur = -1;
    float eSyncBeats = 0.f, grPeak = 0.f;
    double hostBpm = 120.0, hostPpq = 0.0, ppqNow = 0.0, curBpm = 120.0, curPpqStart = 0.0, curPpqInc = 0.0;
    bool hostPlaying = false, hostHasPpq = false;
    long long shLastIdx = -1;
    float dcLp[2] { 0, 0 };
    float lowDbLast = -999.f, midDbLast = -999.f, hiDbLast = -999.f;
    bool filterActive = false;
    float blockPeak = 0, distPeak = 0, lightPeak = 0, presence = 0;
};
} // namespace orage
