#include "SynthEditor.h"

using namespace orage;

namespace { juce::Font bold (float h) { return juce::Font (juce::FontOptions (h, juce::Font::bold)); } }

// ---------------------------------------------------------------- apercu de la forme d'onde du LFO
float LfoView::value (const juce::String& id) const
{
    if (auto* p = apvts.getRawParameterValue (id)) return p->load();
    return 0.f;
}

void LfoView::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff050506));
    g.fillRect (b);
    g.setColour (juce::Colour (0xff3a1c10));
    for (int i = 1; i < 4; ++i) g.drawHorizontalLine ((int) (b.getY() + b.getHeight() * (float) i / 4.f), b.getX() + 2.f, b.getRight() - 2.f);
    for (int i = 1; i < 4; ++i) g.drawVerticalLine ((int) (b.getX() + b.getWidth() * (float) i / 4.f), b.getY() + 2.f, b.getBottom() - 2.f);
    const int w = juce::roundToInt (value (wave));
    const float sl = value (slew);
    static const float steps[4] { 0.55f, -0.7f, 0.25f, -0.25f };                // valeurs types d'un S&H, un cycle = 4 marches
    juce::Path p;
    const int n = (int) b.getWidth();
    for (int i = 0; i <= n; ++i)
    {
        const float ph = (float) i / (float) n * 0.9999f;
        float y;
        if (w == 4)
        {
            const float pos = ph * 4.f;
            const int k = juce::jlimit (0, 3, (int) pos);
            const float frac = pos - (float) k, r = juce::jmax (0.001f, sl * 0.9f);
            const float t = juce::jlimit (0.f, 1.f, (frac - (1.f - r)) / r);
            y = steps[k] + (steps[(k + 1) % 4] - steps[k]) * t * t * (3.f - 2.f * t);
        }
        else
            y = orage::Synth::lfoWave (ph, w, 0.f);
        const float px = b.getX() + (float) i, py = b.getCentreY() - y * (b.getHeight() * 0.38f);
        if (i == 0) p.startNewSubPath (px, py); else p.lineTo (px, py);
    }
    g.setColour (juce::Colour (0xffff7a1a).withAlpha (0.3f));
    g.strokePath (p, juce::PathStrokeType (4.f));
    g.setColour (juce::Colour (0xffffb05a));
    g.strokePath (p, juce::PathStrokeType (1.6f));
    g.setColour (juce::Colour (0xffe9e5d8).withAlpha (0.6f));
    g.drawRect (b, 1.2f);
}

// ---------------------------------------------------------------- panneau synthe
KnobUnit* SynthPanel::knob (juce::Component& parent, const char* id, const juce::String& text, const char* lo, const char* hi)
{
    knobs.push_back (std::make_unique<KnobUnit> (proc.apvts, id, text, lo, hi));
    auto* k = knobs.back().get();
    parent.addAndMakeVisible (*k);
    return k;
}

ModuleUnit* SynthPanel::unit (const juce::String& title)
{
    units.push_back (std::make_unique<ModuleUnit> (nullptr, -1, title, 100, true));
    addAndMakeVisible (*units.back());
    return units.back().get();
}

