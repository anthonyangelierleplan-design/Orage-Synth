#include "PluginEditor.h"

using namespace orage;

namespace
{
juce::String fr (const char* utf8) { return juce::String::fromUTF8 (utf8); }
juce::Font font (float h, bool bold = true) { return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain)); }

juce::Colour heatColour (float t)
{
    t = juce::jlimit (0.f, 1.f, t);
    const juce::Colour a (0xff1a1412), b (0xffa11a08), c (0xffff7a1a), d (0xffffe9b0);
    if (t < 0.4f)  return a.interpolatedWith (b, t / 0.4f);
    if (t < 0.75f) return b.interpolatedWith (c, (t - 0.4f) / 0.35f);
    return c.interpolatedWith (d, (t - 0.75f) / 0.25f);
}

// ecaillures de peinture : une tuile transparente repetee par-dessus les panneaux
const juce::Image& speckTile()
{
    static const juce::Image img = []
    {
        juce::Image i (juce::Image::ARGB, 256, 256, true);
        juce::Graphics g (i);
        juce::Random rr (99);
        for (int k = 0; k < 820; ++k)
        {
            g.setColour (look::paint.withAlpha (0.92f));
            g.fillRect (rr.nextInt (256), rr.nextInt (256), 1 + rr.nextInt (2), 1 + rr.nextInt (2));
        }
        return i;
    }();
    return img;
}

void drawSpecks (juce::Graphics& g, juce::Rectangle<int> r)
{
    juce::Graphics::ScopedSaveState ss (g);
    g.reduceClipRegion (r);
    const auto& t = speckTile();
    for (int y = r.getY(); y < r.getBottom(); y += 256)
        for (int x = r.getX(); x < r.getRight(); x += 256)
            g.drawImageAt (t, x, y);
}

void drawPlate (juce::Graphics& g, juce::Rectangle<float> b)
{
    g.setColour (juce::Colour (0xff0b0b0c).withAlpha (0.45f));                  // panneau noir, zone sérigraphiée
    g.fillRect (b.reduced (1.f));
    g.setColour (look::print.withAlpha (0.72f));
    g.drawRect (b.reduced (1.f), 1.4f);
}

// pictogrammes d'ondes (selecteurs de forme d'onde, facon MS20)
bool drawGlyph (juce::Graphics& g, const juce::String& name, juce::Rectangle<float> r)
{
    juce::Path p;
    auto pt = [&r] (float x, float y) { return juce::Point<float> (r.getX() + x * r.getWidth(), r.getY() + y * r.getHeight()); };
    if (name == "SAW")                { p.startNewSubPath (pt (0.f, 1.f)); p.lineTo (pt (0.5f, 0.f)); p.lineTo (pt (0.5f, 1.f)); p.lineTo (pt (1.f, 0.f)); }
    else if (name == "PULSE" || name == "SQR") { p.startNewSubPath (pt (0.f, 1.f)); p.lineTo (pt (0.f, 0.f)); p.lineTo (pt (0.5f, 0.f)); p.lineTo (pt (0.5f, 1.f)); p.lineTo (pt (1.f, 1.f)); p.lineTo (pt (1.f, 0.f)); }
    else if (name == "TRI")           { p.startNewSubPath (pt (0.f, 0.5f)); p.lineTo (pt (0.25f, 0.f)); p.lineTo (pt (0.75f, 1.f)); p.lineTo (pt (1.f, 0.5f)); }
    else if (name == "SIN")
    {
        for (int i = 0; i <= 24; ++i)
        {
            const auto q = pt ((float) i / 24.f, 0.5f - 0.5f * std::sin ((float) i / 24.f * 6.2831853f));
            if (i == 0) p.startNewSubPath (q); else p.lineTo (q);
        }
    }
    else if (name == "S&H")
    {
        p.startNewSubPath (pt (0.f, 0.7f)); p.lineTo (pt (0.25f, 0.7f)); p.lineTo (pt (0.25f, 0.15f)); p.lineTo (pt (0.5f, 0.15f));
        p.lineTo (pt (0.5f, 0.9f)); p.lineTo (pt (0.75f, 0.9f)); p.lineTo (pt (0.75f, 0.4f)); p.lineTo (pt (1.f, 0.4f));
    }
    else return false;
    g.strokePath (p, juce::PathStrokeType (1.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::butt));
    return true;
}

const char* nodeName (int n)
{
    static const char* names[kNumNodes] { "INPUT", "OCTAVER", "DISTORSION", "COLOR", "FILTER", "DESTROY", "COMP", "SPRING", "GATE", "OUTPUT" };
    return names[juce::jlimit (0, (int) kNumNodes - 1, n)];
}

const juce::Colour kCablePalette[6] { juce::Colour (0xffd8482a), juce::Colour (0xffe3b23a), juce::Colour (0xff3aa0d8),
                                      juce::Colour (0xff53b86a), juce::Colour (0xffc05ad8), juce::Colour (0xffe6e4da) };

void drawCable (juce::Graphics& g, juce::Point<float> a, juce::Point<float> b, juce::Colour col, float alpha = 0.8f, float extra = 0.f)
{
    const float dx = juce::jmax (30.f, std::abs (b.x - a.x) * 0.4f), sag = 10.f + a.getDistanceFrom (b) * 0.08f;
    juce::Path p;
    p.startNewSubPath (a);
    p.cubicTo (a.x + dx, a.y + sag, b.x - dx, b.y + sag, b.x, b.y);
    g.setColour (juce::Colours::black.withAlpha (0.35f * alpha));
    g.strokePath (p, juce::PathStrokeType (4.4f + extra, juce::PathStrokeType::curved, juce::PathStrokeType::rounded),
                  juce::AffineTransform::translation (1.f, 3.f));
    g.setColour (col.darker (0.5f).withAlpha (alpha));
    g.strokePath (p, juce::PathStrokeType (4.f + extra, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (col.withAlpha (alpha));
    g.strokePath (p, juce::PathStrokeType (2.6f + extra, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    for (auto c : { a, b })
    {
        g.setColour (juce::Colour (0xff141417).withAlpha (alpha));
        g.fillEllipse (c.x - 8.f, c.y - 8.f, 16.f, 16.f);
        g.setColour (col.withAlpha (alpha));
        g.drawEllipse (c.x - 6.f, c.y - 6.f, 12.f, 12.f, 1.8f);
    }
}
} // namespace

// ================================================================ look & feel
OrageLookAndFeel::OrageLookAndFeel()
{
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff0b0b0c));
    setColour (juce::ComboBox::textColourId, look::print);
    setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff6a6a70));
    setColour (juce::ComboBox::arrowColourId, look::print);
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff141415));
    setColour (juce::PopupMenu::textColourId, look::print);
    setColour (juce::PopupMenu::headerTextColourId, look::cvRing);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff6a2412));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::TextButton::buttonColourId, juce::Colour (0xff232326));
    setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff6a2412));
    setColour (juce::TextButton::textColourOffId, look::print);
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    setColour (juce::ScrollBar::thumbColourId, juce::Colour (0xff6a6a70));
    setColour (juce::AlertWindow::backgroundColourId, juce::Colour (0xff141415));
    setColour (juce::AlertWindow::textColourId, look::print);
    setColour (juce::AlertWindow::outlineColourId, juce::Colour (0xff6a6a70));
    setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0b0b0c));
    setColour (juce::TextEditor::textColourId, look::print);
    setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff6a6a70));
}

void OrageLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                         float startAngle, float endAngle, juce::Slider& s)
{
    const auto c = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).getCentre();
    const bool numbered = (bool) s.getProperties()["numbered"];
    const float r = (float) juce::jmin (w, h) * 0.5f, kr = r * (numbered ? 0.58f : 0.66f), tr = kr * 0.86f;
    const float angle = startAngle + pos * (endAngle - startAngle);
    juce::Random rr ((int) s.getProperties()["seed"]);
    const float alpha = s.isEnabled() ? 1.f : 0.5f;

    g.setColour (juce::Colours::black.withAlpha (0.6f * alpha));
    g.fillEllipse (c.x - kr + 2.f, c.y - kr + 5.f, kr * 2.f, kr * 2.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff39393c), c.x - kr * 0.6f, c.y - kr * 0.7f,
                                             juce::Colour (0xff060607), c.x + kr * 0.7f, c.y + kr * 0.8f, true));
    g.fillEllipse (c.x - kr, c.y - kr, kr * 2.f, kr * 2.f);
    g.setColour (juce::Colour (0xff77777c).withAlpha (0.55f));
    g.drawEllipse (c.x - kr + 0.5f, c.y - kr + 0.5f, kr * 2.f - 1.f, kr * 2.f - 1.f, 1.2f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2c2c2f), c.x - tr * 0.5f, c.y - tr * 0.6f,
                                             juce::Colour (0xff0b0b0c), c.x + tr * 0.7f, c.y + tr * 0.8f, true));
    g.fillEllipse (c.x - tr, c.y - tr, tr * 2.f, tr * 2.f);
    for (int i = 1; i < 5; ++i)
    {
        const float rr2 = tr * (float) i / 5.f;
        g.setColour (juce::Colours::white.withAlpha (0.035f));
        g.drawEllipse (c.x - rr2, c.y - rr2, rr2 * 2.f, rr2 * 2.f, 0.7f);
    }
    {
        juce::Graphics::ScopedSaveState ss (g);
        juce::Path clip; clip.addEllipse (c.x - tr, c.y - tr, tr * 2.f, tr * 2.f);
        g.reduceClipRegion (clip);
        for (int k = 0; k < 9; ++k)
        {
            const float ax = c.x + (rr.nextFloat() - 0.5f) * tr * 2.f, ay = c.y + (rr.nextFloat() - 0.5f) * tr * 2.f;
            const float a = rr.nextFloat() * juce::MathConstants<float>::twoPi, len = 4.f + rr.nextFloat() * tr * 0.9f;
            g.setColour (juce::Colour::fromRGBA (200, 200, 205, (juce::uint8) (14 + rr.nextInt (40))));
            g.drawLine (ax, ay, ax + len * std::cos (a), ay + len * std::sin (a), 0.5f + rr.nextFloat() * 0.5f);
        }
        for (int k = 0; k < 40; ++k)
        {
            g.setColour (juce::Colour::fromRGBA (150, 150, 150, (juce::uint8) (20 + rr.nextInt (40))));
            g.fillRect (c.x + (rr.nextFloat() - 0.5f) * tr * 2.f, c.y + (rr.nextFloat() - 0.5f) * tr * 2.f, 1.f, 1.f);
        }
    }
    for (int k = 0; k < 16; ++k)
    {
        const float a = rr.nextFloat() * juce::MathConstants<float>::twoPi;
        g.setColour (juce::Colour::fromRGBA (160, 160, 165, (juce::uint8) (40 + rr.nextInt (60))));
        g.fillRect (c.x + kr * 0.97f * std::sin (a), c.y - kr * 0.97f * std::cos (a), 1.6f, 1.6f);
    }
    juce::Path p;
    p.addRoundedRectangle (-1.7f, -tr * 0.98f, 3.4f, tr * 0.64f, 1.2f);
    const auto xf = juce::AffineTransform::rotation (angle).translated (c.x, c.y);
    g.setColour (juce::Colour (0xfff0ece0).withAlpha (0.9f * alpha));
    g.fillPath (p, xf);
    g.setColour (juce::Colour (0xff121213).withAlpha (0.85f));
    juce::Path nick; nick.addRectangle (-2.2f, -tr * (0.55f + 0.3f * rr.nextFloat()), 4.4f, 1.4f);
    g.fillPath (nick, xf);
}

