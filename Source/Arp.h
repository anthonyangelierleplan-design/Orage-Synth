// Arpegiateur / sequenceur facon Korg Monopoly, etendu : grille de pas (silence / note / accent, ratchet), gate, swing, probabilite,
// 6 directions et 4 pistes de modulation (16 pas chacune, longueurs independantes) qui pilotent n'importe quelle destination.
// Les pas sont cadences par une horloge propre (libre ou calee sur le tempo). Le motif des notes depend du mode d'affectation des touches
// (POLY arpege les notes tenues, MONO / UNISON une note a travers les octaves, CHORD l'accord memorise, SHARE le partage des 6 voix).
// Aucune dependance JUCE.
#pragma once
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>

namespace orage
{
constexpr int kArpSteps = 16, kArpLanes = 4;

// motif partage entre l'interface et le thread audio (valeurs atomiques, lecture sans verrou)
struct ArpPattern
{
    std::atomic<int> step[kArpSteps];       // 0 silence, 1 note, 2 accent
    std::atomic<int> ratchet[kArpSteps];    // 1..4 repetitions de la note dans le pas
    std::atomic<float> lane[kArpLanes][kArpSteps];   // 0..1
    std::atomic<int> seqNote[kArpSteps];    // mode SEQ : hauteur MIDI de chaque pas
    std::atomic<int> tie[kArpSteps];        // mode SEQ : 1 = la note tient tout le pas (sans trou avec la suivante)
    std::atomic<int> recPos { 0 };          // mode SEQ : prochain pas a ecrire en enregistrement
    ArpPattern() { reset(); }
    void reset()
    {
        for (int i = 0; i < kArpSteps; ++i) { step[i] = 1; ratchet[i] = 1; seqNote[i] = 60; tie[i] = 0; for (auto& l : lane) l[i] = 0.f; }
        recPos = 0;
    }
    void set (int i, int st, int rt) { step[i] = st; ratchet[i] = rt; }
};

struct ArpParams
{
    int on = 0;            // 0 off, 1 on, 2 latch
    int dir = 0;           // 0 up, 1 down, 2 up-down, 3 played, 4 random, 5 converge
    int range = 0;         // 0 : 1 octave, 1 : 2 octaves, 2 : full (4 octaves)
    int assign = 0;        // mode d'affectation des touches (voir Synth : poly, mono, unison, share, chord)
    bool hold = false;     // HOLD du synthe : meme effet que LATCH
    float gate = 0.5f;     // duree de la note dans le pas (0.1 .. 1)
    float swing = 0.f;     // 0 .. 1 (0.5 de decalage maxi)
    float chance = 1.f;    // probabilite de jouer un pas actif
    bool seq = false;      // mode SEQ : une note memorisee par pas (au lieu des touches tenues)
    bool free = false;     // les pistes de modulation tournent meme sans touche tenue
    int len = 8;           // longueur du motif de notes (1..16)
    int laneLen[kArpLanes] { 8, 8, 8, 8 };
};

class Arp
{
public:
    // lane >= 0 : evenement de piste de modulation (valeur 0..1), sinon evenement de note
    struct Ev { int pos; bool on; int note; float vel; bool stack; int lane = -1; float val = 0.f; };

    void prepare (float sampleRate) { fs = sampleRate; reset(); }
    void reset() { nKeys = 0; sounding = -1; soundingStack = false; idx = 0; phase = 0.999999f; stamp = 0; stepCount = 0; running = false; stepOdd = false; active = false; ratchetN = 1; lastSub = -1; }
    void setParams (const ArpParams& p) { P = p; }
    void setPattern (const ArpPattern* p) { pat = p; }
    void setChord (const int* iv, int n) { chordN = std::min (6, std::max (0, n)); for (int i = 0; i < chordN; ++i) chord[i] = iv[i]; }
    void allOff() { nKeys = 0; }
    int currentStep() const { return running ? (stepCount > 0 ? (stepCount - 1) : 0) : -1; }   // pour l'affichage du pas courant

