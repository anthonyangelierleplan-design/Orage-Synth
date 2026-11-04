// Arpegiateur d'Orage : up / down / up-down / random, octaves, division, gate, ratchet, chance, hold. Aucune dependance JUCE.
// Cale sur le tempo d'Ableton (position PPQ) quand l'hote joue, sinon tourne librement au tempo de l'hote.
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace orage
{
inline const char* const kArpDivNames[6] { "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32" };
inline const float kArpDivBeats[6] { 1.f, 0.5f, 1.f / 3.f, 0.25f, 1.f / 6.f, 0.125f };

struct ArpParams
{
    bool on = false, hold = false;
    int mode = 0;           // 0 up, 1 down, 2 up-down, 3 random
    int div = 3;            // index de kArpDivNames
    int octaves = 1;        // 1..4
    float gate = 0.6f;      // 0..1 (part du pas)
    int ratchet = 1;        // 1..4 repetitions par pas
    float chance = 1.f;     // probabilite qu'un pas soit joue
};

class Arp
{
public:
    struct Ev { int pos; bool on; int note; float vel; };

    void prepare (float sampleRate) { fs = sampleRate; reset(); }
    void reset() { held.clear(); phys = 0; sounding = -1; gateLeft = 0; stepCount = 0; lastIdx = -1; freeBeat = 0.0; }
    void setParams (const ArpParams& p) { P = p; }

    void noteOn (int note, float vel)
    {
        if (P.hold && phys == 0) held.clear();
        ++phys;
        for (auto& h : held) if (h.note == note) { h.vel = vel; return; }
        held.push_back ({ note, vel });
        std::sort (held.begin(), held.end(), [] (const Key& a, const Key& b) { return a.note < b.note; });
    }
    void noteOff (int note)
    {
        phys = std::max (0, phys - 1);
        if (! P.hold) held.erase (std::remove_if (held.begin(), held.end(), [note] (const Key& k) { return k.note == note; }), held.end());
    }
    void allOff() { held.clear(); phys = 0; }
    int soundingNote() const { return sounding; }

    // genere les evenements du bloc (positions en echantillons). Si l'arp est coupe, il faut quand meme l'appeler : il relache la note en cours.
    int process (int n, double bpm, double ppqStart, bool hostPlaying, Ev* out, int maxEv)
    {
        int cnt = 0;
        auto push = [&] (int pos, bool on, int note, float vel) { if (cnt < maxEv) out[cnt++] = { pos, on, note, vel }; };
        if (! P.on || (held.empty() && ! P.hold) || (held.empty()))
        {
            if (sounding >= 0) { push (0, false, sounding, 0.f); sounding = -1; }
            gateLeft = 0; lastIdx = -1; stepCount = 0;
            if (! hostPlaying) freeBeat += (double) n * bpm / (60.0 * (double) fs);
            return cnt;
        }
        const float stepBeats = kArpDivBeats[std::max (0, std::min (5, P.div))];
        const int rat = std::max (1, std::min (4, P.ratchet));
        const double sub = (double) stepBeats / (double) rat;
        const double inc = bpm / (60.0 * (double) fs);
        const int gateSamples = std::max (8, (int) (std::max (0.05f, P.gate) * sub / inc));
        buildSeq();
        for (int i = 0; i < n; ++i)
        {
            const double b = hostPlaying ? ppqStart + (double) i * inc : freeBeat + (double) i * inc;
            const long long k = (long long) std::floor (b / sub + 1e-9);
            if (k != lastIdx)
            {
                lastIdx = k;
                const bool newStep = (((k % rat) + rat) % rat) == 0;
                if (newStep) stepPlays = rnd01() < std::max (0.f, std::min (1.f, P.chance));
                if (sounding >= 0) { push (i, false, sounding, 0.f); sounding = -1; }
                if (stepPlays && ! seq.empty())
                {
                    if (newStep) { curNote = pick(); ++stepCount; }
                    push (i, true, curNote.note, curNote.vel);
                    sounding = curNote.note; gateLeft = gateSamples;
                }
            }
            if (sounding >= 0 && gateLeft > 0 && --gateLeft == 0) { push (i, false, sounding, 0.f); sounding = -1; }
        }
        if (! hostPlaying) freeBeat += (double) n * inc;
        return cnt;
    }

private:
    struct Key { int note; float vel; };
    ArpParams P;
    float fs = 44100.f;
    std::vector<Key> held, seq;
    int phys = 0, sounding = -1, gateLeft = 0; long long stepCount = 0, lastIdx = -1;
    double freeBeat = 0.0; bool stepPlays = true; Key curNote { 60, 0.8f };
    uint32_t seed = 2463534242u;

    float rnd01() { seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5; return (float) (seed & 0xFFFFFF) / 16777216.f; }
    void buildSeq()
    {
        seq.clear();
        for (int o = 0; o < std::max (1, std::min (4, P.octaves)); ++o)
            for (const auto& k : held) if (k.note + 12 * o <= 127) seq.push_back ({ k.note + 12 * o, k.vel });
    }
    Key pick()
    {
        const int L = (int) seq.size();
        if (L == 1) return seq[0];
        switch (P.mode)
        {
            case 1:  return seq[(size_t) (L - 1 - (int) (stepCount % L))];
            case 2:  { const int per = 2 * L - 2, j = (int) (stepCount % per); return seq[(size_t) (j < L ? j : per - j)]; }
            case 3:  return seq[(size_t) std::min (L - 1, (int) (rnd01() * (float) L))];
            default: return seq[(size_t) (stepCount % L)];
        }
    }
};
} // namespace orage
