// Arpegiateur / sequenceur facon Korg Monopoly, etendu : grille de pas (silence / note / accent, ratchet), gate, swing, probabilite,
// 7 directions (dont CHORD : l'accord entier a chaque pas, v23), vitesse propre a chaque pas (v23) et 4 pistes de modulation (16 pas chacune, longueurs independantes) qui pilotent n'importe quelle destination.
// Les pas sont cadences par une horloge propre (libre ou calee sur le tempo). Le motif des notes depend du mode d'affectation des touches
// (POLY arpege les notes tenues, MONO / UNISON une note a travers les octaves, CHORD l'accord memorise, SHARE le partage des 6 voix).
// Aucune dependance JUCE.
#pragma once
#include <vector>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>

namespace orage
{
constexpr int kArpSteps = 16, kArpLanes = 8;
// vitesse de lecture d'un pas (v23) : code enregistre -> multiplicateur (x2 = le pas dure deux fois moins longtemps). Ne jamais reordonner : ajouter a la fin.
constexpr int kNumSpeeds = 5;
inline float speedMul (int code) { static const float m[kNumSpeeds] { 1.f, 2.f, 4.f, 0.5f, 1.5f }; return m[code < 0 || code >= kNumSpeeds ? 0 : code]; }
inline const char* speedLabel (int code) { static const char* t[kNumSpeeds] { "x1", "x2", "x4", "x1/2", "x3/2" }; return t[code < 0 || code >= kNumSpeeds ? 0 : code]; }
constexpr int kArpDirChord = 6;             // direction CHORD : l'accord tenu rejoue en bloc a chaque pas (RANGE le fait monter d'octave en octave)
constexpr int kMaxChordNotes = 8;
constexpr int kSeqChordMax = 6;             // Chord Seq (v24) : jusqu'a 6 notes par pas (la note du pas + 5 notes ajoutees)

// motif partage entre l'interface et le thread audio (valeurs atomiques, lecture sans verrou)
struct ArpPattern
{
    std::atomic<int> step[kArpSteps];       // 0 silence, 1 note, 2 accent
    std::atomic<int> ratchet[kArpSteps];    // 1..4 repetitions de la note dans le pas
    std::atomic<float> lane[kArpLanes][kArpSteps];   // 0..1
    std::atomic<int> seqNote[kArpSteps];    // mode SEQ : hauteur MIDI de chaque pas
    std::atomic<int> tie[kArpSteps];        // mode SEQ : 1 = la note tient tout le pas (sans trou avec la suivante)
    std::atomic<int> speed[kArpSteps];      // vitesse du pas (v23) : code kSpeedX (0 = x1, defaut)
    std::atomic<int> chordNote[kArpSteps][kSeqChordMax - 1];   // Chord Seq (v24) : notes ajoutees au pas (hauteur MIDI, -1 = aucune)
    std::atomic<int> recPos { 0 };          // mode SEQ : prochain pas a ecrire en enregistrement
    ArpPattern() { reset(); }
    // notes du pas (la note du pas d'abord, puis l'accord) ; renvoie leur nombre (1 a kSeqChordMax)
    int stepNotes (int i, int* out) const
    {
        int n = 0; out[n++] = seqNote[i].load();
        for (int k = 0; k < kSeqChordMax - 1; ++k) { const int v = chordNote[i][k].load(); if (v >= 0) out[n++] = v; }
        return n;
    }
    // ecrit un pas : notes[0] = note du pas, les suivantes = accord
    void setStepNotes (int i, const int* notes, int n)
    {
        seqNote[i] = n > 0 ? notes[0] : 60;
        for (int k = 0; k < kSeqChordMax - 1; ++k) chordNote[i][k] = k + 1 < n ? notes[k + 1] : -1;
    }
    void transposeStep (int i, int d)
    {
        int nt[kSeqChordMax]; const int n = stepNotes (i, nt);
        int lo = 127, hi = 0; for (int k = 0; k < n; ++k) { lo = std::min (lo, nt[k]); hi = std::max (hi, nt[k]); }
        d = std::max (-lo, std::min (127 - hi, d));                          // l'accord garde sa forme : il s'arrete au bord du clavier
        for (int k = 0; k < n; ++k) nt[k] += d;
        setStepNotes (i, nt, n);
    }
    void reset()
    {
        for (int i = 0; i < kArpSteps; ++i) { step[i] = 1; ratchet[i] = 1; seqNote[i] = 60; tie[i] = 0; speed[i] = 0; for (auto& c : chordNote[i]) c = -1; for (auto& l : lane) l[i] = 0.f; }
        recPos = 0;
    }
    void set (int i, int st, int rt) { step[i] = st; ratchet[i] = rt; }
    // v26 : copie d'un pas complet (CTRL+D, CTRL+Z)
    struct StepData { int step = 1, ratchet = 1, note = 60, tie = 0, speed = 0; int chord[kSeqChordMax - 1]; float lane[kArpLanes]; };
    StepData get (int i) const
    {
        StepData d; d.step = step[i].load(); d.ratchet = ratchet[i].load(); d.note = seqNote[i].load(); d.tie = tie[i].load(); d.speed = speed[i].load();
        for (int k = 0; k < kSeqChordMax - 1; ++k) d.chord[k] = chordNote[i][k].load();
        for (int l = 0; l < kArpLanes; ++l) d.lane[l] = lane[l][i].load();
        return d;
    }
    void put (int i, const StepData& d)
    {
        step[i] = d.step; ratchet[i] = d.ratchet; seqNote[i] = d.note; tie[i] = d.tie; speed[i] = d.speed;
        for (int k = 0; k < kSeqChordMax - 1; ++k) chordNote[i][k] = d.chord[k];
        for (int l = 0; l < kArpLanes; ++l) lane[l][i] = d.lane[l];
    }
};

// v26 : historique du motif (CTRL+Z annuler, CTRL+Y / CTRL+MAJ+Z refaire) : le motif entier + sa longueur, 64 etapes
struct ArpSnapshot { ArpPattern::StepData st[kArpSteps]; int len = 8; };
struct ArpHistory
{
    std::vector<ArpSnapshot> undo, redo;
    uint32_t lastMs = 0; int lastKey = -1;
    void push (const ArpSnapshot& s) { undo.push_back (s); if (undo.size() > 64) undo.erase (undo.begin()); redo.clear(); }
};

struct ArpParams
{
    int on = 0;            // 0 off, 1 on, 2 latch
    int dir = 0;           // 0 up, 1 down, 2 up-down, 3 played, 4 random, 5 converge, 6 chord
    int range = 0;         // 0..3 : 1 a 4 octaves
    int ratchet = 1;       // ratchet global 1..4 (comme le potard RATCHET de l'Hydrasynth) : repetitions minimum de chaque pas
    int assign = 0;        // mode d'affectation des touches (voir Synth : poly, mono, unison, share, chord)
    bool hold = false;     // HOLD du synthe : meme effet que LATCH
    float gate = 0.5f;     // duree de la note dans le pas (0.1 .. 1)
    float swing = 0.f;     // 0 .. 1 (0.5 de decalage maxi)
    float chance = 1.f;    // probabilite de jouer un pas actif
    bool seq = false;      // mode SEQ : une note memorisee par pas (au lieu des touches tenues)
    bool free = false;     // les pistes de modulation tournent meme sans touche tenue
    int len = 8;           // longueur du motif de notes (1..16)
    int laneLen[kArpLanes] { 8, 8, 8, 8, 8, 8, 8, 8 };
};

class Arp
{
public:
    // lane >= 0 : evenement de piste de modulation (valeur 0..1), sinon evenement de note
    struct Ev { int pos; bool on; int note; float vel; bool stack; int lane = -1; float val = 0.f; };