// ================================================================ potard
KnobUnit::KnobUnit (juce::AudioProcessorValueTreeState& apvts, const juce::String& id, const juce::String& t,
                    const juce::String& lo, const juce::String& hi, int cvIdx)
    : text (t), loText (lo), hiText (hi), cvIndex (cvIdx), attachment (apvts, id, slider)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
    slider.setPopupDisplayEnabled (true, true, nullptr);
    slider.setScrollWheelEnabled (false);                                   // la molette fait defiler le rack
    slider.getProperties().set ("seed", id.hashCode());
    if (auto* prm = apvts.getParameter (id))
        slider.setDoubleClickReturnValue (true, prm->convertFrom0to1 (prm->getDefaultValue()));
    if (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0) slider.detent = 0.0;           // cran au zero des reglages +/-
    if (loText == "0" && hiText == "10") slider.getProperties().set ("numbered", true);
    if (slider.getMinimum() == 0.0 && slider.getMaximum() == 1.0)
        slider.textFromValueFunction = [] (double v) { return juce::String (v * 10.0, 1); };
    addAndMakeVisible (slider);
}

void KnobUnit::resized() { slider.setBounds (getLocalBounds().withTrimmedBottom (16)); }

void KnobUnit::setCvActive (bool a) { if (a != cvActive) { cvActive = a; repaint(); } }

void KnobUnit::paint (juce::Graphics& g)
{
    const auto sb = slider.getBounds().toFloat();
    const auto c = sb.getCentre();
    const float r = juce::jmin (sb.getWidth(), sb.getHeight()) * 0.5f;
    const float sa = juce::MathConstants<float>::pi * 1.2f, ea = juce::MathConstants<float>::pi * 2.8f;
    const float alpha = slider.isEnabled() ? 1.f : 0.45f;
    const bool numbered = (bool) slider.getProperties()["numbered"] && sb.getWidth() >= 58.f;
    for (int i = 0; i <= 10; ++i)
    {
        const float a = sa + (float) i / 10.f * (ea - sa), rad = r * (numbered ? 0.74f : 0.93f), dot = (i % 5 == 0) ? 3.2f : 2.f;
        g.setColour (look::print.withAlpha (0.88f * alpha));
        g.fillEllipse (c.x + rad * std::sin (a) - dot * 0.5f, c.y - rad * std::cos (a) - dot * 0.5f, dot, dot);
        if (numbered)                                                              // chiffres 0..10 autour du bouton, comme le MS20
        {
            g.setFont (font (8.f));
            g.drawText (juce::String (i), (int) (c.x + r * 0.93f * std::sin (a)) - 7, (int) (c.y - r * 0.93f * std::cos (a)) - 5, 14, 10,
                        juce::Justification::centred, false);
        }
    }
    if (! numbered)
    {
        g.setFont (font (8.5f));
        g.setColour (look::print.withAlpha (0.8f * alpha));
        g.drawText (loText, (int) (c.x - r * 0.78f) - 14, (int) (c.y + r * 0.88f), 28, 11, juce::Justification::centred, false);
        g.drawText (hiText, (int) (c.x + r * 0.78f) - 14, (int) (c.y + r * 0.88f), 28, 11, juce::Justification::centred, false);
    }
    if (slider.detent >= slider.getMinimum() && slider.detent <= slider.getMaximum())               // repere orange du cran
    {
        const float a = sa + (float) slider.valueToProportionOfLength (slider.detent) * (ea - sa), rad = r * (numbered ? 0.74f : 0.93f);
        g.setColour (look::cvRing.withAlpha (0.95f * alpha));
        g.fillEllipse (c.x + rad * std::sin (a) - 2.f, c.y - rad * std::cos (a) - 2.f, 4.f, 4.f);
    }
    g.setFont (font (getWidth() < 80 ? 10.f : 11.f));
    g.setColour (look::print.withAlpha (0.95f * alpha));
    g.drawText (text, -4, getHeight() - 15, getWidth() + 8, 14, juce::Justification::centred, false);
    if (cvActive)                                                            // un cable CV est branche sur ce reglage
    {
        g.setColour (look::cvRing.withAlpha (0.3f)); g.fillEllipse ((float) getWidth() - 17.f, 0.f, 15.f, 15.f);
        g.setColour (look::cvRing);                  g.fillEllipse ((float) getWidth() - 14.f, 3.f, 9.f, 9.f);
    }
}

// ================================================================ selecteur / interrupteurs
SegmentSelector::SegmentSelector (juce::AudioProcessorValueTreeState& apvts, const juce::String& id, juce::StringArray names)
    : labels (std::move (names)),
      attachment (*apvts.getParameter (id), [this] (float v) { selected = juce::roundToInt (v); repaint(); })
{
    attachment.sendInitialUpdate();
}

void SegmentSelector::paint (juce::Graphics& g)
{
    const int n = labels.size();
    const float w = (float) getWidth() / (float) juce::jmax (1, n);
    for (int i = 0; i < n; ++i)
    {
        auto r = juce::Rectangle<float> (w * (float) i, 0.f, w, (float) getHeight()).reduced (2.f);
        const bool on = (i == selected);
        g.setGradientFill (juce::ColourGradient (on ? juce::Colour (0xff0d0d0e) : juce::Colour (0xff2b2b2e), r.getX(), r.getY(),
                                                 on ? juce::Colour (0xff1c1c1e) : juce::Colour (0xff111112), r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle (r, 3.f);
        g.setColour (juce::Colour (0xff8a8a90).withAlpha (0.6f));
        g.drawRoundedRectangle (r, 3.f, 1.f);
        const auto led = juce::Rectangle<float> (6.f, 6.f).withCentre ({ r.getCentreX(), r.getY() + 6.f });
        if (on) { g.setColour (juce::Colour (0xffff7a1a).withAlpha (0.35f)); g.fillEllipse (led.expanded (3.f)); }
        g.setColour (on ? juce::Colour (0xffff9a3a) : juce::Colour (0xff3a1408));
        g.fillEllipse (led);
        g.setColour (on ? look::print : look::print.withAlpha (0.6f));
        const auto glyphArea = r.withTrimmedTop (13.f).withTrimmedBottom (r.getHeight() > 36.f ? 11.f : 0.f).reduced (7.f, 2.f);
        if (r.getHeight() > 36.f && drawGlyph (g, labels[i], glyphArea))
        {
            g.setFont (font (8.f));
            g.drawText (labels[i], r.withTrimmedTop (r.getHeight() - 12.f), juce::Justification::centred, false);
        }
        else
        {
            g.setFont (font (9.5f));
            g.drawText (labels[i], r.withTrimmedTop (10.f), juce::Justification::centred, false);
        }
    }
}

void SegmentSelector::mouseDown (const juce::MouseEvent& e)
{
    const int n = labels.size();
    attachment.setValueAsCompleteGesture ((float) juce::jlimit (0, n - 1, (int) (e.position.x / ((float) getWidth() / (float) n))));
}

void LedToggle::paintButton (juce::Graphics& g, bool, bool down)
{
    auto r = getLocalBounds().toFloat().reduced (2.f);
    const bool on = getToggleState();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2b2b2e), r.getX(), r.getY(), juce::Colour (0xff111112), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (down ? r.reduced (0.6f) : r, 3.f);
    g.setColour (juce::Colour (0xff8a8a90).withAlpha (0.6f));
    g.drawRoundedRectangle (r, 3.f, 1.f);
    const auto led = juce::Rectangle<float> (7.f, 7.f).withCentre ({ r.getX() + 11.f, r.getCentreY() });
    if (on) { g.setColour (juce::Colour (0xffff7a1a).withAlpha (0.35f)); g.fillEllipse (led.expanded (3.f)); }
    g.setColour (on ? juce::Colour (0xffff9a3a) : juce::Colour (0xff3a1408));
    g.fillEllipse (led);
    g.setColour (on ? look::print : look::print.withAlpha (0.6f));
    g.setFont (font (10.f));
    g.drawText (getButtonText(), r.withTrimmedLeft (22.f), juce::Justification::centredLeft, false);
}

void FootSwitch::paintButton (juce::Graphics& g, bool, bool down)
{
    auto b = getLocalBounds().toFloat();
    const bool on = getToggleState();
    const auto led = juce::Rectangle<float> (b.getCentreX() - 6.f, b.getY() + 2.f, 12.f, 12.f);
    if (on) { g.setColour (juce::Colour (0xffff2a1a).withAlpha (0.3f)); g.fillEllipse (led.expanded (6.f)); }
    g.setColour (on ? juce::Colour (0xffff3b2a) : juce::Colour (0xff3a0c08));
    g.fillEllipse (led);
    g.setColour (look::print.withAlpha (0.6f));
    g.drawEllipse (led, 1.f);
    const float d = juce::jmin (b.getWidth() - 8.f, b.getHeight() - 22.f) * (down ? 0.93f : 1.f);
    auto s = juce::Rectangle<float> (d, d).withCentre ({ b.getCentreX(), b.getY() + 20.f + (b.getHeight() - 20.f) * 0.5f });
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillEllipse (s.translated (2.f, 4.f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffd9d9dc), s.getX(), s.getY(), juce::Colour (0xff4b4b51), s.getRight(), s.getBottom(), true));
    g.fillEllipse (s);
    g.setColour (juce::Colour (0xff202022));
    g.drawEllipse (s.reduced (d * 0.2f), 2.f);
    g.setColour (juce::Colour (0xff111111));
    g.setFont (font (11.f));
    g.drawText ("ON", s, juce::Justification::centred, false);
}

// ================================================================ vumeter a aiguille (patine, papier jauni)
void VuMeter::tick()
{
    shown += (target > shown ? 0.45f : 0.12f) * (target - shown);          // balistique : montee rapide, retombee lente
    if (std::fabs (shown - lastDrawn) > 0.03f) { lastDrawn = shown; repaint(); }
}