SynthPanel::SynthPanel (OrageAudioProcessor& p) : proc (p)
{
    auto selector = [this] (juce::Component& parent, const char* id, const juce::StringArray& names)
    {
        auto s = std::make_unique<SegmentSelector> (proc.apvts, id, names);
        auto* r = s.get(); parent.addAndMakeVisible (*r); owned.push_back (std::move (s)); return r;
    };
    auto toggle = [this] (juce::Component& parent, const char* id, const char* label)
    {
        auto b = std::make_unique<LedToggle>();
        b->setButtonText (label);
        parent.addAndMakeVisible (*b);
        buttonAtt.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, id, *b));
        auto* r = b.get(); owned.push_back (std::move (b)); return r;
    };
    auto combo = [this] (juce::Component& parent, const char* id)
    {
        auto cb = std::make_unique<juce::ComboBox>();
        if (auto* ch = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter (id))) cb->addItemList (ch->choices, 1);
        cb->setScrollWheelEnabled (false);
        parent.addAndMakeVisible (*cb);
        comboAtt.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, id, *cb));
        auto* r = cb.get(); owned.push_back (std::move (cb)); return r;
    };
    // potards alignes verticalement au centre de la zone, a taille fixe
    auto place = [] (juce::Rectangle<int> r, std::vector<KnobUnit*> ks, int x0, int cellW)
    {
        int x = r.getX() + x0;
        const int kh = juce::jmin (r.getHeight(), cellW + 28), y = r.getY() + (r.getHeight() - kh) / 2;
        for (auto* k : ks) { k->setBounds (x, y, cellW, kh); x += cellW; }
    };
    const juce::StringArray waves { "SAW", "PULSE", "TRI" }, octs { "-2", "-1", "0", "+1", "+2" };
    const juce::StringArray lfoWaves { "SIN", "TRI", "SAW", "SQR", "S&H" };

    // ---- rang A : oscillateurs, melangeur, modulation croisee (indices 0..3)
    {
        auto* u = unit ("VOLTAGE CONTROLLED OSCILLATOR 1");
        auto* w = selector (*u, "s_wave1", waves); auto* o = selector (*u, "s_oct1", octs);
        auto* pw = knob (*u, "s_pw1", "PW");
        u->layoutFn = [=] (juce::Rectangle<int> r)
        {
            w->setBounds (r.getX(), r.getY() + 6, 170, 46); o->setBounds (r.getX(), r.getY() + 62, 170, 34);
            pw->setBounds (r.getX() + 180, r.getY() + 6, 88, r.getHeight() - 12);
        };
    }
    {
        auto* u = unit ("VOLTAGE CONTROLLED OSCILLATOR 2");
        auto* w = selector (*u, "s_wave2", waves); auto* o = selector (*u, "s_oct2", octs);
        auto* tu = knob (*u, "s_tune2", "PITCH", "-12", "+12"); auto* fi = knob (*u, "s_fine2", "FINE", "-", "+"); auto* pw = knob (*u, "s_pw2", "PW");
        u->layoutFn = [=] (juce::Rectangle<int> r)
        {
            w->setBounds (r.getX(), r.getY() + 6, 170, 46); o->setBounds (r.getX(), r.getY() + 62, 170, 34);
            place (r, { tu, fi, pw }, 180, 88);
        };
    }
    {
        auto* u = unit ("VCO MIXER");
        auto* a = knob (*u, "s_lvl1", "VCO 1 LEVEL"); auto* b = knob (*u, "s_lvl2", "VCO 2 LEVEL"); auto* c = knob (*u, "s_noise", "NOISE"); auto* d = knob (*u, "s_master", "VOLUME");
        u->layoutFn = [=] (juce::Rectangle<int> r) { place (r, { a, b, c, d }, 0, 88); };
    }
    {
        auto* u = unit ("MODULATION  (SYNC / FM / RING)");
        auto* sy = toggle (*u, "s_sync", "HARD SYNC");
        auto* fm = knob (*u, "s_fm", "FM"); auto* rg = knob (*u, "s_ring", "RING"); auto* gl = knob (*u, "s_glide", "PORTAMENTO");
        u->layoutFn = [=] (juce::Rectangle<int> r)
        {
            sy->setBounds (r.getX(), r.getY() + 6, 120, 30);
            place (r.withTrimmedTop (38), { fm, rg, gl }, 0, 88);
        };
    }

    // ---- rang B : filtres et enveloppes (indices 4..7)
    {
        auto* u = unit ("VC HIGHPASS FILTER");
        auto* a = knob (*u, "s_hpf", "CUTOFF"); auto* b = knob (*u, "s_hpfRes", "PEAK");
        u->layoutFn = [=] (juce::Rectangle<int> r) { place (r, { a, b }, 0, 88); };
    }
    {
        auto* u = unit ("VC LOWPASS FILTER");
        auto* a = knob (*u, "s_lpf", "CUTOFF"); auto* b = knob (*u, "s_lpfRes", "PEAK"); auto* c = knob (*u, "s_fltEnv", "EG AMOUNT");
        auto* d = knob (*u, "s_key", "KEY TRACK"); auto* e = knob (*u, "s_drive", "DRIVE"); auto* mo = knob (*u, "s_semMorph", "SEM MORPH");
        auto* ft = selector (*u, "s_ftype", juce::StringArray { "MS-20", "SEM", "LADDER" });
        u->layoutFn = [=] (juce::Rectangle<int> r)
        {
            ft->setBounds (r.getX(), r.getY() + 2, juce::jmin (330, r.getWidth()), 34);
            place (r.withTrimmedTop (38), { a, b, c, d, e, mo }, 0, juce::jmin (88, r.getWidth() / 6));
        };
    }
    {
        auto* u = unit ("ENVELOPE GENERATOR 1  (FILTER)");
        auto* a = knob (*u, "s_fA", "ATTACK"); auto* d = knob (*u, "s_fD", "DECAY"); auto* s = knob (*u, "s_fS", "SUSTAIN"); auto* r2 = knob (*u, "s_fR", "RELEASE");
        u->layoutFn = [=] (juce::Rectangle<int> r) { place (r, { a, d, s, r2 }, 0, 88); };
    }
    {
        auto* u = unit ("ENVELOPE GENERATOR 2  (AMP)");
        auto* a = knob (*u, "s_aA", "ATTACK"); auto* d = knob (*u, "s_aD", "DECAY"); auto* s = knob (*u, "s_aS", "SUSTAIN"); auto* r2 = knob (*u, "s_aR", "RELEASE");
        u->layoutFn = [=] (juce::Rectangle<int> r) { place (r, { a, d, s, r2 }, 0, 88); };
    }

    // ---- rang C : un LFO (choisi dans un menu), wavefolder, macros, arpegiateur (indices 8..11)
    {
        auto* u = unit ("MODULATION GENERATOR");
        struct LfoSet { LfoView* view; SegmentSelector* wave; KnobUnit *rate, *slew, *fade; juce::ComboBox* sync; LedToggle* trig; };
        auto sets = std::make_shared<std::vector<LfoSet>>();
        for (int i = 1; i <= kLfos; ++i)
        {
            const juce::String pre ("s_l" + juce::String (i));
            LfoSet st;
            st.wave  = selector (*u, (pre + "Wave").toRawUTF8(), lfoWaves);
            st.rate  = knob (*u, (pre + "Rate").toRawUTF8(), "FREQUENCY");
            st.slew  = knob (*u, (pre + "Slew").toRawUTF8(), "S&H SLEW");
            st.fade  = knob (*u, (pre + "Fade").toRawUTF8(), "FADE-IN");
            st.sync  = combo (*u, (pre + "Sync").toRawUTF8());
            st.trig  = toggle (*u, (pre + "Trig").toRawUTF8(), "RETRIG");
            auto view = std::make_unique<LfoView> (proc.apvts, pre + "Wave", pre + "Slew");
            st.view = view.get(); u->addAndMakeVisible (*view); lfoViews.push_back (st.view); owned.push_back (std::move (view));
            sets->push_back (st);
        }
        auto pick = std::make_unique<juce::ComboBox>();
        for (int i = 1; i <= kLfos; ++i) pick->addItem ("LFO " + juce::String (i), i);
        pick->setSelectedId (1, juce::dontSendNotification);
        pick->setScrollWheelEnabled (false);
        u->addAndMakeVisible (*pick);
        auto* pk = pick.get(); owned.push_back (std::move (pick));
        auto show = [sets, pk]
        {
            const int sel = pk->getSelectedId() - 1;
            for (size_t k = 0; k < sets->size(); ++k)
            {
                auto& st = (*sets)[k]; const bool on = (int) k == sel;
                for (juce::Component* c : { (juce::Component*) st.view, (juce::Component*) st.wave, (juce::Component*) st.rate, (juce::Component*) st.slew,
                                            (juce::Component*) st.fade, (juce::Component*) st.sync, (juce::Component*) st.trig })
                    c->setVisible (on);
            }
        };
        pk->onChange = show; show();
        u->layoutFn = [sets, pk] (juce::Rectangle<int> r)
        {
            pk->setBounds (r.getX(), r.getY() + 2, 104, 28);
            for (auto& st : *sets)
            {
                st.view->setBounds (r.getX() + 114, r.getY(), r.getWidth() - 114, 40);
                st.wave->setBounds (r.getX(), r.getY() + 46, r.getWidth(), 46);
                st.sync->setBounds (r.getX(), r.getY() + 98, r.getWidth() / 2 - 4, 28);
                st.trig->setBounds (r.getX() + r.getWidth() / 2, r.getY() + 96, r.getWidth() / 2, 32);
                const int cw = r.getWidth() / 3, y = r.getY() + 132, h = r.getHeight() - 132;
                st.rate->setBounds (r.getX(), y, cw, h); st.slew->setBounds (r.getX() + cw, y, cw, h); st.fade->setBounds (r.getX() + 2 * cw, y, cw, h);
            }
        };
    }
    {
        auto* u = unit ("WAVEFOLDER");
        auto* a = knob (*u, "s_fold", "FOLD"); auto* b = knob (*u, "s_foldSym", "SYMMETRY", "-", "+");
        u->layoutFn = [=] (juce::Rectangle<int> r) { place (r, { a, b }, (r.getWidth() - 2 * 92) / 2, 92); };
    }
    {
        auto* u = unit ("MACROS");
        auto* a = knob (*u, "s_m1", "MACRO 1"); auto* b = knob (*u, "s_m2", "MACRO 2"); auto* c = knob (*u, "s_m3", "MACRO 3"); auto* d = knob (*u, "s_m4", "MACRO 4");
        u->layoutFn = [=] (juce::Rectangle<int> r)
        {
            const int cw = r.getWidth() / 2, kh = juce::jmin (r.getHeight() / 2, cw + 22);
            a->setBounds (r.getX(), r.getY(), cw, kh); b->setBounds (r.getX() + cw, r.getY(), cw, kh);
            c->setBounds (r.getX(), r.getY() + kh, cw, kh); d->setBounds (r.getX() + cw, r.getY() + kh, cw, kh);
        };
    }
    {
        auto* u = unit ("ARPEGGIATOR");
        auto* on = toggle (*u, "s_arpOn", "ARP ON"); auto* hold = toggle (*u, "s_arpHold", "HOLD");
        auto* div = combo (*u, "s_arpDiv");
        auto* mode = selector (*u, "s_arpMode", juce::StringArray { "UP", "DOWN", "UP-DN", "RAND" });
        auto* oct = selector (*u, "s_arpOct", juce::StringArray { "1 OCT", "2 OCT", "3 OCT", "4 OCT" });
        auto* rat = selector (*u, "s_arpRatchet", juce::StringArray { "RATCH x1", "x2", "x3", "x4" });
        auto* gate = knob (*u, "s_arpGate", "GATE"); auto* chance = knob (*u, "s_arpChance", "CHANCE");
        u->layoutFn = [=] (juce::Rectangle<int> r)
        {
            const int lw = juce::jmin (270, r.getWidth() - 190);
            on->setBounds (r.getX(), r.getY() + 2, 88, 30); hold->setBounds (r.getX() + 92, r.getY() + 2, 88, 30); div->setBounds (r.getX() + 186, r.getY() + 4, lw - 186, 26);
            mode->setBounds (r.getX(), r.getY() + 38, lw, 40); oct->setBounds (r.getX(), r.getY() + 84, lw, 40); rat->setBounds (r.getX(), r.getY() + 130, lw, 40);
            place (r, { gate, chance }, lw + 8, (r.getWidth() - lw - 8) / 2);
        };
    }
    startTimerHz (10);
}