    void prepare (float sampleRate) { fs = sampleRate; reset(); }
    void reset() { nKeys = 0; nSnd = 0; soundingStack = false; idx = 0; phase = 0.999999f; stamp = 0; stepCount = 0; running = false; stepOdd = false; active = false; ratchetN = 1; lastSub = -1; idle = 1.0e12; curMul = 1.f; chordOct = 0; }
    void setParams (const ArpParams& p) { P = p; }
    void setPattern (const ArpPattern* p) { pat = p; }
    void setChord (const int* iv, int n) { chordN = std::min (6, std::max (0, n)); for (int i = 0; i < chordN; ++i) chord[i] = iv[i]; }
    void allOff() { nKeys = 0; }
    // mode SEQ : horloge calee sur le DAW quand il joue (position en pas depuis le debut du morceau, au debut du bloc)
    void setHost (bool lock, double stepPos) { hostLock = lock && stepPos >= 0.0; hostStep = stepPos; }
    int currentStep() const { return running ? (stepCount > 0 ? (stepCount - 1) : 0) : -1; }   // pour l'affichage du pas courant

    void noteOn (int note, float vel)
    {
        const bool latch = P.on == 2 || P.hold;
        if (latch && physCount() == 0 && nKeys > 0) nKeys = 0;           // nouvelle prise apres relachement complet : remplace les notes verrouillees
        for (int i = 0; i < nKeys; ++i) if (keys[i].note == note) { keys[i].vel = vel; keys[i].phys = true; keys[i].t = ++stamp; return; }
        if (nKeys == 0 && ! P.free && ! P.seq) { phase = 0.999999f; idx = 0; stepCount = 0; chordOct = 0; }   // le premier pas part tout de suite (SEQ : l'horloge continue, voir process)
        if (nKeys < kKeys) keys[nKeys++] = { note, vel, true, ++stamp };
    }
    void noteOff (int note)
    {
        for (int i = 0; i < nKeys; ++i)
            if (keys[i].note == note)
            {
                if (P.on == 2 || P.hold) keys[i].phys = false;
                else { for (int j = i; j + 1 < nKeys; ++j) keys[j] = keys[j + 1]; --nKeys; }
                return;
            }
    }