    void noteOn (int note, float vel)
    {
        const bool latch = P.on == 2 || P.hold;
        if (latch && physCount() == 0 && nKeys > 0) nKeys = 0;           // nouvelle prise apres relachement complet : remplace les notes verrouillees
        for (int i = 0; i < nKeys; ++i) if (keys[i].note == note) { keys[i].vel = vel; keys[i].phys = true; keys[i].t = ++stamp; return; }
        if (nKeys == 0 && ! P.free) { phase = 0.999999f; idx = 0; stepCount = 0; }   // le premier pas part tout de suite
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
        auto off = [&] (int pos) { if (sounding >= 0 && cnt < maxOut) out[cnt++] = { pos, false, sounding, 0.f, soundingStack }; sounding = -1; };
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
        const float base = std::max (0.f, stepHz) / fs;
        const float sw = std::min (0.5f, std::max (0.f, P.swing) * 0.5f);
        for (int i = 0; i < n; ++i)
        {
            const bool go = nKeys > 0 || P.free;
            if (! go)
            {
                off (i);
                if (running) { lanesOut (i, true); running = false; }
                phase = 0.999999f; idx = 0; stepCount = 0; active = false;
                continue;
            }
            running = true;
            phase += base / (stepOdd ? (1.f - sw) : (1.f + sw));
            if (phase >= 1.f)
            {
                phase -= std::floor (phase);
                off (i);
                lastSub = -1;
                lanesOut (i, false);
                const int cs = stepCount++;
                stepOdd = (cs & 1) != 0;
                active = false;
                tieNow = false;
                if (pat != nullptr && P.seq && (nKeys > 0 || P.free))
                {
                    const int pos = cs % std::max (1, std::min (kArpSteps, P.len));
                    const int st = pat->step[pos].load();
                    if (st > 0 && uni() < P.chance)
                    {
                        const int trans = nKeys > 0 ? lastNote() - 60 : 0;         // la touche jouee transpose le motif (do central = hauteurs d'origine)
                        curNote = std::min (127, std::max (0, pat->seqNote[pos].load() + trans));
                        const float v = nKeys > 0 ? lastVel() : 0.8f;
                        curVel = st == 2 ? std::min (1.f, v * 1.15f + 0.25f) : v;
                        ratchetN = std::max (1, std::min (4, pat->ratchet[pos].load()));
                        tieNow = pat->tie[pos].load() != 0 && ratchetN == 1;
                        active = true;
                    }
                }
                else if (pat != nullptr && nKeys > 0)
                {
                    const int pos = cs % std::max (1, std::min (kArpSteps, P.len));
                    const int st = pat->step[pos].load();
                    if (st > 0 && uni() < P.chance)
                    {
                        int seq[128]; const int len = buildSeq (seq);
                        if (len > 0)
                        {
                            if (P.dir == 4) idx = (int) (uni() * len) % len; else idx %= len;
                            curNote = seq[idx]; if (P.dir != 4) ++idx;
                            const float v = lastVel();
                            curVel = st == 2 ? std::min (1.f, v * 1.15f + 0.25f) : v;
                            ratchetN = std::max (1, std::min (4, pat->ratchet[pos].load()));
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
                    if (cnt < maxOut) { out[cnt++] = { i, true, curNote, curVel, stack }; sounding = curNote; soundingStack = stack; }
                }
                else if (sounding >= 0 && ! tieNow && sp >= std::min (0.98f, std::max (0.05f, P.gate))) off (i);
            }
            else if (sounding >= 0) off (i);
        }
        return cnt;
    }

private:
    static constexpr int kKeys = 16;
    struct Key { int note; float vel; bool phys; uint64_t t; };
    Key keys[kKeys];
    int nKeys = 0, sounding = -1, idx = 0, chord[6] { 0, 4, 7, 11, 0, 0 }, chordN = 4, stepCount = 0, curNote = 60, ratchetN = 1, lastSub = -1;
    bool soundingStack = false, running = false, stepOdd = false, active = false, tieNow = false;
    float fs = 44100.f, phase = 0.999999f, curVel = 0.8f;
    uint64_t stamp = 0; uint32_t rng = 0x1234567u;
    ArpParams P;
    const ArpPattern* pat = nullptr;

    float uni() { rng = rng * 1664525u + 1013904223u; return (float) (rng >> 8) / 16777216.f; }
    int physCount() const { int c = 0; for (int i = 0; i < nKeys; ++i) c += keys[i].phys ? 1 : 0; return c; }
    float lastVel() const { float v = 0.8f; uint64_t t = 0; for (int i = 0; i < nKeys; ++i) if (keys[i].t >= t) { t = keys[i].t; v = keys[i].vel; } return v; }
    int lastNote() const { int nt = 60; uint64_t t = 0; for (int i = 0; i < nKeys; ++i) if (keys[i].t >= t) { t = keys[i].t; nt = keys[i].note; } return nt; }

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
        const int oct = P.range == 0 ? 1 : (P.range == 1 ? 2 : 4);
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