void SynthPanel::timerCallback() { for (auto* v : lfoViews) v->repaint(); }

void SynthPanel::paint (juce::Graphics& g) { g.drawImageAt (tex, 0, 0); }
void SynthPanel::rebuild() { paintFacade (tex, getWidth(), getHeight(), false); repaint(); for (auto& u : units) u->repaint(); }

void SynthPanel::resized()
{
    const int W = getWidth(), gap = 8;
    auto row = [&] (int y, int h, std::initializer_list<std::pair<size_t, float>> items)
    {
        float total = 0.f; for (auto& it : items) total += it.second;
        const int usable = W - 20 - gap * ((int) items.size() - 1);
        int x = 10;
        for (auto& it : items)
        {
            const int w = juce::roundToInt ((float) usable * it.second / total);
            units[it.first]->fullHeight = h;
            units[it.first]->setBounds (x, y, w, h);
            x += w + gap;
        }
    };
    row (0, 212, { { 0, 290.f }, { 1, 450.f }, { 2, 372.f }, { 3, 300.f } });
    row (220, 212, { { 4, 190.f }, { 5, 510.f }, { 6, 360.f }, { 7, 360.f } });
    row (440, 276, { { 8, 400.f }, { 9, 210.f }, { 10, 240.f }, { 11, 520.f } });

    paintFacade (tex, W, getHeight(), false);
}