    // stepHz : cadence des pas ; renvoie le nombre d'evenements ecrits
    int process (int n, float stepHz, Ev* out, int maxOut)
    {
        int cnt = 0;
        auto off = [&] (int pos)
        {
            for (int k = 0; k < nSnd; ++k) if (cnt < maxOut) out[cnt++] = { pos, false, snd[k], 0.f, soundingStack };
            nSnd = 0;
        };
        auto lanesOut = [&] (int pos, bool zero)
        {
            for (int l = 0; l < kArpLanes && cnt < maxOut; ++l)
            {
                Ev e { pos, false, 0, 0.f, false, l, 0.f };
                if (! zero && pat != nullptr) e.val = pat->lane[l][stepCount % std::max (1, std::min (kArpSteps, P.laneLen[l]))].load();
                out[cnt++] = e;
            }
        };
        if (P.on == 0) { off (0); if (running) { lanesOut (0, true); running = false; } phase = 0.999999f; stepCount = 0; return cnt; }
        incBase = std::max (0.f, stepHz) / fs;
        const float stepLen = stepHz > 0.01f ? fs / stepHz : fs;
        sw = std::min (0.5f, std::max (0.f, P.swing) * 0.5f);
        // nouveau pas : sa vitesse s'applique, le reste de phase deja parcouru est converti a la nouvelle cadence
        auto enterStep = [&] (int cs)
        {
            const float oldInc = incOf (curMul, stepOdd);
            stepOdd = (cs & 1) != 0;
            curMul = mulAt (cs);
            const float newInc = incOf (curMul, stepOdd);
            if (oldInc > 0.f) phase *= newInc / oldInc;
            if (phase >= 1.f) phase = 0.999f;
        };
        if (P.seq && hostLock)
        {
            // SEQ calé sur le DAW : position "sans swing ni vitesse" de l'horloge interne comparee a celle du morceau ; recalage si elles divergent
            const int k = std::max (0, stepCount - 1);
            const double sIn = posOf (k, std::min (phase, 1.f));
            if (std::abs (sIn - hostStep) > 0.02 || stepCount == 0)
            {
                int kk = 0; float ph = 0.f;
                locate (hostStep, kk, ph);
                stepOdd = (kk & 1) != 0; curMul = mulAt (kk);
                if (go0() && ph * stepLen * durOf (kk) < 0.03f * fs) { stepCount = kk; phase = 1.f + ph; }  // le pas vient de commencer (< 30 ms) : il part tout de suite
                else { stepCount = kk + 1; phase = ph; active = false; }
            }
        }
        for (int i = 0; i < n; ++i)
        {
            const bool go = go0();
            if (! go)
            {
                off (i);
                if (running) { lanesOut (i, true); running = false; }
                if (P.seq)
                {
                    // SEQ : l'horloge continue sans jouer, pour qu'une nouvelle note reparte en tempo
                    if (idle < 1.0e9) idle += 1.0;
                    phase += incOf (curMul, stepOdd);
                    if (phase >= 1.f) { phase -= std::floor (phase); enterStep (stepCount++); }
                    active = false;
                    continue;
                }
                phase = 0.999999f; idx = 0; stepCount = 0; active = false; chordOct = 0;
                continue;
            }
            if (P.seq && ! running)
            {
                const double idleMax = (double) stepLen * (double) std::max (4, std::min (kArpSteps, P.len));
                if (! hostLock && idle > idleMax) { phase = 0.999999f; stepCount = 0; idx = 0; }   // longue pause : le motif repart du debut, tout de suite
                else if (phase * stepLen / std::max (0.25f, curMul) < 0.03f * fs && stepCount > 0) { --stepCount; phase += 1.f; }   // reprise en tempo : touche a peine en retard (< 30 ms) -> le pas en cours joue quand meme
            }
            idle = 0.0;
            running = true;
            phase += incOf (curMul, stepOdd);
            if (phase >= 1.f)
            {
                phase -= std::floor (phase);
                off (i);
                lastSub = -1;
                lanesOut (i, false);
                const int cs = stepCount++;
                enterStep (cs);
                active = false;
                tieNow = false;
                if (pat != nullptr && P.seq && (nKeys > 0 || P.free))
                {
                    const int pos = cs % patLen();
                    const int st = pat->step[pos].load();
                    if (st > 0 && uni() < P.chance)
                    {
                        const int trans = nKeys > 0 ? lastNote() - 60 : 0;         // la touche jouee transpose le motif (do central = hauteurs d'origine)
                        int nt[kSeqChordMax]; const int nn = pat->stepNotes (pos, nt);    // Chord Seq (v24) : un pas peut porter un accord
                        nCur = 0;
                        for (int k = 0; k < nn; ++k)
                        {
                            const int v = std::min (127, std::max (0, nt[k] + trans));
                            bool dup = false; for (int q = 0; q < nCur; ++q) dup = dup || cur[q] == v;
                            if (! dup) cur[nCur++] = v;
                        }
                        const float v = nKeys > 0 ? lastVel() : 0.8f;
                        curVel = st == 2 ? std::min (1.f, v * 1.15f + 0.25f) : v;
                        ratchetN = std::max (std::max (1, std::min (4, P.ratchet)), std::min (4, pat->ratchet[pos].load()));
                        tieNow = pat->tie[pos].load() != 0 && ratchetN == 1;
                        active = true;
                    }
                }
                else if (pat != nullptr && nKeys > 0)
                {
                    const int pos = cs % patLen();
                    const int st = pat->step[pos].load();
                    if (st > 0 && uni() < P.chance)
                    {
                        if (P.dir == kArpDirChord)
                        {
                            // CHORD : toutes les notes tenues (ou l'accord memorise) d'un coup ; RANGE : l'accord monte d'une octave a chaque pas joue
                            nCur = buildChord (cur, chordOct);
                            chordOct = (chordOct + 1) % std::max (1, std::min (4, P.range + 1));
                        }
                        else
                        {
                            int seq[128]; const int len = buildSeq (seq);
                            nCur = 0;
                            if (len > 0)
                            {
                                if (P.dir == 4) idx = (int) (uni() * len) % len; else idx %= len;
                                cur[0] = seq[idx]; nCur = 1; if (P.dir != 4) ++idx;
                            }
                        }
                        if (nCur > 0)
                        {
                            const float v = lastVel();
                            curVel = st == 2 ? std::min (1.f, v * 1.15f + 0.25f) : v;
                            ratchetN = std::max (std::max (1, std::min (4, P.ratchet)), std::min (4, pat->ratchet[pos].load()));
                            active = true;
                        }
                    }
                }
            }
            if (active)
            {
                const int sub = std::min (ratchetN - 1, (int) (phase * (float) ratchetN));
                const float sp = phase * (float) ratchetN - (float) sub;
                if (sub != lastSub)
                {
                    off (i); lastSub = sub;
                    const bool stack = P.assign == 2;
                    for (int k = 0; k < nCur && cnt < maxOut; ++k) { out[cnt++] = { i, true, cur[k], curVel, stack }; snd[nSnd++] = cur[k]; }
                    soundingStack = stack;
                }
                else if (nSnd > 0 && ! tieNow && sp >= std::min (0.98f, std::max (0.05f, P.gate))) off (i);
            }
            else if (nSnd > 0) off (i);
        }
        return cnt;
    }

private:
    static constexpr int kKeys = 16;
    struct Key { int note; float vel; bool phys; uint64_t t; };
    Key keys[kKeys];
    int nKeys = 0, idx = 0, chord[6] { 0, 4, 7, 11, 0, 0 }, chordN = 4, stepCount = 0, ratchetN = 1, lastSub = -1, chordOct = 0;
    int snd[kMaxChordNotes] {}, nSnd = 0, cur[kMaxChordNotes] { 60 }, nCur = 0;          // notes qui sonnent / notes du pas en cours (CHORD : plusieurs)
    float incBase = 0.f, sw = 0.f, curMul = 1.f;
    bool soundingStack = false, running = false, stepOdd = false, active = false, tieNow = false;
    float fs = 44100.f, phase = 0.999999f, curVel = 0.8f;
    double idle = 1.0e12, hostStep = 0.0; bool hostLock = false;
    bool go0() const { return nKeys > 0 || P.free; }
    int patLen() const { return std::max (1, std::min (kArpSteps, P.len)); }
    float mulAt (int k) const { return pat != nullptr && k >= 0 ? speedMul (pat->speed[k % patLen()].load()) : 1.f; }
    float incOf (float mul, bool odd) const { return incBase * mul / (odd ? (1.f - sw) : (1.f + sw)); }
    // duree d'un pas en pas "droits" (swing et vitesse du pas compris) : le pas pair dure 1+sw, l'impair 1-sw, divise par la vitesse
    double durOf (int k) const { return (double) ((k & 1) ? (1.f - sw) : (1.f + sw)) / (double) mulAt (k); }
    int cycleSteps() const { const int L = patLen(); return (L & 1) ? 2 * L : L; }    // le motif + la parite du swing se repetent tous les M pas
    double cycleLen() const { double t = 0.0; const int M = cycleSteps(); for (int k = 0; k < M; ++k) t += durOf (k); return t; }
    double posOf (int k, float ph) const
    {
        const int M = cycleSteps(); const double T = cycleLen();
        const int c = k / M, r = k % M; double t = (double) c * T;
        for (int j = 0; j < r; ++j) t += durOf (j);
        return t + (double) ph * durOf (r);
    }
    void locate (double pos, int& k, float& ph) const
    {
        const int M = cycleSteps(); const double T = std::max (1.0e-6, cycleLen());
        const double c = std::floor (pos / T); double u = pos - c * T;
        int r = 0;
        while (r < M - 1 && u >= durOf (r)) { u -= durOf (r); ++r; }
        k = (int) c * M + r; ph = (float) std::min (0.999999, std::max (0.0, u / durOf (r)));
    }
    uint64_t stamp = 0; uint32_t rng = 0x1234567u;
    ArpParams P;
    const ArpPattern* pat = nullptr;