void VuMeter::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat().reduced (1.f);
    juce::Graphics::ScopedSaveState ss (g);
    juce::Path clip; clip.addRoundedRectangle (b, 4.f);
    g.reduceClipRegion (clip);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffddd3ab), b.getX(), b.getY(), juce::Colour (0xff9a8e66), b.getX(), b.getBottom(), false));
    g.fillRect (b);
    juce::Random rr (77);
    for (int k = 0; k < 70; ++k)                                                // taches de vieillissement
    {
        g.setColour (juce::Colour (0xff5a4a28).withAlpha (0.03f + 0.07f * rr.nextFloat()));
        const float s = 2.f + rr.nextFloat() * 7.f;
        g.fillEllipse (b.getX() + rr.nextFloat() * b.getWidth(), b.getY() + rr.nextFloat() * b.getHeight(), s, s);
    }

    const juce::Point<float> pivot (b.getCentreX(), b.getBottom() + 14.f);
    const float R = b.getHeight() * 0.98f + 6.f, a0 = -0.62f, a1 = 0.62f;
    auto dir = [] (float a) { return juce::Point<float> (std::sin (a), -std::cos (a)); };

    for (int i = 0; i <= 20; ++i)                                               // graduations 0..20 dB
    {
        const float a = a0 + (a1 - a0) * (float) i / 20.f, len = (i % 5 == 0) ? 9.f : 5.f;
        const auto p1 = pivot + dir (a) * (R - len), p2 = pivot + dir (a) * R;
        g.setColour (i >= 15 ? juce::Colour (0xffa81c10) : juce::Colour (0xff2a2418));
        g.drawLine (p1.x, p1.y, p2.x, p2.y, (i % 5 == 0) ? 1.6f : 1.f);
        if (i % 5 == 0)
        {
            const auto t = pivot + dir (a) * (R - 19.f);
            g.setFont (font (9.f));
            g.drawText (juce::String (i), (int) t.x - 10, (int) t.y - 6, 20, 12, juce::Justification::centred, false);
        }
    }
    g.setColour (juce::Colour (0xff2a2418));
    g.setFont (font (8.5f));
    g.drawText ("GAIN REDUCTION", b.withTrimmedTop (3.f).withHeight (12.f), juce::Justification::centred, false);
    g.drawText ("dB", b.withTrimmedLeft (6.f).withTrimmedTop (b.getHeight() - 15.f), juce::Justification::centredLeft, false);

    const float v = juce::jlimit (0.f, 21.f, shown), a = a0 + (a1 - a0) * v / 20.f;
    const auto tip = pivot + dir (a) * (R - 3.f);
    g.setColour (juce::Colours::black.withAlpha (0.25f));                        // ombre de l'aiguille
    g.drawLine (pivot.x + 2.f, pivot.y + 1.f, tip.x + 2.f, tip.y + 1.f, 1.6f);
    g.setColour (juce::Colour (0xff101010));
    g.drawLine (pivot.x, pivot.y, tip.x, tip.y, 1.6f);

    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.20f), b.getX(), b.getY(),      // reflet du verre
                                             juce::Colours::white.withAlpha (0.f), b.getX() + b.getWidth() * 0.6f, b.getBottom(), false));
    g.fillRect (b);
    g.setColour (juce::Colour (0xff1b1b1d));
    g.drawRoundedRectangle (b, 4.f, 3.f);
}

void LedLamp::paint (juce::Graphics& g)
{
    const auto led = juce::Rectangle<float> (14.f, 14.f).withCentre ({ (float) getWidth() * 0.5f, 10.f });
    if (lit) { g.setColour (juce::Colour (0xff5be36b).withAlpha (0.35f)); g.fillEllipse (led.expanded (5.f)); }
    g.setColour (lit ? juce::Colour (0xff6bff7a) : juce::Colour (0xff10260f));
    g.fillEllipse (led);
    g.setColour (look::print.withAlpha (0.6f));
    g.drawEllipse (led, 1.f);
    g.setColour (look::print.withAlpha (0.9f));
    g.setFont (font (9.f));
    g.drawText (text, 0, 22, getWidth(), 12, juce::Justification::centred, false);
}

// ================================================================ prise jack
Jack::Jack (PatchBay& o, JackKind k, int i) : kind (k), index (i), owner (o) { setSize (28, 28); }

juce::Point<float> Jack::socketCentre() const { return owner.centreOf (*this); }

void Jack::paint (juce::Graphics& g)
{
    const juce::Rectangle<float> s (3.f, 3.f, 22.f, 22.f);
    const bool cv = kind == JackKind::CvIn || kind == JackKind::CvOut;
    g.setColour (cv ? look::cvRing : look::print.withAlpha (0.9f));                       // anneau serigraphie
    g.drawEllipse (s.expanded (2.5f), 1.6f);
    if (highlight) { g.setColour (juce::Colour (0xff7be37b).withAlpha (0.4f)); g.fillEllipse (s.expanded (5.f)); }
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffbcbcc0), s.getX(), s.getY(), juce::Colour (0xff4a4a50), s.getRight(), s.getBottom(), false));
    g.fillEllipse (s.reduced (2.f));
    g.setColour (juce::Colour (0xff26262a));
    g.drawEllipse (s.reduced (2.f), 1.f);
    juce::Random rr (index * 17 + (int) kind * 101);
    for (int k = 0; k < 7; ++k)
    {
        g.setColour (juce::Colour (0xff2c2c30).withAlpha (0.5f));
        g.fillRect (s.getX() + 4.f + rr.nextFloat() * 14.f, s.getY() + 4.f + rr.nextFloat() * 14.f, 1.5f, 1.5f);
    }
    g.setColour (juce::Colour (0xff050506));
    g.fillEllipse (s.reduced (7.5f));
    g.setColour (juce::Colour (0xff9a9aa0).withAlpha (0.5f));
    g.drawEllipse (s.reduced (7.5f), 1.f);
    if (kind == JackKind::CvOut)                                                          // voyant d'activite
    {
        const float v = owner.proc.engine.modLed[index].load();
        g.setColour (juce::Colour (0xffff3a1a).withAlpha (0.12f + 0.88f * v));
        g.fillEllipse (1.f, 1.f, 6.f, 6.f);
    }
}

void Jack::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu()) { owner.clearJack (*this); return; }
    owner.beginDrag (*this);
}
void Jack::mouseDrag (const juce::MouseEvent& e) { owner.dragMoved (e.getEventRelativeTo (&owner).getPosition()); }
void Jack::mouseUp (const juce::MouseEvent& e)   { owner.endDrag (e.getEventRelativeTo (&owner).getPosition()); }
void Jack::mouseDoubleClick (const juce::MouseEvent&) { owner.clearJack (*this); }
void Jack::mouseEnter (const juce::MouseEvent&) { owner.hover (this); }
void Jack::mouseExit (const juce::MouseEvent&)  { owner.hover (nullptr); }

// ================================================================ calque des cables
PatchBay::PatchBay (OrageAudioProcessor& p) : proc (p)
{
    setInterceptsMouseClicks (false, false);
}

Jack* PatchBay::makeJack (juce::Component& parent, JackKind k, int index)
{
    jacks.push_back (std::make_unique<Jack> (*this, k, index));
    auto* j = jacks.back().get();
    parent.addAndMakeVisible (*j);
    switch (k)
    {
        case JackKind::AudioOut: audioOutJ[index] = j; break;
        case JackKind::AudioIn:  audioInJ[index] = j; break;
        case JackKind::CvOut:    cvOutJ[index] = j; break;
        case JackKind::CvIn:     cvInJ[index] = j; break;
    }
    return j;
}

juce::Point<float> PatchBay::centreOf (const Jack& j) const
{
    return getLocalPoint (&j, j.getLocalBounds().getCentre()).toFloat();
}

bool PatchBay::usable (const Jack* j) const
{
    if (j == nullptr) return false;
    for (const juce::Component* p = j; p != nullptr; p = p->getParentComponent())          // visible = toute la chaine de parents l'est
        if (! p->isVisible()) return false;
    if (auto* vp = j->findParentComponentOfClass<juce::Viewport>())                       // prise defilee hors du rack : pas de cable
        if (! vp->getLocalBounds().contains (vp->getLocalPoint (j, j->getLocalBounds().getCentre()))) return false;
    return true;
}

uint32_t PatchBay::stateHash() const
{
    uint32_t h = cablesVisible ? 17u : 19u;
    for (auto& a : proc.engine.audioIn) h = h * 31u + a.load();
    for (auto& c : proc.engine.cvIn)    h = h * 31u + c.load();
    for (auto& j : jacks)
        if (usable (j.get()))
        {
            const auto p = centreOf (*j);
            h = h * 31u + (uint32_t) ((int) p.x * 4099 + (int) p.y);
        }
    h = h * 31u + (hovered != nullptr ? 1u : 0u);
    return h;
}

void PatchBay::repaintLeds() { for (auto* j : cvOutJ) if (usable (j)) j->repaint(); }

void PatchBay::beginDrag (Jack& j)
{
    dragFrom = &j;
    dragPos = centreOf (j);
    auto isSrc = [] (JackKind k) { return k == JackKind::AudioOut || k == JackKind::CvOut; };
    auto isAudio = [] (JackKind k) { return k == JackKind::AudioOut || k == JackKind::AudioIn; };
    for (auto& o : jacks)
    {
        o->highlight = usable (o.get()) && o.get() != &j && isSrc (o->kind) != isSrc (j.kind) && isAudio (o->kind) == isAudio (j.kind);
        o->repaint();
    }
}

void PatchBay::dragMoved (juce::Point<int> p) { dragPos = p.toFloat(); repaint(); }

void PatchBay::tryConnect (Jack& a, Jack& b)
{
    auto isSrc = [] (JackKind k) { return k == JackKind::AudioOut || k == JackKind::CvOut; };
    Jack* s = &a; Jack* d = &b;
    if (! isSrc (a.kind)) std::swap (s, d);
    if (s->kind == JackKind::AudioOut && d->kind == JackKind::AudioIn)      proc.engine.setAudioCable (s->index, d->index, true);
    else if (s->kind == JackKind::CvOut && d->kind == JackKind::CvIn)       proc.engine.setCvCable (s->index, d->index, true);
}

void PatchBay::endDrag (juce::Point<int> p)
{
    if (dragFrom != nullptr)
    {
        for (auto& o : jacks)
            if (o.get() != dragFrom && usable (o.get()) && centreOf (*o).getDistanceFrom (p.toFloat()) < 18.f) { tryConnect (*dragFrom, *o); break; }
        dragFrom = nullptr;
    }
    for (auto& o : jacks) if (o->highlight) { o->highlight = false; o->repaint(); }
    repaint();
}

void PatchBay::clearJack (Jack& j)
{
    auto& e = proc.engine;
    switch (j.kind)
    {
        case JackKind::AudioIn:  e.audioIn[j.index].store (0); break;
        case JackKind::CvIn:     e.cvIn[j.index].store (0); break;
        case JackKind::AudioOut: for (int d = NOct; d < kNumNodes; ++d) e.setAudioCable (j.index, d, false); break;
        case JackKind::CvOut:    for (int d = 0; d < kNumCv; ++d) e.setCvCable (j.index, d, false); break;
    }
    dragFrom = nullptr;
    repaint();
}