// ---------------------------------------------------------------- volet de la matrice
namespace
{
struct Fam { const char* title; std::vector<int> items; };
void fillGrouped (juce::ComboBox& cb, const std::vector<Fam>& fams, const char* const* names)
{
    cb.addItem ("-", 1);
    for (const auto& f : fams)
    {
        cb.addSectionHeading (f.title);
        for (int i : f.items) cb.addItem (names[i], i + 1);                       // id = index du parametre + 1
    }
}
const std::vector<Fam>& srcFamilies()
{
    static const std::vector<Fam> f { { "LFO", { MsLfo1, MsLfo2, MsLfo3, MsLfo4 } }, { "MACROS", { MsMac1, MsMac2, MsMac3, MsMac4 } },
                                      { "ENVELOPES", { MsEnv, MsAmpEnv } }, { "PERFORMANCE", { MsVel, MsWheel, MsKey, MsChanAT, MsPolyAT } }, { "RANDOM", { MsRandom } } };
    return f;
}
const std::vector<Fam>& dstFamilies()
{
    static const std::vector<Fam> f {
        { "PITCH", { MdPitch, MdPitch2 } },
        { "OSCILLATORS", { MdPw1, MdPw2, MdFm, MdRing, MdMix, MdLvl1, MdNoise } },
        { "FILTERS", { MdHpf, MdHpfReso, MdLpf, MdReso, MdMorph, MdFltEnv, MdDrive } },
        { "WAVEFOLDER", { MdFold, MdFoldSym } },
        { "AMP / SPACE", { MdLevel, MdPan } },
        { "FX RACK", { MdFxPower, MdFxClip, MdFxUnc, MdFxHpf, MdFxLpf, MdFxPeak, MdFxDestroy, MdFxRate, MdFxSpring, MdFxMix } },
        { "BBD DELAY", { MdBbdTime, MdBbdFb, MdBbdMix } },
        { "REVERB", { MdRevMix, MdRevDecay, MdRevDamp } } };
    return f;
}
}