    float uni() { rng = rng * 1664525u + 1013904223u; return (float) (rng >> 8) / 16777216.f; }
    int physCount() const { int c = 0; for (int i = 0; i < nKeys; ++i) c += keys[i].phys ? 1 : 0; return c; }
    float lastVel() const { float v = 0.8f; uint64_t t = 0; for (int i = 0; i < nKeys; ++i) if (keys[i].t >= t) { t = keys[i].t; v = keys[i].vel; } return v; }
    int lastNote() const { int nt = 60; uint64_t t = 0; for (int i = 0; i < nKeys; ++i) if (keys[i].t >= t) { t = keys[i].t; nt = keys[i].note; } return nt; }

    // CHORD : les notes de l'accord (sans doublons, de la plus grave a la plus aigue), transposees de oct octaves
    int buildChord (int* outN, int oct) const
    {
        int b[16], nb = 0;
        switch (P.assign)
        {
            case 1: case 2: b[nb++] = lastNote(); break;                                       // mono / unison : une seule note
            case 4: b[nb++] = lastNote(); for (int i = 0; i < chordN && nb < 12; ++i) b[nb++] = lastNote() + chord[i]; break;   // chord memory
            default: for (int i = 0; i < nKeys && nb < 16; ++i) b[nb++] = keys[i].note; break;  // poly / share : les touches tenues
        }
        std::sort (b, b + nb);
        int m = 0;
        for (int i = 0; i < nb && m < kMaxChordNotes; ++i)
        {
            const int nt = b[i] + 12 * oct;
            if (nt > 127 || (m > 0 && outN[m - 1] == nt)) continue;
            outN[m++] = nt;
        }
        return m;
    }