void PatchBay::paint (juce::Graphics& g)
{
    if (! cablesVisible && dragFrom == nullptr) return;
    auto& e = proc.engine;

    std::vector<int> seq { (int) NIn };                                                  // INPUT, modules du rack, OUTPUT
    for (int n : e.getRack()) seq.push_back (n);
    seq.push_back ((int) NOut);
    uint32_t activeSrc = 0;
    for (int n : seq) if (n != NOut) activeSrc |= 1u << n;

    // cablage par defaut (normalise) : traits pointilles tant que l'entree n'est pas cablee
    const float dashes[2] { 4.f, 4.f };
    for (size_t i = 1; i < seq.size(); ++i)
    {
        const int nd = seq[i], prev = seq[i - 1];
        if ((e.audioIn[nd].load() & activeSrc) != 0 || ! usable (audioOutJ[prev]) || ! usable (audioInJ[nd])) continue;
        juce::Path line, dashed;
        line.startNewSubPath (centreOf (*audioOutJ[prev]));
        line.lineTo (centreOf (*audioInJ[nd]));
        juce::PathStrokeType (1.4f).createDashedStroke (dashed, line, dashes, 2);
        g.setColour (look::print.withAlpha (0.3f));
        g.fillPath (dashed);
    }

    auto involved = [this] (const Jack* a, const Jack* b) { return hovered != nullptr && (hovered == a || hovered == b); };
    auto drawIf = [&] (Jack* a, Jack* b, juce::Colour col)
    {
        if (! usable (a) || ! usable (b)) return;
        const bool hot = involved (a, b);
        drawCable (g, centreOf (*a), centreOf (*b), col, hot ? 1.f : 0.72f, hot ? 1.2f : 0.f);
    };
    for (int n : seq)
    {
        if (n == NIn) continue;
        const uint32_t m = e.audioIn[n].load() & activeSrc;
        for (int s = 0; s < NOut; ++s)
            if (m & (1u << s)) drawIf (audioOutJ[s], audioInJ[n], kCablePalette[(s * 3 + n) % 6]);
    }
    for (int d = 0; d < kNumCv; ++d)
    {
        const uint32_t m = e.cvIn[d].load();
        for (int s = 0; s < kNumSrc; ++s)
            if (m & (1u << s)) drawIf (cvOutJ[s], cvInJ[d], kCablePalette[(s + 2 * d + 1) % 6]);
    }
    if (dragFrom != nullptr)
        drawCable (g, centreOf (*dragFrom), dragPos, juce::Colour (0xffe6e4da), 0.9f);
}

// ================================================================ panneau patchbay
namespace
{
const char* cvName (int d)
{
    static const char* n[orage::kNumCv] { "DRIVE", "CLIP", "UNCERT.", "HPF", "LPF", "PEAK", "DESTROY", "D.RATE", "SPRING", "MIX", "S&H IN", "S&H CLK" };
    return n[juce::jlimit (0, (int) orage::kNumCv - 1, d)];
}
int cvOwner (int d)                                         // module qui porte la destination (-1 = toujours disponible)
{
    switch (d)
    {
        case CvDrive: case CvClip: case CvUnc: return NDist;
        case CvHpf: case CvLpf: case CvPeak:   return NFilter;
        case CvDestroy: case CvDRate:          return NDestroy;
        case CvSpring:                         return NSpring;
        default:                               return -1;
    }
}
}

void PatchPanel::layoutJacks()
{
    labels.clear();
    auto& e = owner.proc.engine;
    std::vector<int> seq { (int) NIn };
    for (int n : e.getRack()) seq.push_back (n);
    seq.push_back ((int) NOut);
    for (int n = 0; n < kNumNodes; ++n)
    {
        if (audioOut[n] != nullptr) audioOut[n]->setVisible (false);
        if (audioIn[n] != nullptr)  audioIn[n]->setVisible (false);
    }
    const int x0 = 12, y0 = 64, pitch = 34, inX = 196, outX = 252;
    for (size_t i = 0; i < seq.size(); ++i)
    {
        const int n = seq[i], y = y0 + (int) i * pitch;
        labels.push_back ({ nodeName (n), { x0, y - 7, 150, 14 }, false, false });
        if (audioIn[n] != nullptr)  { audioIn[n]->setVisible (true);  audioIn[n]->setCentrePosition (inX, y); }
        if (audioOut[n] != nullptr) { audioOut[n]->setVisible (true); audioOut[n]->setCentrePosition (outX, y); }
    }
    const int ySrc = y0 + 10 * pitch + 44;                                                   // sorties de modulation
    static const char* srcNames[kNumSrc] { "LFO 1", "LFO 2", "S&H", "ENV" };
    for (int s = 0; s < kNumSrc; ++s)
    {
        const int cx = 12 + 36 + s * 70;
        cvOut[s]->setCentrePosition (cx, ySrc);
        labels.push_back ({ srcNames[s], { cx - 35, ySrc + 20, 70, 12 }, false, true });
    }
    const int yCv = ySrc + 88;                                                               // entrees CV
    for (int d = 0; d < kNumCv; ++d)
    {
        const int cx = 12 + 36 + (d % 4) * 70, cy = yCv + (d / 4) * 58;
        const int ow = cvOwner (d);
        const bool avail = ow < 0 || e.inRack (ow);
        cvIn[d]->setVisible (avail);
        cvIn[d]->setCentrePosition (cx, cy);
        labels.push_back ({ cvName (d), { cx - 35, cy + 20, 70, 12 }, ! avail, true });
    }
    repaint();
}

void PatchPanel::paint (juce::Graphics& g)
{
    drawPlate (g, getLocalBounds().toFloat());
    auto head = [&] (const juce::String& t, int y)
    {
        g.setColour (look::print.withAlpha (0.95f));
        g.setFont (font (11.f));
        g.drawText (t, 12, y, getWidth() - 24, 16, juce::Justification::centredLeft, false);
        g.setColour (look::print.withAlpha (0.35f));
        g.fillRect (12, y + 19, getWidth() - 24, 1);
    };
    head ("PATCHBAY", 5);
    const int y0 = 64, pitch = 34;
    g.setColour (look::print.withAlpha (0.7f));
    g.setFont (font (9.f));
    g.drawText ("IN", 196 - 20, 36, 40, 11, juce::Justification::centred, false);
    g.drawText ("OUT", 252 - 20, 36, 40, 11, juce::Justification::centred, false);
    g.drawText ("AUDIO", 12, 36, 100, 11, juce::Justification::centredLeft, false);
    const int ySrc = y0 + 10 * pitch + 44;
    head ("MODULATION OUT", ySrc - 46);
    head ("CV IN", ySrc + 44);
    g.setColour (look::print.withAlpha (0.55f));
    g.setFont (font (8.5f, false));
    g.drawFittedText (fr ("Glisser d'une sortie vers une entr\xc3\xa9" "e.  Double-clic sur une prise : d\xc3\xa9" "brancher."),
                      14, getHeight() - 40, getWidth() - 28, 30, juce::Justification::centredLeft, 2);
}

void PatchPanel::paintOverChildren (juce::Graphics& g)
{
    g.setFont (font (9.5f));
    for (const auto& l : labels)
    {
        g.setColour ((l.cv ? look::cvRing : look::print).withAlpha (l.dim ? 0.3f : 0.95f));
        g.drawText (l.text, l.r, l.cv ? juce::Justification::centred : juce::Justification::centredLeft, false);
    }
    drawSpecks (g, getLocalBounds());
}

// ================================================================ module
ModuleUnit::ModuleUnit (RackView* rack, int n, const juce::String& t, int h, bool f)
    : node (n), title (t), fullHeight (h), fixed (f), rackView (rack) {}

ModuleUnit::Region ModuleUnit::regionAt (juce::Point<int> p) const
{
    if (p.y > 24) return None;
    if (fixed) return None;
    if (p.x > getWidth() - 30) return Remove;
    if (p.x > getWidth() - 58) return Collapse;
    return Handle;
}

void ModuleUnit::addJack (Jack* j, int col, int row, const juce::String& label)
{
    jackSlots.push_back ({ j, col, row, label });
    jackCols = juce::jmax (jackCols, col + 1);
}

void ModuleUnit::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    drawPlate (g, b);
    g.setColour (look::print.withAlpha (0.95f));
    g.setFont (font (11.f));
    g.drawText (title, fixed ? 12 : 32, 5, getWidth() - (fixed ? 24 : 100), 16, juce::Justification::centredLeft, false);
    g.setColour (look::print.withAlpha (0.35f));
    g.fillRect (fixed ? 12 : 32, 24, getWidth() - (fixed ? 24 : 100) - 12, 1);
    if (! fixed)
    {
        g.setColour (look::print.withAlpha (0.5f));                                      // poignee de deplacement
        for (int cx = 0; cx < 2; ++cx) for (int cy = 0; cy < 3; ++cy)
            g.fillEllipse (11.f + (float) cx * 7.f, 6.f + (float) cy * 5.f, 3.f, 3.f);
        juce::Path tri;                                                                  // replier / deplier
        const float tx = (float) getWidth() - 46.f;
        if (collapsed) tri.addTriangle (tx, 7.f, tx, 19.f, tx + 10.f, 13.f);
        else           tri.addTriangle (tx - 1.f, 8.f, tx + 11.f, 8.f, tx + 5.f, 18.f);
        g.setColour (look::print.withAlpha (0.85f));
        g.fillPath (tri);
        const float xx = (float) getWidth() - 20.f;                                      // retirer du rack
        g.setColour (look::cvRing.withAlpha (0.95f));
        g.drawLine (xx, 8.f, xx + 10.f, 18.f, 2.f);
        g.drawLine (xx + 10.f, 8.f, xx, 18.f, 2.f);
    }
    if (! collapsed && ! jackSlots.empty())                                              // etiquettes des prises + filet de separation
    {
        const int x0 = getWidth() - 10 - jackCols * 58;
        g.setColour (look::print.withAlpha (0.3f));
        g.fillRect (x0 - 8, 32, 1, getHeight() - 42);
        g.setFont (font (8.f));
        for (const auto& s : jackSlots)
        {
            const bool cv = s.jack->kind == JackKind::CvIn || s.jack->kind == JackKind::CvOut;
            g.setColour (cv ? look::cvRing : look::print.withAlpha (0.9f));
            g.drawText (s.label, x0 + s.col * 58, 34 + s.row * 38 + 28, 58, 10, juce::Justification::centred, false);
        }
    }
}

void ModuleUnit::paintOverChildren (juce::Graphics& g) { drawSpecks (g, getLocalBounds()); }

void ModuleUnit::resized()
{
    for (auto* c : getChildren()) c->setVisible (! collapsed);
    if (collapsed) return;
    const int x0 = getWidth() - 10 - jackCols * 58;
    for (const auto& s : jackSlots)
        s.jack->setCentrePosition (x0 + s.col * 58 + 29, 34 + s.row * 38 + 14);
    if (layoutFn) layoutFn (getLocalBounds().withTrimmedTop (28).withTrimmedBottom (4).withTrimmedLeft (10).withTrimmedRight (10 + jackStripWidth()));
}

void ModuleUnit::mouseDown (const juce::MouseEvent& e)
{
    pressed = regionAt (e.getPosition());
    if (pressed == Handle && rackView != nullptr) { grabOffset = e.y; rackView->dragStart (this, grabOffset); }
}