MatrixDrawer::MatrixDrawer (juce::AudioProcessorValueTreeState& apvts)
{
    for (int i = 1; i <= kSlots; ++i)
    {
        const juce::String pre ("s_slot" + juce::String (i));
        auto r = std::make_unique<Row>();
        for (auto* cb : { &r->src, &r->dst }) cb->setScrollWheelEnabled (false);
        fillGrouped (r->src, srcFamilies(), kModSrcNames);
        fillGrouped (r->dst, dstFamilies(), kModDstNames);
        r->amt.detent = 0.0; r->amt.showMark = true;
        r->amt.setPopupDisplayEnabled (true, true, nullptr);
        r->amt.setScrollWheelEnabled (false);
        r->amt.setDoubleClickReturnValue (true, 0.0);
        addAndMakeVisible (r->src); addAndMakeVisible (r->dst); addAndMakeVisible (r->amt);
        comboAtt.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, pre + "Src", r->src));
        comboAtt.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, pre + "Dst", r->dst));
        sliderAtt.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, pre + "Amt", r->amt));
        rows.push_back (std::move (r));
    }
}

void MatrixDrawer::paint (juce::Graphics& g)
{
    const auto& th = look::T();
    const auto b = getLocalBounds().toFloat();
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (b.translated (0.f, 3.f), 7.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (th.plateTop), 0.f, 0.f, juce::Colour (th.plateBot), 0.f, b.getBottom(), false));
    g.fillRoundedRectangle (b, 7.f);
    g.setColour (juce::Colour (th.trim).withAlpha (0.7f));
    g.drawRoundedRectangle (b.reduced (1.f), 7.f, 1.3f);
    g.setColour (look::print.withAlpha (0.95f));
    g.setFont (bold (13.f));
    g.drawText ("MODULATION MATRIX   (reaches the filters, FX rack, BBD and reverb)", 14, 6, getWidth() - 28, 18, juce::Justification::centredLeft, false);
    g.setColour (juce::Colour (th.trim).withAlpha (0.45f));
    g.fillRect (14, 28, getWidth() - 28, 1);
    g.setColour (look::print.withAlpha (0.6f));
    g.setFont (bold (9.f));
    g.drawText ("SOURCE", 14, 30, 150, 8, juce::Justification::centredLeft, false);
    g.drawText ("DESTINATION", 172, 30, 176, 8, juce::Justification::centredLeft, false);
    g.drawText ("AMOUNT", 358, 30, 100, 8, juce::Justification::centredLeft, false);
}