    int buildSeq (int* seq) const
    {
        int base[16], nb = 0;
        switch (P.assign)
        {
            case 1: case 2: base[nb++] = lastNote(); break;                                   // mono / unison : une seule note
            case 4:                                                                           // chord memory
                base[nb++] = lastNote();
                for (int i = 0; i < chordN && nb < 12; ++i) base[nb++] = lastNote() + chord[i];
                break;
            case 3:                                                                           // share : 6 voix reparties dans l'ordre de jeu
            {
                Key byTime[kKeys]; for (int i = 0; i < nKeys; ++i) byTime[i] = keys[i];
                std::sort (byTime, byTime + nKeys, [] (const Key& a, const Key& b) { return a.t < b.t; });
                for (int v = 0; v < 6; ++v) base[nb++] = byTime[v % nKeys].note;
                break;
            }
            default:                                                                          // poly : notes tenues
            {
                if (P.dir == 3)                                                               // ordre de jeu
                {
                    Key byTime[kKeys]; for (int i = 0; i < nKeys; ++i) byTime[i] = keys[i];
                    std::sort (byTime, byTime + nKeys, [] (const Key& a, const Key& b) { return a.t < b.t; });
                    for (int i = 0; i < nKeys; ++i) base[nb++] = byTime[i].note;
                }
                else { for (int i = 0; i < nKeys; ++i) base[nb++] = keys[i].note; std::sort (base, base + nb); }
                break;
            }
        }
        const int oct = std::max (1, std::min (4, P.range + 1));
        int up[64], nu = 0;
        for (int o = 0; o < oct; ++o)
            for (int i = 0; i < nb && nu < 64; ++i) if (base[i] + 12 * o <= 127) up[nu++] = base[i] + 12 * o;
        int len = 0;
        if (P.dir == 1)       { for (int i = nu - 1; i >= 0; --i) seq[len++] = up[i]; }
        else if (P.dir == 2)
        {
            for (int i = 0; i < nu; ++i) seq[len++] = up[i];
            for (int i = nu - 2; i >= 1; --i) seq[len++] = up[i];
        }
        else if (P.dir == 5)                                                                  // converge : bas, haut, bas+1, haut-1...
        {
            int lo = 0, hi = nu - 1;
            while (lo <= hi) { seq[len++] = up[lo++]; if (lo <= hi) seq[len++] = up[hi--]; }
        }
        else                  { for (int i = 0; i < nu; ++i) seq[len++] = up[i]; }            // up, played (ordre deja fait), random (tire dans la liste)
        return len;
    }
};
} // namespace orage