void ModuleUnit::mouseDrag (const juce::MouseEvent& e)
{
    if (pressed == Handle && rackView != nullptr) rackView->dragMove (this, e.getEventRelativeTo (rackView).y, grabOffset);
}

void ModuleUnit::mouseUp (const juce::MouseEvent& e)
{
    const auto region = regionAt (e.getPosition());
    if (pressed == Handle && rackView != nullptr) rackView->dragEnd (this);
    else if (pressed == Collapse && region == Collapse)
    {
        collapsed = ! collapsed;
        resized(); repaint();
        if (rackView != nullptr) rackView->relayout();
    }
    else if (pressed == Remove && region == Remove && onRemove) onRemove();
    pressed = None;
}

// ================================================================ rack
void RackView::paint (juce::Graphics& g)
{
    if (order.empty())
    {
        g.setColour (look::print.withAlpha (0.7f));
        g.setFont (font (14.f));
        g.drawText (fr ("RACK VIDE - clique sur un module \xc3\xa0 gauche pour l'ajouter"), getLocalBounds().withHeight (80),
                    juce::Justification::centred, false);
    }
}

void RackView::setUnits (std::vector<ModuleUnit*> list)
{
    for (auto* c : getChildren()) c->setVisible (false);
    order = std::move (list);
    for (auto* u : order) u->setVisible (true);
    layoutUnits (order);
}

void RackView::layoutUnits (const std::vector<ModuleUnit*>& list, ModuleUnit* dragged, int draggedY)
{
    const int w = getWidth(), gap = 8;
    int y = 0;
    for (auto* u : list)
    {
        if (u != dragged) u->setBounds (0, y, w, u->currentHeight());
        y += u->currentHeight() + gap;
    }
    const int total = juce::jmax (y, 90);
    if (dragged != nullptr)
    {
        dragged->setBounds (0, juce::jlimit (0, juce::jmax (0, total - dragged->currentHeight()), draggedY), w, dragged->currentHeight());
        dragged->toFront (false);
    }
    if (getHeight() != total) setSize (w, total);
}

void RackView::dragStart (ModuleUnit*, int) { temp = order; }

void RackView::dragMove (ModuleUnit* u, int pointerY, int grab)
{
    std::vector<ModuleUnit*> others;
    for (auto* o : order) if (o != u) others.push_back (o);
    const int centre = pointerY - grab + u->currentHeight() / 2;
    int idx = 0, y = 0;
    for (auto* o : others) { const int h = o->currentHeight(); if (centre > y + h / 2) ++idx; y += h + 8; }
    temp = others;
    temp.insert (temp.begin() + idx, u);
    layoutUnits (temp, u, pointerY - grab);
}

void RackView::dragEnd (ModuleUnit*)
{
    if (! temp.empty()) order = temp;
    temp.clear();
    layoutUnits (order);
    std::vector<int> nodes;
    for (auto* o : order) nodes.push_back (o->node);
    owner.rackChangedByUser (nodes);
}

// ================================================================ panneau lateral
void PlatePanel::paint (juce::Graphics& g)
{
    drawPlate (g, getLocalBounds().toFloat());
    auto head = [&] (const juce::String& t, int y)
    {
        g.setColour (look::print.withAlpha (0.95f));
        g.setFont (font (11.f));
        g.drawText (t, 12, y, getWidth() - 24, 16, juce::Justification::centredLeft, false);
        g.setColour (look::print.withAlpha (0.35f));
        g.fillRect (12, y + 19, getWidth() - 24, 1);
    };
    head (title, 5);
    for (const auto& s : subtitles) head (s.first, s.second);
}
void PlatePanel::paintOverChildren (juce::Graphics& g)
{
    g.setFont (font (8.f));
    g.setColour (look::print.withAlpha (0.9f));
    for (const auto& n : notes)
        g.drawText (std::get<0> (n), std::get<1> (n) - 30, std::get<2> (n), 60, 10, juce::Justification::centred, false);
    drawSpecks (g, getLocalBounds());
}

// ================================================================ boite de dialogue integree
OrageDialog::OrageDialog (juce::String t, juce::String m, bool askText, juce::String initialText, juce::String okText,
                          std::function<void (bool, const juce::String&)> callback)
    : titleText (std::move (t)), messageText (std::move (m)), hasText (askText), cb (std::move (callback))
{
    setWantsKeyboardFocus (true);
    okBtn.setButtonText (okText);
    cancelBtn.setButtonText ("ANNULER");
    okBtn.onClick = [this] { finish (true); };
    cancelBtn.onClick = [this] { finish (false); };
    addAndMakeVisible (okBtn);
    addAndMakeVisible (cancelBtn);
    if (hasText)
    {
        editor.setText (initialText, false);
        editor.setFont (font (16.f));
        editor.setSelectAllWhenFocused (true);
        editor.setIndents (8, 6);
        editor.onReturnKey = [this] { finish (true); };
        editor.onEscapeKey = [this] { finish (false); };
        addAndMakeVisible (editor);
    }
}

void OrageDialog::focus()
{
    if (hasText) editor.grabKeyboardFocus(); else grabKeyboardFocus();
}

bool OrageDialog::keyPressed (const juce::KeyPress& k)
{
    if (k == juce::KeyPress::escapeKey) { finish (false); return true; }
    if (k == juce::KeyPress::returnKey) { finish (true); return true; }
    return true;                                                      // modal : rien ne passe a travers
}

void OrageDialog::finish (bool ok)
{
    if (done) return;
    done = true;
    const auto text = editor.getText();
    setVisible (false);
    if (cb) cb (ok, text);
    if (onFinished) onFinished();
}

void OrageDialog::resized()
{
    const int w = 520, h = hasText ? 230 : 200;
    box = juce::Rectangle<int> (w, h).withCentre (getLocalBounds().getCentre());
    if (hasText) editor.setBounds (box.getX() + 28, box.getY() + 96, w - 56, 34);
    cancelBtn.setBounds (box.getRight() - 28 - 140 - 10 - 140, box.getBottom() - 28 - 34, 140, 34);
    okBtn.setBounds (box.getRight() - 28 - 140, box.getBottom() - 28 - 34, 140, 34);
}

void OrageDialog::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.62f));
    const auto b = box.toFloat();
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillRect (b.translated (4.f, 6.f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1c1c1f), 0.f, b.getY(), juce::Colour (0xff0d0d0f), 0.f, b.getBottom(), false));
    g.fillRect (b);
    g.setColour (look::print.withAlpha (0.8f));
    g.drawRect (b, 1.6f);
    g.drawRect (b.reduced (6.f), 0.8f);
    for (auto p : { juce::Point<float> (b.getX() + 15.f, b.getY() + 15.f), { b.getRight() - 15.f, b.getY() + 15.f },
                    { b.getX() + 15.f, b.getBottom() - 15.f }, { b.getRight() - 15.f, b.getBottom() - 15.f } })
    {
        g.setColour (juce::Colour (0xff8a8a90)); g.fillEllipse (p.x - 4.f, p.y - 4.f, 8.f, 8.f);
        g.setColour (juce::Colour (0xff2a2a2d)); g.drawLine (p.x - 3.f, p.y - 1.f, p.x + 3.f, p.y + 1.f, 1.2f);
    }
    g.setColour (look::print);
    g.setFont (font (16.f));
    g.drawText (titleText, box.getX() + 32, box.getY() + 22, box.getWidth() - 64, 22, juce::Justification::centredLeft, false);
    g.setColour (juce::Colour (0xffc8281c));
    g.fillRect (box.getX() + 32, box.getY() + 48, 60, 3);
    g.setColour (look::print.withAlpha (0.9f));
    g.setFont (font (13.f, false));
    g.drawFittedText (messageText, box.getX() + 32, box.getY() + 60, box.getWidth() - 64, hasText ? 30 : 70, juce::Justification::topLeft, 3);
    drawSpecks (g, box);
}

// ================================================================ editeur
OrageAudioProcessorEditor::OrageAudioProcessorEditor (OrageAudioProcessor& p, juce::String title, juce::String subtitle)
    : AudioProcessorEditor (&p), proc (p), titleText (std::move (title)), subText (std::move (subtitle)), patchBay (p)
{
    setLookAndFeel (&laf);

    addAndMakeVisible (prevBtn); addAndMakeVisible (nameBtn); addAndMakeVisible (nextBtn); addAndMakeVisible (saveBtn);
    prevBtn.onClick = [this] { stepPreset (-1); };
    nextBtn.onClick = [this] { stepPreset (1); };
    nameBtn.onClick = [this] { presetMenu(); };
    saveBtn.onClick = [this] { saveDialog(); };
    nameBtn.setButtonText (proc.presetName);

    viewport.setViewedComponent (&rackView, false);
    viewport.setScrollBarsShown (true, false);
    viewport.setScrollBarThickness (10);
    addAndMakeVisible (viewport);
    addAndMakeVisible (side);
    addAndMakeVisible (patchPanel);

    buildUnits();

    addAndMakeVisible (cablesBtn);                                  // le calque des cables passe par-dessus tout
    cablesBtn.setButtonText ("CABLES");
    cablesBtn.setToggleState (true, juce::dontSendNotification);
    cablesBtn.onClick = [this] { patchBay.cablesVisible = cablesBtn.getToggleState(); patchBay.repaint(); };
    addAndMakeVisible (patchBay);

    setSize (1440, 900);
    refreshRack();
    startTimerHz (30);
}

OrageAudioProcessorEditor::~OrageAudioProcessorEditor() { stopTimer(); setLookAndFeel (nullptr); }

KnobUnit* OrageAudioProcessorEditor::mk (juce::Component& parent, const char* id, const juce::String& text,
                                         const char* lo, const char* hi, int cv)
{
    knobs.push_back (std::make_unique<KnobUnit> (proc.apvts, id, text, lo, hi, cv));
    auto* k = knobs.back().get();
    knobById[id] = k;
    parent.addAndMakeVisible (*k);
    return k;
}

ModuleUnit* OrageAudioProcessorEditor::newUnit (int node, const juce::String& title, int height, bool fixed)
{
    if (fixed)
    {
        stripUnits.push_back (std::make_unique<ModuleUnit> (nullptr, node, title, height, true));
        addAndMakeVisible (*stripUnits.back());
        return stripUnits.back().get();
    }
    auto u = std::make_unique<ModuleUnit> (&rackView, node, title, height, false);
    auto* raw = u.get();
    u->onRemove = [this, node] { proc.engine.removeFromRack (node); refreshRack(); };
    rackView.addChildComponent (*raw);
    rackUnits[node] = std::move (u);
    return raw;
}