void MatrixDrawer::resized()
{
    const int rowH = 32;
    for (size_t i = 0; i < rows.size(); ++i)
    {
        const int y = 42 + (int) i * rowH;
        rows[i]->src.setBounds (14, y, 150, 26);
        rows[i]->dst.setBounds (172, y, 176, 26);
        rows[i]->amt.setBounds (358, y, getWidth() - 358 - 16, 26);
    }
}

// ---------------------------------------------------------------- fenetre : onglets (synthe) + mise a l'echelle

int SynthEditor::designHeight() const { return ORAGE_SYNTH ? 30 + kFxH : kFxH; }
int SynthEditor::designWidth() const  { return 1440; }

SynthEditor::SynthEditor (OrageAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
#if ORAGE_SYNTH
      fx (p, "ORAGE", "6-VOICE SEMI-MODULAR SYNTH + HIGH VOLTAGE FX")
#else
      fx (p)
#endif
{
    setLookAndFeel (&laf);
    addAndMakeVisible (content);
    content.addAndMakeVisible (fx);
    fx.onThemeChanged = [this] { if (synthPanel != nullptr) synthPanel->rebuild(); repaint(); };
    fx.dialogHost = &content;
#if ORAGE_SYNTH
    synthPanel = std::make_unique<SynthPanel> (p);
    content.addAndMakeVisible (*synthPanel);
    for (auto* b : { &synthTab, &fxTab })
    {
        b->setClickingTogglesState (true);
        b->setRadioGroupId (4242);
        content.addAndMakeVisible (*b);
    }
    synthTab.onClick = [this] { showTab (0); };
    fxTab.onClick    = [this] { showTab (1); };
    matrix = std::make_unique<MatrixDrawer> (p.apvts);
    content.addChildComponent (*matrix);
    matrixBtn.setClickingTogglesState (true);
    content.addAndMakeVisible (matrixBtn);
    matrixBtn.onClick = [this] { matrix->setVisible (matrixBtn.getToggleState()); matrix->toFront (false); };
    synthTab.setToggleState (true, juce::dontSendNotification);
    showTab (0);
#endif

    const int dh = designHeight();
    float s = 1.f;                                                  // taille initiale : tient dans l'ecran
    if (auto* d = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
        s = juce::jmin (1.f, (float) (d->userArea.getHeight() - 90) / (float) dh, (float) (d->userArea.getWidth() - 20) / (float) designWidth());
    s = juce::jmax (0.6f, s);
    constrainer.setFixedAspectRatio ((double) designWidth() / (double) dh);
    constrainer.setSizeLimits (designWidth() * 6 / 10, dh * 6 / 10, designWidth() * 3 / 2, dh * 3 / 2);
    setConstrainer (&constrainer);
    setResizable (true, true);
    setSize (juce::roundToInt ((float) designWidth() * s), juce::roundToInt ((float) dh * s));
}

SynthEditor::~SynthEditor() { setLookAndFeel (nullptr); }

void SynthEditor::showTab (int t)
{
    if (synthPanel != nullptr) synthPanel->setVisible (t == 0);
    fx.repaint();
}

void SynthEditor::paint (juce::Graphics& g) { g.fillAll (juce::Colour (look::T().panelBot)); }

void SynthEditor::resized()
{
    const int dh = designHeight();
    content.setTransform ({});
    content.setBounds (0, 0, designWidth(), dh);
    content.setTransform (juce::AffineTransform::scale ((float) getWidth() / (float) designWidth()));
#if ORAGE_SYNTH
    synthTab.setBounds (10, 3, 150, 24);
    fxTab.setBounds (166, 3, 150, 24);
    matrixBtn.setBounds (322, 3, 150, 24);
    fx.setBounds ((designWidth() - 1440) / 2, 30, 1440, kFxH);
    synthPanel->setBounds (0, 30 + kHeaderH + 4, designWidth(), kFxH - kHeaderH - 4);
    matrix->setBounds (322, 32, 640, 42 + kSlots * 32 + 10);
    matrix->toFront (false);
#else
    fx.setBounds (0, 0, 1440, kFxH);
#endif
}
