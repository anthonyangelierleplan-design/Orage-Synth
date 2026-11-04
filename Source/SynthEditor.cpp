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
        const int kh = juce::jmin (r.getHeight(), cellW + 24), y = r.getY() + (r.getHeight() - kh) / 2;
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
            pw->setBounds (r.getX() + 180, r.getY() + 6, 84, r.getHeight() - 12);
        };
    }
    {
        auto* u = unit ("VOLTAGE CONTROLLED OSCILLATOR 2");
        auto* w = selector (*u, "s_wave2", waves); auto* o = selector (*u, "s_oct2", octs);
        auto* tu = knob (*u, "s_tune2", "PITCH", "-12", "+12"); auto* fi = knob (*u, "s_fine2", "FINE", "-", "+"); auto* pw = knob (*u, "s_pw2", "PW");
        u->layoutFn = [=] (juce::Rectangle<int> r)
        {
            w->setBounds (r.getX(), r.getY() + 6, 170, 46); o->setBounds (r.getX(), r.getY() + 62, 170, 34);
            place (r, { tu, fi, pw }, 180, 84);
        };
    }
    {
        auto* u = unit ("VCO MIXER");
        auto* a = knob (*u, "s_lvl1", "VCO 1 LEVEL"); auto* b = knob (*u, "s_lvl2", "VCO 2 LEVEL"); auto* c = knob (*u, "s_noise", "NOISE"); auto* d = knob (*u, "s_master", "VOLUME");
        u->layoutFn = [=] (juce::Rectangle<int> r) { place (r, { a, b, c, d }, 0, 84); };
    }
    {
        auto* u = unit ("MODULATION  (SYNC / FM / RING)");
        auto* sy = toggle (*u, "s_sync", "HARD SYNC");
        auto* fm = knob (*u, "s_fm", "FM"); auto* rg = knob (*u, "s_ring", "RING"); auto* gl = knob (*u, "s_glide", "PORTAMENTO");
        u->layoutFn = [=] (juce::Rectangle<int> r)
        {
            sy->setBounds (r.getX(), r.getY() + 6, 120, 30);
            place (r.withTrimmedTop (38), { fm, rg, gl }, 0, 84);
        };
    }

    // ---- rang B : filtres et enveloppes (indices 4..7)
    {
        auto* u = unit ("VC HIGHPASS FILTER");
        auto* a = knob (*u, "s_hpf", "CUTOFF"); auto* b = knob (*u, "s_hpfRes", "PEAK");
        u->layoutFn = [=] (juce::Rectangle<int> r) { place (r, { a, b }, 0, 84); };
    }
    {
        auto* u = unit ("VC LOWPASS FILTER");
        auto* a = knob (*u, "s_lpf", "CUTOFF"); auto* b = knob (*u, "s_lpfRes", "PEAK"); auto* c = knob (*u, "s_fltEnv", "EG AMOUNT");
        auto* d = knob (*u, "s_key", "KEY TRACK"); auto* e = knob (*u, "s_drive", "DRIVE");
        u->layoutFn = [=] (juce::Rectangle<int> r) { place (r, { a, b, c, d, e }, 0, 84); };
    }
    {
        auto* u = unit ("ENVELOPE GENERATOR 1  (FILTER)");
        auto* a = knob (*u, "s_fA", "ATTACK"); auto* d = knob (*u, "s_fD", "DECAY"); auto* s = knob (*u, "s_fS", "SUSTAIN"); auto* r2 = knob (*u, "s_fR", "RELEASE");
        u->layoutFn = [=] (juce::Rectangle<int> r) { place (r, { a, d, s, r2 }, 0, 84); };
    }
    {
        auto* u = unit ("ENVELOPE GENERATOR 2  (AMP)");
        auto* a = knob (*u, "s_aA", "ATTACK"); auto* d = knob (*u, "s_aD", "DECAY"); auto* s = knob (*u, "s_aS", "SUSTAIN"); auto* r2 = knob (*u, "s_aR", "RELEASE");
        u->layoutFn = [=] (juce::Rectangle<int> r) { place (r, { a, d, s, r2 }, 0, 84); };
    }

    // ---- rang C : 4 LFO avec S&H (indices 8..11)
    for (int i = 1; i <= kLfos; ++i)
    {
        const juce::String pre ("s_l" + juce::String (i));
        auto* u = unit ("MODULATION GENERATOR " + juce::String (i));
        auto* wv    = selector (*u, (pre + "Wave").toRawUTF8(), lfoWaves);
        auto* rate  = knob (*u, (pre + "Rate").toRawUTF8(), "FREQUENCY");
        auto* slew  = knob (*u, (pre + "Slew").toRawUTF8(), "S&H SLEW");
        auto* fade  = knob (*u, (pre + "Fade").toRawUTF8(), "FADE-IN");
        auto* sync  = combo (*u, (pre + "Sync").toRawUTF8());
        auto* trig  = toggle (*u, (pre + "Trig").toRawUTF8(), "RETRIG");
        auto view = std::make_unique<LfoView> (proc.apvts, pre + "Wave", pre + "Slew");
        auto* vp = view.get(); u->addAndMakeVisible (*view); lfoViews.push_back (vp); owned.push_back (std::move (view));
        u->layoutFn = [=] (juce::Rectangle<int> r)
        {
            vp->setBounds (r.getX(), r.getY(), r.getWidth(), 40);
            wv->setBounds (r.getX(), r.getY() + 46, r.getWidth(), 44);
            sync->setBounds (r.getX(), r.getY() + 96, r.getWidth() / 2 - 4, 26);
            trig->setBounds (r.getX() + r.getWidth() / 2, r.getY() + 94, r.getWidth() / 2, 30);
            const int cw = r.getWidth() / 3, y = r.getY() + 126, h = r.getHeight() - 126;
            rate->setBounds (r.getX(), y, cw, h); slew->setBounds (r.getX() + cw, y, cw, h); fade->setBounds (r.getX() + 2 * cw, y, cw, h);
        };
    }

    // ---- rang D : macros + matrice de modulation (indices 12, 13)
    {
        auto* u = unit ("MACROS");
        auto* a = knob (*u, "s_m1", "MACRO 1"); auto* b = knob (*u, "s_m2", "MACRO 2"); auto* c = knob (*u, "s_m3", "MACRO 3"); auto* d = knob (*u, "s_m4", "MACRO 4");
        u->layoutFn = [=] (juce::Rectangle<int> r) { place (r, { a, b, c, d }, 0, r.getWidth() / 4); };
    }
    {
        auto* u = unit ("MODULATION MATRIX   source > destination > quantite");
        struct Cell { juce::ComboBox *s, *d; DetentSlider* a; };
        auto cells = std::make_shared<std::vector<Cell>>();
        for (int i = 1; i <= kSlots; ++i)
        {
            const juce::String pre ("s_slot" + juce::String (i));
            auto* s = combo (*u, (pre + "Src").toRawUTF8());
            auto* d = combo (*u, (pre + "Dst").toRawUTF8());
            auto sl = std::make_unique<DetentSlider> (juce::Slider::LinearHorizontal, juce::Slider::NoTextBox);
            sl->detent = 0.0; sl->showMark = true;
            sl->setPopupDisplayEnabled (true, true, nullptr);
            sl->setScrollWheelEnabled (false);
            sl->setDoubleClickReturnValue (true, 0.0);
            sl->setColour (juce::Slider::thumbColourId, juce::Colour (0xffe9e5d8));
            sl->setColour (juce::Slider::trackColourId, juce::Colour (0xffc8281c));
            sl->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0b0b0c));
            u->addAndMakeVisible (*sl);
            sliderAtt.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, pre + "Amt", *sl));
            cells->push_back ({ s, d, sl.get() });
            owned.push_back (std::move (sl));
        }
        u->layoutFn = [cells] (juce::Rectangle<int> r)
        {
            const int colW = r.getWidth() / 2, rowH = 26;
            for (size_t i = 0; i < cells->size(); ++i)
            {
                const int cx = r.getX() + (int) (i / 4) * colW, cy = r.getY() + 2 + (int) (i % 4) * rowH;
                (*cells)[i].s->setBounds (cx, cy, 116, 22);
                (*cells)[i].d->setBounds (cx + 122, cy, 116, 22);
                (*cells)[i].a->setBounds (cx + 246, cy, colW - 268, 22);
            }
        };
    }
    startTimerHz (10);
}

