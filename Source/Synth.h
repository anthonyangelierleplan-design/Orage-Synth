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
    int ftype = 0;                            // filtre passe-bas : 0 MS20, 1 SEM, 2 Ladder, 3 MS-10 (circuit), 4 Ladder CEM3320 (circuit), 5 SEM (circuit)
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
inline float blamp (float t, float dt)                                     // integrale du blep : arrondit les angles du triangle (anti-repliement)
{
    if (t < dt) { t = t / dt - 1.f; return -(1.f / 3.f) * t * t * t; }
    if (t > 1.f - dt) { t = (t - 1.f) / dt + 1.f; return (1.f / 3.f) * t * t * t; }
    return 0.f;
}
inline float osc (int wave, float ph, float dt, float pw)
{
    switch (wave)
    {
        case 0: return 2.f * ph - 1.f - blep (ph, dt);
        case 1: { float t2 = ph - pw; if (t2 < 0.f) t2 += 1.f; return (ph < pw ? 1.f : -1.f) + blep (ph, dt) - blep (t2, dt); }
        default: { float t2 = ph + 0.5f; if (t2 >= 1.f) t2 -= 1.f; return 4.f * std::fabs (ph - 0.5f) - 1.f + 4.f * dt * (blamp (t2, dt) - blamp (ph, dt)); }
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
    float s1 = 0.f, s2 = 0.f, bp = 0.f;
    float run (float x, float g, float k, float res)
    {
        const float a1 = 1.f / (1.f + g * (g + k)), a2 = g * a1, a3 = g * a2;
        const float v3 = x - s2, v1 = a1 * s1 + a2 * v3, v2 = s2 + a2 * s1 + a3 * v3;
        s1 = 2.f * v1 - s1;
        const float wild = 0.10f + 0.9f * res * res;
        s1 = s1 > 0.f ? s1 / (1.f + 0.10f * s1 * s1) : s1 / (1.f + wild * 0.6f * s1 * s1);   // diode : les deux alternances ne saturent pas pareil
        s2 = 2.f * v2 - s2;
        // la bande passante est ajoutee a la sortie (le pic devient criard) ; elle est refiltree a ~2x le cutoff : les harmoniques de la saturation ne fuient plus dans les aigus quand le filtre est ferme
        const float g2 = std::min (0.9f, 2.f * g), G2 = g2 / (1.f + g2);
        bp += G2 * (v1 - bp);
        return v2 + res * res * 1.1f * bp;
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
// tanh rapide (Pade 7/6) : erreur < 1e-4 sur +/-4.5, borne au-dela ; utilisee dans les etages surechantillonnes
inline float tanhFast (float x)
{
    if (x > 4.5f) return 1.f; if (x < -4.5f) return -1.f;
    const float x2 = x * x;
    return x * (135135.f + x2 * (17325.f + x2 * (378.f + x2))) / (135135.f + x2 * (62370.f + x2 * (3150.f + x2 * 28.f)));
}
// ---- MS-10 : modele du CIRCUIT du VCF Korg35 (schemas Korg MS-10 / Haible), valide sur des enregistrements d'un MS-10 de 1978
// deux etages RC a transconductance (OTA, tanh) 3,3 nF / 1 nF, attenuation d'entree 1/101, ampli de sortie 270k/4,7k (gain 58) limite par des diodes 1N4148,
// retour PEAK par 2,2k + 5,6k + 1 nF vers le noeud intermediaire. Integration trapezoidale + Newton 4x4 : le retour est implicite (pas de retard d'un echantillon).
inline double tanhSmall (double x)                                  // tanh : serie pour |x| < 0.5 (signal faible devant l'echelle de l'OTA), Pade rapide sinon
{
    if (std::fabs (x) < 0.5) { const double x2 = x * x; return x * (1.0 + x2 * (-1.0 / 3 + x2 * (2.0 / 15 + x2 * (-17.0 / 315 + x2 * (62.0 / 2835 - x2 * (1382.0 / 155925)))))); }
    return (double) tanhFast ((float) x);
}
inline double atanhFast (double z)                                  // atanh : serie pour |z| < 0.3 (le cas courant : signal faible devant l'echelle de l'OTA), logarithme sinon
{
    const double a = std::fabs (z);
    if (a < 0.15) { const double z2 = z * z; return z * (1.0 + z2 * (1.0 / 3 + z2 * (1.0 / 5 + z2 * (1.0 / 7)))); }
    if (a < 0.4) { const double z2 = z * z; return z * (1.0 + z2 * (1.0 / 3 + z2 * (1.0 / 5 + z2 * (1.0 / 7 + z2 * (1.0 / 9 + z2 * (1.0 / 11 + z2 * (1.0 / 13 + z2 * (1.0 / 15 + z2 * (1.0 / 17))))))))); }
    return 0.5 * std::log ((1.0 + z) / (1.0 - z));
}
struct Ms10Cst
{
    double Gc6 = 0, Gc3 = 0, Gc5 = 0, kdc = 0;
    void prepare (double fs) { const double T = 1.0 / fs; Gc6 = 2 * 3.3e-9 / T; Gc3 = 2 * 1e-9 / T; Gc5 = 2 * 1e-9 / T; kdc = 1.0 - std::exp (-6.2831853 * 3.4 / fs); }
};
struct Ms10Tab
{
    static constexpr int N = 34;
    double lRp[N], lOm[N], lRa[N], kF[N];
    Ms10Tab()
    {
        static const double kRp[N] = {1000,1258.93,1584.89,1995.26,2511.89,3162.28,3981.07,5011.87,6309.57,7943.28,10000,12589.3,15848.9,19952.6,25118.9,31622.8,39810.7,50118.7,63095.7,79432.8,100000,125893,158489,199526,251189,316228,398107,501187,630957,794328,1e+06,1.25893e+06,1.58489e+06,1.99526e+06};
        static const double kOm[N] = {124095,118186,102951,96000,80963.9,70034.1,60589.5,52329.5,44846.7,38201.2,32343,27211.2,22748.4,18899.1,15608.1,12819.6,10477,8525.03,6910.58,5583.98,4500.37,3619.51,2906.44,2331.28,1868.67,1497.49,1200.23,962.555,772.729,621.259,500.481,404.213,327.482,266.319};
        static const double kFt[N] = {1.9000,1.9000,1.7491,1.6235,1.5077,1.4054,1.3183,1.2463,1.1883,1.1424,1.1067,1.0794,1.0586,1.0431,1.0316,1.0231,1.0169,1.0124,1.0091,1.0068,1.0050,1.0037,1.0028,1.0021,1.0016,1.0012,1.0009,1.0006,1.0004,1.0003,1.0002,1.0001,1.0001,1.0000};
        for (int i = 0; i < N; ++i) { lRp[i] = std::log (kRp[N - 1 - i]); lOm[i] = std::log (kOm[N - 1 - i]); lRa[i] = std::log (kRp[i]); kF[i] = kFt[i]; }
    }
    static const Ms10Tab& get() { static const Ms10Tab t; return t; }
    double rpFor (double om) const                                  // resistance des etages pour une pulsation analogique (la frequence d'auto-oscillation = le cutoff)
    {
        const double l = std::log (std::max (om, 1.0));
        if (l <= lOm[0]) return std::exp (lRp[0] + (lOm[0] - l));
        if (l >= lOm[N - 1]) return std::exp (lRp[N - 1]);
        int i = 0; while (i < N - 2 && lOm[i + 1] < l) ++i;
        const double t = (l - lOm[i]) / (lOm[i + 1] - lOm[i]); return std::exp (lRp[i] + t * (lRp[i + 1] - lRp[i]));
    }
    double kFor (double rp) const                                   // compense la perte de gain de boucle dans les aigus
    {
        const double l = std::log (rp); if (l <= lRa[0]) return kF[0]; if (l >= lRa[N - 1]) return kF[N - 1];
        int i = 0; while (i < N - 2 && lRa[i + 1] < l) ++i;
        const double t = (l - lRa[i]) / (lRa[i + 1] - lRa[i]); return kF[i] + t * (kF[i + 1] - kF[i]);
    }
    static double potOf (double res)                                // reglage de resonance (0..1) -> position electrique du potard PEAK, calee sur les mesures (30 % : +2,5 dB, 60 % : +6 dB, 100 % : oscille)
    {
        static const double x[] = { 0.0, 0.15, 0.30, 0.45, 0.60, 0.75, 0.88, 1.0 }, y[] = { 0.0, 0.36, 0.47, 0.52, 0.57, 0.60, 0.64, 0.75 };
        if (res <= 0.0) return 0.0; if (res >= 1.0) return y[7];
        int i = 0; while (i < 6 && x[i + 1] < res) ++i;
        return y[i] + (res - x[i]) / (x[i + 1] - x[i]) * (y[i + 1] - y[i]);
    }
    void controls (double om, double res, const Ms10Cst& c, float& Gp, float& pk, float& Ge) const
    {
        const double Rp = rpFor (om), p = potOf (res), Rs = 2200.0 + 5600.0 + 10000.0 * p * (1.0 - p);
        Gp = (float) (1.0 / Rp); pk = (float) (0.2 * p * kFor (Rp)); Ge = (float) (1.0 / (Rs + 1.0 / c.Gc5));
    }
};
#ifdef MS10_COUNT
inline long g_ms10It = 0;
#endif
struct Ms10
{
    double Vm = 0, Vo = 0, Vf = 0, Vd = 0, Vd1 = 0, h6 = 0, h3 = 0, h5 = 0, vdc = 0; uint32_t rng = 2463534242u;
    void reset() { Vm = Vo = Vf = Vd = Vd1 = h6 = h3 = h5 = vdc = 0; }
    // Le systeme (4 inconnues : noeud intermediaire m, sortie o, sortie de l'ampli f, tension de diode d) se ramene a UNE inconnue d :
    //   f = d + Rd.Id(d) ; o = vdc + R63 (f/R64 + Id) ; m = o + Vs.atanh ((Gc3 o - h3) / (Gp Vs)) ; il reste l'equation du noeud m, resolue par Newton 1D.
    float run (float xin, float Gpf, float pkf, float Gef, const Ms10Cst& C)
    {
        constexpr double kSc = 0.4, att = 1.0 / 101.0, R63 = 4700.0, R64 = 270000.0, Is = 2e-8, nVt = 0.07, da = 0.3, Rd = 2200.0, Vs = 0.1, makeup = 1.15;
        const double Gp = Gpf, pk = pkf, Ge = Gef;
        rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;                                   // bruit minuscule : amorce l'auto-oscillation, comme le bruit du vrai circuit
        const double vi = xin * kSc * att + ((double) rng * (1.0 / 4294967296.0) - 0.5) * 2e-7;
        const double g5 = h5 / C.Gc5, GpVs = Gp * Vs, iGpVs = 1.0 / GpVs, kG = C.Gc3 / Gp, iVs = 1.0 / Vs, iNv = 1.0 / nVt;
        double d = 2.0 * Vd - Vd1, m = Vm, o = Vo, f = Vf;      // depart : extrapolation lineaire de la tension de diode
        for (int it = 0; it < 10; ++it)
        {
            const double ex = (double) expf ((float) std::min (d * iNv, 40.0)), en = 1.0 / ex;
            const double Id = Is * ((1 + da) * ex - (1 - da) * en), dId = Is * iNv * ((1 + da) * ex + (1 - da) * en);
            f = d + Rd * Id;                         const double f1 = 1.0 + Rd * dId;
            o = vdc + R63 * (f / R64 + Id);          const double o1 = R63 * (f1 / R64 + dId);
            const double I2 = C.Gc3 * o - h3;
            const double z = std::max (-0.999999, std::min (0.999999, I2 * iGpVs)), w = 1.0 - z * z;
            m = o + Vs * atanhFast (z);
            const double m1 = o1 * (1.0 + kG / w);
            const double t1 = tanhSmall ((vi - m) * iVs), I1 = GpVs * t1, dI1 = Gp * (1.0 - t1 * t1);
            const double F = I1 - I2 - C.Gc6 * m + h6 + Ge * (pk * (o + f) - m - g5);
            const double dF = -dI1 * m1 - C.Gc3 * o1 - C.Gc6 * m1 + Ge * (pk * (o1 + f1) - m1);
            double step = -F / dF; const double as = std::fabs (step); if (as > 0.1) step *= 0.1 / as;
            d += step;
#ifdef MS10_COUNT
            ++g_ms10It;
#endif
            if (as < 4e-7) break;
        }
        const double Va = o + f;
        if (! std::isfinite (Va)) { reset(); return 0.f; }
        const double iF = Ge * (pk * Va - m - g5);
        h6 = 2 * C.Gc6 * m - h6; h3 = 2 * C.Gc3 * o - h3; h5 += 2 * iF;
        Vm = m; Vo = o; Vf = f; Vd1 = Vd; Vd = d; vdc += (o - vdc) * C.kdc;
        return (float) (Va * (makeup / (att * (1.0 + R64 / R63) * kSc)));
    }
};
// ---- LADDER "CEM3320" : modele du circuit du filtre 4 poles CEM3320 (Sequential Pro-One) : 4 etages OTA (tanh) + condensateur, retour de la sortie par le VCA de resonance.
// Chaque etage : C dv/dt = Gm*Vs*tanh((entree - v)/Vs), Vs = 2*Vt = 52 mV ; le signal est ramene au niveau du mV par les resistances d'entree.
// Integration trapezoidale + Newton 4x4 (jacobienne bidiagonale + 1 terme de retour, resolue en O(n)) : le retour est implicite, la frequence d'auto-oscillation = le cutoff.
struct Cem3320
{
    float v[4] {}, t[4] {}; unsigned rs = 12345u;
    void reset() { for (int i = 0; i < 4; ++i) v[i] = t[i] = 0.f; }
    float run (float x, float g, float k)                       // g = tan(pi f/fs), k = gain du retour (auto-oscillation vers 4)
    {
        constexpr float sc = 1.0f;                              // niveau moteur -> unites de Vs (0,052 V par unite)
        rs = rs * 1664525u + 1013904223u;
        const float u = x * sc + (float) (int) (rs >> 8) * 1e-9f;     // bruit thermique infime : amorce l'auto-oscillation

        float w[4] = { v[0], v[1], v[2], v[3] }, s[4];
        for (int i = 0; i < 4; ++i) s[i] = v[i] + g * t[i];
        for (int it = 0; it < 8; ++it)
        {
            float a = u - k * w[3], al = 0.f, be = 0.f, A[3], B[3];
            for (int i = 0; i < 4; ++i)
            {
                const float tn = tanhFast (a - w[i]), d = 1.f - tn * tn, F = w[i] - g * tn - s[i], Jd = 1.f + g * d;
                if (i == 0) { al = -F / Jd; be = -g * d * k / Jd; }
                else        { al = (-F + g * d * al) / Jd; be = g * d * be / Jd; }
                if (i < 3) { A[i] = al; B[i] = be; }
                a = w[i];
            }
            const float d4 = al / (1.f - be);
            float step = std::fabs (d4);
            for (int i = 0; i < 3; ++i) { const float di = A[i] + B[i] * d4; w[i] += di; step = std::max (step, std::fabs (di)); }
            w[3] += d4;
            if (step < 2e-6f) break;
        }
        float a = u - k * w[3];
        for (int i = 0; i < 4; ++i) { t[i] = tanhFast (a - w[i]); v[i] = w[i]; a = w[i]; }
        if (! (std::fabs (v[3]) < 1e3f)) reset();
        return v[3] * (1.f / sc);
    }
};
// ---- SEM "circuit" : filtre a variables d'etat Oberheim SEM : melangeur A12 (entree - LP - amortissement*BP), deux integrateurs CA3080 (tanh) A13/A15 precedes de diviseurs,
// suiveurs Q39/Q40, BP en A14, LP en A16, amortissement fixe par le potard RES (50k) avec ecretage par les diodes D11/D12. Trapezoide + Newton 2x2 (retour implicite).
struct SemCkt
{
    float bp = 0.f, lp = 0.f, t1 = 0.f, t2 = 0.f;
    void reset() { bp = lp = t1 = t2 = 0.f; }
    // x entree, g = tan(pi f/fs), d = amortissement (2 = sans resonance), morph LP -> notch -> HP
    float run (float x, float g, float d, float morph)
    {
        constexpr float sc = 1.0f / 1.2f;                       // niveau moteur -> echelle des OTA (1,2 par unite d'echelle)
        constexpr float il = 1.f / 1.6f;                        // seuil des diodes D11/D12 (en echelle OTA)
        const float xn = x * sc;
        const float s1 = bp + g * t1, s2 = lp + g * t2;
        float b = bp, l = lp, hp = 0.f;
        for (int it = 0; it < 8; ++it)
        {
            const float tl = tanhFast (b * il), Lb = tl / il, dL = 1.f - tl * tl;
            hp = xn - l - d * Lb;
            const float ta = tanhFast (hp), tb = tanhFast (b), ea = 1.f - ta * ta, eb = 1.f - tb * tb;
            const float F1 = b - g * ta - s1, F2 = l - g * tb - s2;
            const float a11 = 1.f + g * ea * d * dL, a12 = g * ea, a21 = -g * eb, a22 = 1.f;
            const float det = a11 * a22 - a12 * a21;
            const float db = (-F1 * a22 + F2 * a12) / det, dl = (-a11 * F2 + a21 * F1) / det;
            b += db; l += dl;
            if (std::fabs (db) + std::fabs (dl) < 2e-6f) break;
        }
        const float tl = tanhFast (b * il);
        hp = xn - l - d * (tl / il);
        t1 = tanhFast (hp); t2 = tanhFast (b); bp = b; lp = l;
        if (! (std::fabs (lp) < 1e3f && std::fabs (bp) < 1e3f)) reset();
        const float lg = std::min (1.f, std::max (0.f, 2.f * (1.f - morph))), hg = std::min (1.f, std::max (0.f, 2.f * morph));
        return (l * lg + hp * hg) * (1.f / sc);
    }
};
// ---- surechantillonnage x4 autour d'une non-linearite (2 etages demi-bande) : les harmoniques creees au-dessus de Nyquist sont filtrees au lieu de se replier
template <int L> struct HalfBand                                    // L impair avec (L-1)/2 impair (11, 15, 23...) : un tap sur deux est nul, on ne calcule que les autres
{
    float h[L] {}, xh[L] {}, zh[L] {};
    HalfBand()
    {
        const int c = (L - 1) / 2; double sum = 0;
        for (int k = 0; k < L; ++k)
        {
            const double t = (k - c) * 0.5, sinc = (k == c) ? 1.0 : std::sin (3.14159265358979 * t) / (3.14159265358979 * t);
            const double w = 0.42 - 0.5 * std::cos (6.283185307179586 * k / (L - 1)) + 0.08 * std::cos (2.0 * 6.283185307179586 * k / (L - 1));   // Blackman
            h[k] = (float) (0.5 * sinc * w); sum += h[k];
        }
        for (auto& v : h) v = (float) (v / sum);
    }
    void up (float x, float& y0, float& y1)                             // 1 echantillon -> 2
    {
        const int n = (L + 1) / 2;
        for (int k = n - 1; k > 0; --k) xh[k] = xh[k - 1];
        xh[0] = x;
        float a = 0.f;
        for (int j = 0; j < n; ++j) a += h[2 * j] * xh[j];
        y0 = 2.f * a; y1 = 2.f * h[(L - 1) / 2] * xh[(L - 3) / 4];
    }
    float down (float z0, float z1)                                     // 2 echantillons -> 1
    {
        for (int k = L - 1; k > 1; --k) zh[k] = zh[k - 2];
        zh[1] = z0; zh[0] = z1;
        float y = h[(L - 1) / 2] * zh[(L - 1) / 2];
        for (int k = 0; k < L; k += 2) y += h[k] * zh[k];
        return y;
    }
};
struct Over4
{
    HalfBand<23> a; HalfBand<11> b;
    template <typename F> float run (float x, F&& nl)
    {
        float u0, u1; a.up (x, u0, u1);
        float w0, w1, w2, w3; b.up (u0, w0, w1); b.up (u1, w2, w3);
        w0 = nl (w0); w1 = nl (w1); w2 = nl (w2); w3 = nl (w3);
        const float d0 = b.down (w0, w1), d1 = b.down (w2, w3);
        return a.down (d0, d1);
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
        fs = sampleRate; msC.prepare (fs); sy::Ms10Tab::get();
        for (auto& v : voices) v = Voice();
        for (int i = 0; i < kLfos; ++i) { gPhase[i] = 0.f; gSh[i] = 0.f; gCur[i] = 0.f; }
        counter = 0; seed = 12345u;
    }
    void setParams (const SynthParams& p)
    {
        const int oldAssign = P.assign; const bool oldHold = P.hold != 0;
        P = p; Tg = p;
        if (! smInit) { for (int i = 0; i < kSmN; ++i) sm[i] = sm1[i] = p.*kSm[i]; smInit = true; }
        if (P.assign != oldAssign) allNotesOff();
        else if (oldHold && ! P.hold) dropLatched();
    }
    void setChord (const int* iv, int n) { chordN = std::max (0, std::min (6, n)); for (int i = 0; i < chordN; ++i) chord[i] = iv[i]; }
    float lfoLive (int i) const { return lfoWave (gPhase[i], P.lfo[i].wave, gCur[i]); }   // valeur courante d'un LFO global (affichage des anneaux)
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
            {   // reglages continus lisses (~20 ms) : un reglage qui saute d'un bloc hote a l'autre devient une rampe, plus d'escalier audible
                const float sc = 1.f - std::exp (-dt / 0.0055f);                                   // deux lisseurs en cascade : une marche (souris, CC MIDI a 7 bits) devient une courbe en S sans cassure
                for (int i = 0; i < kSmN; ++i) { sm1[i] += (Tg.*kSm[i] - sm1[i]) * sc; sm[i] += (sm1[i] - sm[i]) * sc; P.*kSm[i] = sm[i]; }
            }
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
        const float dcC = 1.f - std::exp (-6.2831853f * 6.f / fs);                              // coupe-continu a 6 Hz : la saturation asymetrique ne laisse plus de composante continue en sortie
        for (int i = 0; i < n; ++i)
        {
            float l = std::tanh (L[i] * P.master * 0.7f), r = std::tanh (R[i] * P.master * 0.7f);
            dcL += dcC * (l - dcL); dcR += dcC * (r - dcR);
            L[i] = l - dcL; R[i] = r - dcR;
        }
        computeFxMod();
    }

private:
    static constexpr int kCtl = 16;
    static constexpr float SynthParams::* kSm[] = { &SynthParams::lpf, &SynthParams::lpfRes, &SynthParams::hpf, &SynthParams::hpfRes, &SynthParams::drive, &SynthParams::semMorph, &SynthParams::fold, &SynthParams::foldSym,
        &SynthParams::fm, &SynthParams::ring, &SynthParams::lvl1, &SynthParams::lvl2, &SynthParams::noise, &SynthParams::pw1, &SynthParams::pw2, &SynthParams::fltEnv, &SynthParams::keytrack, &SynthParams::master };
    static constexpr int kSmN = (int) (sizeof (kSm) / sizeof (kSm[0]));
    sy::Ms10Cst msC;
    SynthParams Tg; float sm[kSmN] {}, sm1[kSmN] {}; float dcL = 0.f, dcR = 0.f; bool smInit = false;
    struct Voice
    {
        bool active = false; int note = 60; float vel = 1.f; uint64_t age = 0;
        bool gated = false, raw = false; int slot = -1; float uniCents = 0.f, uniPan = 0.f, gainV = 1.f;
        sy::Env aenv, fenv; float pitch = 60.f, ph1 = 0.f, ph2 = 0.f, prev1 = 0.f, prev2 = 0.f, fmT = 0.f;
        sy::Hp hp; sy::Lp lp; sy::Sem sem; sy::Ladder lad; sy::Ms10 ms; sy::Cem3320 cem; sy::SemCkt sck; float pat = 0.f, patS = 0.f, rndv = 0.f; float xPrev = 0.f, drvLp = 0.f; sy::Over4 osF, osD; bool cInit = false; float c0[20] {}; float lph[kLfos] {}, lfade[kLfos] {}, lsh[kLfos] {}, lcur[kLfos] {}; float envM[8] {};     // envM : modulation des enveloppes (bloc precedent)
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
        // valeurs de controle : calculees une fois par bloc de 16 echantillons, puis INTERPOLEES echantillon par echantillon (pas de marches)
        float c1[20] = { gL, gH, kL, kH, kSem, kLad, res, morph, drive, drvEmph, foldA, foldS, level, lv1, l2, noiseA, 0.f, 0.f, 0.f, 0.f };
        if (P.ftype == 3) sy::Ms10Tab::get().controls (2.0 * fs * (double) gL, (double) res, msC, c1[16], c1[17], c1[18]);       // MS-10 : resistance des etages, retour PEAK
        if (P.ftype == 4) c1[16] = 4.04f * res;                                                                                  // CEM3320 : gain de retour (auto-oscillation au-dessus de 4)
        else if (P.ftype == 5) c1[16] = 1.9f * std::pow (1.f - res, 1.3f) + 0.03f;                                               // SEM : amortissement (potard RES)
        if (! v.cInit) { for (int q = 0; q < 20; ++q) v.c0[q] = c1[q]; v.cInit = true; }
        float cd[20]; for (int q = 0; q < 20; ++q) cd[q] = (c1[q] - v.c0[q]) / (float) len;
        float cc[20]; for (int q = 0; q < 20; ++q) cc[q] = v.c0[q];

        for (int i = 0; i < len; ++i)
        {
            for (int q = 0; q < 20; ++q) cc[q] += cd[q];
            const float gL = cc[0], gH = cc[1], kL = cc[2], kH = cc[3], kSem = cc[4], kLad = cc[5], res = cc[6], morph = cc[7], drive = cc[8], drvEmph = cc[9];
            const float foldA = cc[10], foldS = cc[11], level = cc[12], lv1 = cc[13], l2 = cc[14], noiseA = cc[15];
            // FM audio (sens au choix) ; VCO2 est recale par VCO1 si le hard sync est actif
            // increments de phase bornes sous Nyquist (une FM extreme sur une note aigue pouvait les faire exploser -> phase hors [0,1[ -> NaN)
            const float inc1 = std::min (0.49f, std::max (0.f, (P.fmDir != 1 && fmA > 1e-5f) ? dt1 * std::pow (2.f, fmA * 2.5f * v.prev2) : dt1));
            const float inc2 = std::min (0.49f, std::max (0.f, (P.fmDir != 0 && fmA > 1e-5f) ? dt2 * std::pow (2.f, fmA * 2.5f * v.prev1) : dt2));
            v.ph1 += inc1;
            if (v.ph1 >= 1.f) { v.ph1 -= std::floor (v.ph1); if (P.sync) v.ph2 = v.ph1 * inc2 / std::max (inc1, 1e-6f); }
            v.ph2 += inc2; if (v.ph2 >= 1.f) v.ph2 -= std::floor (v.ph2);
            const float o1 = sy::osc (P.wave1, v.ph1, std::min (0.45f, inc1), pw1);
            const float o2 = sy::osc (P.wave2, v.ph2, std::min (0.45f, inc2), pw2);
            if (P.fmWave == 0) { v.prev1 = o1; v.prev2 = o2; }
            else                                                                  // modulateur dedie : sinus, triangle ou carre
            {
                auto m = [this] (float ph, float inc) { return P.fmWave == 1 ? std::sin (6.2831853f * ph) : sy::osc (P.fmWave == 2 ? 2 : 1, ph, std::min (0.45f, inc), 0.5f); };
                v.prev1 = m (v.ph1, inc1); v.prev2 = m (v.ph2, inc2);
            }
            float x = o1 * lv1 + o2 * l2 + o1 * o2 * ringA * 1.4f + rnd() * noiseA * 0.7f;
            x = v.osF.run (x, [foldA, foldS] (float u) { return sy::fold (u, foldA, foldS); });      // wavefolder surechantillonne x4 (plus de repliement de spectre)
            const float hp = v.hp.run (x * 0.7f, gH, kH);                  // HPF puis LPF, comme sur le MS20
            // DRIVE "shiny" : on accentue le haut du spectre AVANT la saturation (les graves gonflent moins, les harmoniques brillent), puis on rend le niveau
            v.drvLp += drvC * (hp - v.drvLp);
            const float hiPart = hp - v.drvLp;
            const float dIn = hp + drvEmph * hiPart;
            const float dBias = 0.06f * (drive - 1.f), dOff = std::tanh (dBias), dNorm = 1.f / (1.f + 0.12f * (drive - 1.f));
            const float dx = v.osD.run (dIn, [drive, dBias, dOff, dNorm] (float u) { return (sy::tanhFast (u * drive + dBias) - dOff) * dNorm; });   // saturation surechantillonnee x4
            float lp;
            if (P.ftype == 1)      lp = v.sem.run (dx, gL, kSem, morph) * 1.15f;
            else if (P.ftype == 2) lp = v.lad.run (dx * 1.2f, gL, kLad) * (1.f + 1.0f * res);
            else if (P.ftype == 3) lp = v.ms.run (dx, cc[16], cc[17], cc[18], msC);
            else if (P.ftype == 4) lp = v.cem.run (dx, gL * (1.f + 0.075f * std::min (1.f, std::max (0.f, (cc[16] - 3.8f) * 4.2f))), cc[16]) * (1.f + 1.2f * res);
            else if (P.ftype == 5) lp = v.sck.run (dx, gL, cc[16], morph);
            else                   lp = v.lp.run (dx, gL, kL, res);
            const float a = v.aenv.tick (aA2, aD2, aS2, aR2, fs);
            const float y = lp * a * level * v.gainV * (0.35f + 0.65f * v.vel);
            if (! (std::fabs (y) < 8.f))                                   // garde-fou : NaN, infini ou valeur folle -> on remet les etats a zero (jamais de bruit ni de blocage)
            {
                v.hp = sy::Hp(); v.lp = sy::Lp(); v.sem = sy::Sem(); v.lad = sy::Ladder(); v.ms.reset(); v.cem.reset(); v.sck.reset(); v.drvLp = 0.f; v.xPrev = 0.f; v.prev1 = v.prev2 = 0.f; v.osF = sy::Over4(); v.osD = sy::Over4();
                if (! std::isfinite (v.ph1)) v.ph1 = 0.f;
                if (! std::isfinite (v.ph2)) v.ph2 = 0.f;
                continue;
            }
            L[i] += y * gl; R[i] += y * gr;
        }
        for (int q = 0; q < 20; ++q) v.c0[q] = c1[q];
        if (v.aenv.st == sy::Env::Idle) v.active = false;
    }
};
} // namespace orage