void OrageAudioProcessorEditor::buildUnits()
{
    constexpr int kModH = 150, kCell = 108;
    auto addSelector = [this] (juce::Component& parent, const char* id, const juce::StringArray& names)
    {
        auto sel = std::make_unique<SegmentSelector> (proc.apvts, id, names);
        auto* r = sel.get();
        parent.addAndMakeVisible (*r);
        owned.push_back (std::move (sel));
        return r;
    };
    auto addSync = [this] (juce::Component& parent, const char* id, KnobUnit* rate)
    {
        auto cb = std::make_unique<juce::ComboBox>();
        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter (id)))
            cb->addItemList (choice->choices, 1);
        cb->setScrollWheelEnabled (false);
        parent.addAndMakeVisible (*cb);
        comboAtt.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, id, *cb));
        auto* r = cb.get();
        combos.push_back (std::move (cb));
        syncLinks.push_back ({ r, rate, true });
        return r;
    };
    auto addToggle = [this] (juce::Component& parent, const char* id, const char* label)
    {
        auto b = std::make_unique<LedToggle>();
        b->setButtonText (label);
        parent.addAndMakeVisible (*b);
        buttonAtt.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, id, *b));
        auto* r = b.get();
        owned.push_back (std::move (b));
        return r;
    };
    auto place = [] (juce::Rectangle<int> r, std::vector<KnobUnit*> ks, int x0, int cellW)
    {
        int x = r.getX() + x0;
        const int kh = juce::jmin (r.getHeight(), cellW + 22), y = r.getY() + (r.getHeight() - kh) / 2;
        for (auto* k : ks) { k->setBounds (x, y, cellW, kh); x += cellW; }
    };

    // ---------------- modules du rack
    {   // GATE
        auto* u = newUnit (NGate, "NOISE GATE", kModH, false);
        auto* a = mk (*u, "gateThresh", "THRESHOLD");
        auto* b = mk (*u, "gateRel", "RELEASE");
        auto* c = mk (*u, "gateRange", "RANGE");
        auto lamp = std::make_unique<LedLamp> ("OPEN");
        gateLamp = lamp.get();
        u->addAndMakeVisible (*lamp);
        owned.push_back (std::move (lamp));
        auto* lp = gateLamp;
        u->layoutFn = [=] (juce::Rectangle<int> r) { place (r, { a, b, c }, 0, kCell); lp->setBounds (r.getX() + 3 * kCell + 20, r.getCentreY() - 24, 56, 44); };
    }
    {   // OCTAVER
        auto* u = newUnit (NOct, "OCTAVER", kModH, false);
        auto* sel = addSelector (*u, "octMode", juce::StringArray { "OFF", "DOWN", "UP", "BOTH" });
        auto* dose = mk (*u, "octAmt", "DOSE");
        u->layoutFn = [=] (juce::Rectangle<int> r) { sel->setBounds (r.getX(), r.getCentreY() - 18, 280, 36); dose->setBounds (r.getX() + 300, r.getY() + 2, kCell, r.getHeight() - 4); };
    }
    {   // DISTORSION
        auto* u = newUnit (NDist, "DISTORSION", kModH, false);
        juce::StringArray mats; mats.add ("GERMANIUM"); mats.add ("LAMPE"); mats.add (fr ("X" "\xc3\x89" "NON"));
        auto* sel = addSelector (*u, "material", mats);
        auto* a = mk (*u, "power", "PUISSANCE", "0", "10", CvDrive);
        auto* b = mk (*u, "clip", fr ("\xc3\x89" "CR" "\xc3\x8a" "TAGE"), "0", "10", CvClip);
        auto* c = mk (*u, "unc", "INCERTITUDE", "0", "10", CvUnc);
        auto* d = mk (*u, "lightning", "FOUDRE");
        u->layoutFn = [=] (juce::Rectangle<int> r) { sel->setBounds (r.getX(), r.getCentreY() - 18, 270, 36); place (r, { a, b, c, d }, 290, kCell - 8); };
    }
    {   // COLOR
        auto* u = newUnit (NColor, "COLOR", kModH, false);
        auto* a = mk (*u, "low", "LOW", "-12", "+12"); auto* b = mk (*u, "mid", "MID", "-12", "+12"); auto* c = mk (*u, "high", "HI", "-12", "+12");
        u->layoutFn = [=] (juce::Rectangle<int> r) { place (r, { a, b, c }, 0, kCell); };
    }
    {   // FILTER
        auto* u = newUnit (NFilter, "FILTER  HPF > LPF", kModH, false);
        auto* a = mk (*u, "hpf", "HPF CUTOFF", "0", "10", CvHpf);   auto* b = mk (*u, "hpfPeak", "PEAK", "0", "10", CvPeak);
        auto* c = mk (*u, "lpf", "LPF CUTOFF", "0", "10", CvLpf);   auto* d = mk (*u, "lpfPeak", "PEAK", "0", "10", CvPeak);
        u->layoutFn = [=] (juce::Rectangle<int> r) { place (r, { a, b, c, d }, 0, kCell); };
    }
    {   // DESTROY MACHINE
        auto* u = newUnit (NDestroy, "DESTROY MACHINE", kModH, false);
        auto* a = mk (*u, "destroy", "AMOUNT", "0", "10", CvDestroy);
        auto* b = mk (*u, "dRate", "RATE", "0", "10", CvDRate);
        auto* c = mk (*u, "dGrain", "GRAIN");
        auto* sync = addSync (*u, "dSync", b);
        static const struct { const char* id; const char* label; } t[] = {
            { "dStutter", "STUTTER" }, { "dReverse", "REVERSE" }, { "dTape", "TAPE STOP" },
            { "dShred", "SHRED" }, { "dDrop", "DROPOUT" }, { "dBuzz", "BUZZ 50Hz" } };
        std::vector<LedToggle*> tg;
        for (const auto& x : t) tg.push_back (addToggle (*u, x.id, x.label));
        u->layoutFn = [=] (juce::Rectangle<int> r)
        {
            place (r, { a, b, c }, 0, kCell - 12);
            sync->setBounds (r.getX() + 330, r.getCentreY() - 13, 110, 26);
            for (size_t i = 0; i < tg.size(); ++i) tg[i]->setBounds (r.getX() + 456 + (int) (i % 3) * 124, r.getY() + 14 + (int) (i / 3) * 44, 120, 36);
        };
    }
    {   // COMP : type, amount, threshold, attack, vumeter
        auto* u = newUnit (NComp, "TUBE COMP", kModH, false);
        auto* sel = addSelector (*u, "compType", juce::StringArray { "OFF", "EAR 660", "FAIRCHILD" });
        auto* a = mk (*u, "compAmt", "COMP");
        auto* b = mk (*u, "compThresh", "THRESHOLD");
        auto* c = mk (*u, "compAtt", "ATTACK");
        auto meter = std::make_unique<VuMeter>();
        vu = meter.get();
        u->addAndMakeVisible (*meter);
        owned.push_back (std::move (meter));
        auto* mp = vu;
        u->layoutFn = [=] (juce::Rectangle<int> r)
        {
            sel->setBounds (r.getX(), r.getCentreY() - 18, 230, 36);
            place (r, { a, b, c }, 244, 90);
            mp->setBounds (r.getX() + 530, r.getY() + 2, 170, r.getHeight() - 4);
        };
    }
    {   // SPRING
        auto* u = newUnit (NSpring, "SPRING REVERB", kModH, false);
        auto* a = mk (*u, "springMix", "MIX", "0", "10", CvSpring); auto* b = mk (*u, "springDecay", "DECAY"); auto* c = mk (*u, "springDrip", "DRIP");
        u->layoutFn = [=] (juce::Rectangle<int> r) { place (r, { a, b, c }, 0, kCell); };
    }

    // ---------------- bandeau de modulation (fixe) : sources CV a droite de chaque module
    auto strip = [&] (const char* title, const char* prefix, int srcIdx, bool hasSync)
    {
        auto* u = newUnit (-1, title, 172, true);
        const juce::String pre (prefix);
        auto* rate  = mk (*u, (pre + "Rate").toRawUTF8(), "RATE");
        auto* shape = mk (*u, (pre + (pre == "sh" ? "Slew" : "Shape")).toRawUTF8(), pre == "sh" ? "SLEW" : "SHAPE", pre == "sh" ? "0" : "SIN", pre == "sh" ? "10" : "SQR");
        auto* level = mk (*u, (pre + "Level").toRawUTF8(), "LEVEL");
        juce::ComboBox* sync = hasSync ? addSync (*u, (pre + "Sync").toRawUTF8(), rate) : nullptr;
        u->layoutFn = [=] (juce::Rectangle<int> r)
        {
            if (sync != nullptr) sync->setBounds (r.getX(), r.getY() + 2, r.getWidth(), 24);
            auto k = r.withTrimmedTop (30);
            const int cw = k.getWidth() / 3;
            rate->setBounds (k.getX(), k.getY(), cw, k.getHeight());
            shape->setBounds (k.getX() + cw, k.getY(), cw, k.getHeight());
            level->setBounds (k.getX() + 2 * cw, k.getY(), cw, k.getHeight());
        };
    };
    strip ("LFO 1", "lfo1", SrcLfo1, true);
    strip ("LFO 2", "lfo2", SrcLfo2, true);
    strip ("SAMPLE & HOLD", "sh", SrcSh, true);
    {   // ENVELOPE FOLLOWER
        auto* u = newUnit (-1, "ENVELOPE FOLLOWER", 172, true);
        auto* a = mk (*u, "envRel", "RELEASE"); auto* b = mk (*u, "envLevel", "LEVEL");
        u->layoutFn = [=] (juce::Rectangle<int> r) { a->setBounds (r.getX(), r.getY() + 30, r.getWidth() / 2, r.getHeight() - 30); b->setBounds (r.getX() + r.getWidth() / 2, r.getY() + 30, r.getWidth() / 2, r.getHeight() - 30); };
    }

    // ---------------- panneau lateral : tuiles de modules + master + remises a zero + entree/sortie
    static const int tileNodes[] = { NOct, NDist, NColor, NFilter, NDestroy, NComp, NSpring, NGate };
    for (int n : tileNodes)
    {
        auto b = std::make_unique<juce::TextButton> (nodeName (n));
        b->setClickingTogglesState (false);
        b->onClick = [this, n] { if (proc.engine.inRack (n)) proc.engine.removeFromRack (n); else proc.engine.addToRack (n); refreshRack(); };
        side.addAndMakeVisible (*b);
        tiles.push_back (std::move (b));
    }
    side.subtitles.push_back ({ "MASTER", 254 });
    side.subtitles.push_back ({ "RESET", 512 });
    for (int n = 0; n < kNumNodes; ++n)                                                   // toutes les prises vivent dans le panneau patchbay
    {
        if (n != NOut) patchPanel.audioOut[n] = patchBay.makeJack (patchPanel, JackKind::AudioOut, n);
        if (n != NIn)  patchPanel.audioIn[n]  = patchBay.makeJack (patchPanel, JackKind::AudioIn, n);
    }
    for (int s = 0; s < kNumSrc; ++s) patchPanel.cvOut[s] = patchBay.makeJack (patchPanel, JackKind::CvOut, s);
    for (int d = 0; d < kNumCv; ++d)  patchPanel.cvIn[d]  = patchBay.makeJack (patchPanel, JackKind::CvIn, d);
    mk (side, "mix", "DRY / WET", "DRY", "WET", CvMix);
    mk (side, "output", "OUTPUT", "-", "+");
    mk (side, "chaos", "CHAOS");
    side.addAndMakeVisible (foot);
    footAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "on", foot);

    side.addAndMakeVisible (dice);
    dice.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3a0d08));
    dice.onClick = [this] { rollTheDice(); };

    clearRackBtn.setButtonText ("VIDER LE RACK");
    clearPatchBtn.setButtonText ("VIDER LE PATCHBAY");
    resetBtn.setButtonText (fr ("RESET R\xc3\x89" "GLAGES"));
    for (auto* b : { &clearRackBtn, &clearPatchBtn, &resetBtn }) side.addAndMakeVisible (*b);
    clearRackBtn.onClick  = [this] { confirm ("VIDER LE RACK", fr ("Retirer tous les modules du rack ?\nLes r\xc3\xa9" "glages sont conserv\xc3\xa9s."), [this] { proc.clearRack(); refreshRack(); }); };
    clearPatchBtn.onClick = [this] { confirm ("VIDER LE PATCHBAY", "Retirer tous les cables du patchbay ?", [this] { proc.clearPatchbay(); patchBay.repaint(); }); };
    resetBtn.onClick      = [this] { confirm (fr ("RESET DES R\xc3\x89" "GLAGES"), fr ("Remettre tous les r\xc3\xa9" "glages des modules \xc3\xa0 leur valeur par d\xc3\xa9" "faut ?\nLe rack et les c\xc3\xa2" "bles sont conserv\xc3\xa9s."), [this] { proc.resetModules(); }); };
}