void SynthPanel::timerCallback() { for (auto* v : lfoViews) v->repaint(); }

void SynthPanel::paint (juce::Graphics& g) { g.drawImageAt (tex, 0, 0); }

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
    row (0, 176, { { 0, 290.f }, { 1, 450.f }, { 2, 372.f }, { 3, 300.f } });
    row (184, 176, { { 4, 200.f }, { 5, 470.f }, { 6, 372.f }, { 7, 372.f } });
    row (368, 246, { { 8, 1.f }, { 9, 1.f }, { 10, 1.f }, { 11, 1.f } });
    row (622, 150, { { 12, 3.2f }, { 13, 9.f } });

    tex = juce::Image (juce::Image::RGB, juce::jmax (1, W), juce::jmax (1, getHeight()), false);
    juce::Graphics g (tex);
    g.fillAll (juce::Colour (0xff131314));
    juce::Random rr (555);
    for (int i = 0; i < 260; ++i)
    {
        const float x = rr.nextFloat() * (float) W, y = rr.nextFloat() * (float) getHeight(), a = (rr.nextFloat() - 0.5f) * 0.8f, len = 10.f + rr.nextFloat() * 120.f;
        g.setColour (juce::Colour::fromRGBA (205, 205, 210, (juce::uint8) (6 + rr.nextInt (22))));
        g.drawLine (x, y, x + len * std::cos (a), y + len * std::sin (a), 0.6f);
    }
    for (int i = 0; i < 4000; ++i)
    {
        g.setColour (juce::Colour (0xff0e0e0f));
        g.fillRect (rr.nextInt (W), rr.nextInt (juce::jmax (1, getHeight())), 1 + rr.nextInt (2), 1 + rr.nextInt (2));
    }
}

