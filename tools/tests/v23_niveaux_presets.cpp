// Joue chaque preset d'usine (accord pour pads / claviers / arpeges, une note sinon) et mesure RMS, crete, NaN.
// Sert a caler le niveau ("output") des nouveaux presets sur ceux de leur categorie.
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include <cstdio>
#include <map>
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();
int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    std::unique_ptr<juce::AudioProcessor> base (createPluginFilter());
    auto* ob = dynamic_cast<OrageAudioProcessor*> (base.get());
    const int np = ob->getNumFactoryPresets(), from = argc > 1 ? atoi (argv[1]) : 1;
    const double fs = 48000.0; const int bs = 512, total = (int) (fs * 4.0);
    int bad = 0;
    for (int pi = from; pi < np; ++pi)
    {
        std::unique_ptr<juce::AudioProcessor> p (createPluginFilter());
        p->setPlayConfigDetails (0, 2, fs, bs);
        p->prepareToPlay (fs, bs);
        auto* op = dynamic_cast<OrageAudioProcessor*> (p.get());
        op->loadFactoryPreset (pi);
        const juce::String name = op->getFactoryPresetName (pi);
        const juce::String cat = op->presetCategory;
        const bool chord = cat == "Pad" || cat == "Pluck / Keys" || cat == "Arp / Seq" || cat == "FX / Texture";
        juce::AudioBuffer<float> buf (2, bs);
        double s2 = 0; long cnt = 0; float pk = 0; bool nan = false;
        for (int pos = 0; pos < total; pos += bs)
        {
            buf.clear(); juce::MidiBuffer mb;
            if (pos == 0) { mb.addEvent (juce::MidiMessage::noteOn (1, 48, (juce::uint8) 100), 0); if (chord) { mb.addEvent (juce::MidiMessage::noteOn (1, 52, (juce::uint8) 100), 0); mb.addEvent (juce::MidiMessage::noteOn (1, 55, (juce::uint8) 100), 0); } }
            if (pos <= (int) (fs * 3.0) && pos + bs > (int) (fs * 3.0)) for (int n : { 48, 52, 55 }) mb.addEvent (juce::MidiMessage::noteOff (1, n), (int) (fs * 3.0) - pos);
            p->processBlock (buf, mb);
            for (int c = 0; c < 2; ++c) for (int i = 0; i < bs; ++i)
            {
                const float v = buf.getSample (c, i);
                if (! std::isfinite (v)) nan = true;
                pk = std::max (pk, std::fabs (v));
                if (pos + i > (int) (fs * 0.5) && pos + i < (int) (fs * 3.0)) { s2 += (double) v * v; ++cnt; }
            }
        }
        const double rms = 10.0 * std::log10 (std::max (1e-12, s2 / (double) std::max (1L, cnt)));
        if (nan || pk > 1.0f) ++bad;
        printf ("%3d %-28s %-14s rms %6.1f dB  crete %.2f%s\n", pi, name.toRawUTF8(), cat.toRawUTF8(), rms, pk, nan ? "  NaN !" : "");
    }
    printf (bad ? "%d PROBLEME(S)\n" : "aucun NaN, aucune crete > 1\n", bad);
    return 0;
}