// ---------------------------------------------------------------- rack <-> moteur
void OrageAudioProcessorEditor::rackChangedByUser (const std::vector<int>& newOrder)
{
    proc.engine.setRack (newOrder);
    patchBay.repaint();
}

void OrageAudioProcessorEditor::refreshRack()
{
    std::vector<ModuleUnit*> list;
    for (int n : proc.engine.getRack())
    {
        auto it = rackUnits.find (n);
        if (it != rackUnits.end()) list.push_back (it->second.get());
    }
    rackView.setUnits (list);
    patchPanel.layoutJacks();
    static const int tileNodes[] = { NOct, NDist, NColor, NFilter, NDestroy, NComp, NSpring, NGate };
    for (size_t i = 0; i < tiles.size(); ++i) tiles[i]->setToggleState (proc.engine.inRack (tileNodes[i]), juce::dontSendNotification);
    patchBay.repaint();
    lastRackHash = 0;
    for (auto& s : proc.engine.rackSlot) lastRackHash = lastRackHash * 31u + (uint32_t) (s.load() + 2);
}

void OrageAudioProcessorEditor::afterPreset()
{
    nameBtn.setButtonText (proc.presetName);
    refreshRack();
    patchBay.repaint();
}

// ---------------------------------------------------------------- presets
void OrageAudioProcessorEditor::stepPreset (int dir)
{
    juce::StringArray all;
    const int nf = OrageAudioProcessor::getNumFactoryPresets();
    for (int i = 0; i < nf; ++i) all.add (OrageAudioProcessor::getFactoryPresetName (i));
    const auto user = proc.getUserPresetNames();
    all.addArray (user);
    int cur = all.indexOf (proc.presetName);
    cur = cur < 0 ? (dir > 0 ? -1 : 0) : cur;
    const int next = (cur + dir + all.size()) % juce::jmax (1, all.size());
    if (next < nf) proc.loadFactoryPreset (next); else proc.loadUserPreset (all[next]);
    afterPreset();
}

void OrageAudioProcessorEditor::presetMenu()
{
    juce::PopupMenu m;
    m.addSectionHeader ("FACTORY");
    for (int i = 0; i < OrageAudioProcessor::getNumFactoryPresets(); ++i)
        m.addItem (100 + i, OrageAudioProcessor::getFactoryPresetName (i), true, OrageAudioProcessor::getFactoryPresetName (i) == proc.presetName);
    const auto user = proc.getUserPresetNames();
    if (user.size() > 0)
    {
        m.addSeparator();
        m.addSectionHeader ("USER");
        for (int i = 0; i < user.size(); ++i) m.addItem (1000 + i, user[i], true, user[i] == proc.presetName);
    }
    m.addSeparator();
    m.addItem (1, "Save as...");
    if (user.contains (proc.presetName))
    {
        m.addItem (2, "Overwrite \"" + proc.presetName + "\"");
        m.addItem (3, "Delete \"" + proc.presetName + "\"");
    }
    m.addItem (4, "Open preset folder");

    juce::Component::SafePointer<OrageAudioProcessorEditor> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&nameBtn), [safe, user] (int id)
    {
        if (safe == nullptr || id == 0) return;
        auto& pr = safe->proc;
        if (id >= 1000)      { pr.loadUserPreset (user[id - 1000]); safe->afterPreset(); }
        else if (id >= 100)  { pr.loadFactoryPreset (id - 100); safe->afterPreset(); }
        else if (id == 1)    safe->saveDialog();
        else if (id == 2)    { pr.saveUserPreset (pr.presetName); safe->afterPreset(); }
        else if (id == 3)
        {
            const auto name = pr.presetName;
            safe->confirm ("Supprimer le preset", "Supprimer definitivement \"" + name + "\" ?", [safe, name]
            {
                if (safe != nullptr) { safe->proc.deleteUserPreset (name); safe->proc.presetName = "Init"; safe->afterPreset(); }
            });
        }
        else if (id == 4)    { pr.getPresetDir().createDirectory(); pr.getPresetDir().revealToUser(); }
    });
}

void OrageAudioProcessorEditor::showDialog (const juce::String& title, const juce::String& message, bool askText, const juce::String& initial,
                                            const juce::String& okText, std::function<void (bool, const juce::String&)> cb)
{
    juce::Component* host = dialogHost != nullptr ? dialogHost : this;
    dialog = std::make_unique<OrageDialog> (title, message, askText, initial, okText, std::move (cb));
    juce::Component::SafePointer<OrageAudioProcessorEditor> safe (this);
    dialog->onFinished = [safe]
    {
        juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->dialog.reset(); });
    };
    host->addAndMakeVisible (*dialog);
    dialog->setBounds (host->getLocalBounds());
    dialog->toFront (true);
    dialog->focus();
}

void OrageAudioProcessorEditor::saveDialog()
{
    juce::Component::SafePointer<OrageAudioProcessorEditor> safe (this);
    showDialog ("SAUVEGARDER LE PRESET", "Nom du preset :", true, proc.presetName, "SAUVEGARDER", [safe] (bool ok, const juce::String& text)
    {
        if (ok && safe != nullptr) { safe->proc.saveUserPreset (text); safe->afterPreset(); }
    });
}

void OrageAudioProcessorEditor::confirm (const juce::String& title, const juce::String& message, std::function<void()> action)
{
    showDialog (title, message, false, {}, "OK", [action] (bool ok, const juce::String&) { if (ok) action(); });
}

void OrageAudioProcessorEditor::rollTheDice()
{
    auto& r = juce::Random::getSystemRandom();
    auto set = [this] (const char* id, float real)
    {
        if (auto* prm = proc.apvts.getParameter (id))
        {
            prm->beginChangeGesture();
            prm->setValueNotifyingHost (prm->convertTo0to1 (real));
            prm->endChangeGesture();
        }
    };
    const auto maybe = [&r] (float chance, float maxVal) { return r.nextFloat() < chance ? maxVal * r.nextFloat() : 0.f; };
    set ("unc", 0.2f + 0.8f * r.nextFloat());
    set ("clip", 0.9f * r.nextFloat());
    set ("material", (float) r.nextInt (3));
    set ("lightning", maybe (0.8f, 0.9f));
    set ("chaos", maybe (0.7f, 0.8f));
    set ("destroy", maybe (0.6f, 0.8f));
    set ("dRate", r.nextFloat());
    proc.engine.setCvCable (r.nextInt (kNumSrc), r.nextInt (CvMix + 1), true);
}

// ---------------------------------------------------------------- horloge de l'interface
void OrageAudioProcessorEditor::timerCallback()
{
    heatShown += 0.2f * (proc.engine.heat.load() - heatShown);
    arcShown = juce::jmax (arcShown * 0.82f, proc.engine.arc.load());
    repaint (headerArea);
    patchBay.repaintLeds();

    if (vu != nullptr)       { vu->setValue (proc.engine.compGr.load()); vu->tick(); }
    if (gateLamp != nullptr) gateLamp->set (proc.engine.gateLed.load() > 0.5f);

    const uint32_t h = patchBay.stateHash();                                  // cables : on ne redessine que si quelque chose a bouge
    if (h != lastPatchHash) { lastPatchHash = h; patchBay.repaint(); }
    for (auto& k : knobs)
        if (k->cvIndex >= 0) k->setCvActive (proc.engine.cvIn[k->cvIndex].load() != 0);

    uint32_t rh = 0;
    for (auto& s : proc.engine.rackSlot) rh = rh * 31u + (uint32_t) (s.load() + 2);
    if (rh != lastRackHash) refreshRack();

    for (auto& l : syncLinks)                                                     // le potard RATE est inutile quand la synchro est active
    {
        const bool free = l.box->getSelectedItemIndex() <= 0;
        if (free != l.lastFree)
        {
            l.lastFree = free;
            l.rate->slider.setEnabled (free);
            l.rate->setAlpha (free ? 1.f : 0.5f);
            l.rate->repaint();
        }
    }
}