// ---------------------------------------------------------------- fenetre : onglets (synthe) + mise a l'echelle

int SynthEditor::designHeight() const { return ORAGE_SYNTH ? 990 : 900; }
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
    keys = std::make_unique<juce::MidiKeyboardComponent> (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard);
    content.addAndMakeVisible (*keys);
    keys->setKeyPressBaseOctave (4);
    keys->setLowestVisibleKey (36);
    keys->setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour (0xffff7a1a).withAlpha (0.8f));
    synthTab.setToggleState (true, juce::dontSendNotification);
    showTab (0);
#endif

    const int dh = designHeight();
    float s = 1.f;                                                  // taille initiale : tient dans l'ecran
    if (auto* d = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
        s = juce::jmin (1.f, (float) (d->userArea.getHeight() - 90) / (float) dh, (float) (d->userArea.getWidth() - 20) / (float) designWidth());
    s = juce::jmax (0.5f, s);
    constrainer.setFixedAspectRatio ((double) designWidth() / (double) dh);
    constrainer.setSizeLimits (designWidth() / 2, dh / 2, designWidth() * 3 / 2, dh * 3 / 2);
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

void SynthEditor::paint (juce::Graphics& g) { g.fillAll (juce::Colour (0xff0b0b0c)); }

void SynthEditor::resized()
{
    const int dh = designHeight();
    content.setTransform ({});
    content.setBounds (0, 0, designWidth(), dh);
    content.setTransform (juce::AffineTransform::scale ((float) getWidth() / (float) designWidth()));
#if ORAGE_SYNTH
    synthTab.setBounds (10, 3, 150, 24);
    fxTab.setBounds (166, 3, 150, 24);
    fx.setBounds ((designWidth() - 1440) / 2, 30, 1440, 900);
    synthPanel->setBounds (0, 30 + 118, designWidth(), 900 - 118);
    keys->setBounds (10, 934, designWidth() - 20, 52);
#else
    fx.setBounds (0, 0, 1440, 900);
#endif
}