// ---------------------------------------------------------------- mise en page
void OrageAudioProcessorEditor::resized()
{
    const int W = getWidth(), H = getHeight();
    headerArea = { 0, 0, W, 108 };

    // ---- en-tete : presets
    prevBtn.setBounds (W - 400, 40, 30, 28);
    nameBtn.setBounds (W - 364, 40, 220, 28);
    nextBtn.setBounds (W - 140, 40, 30, 28);
    saveBtn.setBounds (W - 104, 40, 70, 28);
    cablesBtn.setBounds (W - 140, 76, 108, 24);

    // ---- colonnes : panneau lateral | rack | (bandeau de modulation en bas)
    const int vpW = W - 170 - 310;
    patchPanel.setBounds (W - 310, 118, 300, H - 128);
    side.setBounds (10, 118, 140, H - 128);
    viewport.setBounds (160, 118, vpW, H - 118 - 8 - 172 - 8);
    rackView.setSize (viewport.getMaximumVisibleWidth(), juce::jmax (90, rackView.getHeight()));
    patchBay.setBounds (getLocalBounds());
    patchBay.toFront (false);

    // ---- bandeau de modulation
    {
        const int y = H - 8 - 172, gap = 8;
        const float weights[4] { 3.f, 3.f, 3.f, 2.6f };
        const int usable = vpW - 3 * gap;
        int x = 160;
        for (size_t i = 0; i < stripUnits.size() && i < 4; ++i)
        {
            const int w = juce::roundToInt ((float) usable * weights[i] / 11.6f);
            stripUnits[i]->setBounds (x, y, w, 172);
            x += w + gap;
        }
    }

    // ---- panneau lateral
    for (size_t i = 0; i < tiles.size(); ++i) tiles[i]->setBounds (8, 32 + (int) i * 27, 124, 23);
    knobById.at ("mix")->setBounds (4, 274, 66, 94);
    knobById.at ("output")->setBounds (70, 274, 66, 94);
    knobById.at ("chaos")->setBounds (4, 372, 66, 94);
    foot.setBounds (74, 374, 58, 90);
    dice.setBounds (8, 472, 124, 26);
    clearRackBtn.setBounds (8, 536, 124, 26);
    clearPatchBtn.setBounds (8, 568, 124, 26);
    resetBtn.setBounds (8, 600, 124, 26);

    // ---- texture de la facade (metal peint en noir, use) + serigraphies de l'en-tete
    panelTex = juce::Image (juce::Image::RGB, W, H, false);
    juce::Random rr (4242);
    {
        juce::Image::BitmapData bd (panelTex, juce::Image::BitmapData::writeOnly);
        for (int y = 0; y < H; ++y)
        {
            const int streak = rr.nextInt (3);
            for (int x = 0; x < W; ++x)
            {
                const int v = 14 + rr.nextInt (6) + streak;
                bd.setPixelColour (x, y, juce::Colour::fromRGB ((juce::uint8) v, (juce::uint8) v, (juce::uint8) (v + 1)));
            }
        }
    }
    juce::Graphics g (panelTex);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.05f), 0.f, 0.f, juce::Colours::transparentWhite, 0.f, (float) H * 0.5f, false));
    g.fillRect (0, 0, W, H);
    for (int i = 0; i < 9; ++i)
    {
        g.setColour (juce::Colours::white.withAlpha (0.018f));
        g.fillEllipse (rr.nextFloat() * (float) W, rr.nextFloat() * (float) H, 40.f + rr.nextFloat() * 60.f, 30.f + rr.nextFloat() * 40.f);
    }
    for (int i = 0; i < 320; ++i)
    {
        const float x = rr.nextFloat() * (float) W, y = rr.nextFloat() * (float) H;
        const float a = (rr.nextFloat() - 0.5f) * (rr.nextInt (4) == 0 ? 3.1f : 0.7f), len = 12.f + rr.nextFloat() * 130.f;
        g.setColour (juce::Colour::fromRGBA (205, 205, 210, (juce::uint8) (8 + rr.nextInt (28))));
        g.drawLine (x, y, x + len * std::cos (a), y + len * std::sin (a), 0.5f + rr.nextFloat() * 0.7f);
    }
    g.setColour (juce::Colour (0xff5a5b60));
    g.drawRect (0, 0, W, H, 5);
    g.setColour (juce::Colour (0xff1b1b1d));
    g.drawRect (5, 5, W - 10, H - 10, 1);

    g.setColour (look::print.withAlpha (0.93f));
    g.setFont (font (50.f));
    g.drawText (titleText, 30, 6, 340, 58, juce::Justification::centredLeft, false);
    g.setColour (juce::Colour (0xffc8281c));
    g.fillRect (32, 66, 180, 4);
    g.setColour (look::print.withAlpha (0.85f));
    g.setFont (font (10.5f));
    g.drawText (subText, 32, 76, 380, 14, juce::Justification::centredLeft, false);
    g.setFont (font (9.f, false));
    g.drawText ("FOUDRE AUDIO  -  PATCH IT. BREAK IT.", 32, 91, 360, 12, juce::Justification::centredLeft, false);
    g.setFont (font (9.5f));
    g.drawText ("PRESET", W - 400, 24, 80, 13, juce::Justification::centredLeft, false);

    const juce::Point<float> screws[] = { { 16.f, 16.f }, { (float) W - 16.f, 16.f }, { 16.f, (float) H - 16.f }, { (float) W - 16.f, (float) H - 16.f } };
    for (auto p : screws)
    {
        g.setColour (juce::Colour (0xff8a8a90)); g.fillEllipse (p.x - 5.f, p.y - 5.f, 10.f, 10.f);
        g.setColour (juce::Colour (0xff2a2a2d)); g.drawLine (p.x - 3.5f, p.y - 1.f, p.x + 3.5f, p.y + 1.5f, 1.4f);
    }
    for (int i = 0; i < 12000; ++i)                                                       // la peinture saute
    {
        g.setColour (juce::Colour (0xff121213).withAlpha (0.92f));
        g.fillRect (rr.nextInt (W), rr.nextInt (H), 1 + rr.nextInt (2), 1 + rr.nextInt (2));
    }
    for (int i = 0; i < 240; ++i)                                                         // l'acier apparait
    {
        const bool edge = rr.nextInt (2) == 0;
        const int x = edge ? (rr.nextBool() ? rr.nextInt (14) : W - 1 - rr.nextInt (14)) : rr.nextInt (W);
        const int y = edge ? rr.nextInt (H) : (rr.nextBool() ? rr.nextInt (14) : H - 1 - rr.nextInt (14));
        g.setColour (juce::Colour (0xffb5b5ba).withAlpha (0.25f + 0.4f * rr.nextFloat()));
        g.fillRect (x, y, 1 + rr.nextInt (3), 1);
    }
}

// ---------------------------------------------------------------- dessin dynamique (lampes)
void OrageAudioProcessorEditor::drawBolt (juce::Graphics& g, juce::Point<float> from, juce::Point<float> to)
{
    juce::Path p;
    p.startNewSubPath (from);
    const int seg = 7;
    for (int i = 1; i < seg; ++i)
    {
        const float t = (float) i / (float) seg;
        p.lineTo (from + (to - from) * t + juce::Point<float> ((rnd.nextFloat() - 0.5f) * 16.f, (rnd.nextFloat() - 0.5f) * 10.f));
    }
    p.lineTo (to);
    g.setColour (juce::Colour (0xff8fb8ff).withAlpha (0.35f));
    g.strokePath (p, juce::PathStrokeType (5.f));
    g.setColour (juce::Colour (0xfff2f7ff));
    g.strokePath (p, juce::PathStrokeType (1.6f));
}

void OrageAudioProcessorEditor::drawTube (juce::Graphics& g, juce::Rectangle<float> r, float glow, float arc)
{
    const auto base = r.removeFromBottom (r.getHeight() * 0.13f);
    const auto glass = r.withTrimmedBottom (-8.f);
    const auto col = heatColour (glow);
    if (glow > 0.02f)
    {
        g.setGradientFill (juce::ColourGradient (col.withAlpha (0.55f * glow), glass.getCentreX(), glass.getCentreY() + 4.f,
                                                 col.withAlpha (0.f), glass.getCentreX(), glass.getCentreY() - glass.getHeight() * 0.95f, true));
        g.fillEllipse (glass.expanded (glass.getWidth() * 0.9f));
    }
    g.setColour (juce::Colour (0xff050505));
    g.fillRoundedRectangle (glass, glass.getWidth() * 0.5f);
    auto plate = glass.reduced (glass.getWidth() * 0.22f, 0.f).withTrimmedTop (glass.getHeight() * 0.22f).withTrimmedBottom (glass.getHeight() * 0.18f);
    g.setColour (col);
    g.fillRoundedRectangle (plate, 3.f);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    for (float x = plate.getX() + 3.f; x < plate.getRight(); x += 5.f)
        g.drawVerticalLine ((int) x, plate.getY(), plate.getBottom());
    g.setColour (juce::Colour (0xffff9a3a).withAlpha (0.25f + 0.7f * glow));
    g.fillEllipse (glass.getCentreX() - 4.f, glass.getBottom() - glass.getHeight() * 0.14f, 8.f, 6.f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.28f), glass.getX(), glass.getY(),
                                             juce::Colours::white.withAlpha (0.f), glass.getX() + glass.getWidth() * 0.45f, glass.getY(), false));
    g.fillRoundedRectangle (glass.reduced (2.f), glass.getWidth() * 0.45f);
    g.setColour (juce::Colour (0xff8d8d94).withAlpha (0.8f));
    g.drawRoundedRectangle (glass, glass.getWidth() * 0.5f, 1.4f);
    if (arc > 0.2f)
        drawBolt (g, { glass.getCentreX(), glass.getY() + 4.f }, { plate.getCentreX() + (rnd.nextFloat() - 0.5f) * 10.f, plate.getBottom() });
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff6c6c72), base.getX(), base.getY(),
                                             juce::Colour (0xff1a1a1c), base.getX(), base.getBottom(), false));
    g.fillRoundedRectangle (base.expanded (3.f, 0.f), 3.f);
}

void OrageAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.drawImageAt (panelTex, 0, 0);

    const juce::Rectangle<float> win (440.f, 6.f, 400.f, 98.f);
    g.setColour (juce::Colour (0xff040404));
    g.fillRoundedRectangle (win, 6.f);
    g.setColour (juce::Colour (0xff8d8d92).withAlpha (0.8f));
    g.drawRoundedRectangle (win, 6.f, 2.f);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawRoundedRectangle (win.reduced (3.f), 5.f, 2.f);

    const float h = heatShown;
    struct T { float x, w, hgt, thr; };
    const T tubes[] = { { 490.f, 34.f, 70.f, 0.00f }, { 570.f, 34.f, 70.f, 0.12f },
                        { 655.f, 46.f, 88.f, 0.25f }, { 745.f, 46.f, 88.f, 0.40f } };
    {
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (win.reduced (3.f).toNearestInt());
        for (const auto& t : tubes)
            drawTube (g, { t.x, 100.f - t.hgt, t.w, t.hgt }, juce::jlimit (0.f, 1.f, (h - t.thr) / (1.f - t.thr)), arcShown);
        if (arcShown > 0.35f)
            drawBolt (g, { 510.f + rnd.nextFloat() * 40.f, 22.f }, { 800.f - rnd.nextFloat() * 40.f, 26.f });
    }

    const float lamp = juce::jlimit (0.f, 1.f, 0.30f + 0.70f * h) * (arcShown > 0.3f ? 1.f : 0.92f + 0.08f * rnd.nextFloat());
    const juce::Point<float> c (385.f, 54.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffff2a1a).withAlpha (0.55f * lamp), c.x, c.y,
                                             juce::Colour (0xffff2a1a).withAlpha (0.f), c.x + 40.f, c.y, true));
    g.fillEllipse (c.x - 40.f, c.y - 40.f, 80.f, 80.f);
    g.setColour (juce::Colour (0xff9a9aa0));
    g.fillEllipse (c.x - 18.f, c.y - 18.f, 36.f, 36.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffffb09a).interpolatedWith (juce::Colour (0xffffe9b0), h * lamp), c.x - 4.f, c.y - 6.f,
                                             juce::Colour (0xff4a0804).interpolatedWith (juce::Colour (0xffd01808), lamp), c.x + 12.f, c.y + 14.f, true));
    g.fillEllipse (c.x - 14.f, c.y - 14.f, 28.f, 28.f);
    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.fillEllipse (c.x - 8.f, c.y - 10.f, 9.f, 5.f);
    g.setColour (look::print.withAlpha (0.9f));
    g.setFont (font (10.f));
    g.drawText ("HEAT", (int) c.x - 30, (int) c.y + 24, 60, 14, juce::Justification::centred, false);
}
