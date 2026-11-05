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

// (l'usure a ete retiree : plus d'ecaillures)
void drawSpecks (juce::Graphics&, juce::Rectangle<int>) {}

juce::Colour C (juce::uint32 c) { return juce::Colour (c); }

void drawPlate (juce::Graphics& g, juce::Rectangle<float> b)
{
    const auto& th = look::T();
    const auto r = b.reduced (1.f);
    g.setGradientFill (juce::ColourGradient (C (th.plateTop).withAlpha (0.86f), r.getX(), r.getY(), C (th.plateBot).withAlpha (0.92f), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 5.f);
    g.setColour (juce::Colours::white.withAlpha (0.05f));                         // filet de lumiere en haut
    g.drawHorizontalLine ((int) r.getY() + 1, r.getX() + 6.f, r.getRight() - 6.f);
    g.setColour (C (th.trim).withAlpha (0.42f));
    g.drawRoundedRectangle (r, 5.f, 1.1f);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawRoundedRectangle (r.expanded (1.f), 6.f, 1.f);
}

// pierre lumineuse (LED) en relief
void drawJewel (juce::Graphics& g, juce::Rectangle<float> r, bool on, juce::Colour ledCol)
{
    if (on)
    {
        g.setGradientFill (juce::ColourGradient (ledCol.withAlpha (0.55f), r.getCentreX(), r.getCentreY(), ledCol.withAlpha (0.f), r.getCentreX(), r.getY() - r.getHeight() * 0.9f, true));
        g.fillEllipse (r.expanded (r.getWidth() * 1.1f));
    }
    g.setGradientFill (juce::ColourGradient (on ? ledCol.brighter (0.7f) : juce::Colour (0xff4a3a34), r.getX() + r.getWidth() * 0.3f, r.getY() + r.getHeight() * 0.25f,
                                             on ? ledCol.darker (0.6f) : juce::Colour (0xff0d0908), r.getRight(), r.getBottom(), true));
    g.fillEllipse (r);
    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.drawEllipse (r, 1.f);
    g.setColour (juce::Colours::white.withAlpha (on ? 0.75f : 0.22f));                // reflet
    g.fillEllipse (r.getX() + r.getWidth() * 0.22f, r.getY() + r.getHeight() * 0.14f, r.getWidth() * 0.34f, r.getHeight() * 0.26f);
}

// touche metallique : en relief au repos, enfoncee quand elle est active
void drawKey (juce::Graphics& g, juce::Rectangle<float> r, bool pressed, bool hover = false)
{
    const auto& th = look::T();
    const juce::Colour mid = C (th.metalMid), lo = C (th.metalLo), hi = C (th.metalHi);
    if (! pressed)
    {
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.fillRoundedRectangle (r.translated (0.f, 1.6f), 4.f);
        juce::ColourGradient gr (mid.withMultipliedBrightness (hover ? 0.62f : 0.55f), r.getX(), r.getY(), lo.withMultipliedBrightness (0.95f), r.getX(), r.getBottom(), false);
        gr.addColour (0.5, mid.withMultipliedBrightness (0.40f));
        g.setGradientFill (gr);
        g.fillRoundedRectangle (r, 4.f);
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.13f), r.getX(), r.getY(), juce::Colours::white.withAlpha (0.f), r.getX(), r.getCentreY(), false));
        g.fillRoundedRectangle (r.reduced (1.f, 1.f).withHeight (r.getHeight() * 0.5f), 3.f);
        g.setColour (hi.withAlpha (0.55f));                                          // arete haute
        g.drawHorizontalLine ((int) (r.getY() + 1.f), r.getX() + 4.f, r.getRight() - 4.f);
        g.setColour (juce::Colours::black.withAlpha (0.55f));                       // arete basse
        g.drawHorizontalLine ((int) (r.getBottom() - 1.f), r.getX() + 3.f, r.getRight() - 3.f);
        g.setColour (C (th.trim).withAlpha (0.38f));
        g.drawRoundedRectangle (r, 4.f, 1.f);
    }
    else
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff060505), r.getX(), r.getY(), lo.withMultipliedBrightness (0.75f), r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle (r.translated (0.f, 0.6f), 4.f);
        g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.55f), r.getX(), r.getY(), juce::Colours::black.withAlpha (0.f), r.getX(), r.getY() + 7.f, false));
        g.fillRoundedRectangle (r.translated (0.f, 0.6f), 4.f);                      // ombre interieure
        if (look::themeIdx == 3)                                                      // theme d'origine : touche active teintee d'orange
        {
            g.setColour (C (th.led).withAlpha (0.16f));
            g.fillRoundedRectangle (r.translated (0.f, 0.6f), 4.f);
        }
        g.setColour (look::themeIdx == 3 ? C (th.led).withAlpha (0.8f) : C (th.trim).withAlpha (0.75f));
        g.drawRoundedRectangle (r.translated (0.f, 0.6f), 4.f, 1.1f);
    }
}

// corps metallique d'un potard (lumiere en haut a gauche) ; angle < 0 : sans repere
void drawKnobBody (juce::Graphics& g, juce::Point<float> c, float kr, float angle, float alpha, bool pointer = true)
{
    const auto& th = look::T();
    const juce::Colour hi = C (th.metalHi), mid = C (th.metalMid), lo = C (th.metalLo);
    for (int i = 0; i < 5; ++i)                                                       // ombre portee douce
    {
        const float e = (float) i * 1.1f;
        g.setColour (juce::Colours::black.withAlpha (0.15f * alpha));
        g.fillEllipse (c.x - kr - e + 1.f, c.y - kr - e + 3.f + (float) i * 0.5f, (kr + e) * 2.f, (kr + e) * 2.f);
    }
    juce::ColourGradient bez (hi, c.x - kr * 0.8f, c.y - kr * 0.9f, lo.darker (0.5f), c.x + kr * 0.8f, c.y + kr * 0.9f, false);   // couronne chromee
    bez.addColour (0.28, mid); bez.addColour (0.52, lo.brighter (0.2f)); bez.addColour (0.78, mid.darker (0.15f));
    g.setGradientFill (bez);
    g.fillEllipse (c.x - kr, c.y - kr, kr * 2.f, kr * 2.f);
    for (int i = 0; i < 56; ++i)                                                      // moletage
    {
        const float a = (float) i / 56.f * juce::MathConstants<float>::twoPi, s = std::sin (a), co = std::cos (a);
        g.setColour (juce::Colours::black.withAlpha (0.30f * alpha));
        g.drawLine (c.x + kr * 0.91f * s, c.y - kr * 0.91f * co, c.x + kr * 0.995f * s, c.y - kr * 0.995f * co, 0.9f);
    }
    const float cr = kr * 0.8f;
    g.setColour (juce::Colours::black.withAlpha (0.7f * alpha));                      // gorge
    g.drawEllipse (c.x - kr * 0.86f, c.y - kr * 0.86f, kr * 1.72f, kr * 1.72f, 1.3f);
    juce::ColourGradient cap (hi.interpolatedWith (mid, 0.3f), c.x - cr * 0.7f, c.y - cr * 0.85f, lo.brighter (0.15f), c.x + cr * 0.8f, c.y + cr * 0.95f, false);
    cap.addColour (0.42, mid.interpolatedWith (hi, 0.15f)); cap.addColour (0.75, mid.darker (0.3f));
    g.setGradientFill (cap);
    g.fillEllipse (c.x - cr, c.y - cr, cr * 2.f, cr * 2.f);
    {
        juce::Graphics::ScopedSaveState ss (g);
        juce::Path clip; clip.addEllipse (c.x - cr, c.y - cr, cr * 2.f, cr * 2.f);
        g.reduceClipRegion (clip);
        for (int i = 1; i < 8; ++i)                                                   // brossage circulaire
        {
            const float r2 = cr * (float) i / 8.f;
            g.setColour ((i % 2 ? juce::Colours::white : juce::Colours::black).withAlpha (0.045f));
            g.drawEllipse (c.x - r2, c.y - r2, r2 * 2.f, r2 * 2.f, 0.8f);
        }
        for (float a0 : { -1.15f, 1.99f })                                            // reflets anisotropes fixes (comme un metal brosse)
        {
            juce::Path w; w.addPieSegment (c.x - cr, c.y - cr, cr * 2.f, cr * 2.f, a0, a0 + 0.55f, 0.f);
            g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.02f), c.x, c.y, juce::Colours::white.withAlpha (0.30f * alpha), c.x + cr, c.y, true));
            g.fillPath (w);
        }
    }
    juce::Path rimHi, rimLo;
    rimHi.addCentredArc (c.x, c.y, cr - 0.5f, cr - 0.5f, 0.f, -2.5f, 0.35f, true);
    rimLo.addCentredArc (c.x, c.y, cr - 0.5f, cr - 0.5f, 0.f, 0.65f, 3.7f, true);
    g.setColour (juce::Colours::white.withAlpha (0.55f * alpha)); g.strokePath (rimHi, juce::PathStrokeType (1.2f));
    g.setColour (juce::Colours::black.withAlpha (0.55f * alpha)); g.strokePath (rimLo, juce::PathStrokeType (1.2f));
    if (pointer)
    {
        const auto xf = juce::AffineTransform::rotation (angle).translated (c.x, c.y);
        juce::Path p; p.addRoundedRectangle (-1.5f, -cr * 0.93f, 3.f, cr * 0.62f, 1.4f);
        g.setColour (juce::Colours::black.withAlpha (0.55f * alpha));
        g.fillPath (p, xf.translated (0.8f, 1.2f));
        g.setColour (C (th.pointer).withAlpha (alpha));
        g.fillPath (p, xf);
        juce::Path hl; hl.addRoundedRectangle (-0.5f, -cr * 0.91f, 1.f, cr * 0.5f, 0.5f);
        g.setColour (juce::Colours::white.withAlpha (0.55f * alpha));
        g.fillPath (hl, xf);
    }
}

// facade : degrade, brossage fin, vignette, filet d'en-tete ; cadre et vis polies si frame
void paintFacadeImpl (juce::Image& img, int W, int H, bool frame)
{
    const auto& th = look::T();
    img = juce::Image (juce::Image::RGB, juce::jmax (1, W), juce::jmax (1, H), false);
    juce::Graphics g (img);
    g.setGradientFill (juce::ColourGradient (C (th.panelTop), 0.f, 0.f, C (th.panelBot), 0.f, (float) H, false));
    g.fillAll();
    juce::Random rr (4242);
    for (int y = 0; y < H; y += 2)                                                    // brossage horizontal tres fin
    {
        g.setColour (juce::Colours::white.withAlpha (0.006f + 0.012f * rr.nextFloat()));
        g.drawHorizontalLine (y, 0.f, (float) W);
    }
    g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack, (float) W * 0.5f, (float) H * 0.45f, juce::Colours::black.withAlpha (0.38f), 0.f, 0.f, true));
    g.fillAll();
    if (frame)
    {
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.05f), 0.f, 0.f, juce::Colours::transparentWhite, 0.f, (float) kHeaderH, false));
        g.fillRect (0, 0, W, kHeaderH);
        g.setColour (C (th.trim).withAlpha (0.5f));
        g.fillRect (12, kHeaderH - 1, W - 24, 1);
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.fillRect (12, kHeaderH, W - 24, 1);
        g.setColour (C (th.trim).withAlpha (0.55f));
        g.drawRect (0, 0, W, H, 2);
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawRect (2, 2, W - 4, H - 4, 1);
        const juce::Point<float> screws[] = { { 16.f, 16.f }, { (float) W - 16.f, 16.f }, { 16.f, (float) H - 16.f }, { (float) W - 16.f, (float) H - 16.f } };
        for (auto p : screws)
        {
            g.setColour (juce::Colours::black.withAlpha (0.5f)); g.fillEllipse (p.x - 5.5f, p.y - 4.f, 11.f, 11.f);
            g.setGradientFill (juce::ColourGradient (C (th.metalHi), p.x - 4.f, p.y - 4.f, C (th.metalLo), p.x + 4.f, p.y + 4.f, false));
            g.fillEllipse (p.x - 5.f, p.y - 5.f, 10.f, 10.f);
            g.setColour (juce::Colours::black.withAlpha (0.65f)); g.drawLine (p.x - 3.4f, p.y + 1.f, p.x + 3.4f, p.y - 1.f, 1.3f);
        }
    }
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

juce::File themeFile() { return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("Foudre Audio").getChildFile ("theme.txt"); }
int loadThemePref() { const auto f = themeFile(); return f.existsAsFile() ? f.loadFileAsString().trim().getIntValue() : 0; }
void saveThemePref (int i) { const auto f = themeFile(); f.getParentDirectory().createDirectory(); f.replaceWithText (juce::String (i)); }

const char* nodeName (int n)
{
    static const char* names[kNumNodes] { "INPUT", "OCTAVER", "DISTORSION", "COLOR", "FILTER", "DESTROY", "COMP", "SPRING", "GATE", "BBD DELAY", "REVERB", "OUTPUT" };
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

void paintFacade (juce::Image& img, int W, int H, bool frame) { paintFacadeImpl (img, W, H, frame); }

// ================================================================ look & feel
namespace { std::vector<OrageLookAndFeel*>& lafList() { static std::vector<OrageLookAndFeel*> v; return v; } }

OrageLookAndFeel::OrageLookAndFeel() { lafList().push_back (this); refresh(); }
OrageLookAndFeel::~OrageLookAndFeel() { auto& v = lafList(); v.erase (std::remove (v.begin(), v.end(), this), v.end()); }
void OrageLookAndFeel::refreshAll() { for (auto* l : lafList()) l->refresh(); }

void OrageLookAndFeel::refresh()
{
    const auto& th = look::T();
    const juce::Colour trim (th.trim), dark (0xff0b0b0c);
    setColour (juce::ComboBox::backgroundColourId, dark);
    setColour (juce::ComboBox::textColourId, look::print);
    setColour (juce::ComboBox::outlineColourId, trim.withAlpha (0.55f));
    setColour (juce::ComboBox::arrowColourId, trim);
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff121213));
    setColour (juce::PopupMenu::textColourId, look::print);
    setColour (juce::PopupMenu::headerTextColourId, trim);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (th.metalMid).withMultipliedBrightness (0.5f));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::TextButton::buttonColourId, juce::Colour (0xff232326));
    setColour (juce::TextButton::buttonOnColourId, juce::Colour (th.metalMid).withMultipliedBrightness (0.5f));
    setColour (juce::TextButton::textColourOffId, look::print);
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    setColour (juce::ScrollBar::thumbColourId, trim.withAlpha (0.55f));
    setColour (juce::AlertWindow::backgroundColourId, juce::Colour (0xff141415));
    setColour (juce::AlertWindow::textColourId, look::print);
    setColour (juce::AlertWindow::outlineColourId, trim.withAlpha (0.6f));
    setColour (juce::TextEditor::backgroundColourId, dark);
    setColour (juce::TextEditor::textColourId, look::print);
    setColour (juce::TextEditor::outlineColourId, trim.withAlpha (0.6f));
    setColour (juce::Slider::thumbColourId, juce::Colour (th.metalHi));
    setColour (juce::Slider::trackColourId, juce::Colour (th.metalMid).withMultipliedBrightness (0.8f));
    setColour (juce::Slider::backgroundColourId, dark);
}

void OrageLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    drawKey (g, b.getLocalBounds().toFloat().reduced (1.5f), down || b.getToggleState(), over);
}

void OrageLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool down)
{
    g.setFont (font (juce::jmin (12.f, (float) b.getHeight() * 0.5f + 2.f)));
    g.setColour ((b.getToggleState() || down) ? (look::themeIdx == 3 ? juce::Colour (look::T().led).brighter (0.3f) : juce::Colours::white) : look::print.withAlpha (0.95f));
    g.drawText (b.getButtonText(), b.getLocalBounds().translated (0, down ? 1 : 0), juce::Justification::centred, false);
}

void OrageLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box)
{
    const auto r = juce::Rectangle<float> (0.f, 0.f, (float) w, (float) h).reduced (1.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff050505), r.getX(), r.getY(), juce::Colour (0xff151516), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 4.f);
    g.setColour (juce::Colour (look::T().trim).withAlpha (box.isEnabled() ? 0.6f : 0.3f));
    g.drawRoundedRectangle (r, 4.f, 1.1f);
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.drawHorizontalLine ((int) r.getY() + 1, r.getX() + 4.f, r.getRight() - 4.f);
    juce::Path chev;
    const float cx = (float) w - 14.f, cy = (float) h * 0.5f;
    chev.startNewSubPath (cx - 4.f, cy - 2.f); chev.lineTo (cx, cy + 2.5f); chev.lineTo (cx + 4.f, cy - 2.f);
    g.setColour (juce::Colour (look::T().trim));
    g.strokePath (chev, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void OrageLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                         float startAngle, float endAngle, juce::Slider& s)
{
    const auto c = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).getCentre();
    const bool numbered = (bool) s.getProperties()["numbered"];
    const float r = (float) juce::jmin (w, h) * 0.5f, kr = r * (numbered ? 0.58f : 0.66f);
    const float angle = startAngle + pos * (endAngle - startAngle);
    drawKnobBody (g, c, kr, angle, s.isEnabled() ? 1.f : 0.5f);
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
            g.setFont (font (9.f));
            g.drawText (juce::String (i), (int) (c.x + r * 0.93f * std::sin (a)) - 7, (int) (c.y - r * 0.93f * std::cos (a)) - 5, 14, 10,
                        juce::Justification::centred, false);
        }
    }
    if (! numbered)
    {
        g.setFont (font (9.5f));
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
    g.setFont (font (getWidth() < 80 ? 11.f : 12.f));
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
        drawKey (g, r, on);
        drawJewel (g, juce::Rectangle<float> (6.f, 6.f).withCentre ({ r.getCentreX(), r.getY() + 6.5f }), on, juce::Colour (look::T().led));
        g.setColour (on ? (look::themeIdx == 3 ? juce::Colour (look::T().led).brighter (0.4f) : look::print) : look::print.withAlpha (0.72f));
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

void LedToggle::paintButton (juce::Graphics& g, bool over, bool down)
{
    auto r = getLocalBounds().toFloat().reduced (2.f);
    const bool on = getToggleState();
    drawKey (g, r, on || down, over);
    drawJewel (g, juce::Rectangle<float> (7.f, 7.f).withCentre ({ r.getX() + 12.f, r.getCentreY() }), on, juce::Colour (look::T().led));
    g.setColour (on ? (look::themeIdx == 3 ? juce::Colour (look::T().led).brighter (0.4f) : look::print) : look::print.withAlpha (0.72f));
    g.setFont (font (10.f));
    g.drawText (getButtonText(), r.withTrimmedLeft (24.f), juce::Justification::centredLeft, false);
}

void FootSwitch::paintButton (juce::Graphics& g, bool, bool down)
{
    auto b = getLocalBounds().toFloat();
    const bool on = getToggleState();
    drawJewel (g, juce::Rectangle<float> (12.f, 12.f).withCentre ({ b.getCentreX(), b.getY() + 8.f }), on, juce::Colour (0xffff3b2a));
    const float d = juce::jmin (b.getWidth() - 8.f, b.getHeight() - 22.f) * (down ? 0.95f : 1.f);
    const juce::Point<float> c (b.getCentreX(), b.getY() + 20.f + (b.getHeight() - 20.f) * 0.5f);
    drawKnobBody (g, c, d * 0.5f, 0.f, 1.f, false);
    g.setColour (juce::Colour (0xff1a1816));
    g.setFont (font (11.f));
    g.drawText ("ON", juce::Rectangle<float> (d, d).withCentre (c), juce::Justification::centred, false);
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
    const int x0 = 12, y0 = 58, pitch = 30, inX = 196, outX = 252;
    for (size_t i = 0; i < seq.size(); ++i)
    {
        const int n = seq[i], y = y0 + (int) i * pitch;
        labels.push_back ({ nodeName (n), { x0, y - 7, 150, 14 }, false, false });
        if (audioIn[n] != nullptr)  { audioIn[n]->setVisible (true);  audioIn[n]->setCentrePosition (inX, y); }
        if (audioOut[n] != nullptr) { audioOut[n]->setVisible (true); audioOut[n]->setCentrePosition (outX, y); }
    }
    const int ySrc = y0 + 12 * pitch + 40;                                                   // sorties de modulation
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
        const int cx = 12 + 36 + (d % 4) * 70, cy = yCv + (d / 4) * 56;
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
    const int y0 = 58, pitch = 30;
    g.setColour (look::print.withAlpha (0.7f));
    g.setFont (font (9.f));
    g.drawText ("IN", 196 - 20, 36, 40, 11, juce::Justification::centred, false);
    g.drawText ("OUT", 252 - 20, 36, 40, 11, juce::Justification::centred, false);
    g.drawText ("AUDIO", 12, 36, 100, 11, juce::Justification::centredLeft, false);
    const int ySrc = y0 + 12 * pitch + 40;
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
    g.setFont (font (12.5f));
    g.drawText (title, fixed ? 12 : 32, 4, getWidth() - (fixed ? 24 : 100), 16, juce::Justification::centredLeft, false);
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
    look::applyTheme (loadThemePref());
    OrageLookAndFeel::refreshAll();

    addAndMakeVisible (themeBtn);
    themeBtn.onClick = [this] { cycleTheme(); };
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

    setSize (1440, kFxH);
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

    {   // BBD DELAY
        auto* u = newUnit (NBbd, "BBD DELAY", kModH, false);
        auto* a = mk (*u, "bbdTime", "TIME"); auto* b = mk (*u, "bbdFb", "FEEDBACK"); auto* c = mk (*u, "bbdTone", "TONE");
        auto* d = mk (*u, "bbdMod", "WOBBLE"); auto* e = mk (*u, "bbdMix", "MIX");
        auto* sync = addSync (*u, "bbdSync", a);
        auto* ping = addToggle (*u, "bbdPing", "PING-PONG");
        u->layoutFn = [=] (juce::Rectangle<int> r)
        {
            place (r, { a, b, c, d, e }, 0, kCell - 14);
            sync->setBounds (r.getX() + 5 * (kCell - 14) + 20, r.getCentreY() - 40, 120, 26);
            ping->setBounds (r.getX() + 5 * (kCell - 14) + 20, r.getCentreY() + 4, 120, 34);
        };
    }

    {   // PLATE REVERB
        auto* u = newUnit (NVerb, "PLATE REVERB + SHIMMER", kModH, false);
        auto* a = mk (*u, "revMix", "MIX"); auto* b = mk (*u, "revSize", "SIZE"); auto* c = mk (*u, "revDecay", "DECAY");
        auto* d = mk (*u, "revPre", "PRE-DELAY"); auto* e = mk (*u, "revDamp", "DAMPING"); auto* f = mk (*u, "revMod", "MOTION");
        auto* g2 = mk (*u, "revWidth", "WIDTH"); auto* sh = mk (*u, "revShimmer", "SHIMMER"); auto* lc = mk (*u, "revLowCut", "LOW CUT");
        auto* frz = addToggle (*u, "revFreeze", "FREEZE");
        u->layoutFn = [=] (juce::Rectangle<int> r)
        {
            place (r, { a, b, c, d, e, f, g2, sh, lc }, 0, 76);
            frz->setBounds (r.getX() + 9 * 76 + 14, r.getCentreY() - 17, 110, 34);
        };
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
    static const int tileNodes[] = { NOct, NDist, NColor, NFilter, NDestroy, NComp, NSpring, NBbd, NVerb, NGate };
    for (int n : tileNodes)
    {
        auto b = std::make_unique<juce::TextButton> (nodeName (n));
        b->setClickingTogglesState (false);
        b->onClick = [this, n] { if (proc.engine.inRack (n)) proc.engine.removeFromRack (n); else proc.engine.addToRack (n); refreshRack(); };
        side.addAndMakeVisible (*b);
        tiles.push_back (std::move (b));
    }
    side.subtitles.push_back ({ "MASTER", 286 });
    side.subtitles.push_back ({ "RESET", 516 });
    for (int n = 0; n < kNumNodes; ++n)                                                   // toutes les prises vivent dans le panneau patchbay
    {
        if (n != NOut) patchPanel.audioOut[n] = patchBay.makeJack (patchPanel, JackKind::AudioOut, n);
        if (n != NIn)  patchPanel.audioIn[n]  = patchBay.makeJack (patchPanel, JackKind::AudioIn, n);
    }
    for (int s = 0; s < kNumSrc; ++s) patchPanel.cvOut[s] = patchBay.makeJack (patchPanel, JackKind::CvOut, s);
    for (int d = 0; d < kNumCv; ++d)  patchPanel.cvIn[d]  = patchBay.makeJack (patchPanel, JackKind::CvIn, d);
    mk (side, "mix", "DRY / WET", "DRY", "WET", CvMix);
    mk (side, "output", "OUTPUT", "-", "+");
    side.addAndMakeVisible (foot);
    footAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "on", foot);

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
    static const int tileNodes[] = { NOct, NDist, NColor, NFilter, NDestroy, NComp, NSpring, NBbd, NGate };
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
    const auto cats = OrageAudioProcessor::getCategories();
    const int nf = OrageAudioProcessor::getNumFactoryPresets();
    m.addSectionHeader ("FACTORY");
    m.addItem (100, "Init", true, proc.presetName == "Init");
    for (const auto& c : cats)
    {
        juce::PopupMenu sub;
        int cnt = 0;
        for (int i = 1; i < nf; ++i)
            if (OrageAudioProcessor::getFactoryPresetCategory (i) == c)
            {
                const auto nm = OrageAudioProcessor::getFactoryPresetName (i);
                sub.addItem (100 + i, nm, true, nm == proc.presetName); ++cnt;
            }
        if (cnt > 0) m.addSubMenu (c.toUpperCase() + "  (" + juce::String (cnt) + ")", sub, true, nullptr, proc.presetCategory == c);
    }
    const auto user = proc.getUserPresetNames();
    if (user.size() > 0)
    {
        m.addSeparator();
        m.addSectionHeader ("USER");
        juce::StringArray ucats;
        for (const auto& u : user) { const auto c = proc.getUserPresetCategory (u); if (! ucats.contains (c)) ucats.add (c); }
        ucats.sort (true);
        for (const auto& c : ucats)
        {
            juce::PopupMenu sub;
            for (int i = 0; i < user.size(); ++i)
                if (proc.getUserPresetCategory (user[i]) == c) sub.addItem (1000 + i, user[i], true, user[i] == proc.presetName);
            m.addSubMenu ((c.isEmpty() ? juce::String ("SANS CATEGORIE") : c.toUpperCase()), sub);
        }
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
        else if (id == 2)    { pr.saveUserPreset (pr.presetName, pr.presetCategory); safe->afterPreset(); }
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
    juce::PopupMenu m;
    m.addSectionHeader ("CATEGORIE DU PRESET");
    const auto cats = OrageAudioProcessor::getCategories();
    for (int i = 0; i < cats.size(); ++i) m.addItem (10 + i, cats[i], true, proc.presetCategory == cats[i]);
    m.addSeparator();
    m.addItem (9, "Sans categorie");
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&nameBtn), [safe, cats] (int id)
    {
        if (safe == nullptr || id == 0) return;
        const juce::String cat = id >= 10 ? cats[id - 10] : juce::String();
        safe->showDialog ("SAUVEGARDER LE PRESET", "Nom du preset" + (cat.isEmpty() ? juce::String() : " (" + cat + ")") + " :", true, safe->proc.presetName, "SAUVEGARDER",
                          [safe, cat] (bool ok, const juce::String& text)
        {
            if (ok && safe != nullptr) { safe->proc.saveUserPreset (text, cat); safe->afterPreset(); }
        });
    });
}

void OrageAudioProcessorEditor::confirm (const juce::String& title, const juce::String& message, std::function<void()> action)
{
    showDialog (title, message, false, {}, "OK", [action] (bool ok, const juce::String&) { if (ok) action(); });
}

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
    headerArea = { 0, 0, W, kHeaderH };

    // ---- en-tete : presets
    prevBtn.setBounds (W - 400, 36, 30, 28);
    nameBtn.setBounds (W - 364, 36, 220, 28);
    nextBtn.setBounds (W - 140, 36, 30, 28);
    saveBtn.setBounds (W - 104, 36, 70, 28);
    cablesBtn.setBounds (W - 540, 38, 108, 24);
    themeBtn.setBounds (W - 640, 38, 100, 24);

    // ---- colonnes : panneau lateral | rack | (bandeau de modulation en bas)
    const int vpW = W - 170 - 310;
    const int top = kHeaderH + 8;
    patchPanel.setBounds (W - 310, top, 300, H - top - 10);
    side.setBounds (10, top, 140, H - top - 10);
    viewport.setBounds (160, top, vpW, H - top - 8 - 172 - 8);
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
    for (size_t i = 0; i < tiles.size(); ++i) tiles[i]->setBounds (8, 32 + (int) i * 25, 124, 22);
    knobById.at ("mix")->setBounds (4, 308, 66, 94);
    knobById.at ("output")->setBounds (70, 308, 66, 94);
    foot.setBounds (41, 414, 58, 90);
    clearRackBtn.setBounds (8, 544, 124, 28);
    clearPatchBtn.setBounds (8, 578, 124, 28);
    resetBtn.setBounds (8, 612, 124, 28);

    rebuildFacade();
}

void OrageAudioProcessorEditor::rebuildFacade()
{
    const int W = getWidth(), H = getHeight();
    const auto& th = look::T();
    paintFacade (panelTex, W, H, true);
    juce::Graphics g (panelTex);
    g.setColour (look::print.withAlpha (0.95f));
    g.setFont (font (46.f));
    g.drawText (titleText, 30, 2, 340, 52, juce::Justification::centredLeft, false);
    g.setGradientFill (juce::ColourGradient (juce::Colour (th.trim), 32.f, 0.f, juce::Colour (th.trim).withAlpha (0.f), 212.f, 0.f, false));
    g.fillRect (32, 55, 180, 2);
    g.setColour (look::print.withAlpha (0.8f));
    g.setFont (font (10.5f));
    g.drawText (subText, 32, 60, 420, 13, juce::Justification::centredLeft, false);
    g.setFont (font (9.f, false));
    g.setColour (juce::Colour (th.trim).withAlpha (0.9f));
    g.drawText ("FOUDRE AUDIO  -  PATCH IT. BREAK IT.", 32, 71, 360, 11, juce::Justification::centredLeft, false);
    g.setColour (look::print.withAlpha (0.8f));
    g.setFont (font (9.5f));
    g.drawText ("PRESET", W - 400, 20, 80, 13, juce::Justification::centredLeft, false);
    repaint();
}

void OrageAudioProcessorEditor::cycleTheme()
{
    look::applyTheme (look::themeIdx + 1 > 3 ? 0 : look::themeIdx + 1);
    saveThemePref (look::themeIdx);
    OrageLookAndFeel::refreshAll();
    themeBtn.setButtonText (look::T().name);
    if (auto* top = getTopLevelComponent()) top->repaint();
    rebuildFacade();
    if (onThemeChanged) onThemeChanged();
}

void OrageAudioProcessorEditor::mouseUp (const juce::MouseEvent& e)
{
    if (lampWindow().contains (e.position) && ! e.mouseWasDraggedSinceMouseDown())
    {
        proc.headerScope.store (proc.headerScope.load() == 0 ? 1 : 0);
        repaint (headerArea);
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

void OrageAudioProcessorEditor::drawScope (juce::Graphics& g, juce::Rectangle<float> scr)
{
    const auto& th = look::T();
    const juce::Colour ph (th.scope);
    g.setGradientFill (juce::ColourGradient (ph.withAlpha (0.10f), scr.getCentreX(), scr.getCentreY(), juce::Colour (0xff020303), scr.getX(), scr.getY(), true));
    g.fillRect (scr);
    g.setColour (ph.withAlpha (0.10f));
    for (int i = 1; i < 4; ++i) g.drawHorizontalLine ((int) (scr.getY() + scr.getHeight() * (float) i / 4.f), scr.getX(), scr.getRight());
    for (int i = 1; i < 8; ++i) g.drawVerticalLine ((int) (scr.getX() + scr.getWidth() * (float) i / 8.f), scr.getY(), scr.getBottom());
    g.setColour (ph.withAlpha (0.2f));
    g.drawHorizontalLine ((int) scr.getCentreY(), scr.getX(), scr.getRight());

    static float buf[4096];
    proc.copyScope (buf, 4096);
    // declenchement : passage par zero montant ; l'affichage couvre ~2,5 periodes
    int first = -1, last = -1, count = 0;
    for (int i = 40; i < 4096 - 8; ++i)
        if (buf[i - 1] < -0.004f && buf[i] >= 0.f) { if (first < 0) first = i; last = i; ++count; }
    int span = 1024, start = 4096 - 1024;
    if (count >= 2)
    {
        const float period = (float) (last - first) / (float) (count - 1);
        span = juce::jlimit (96, 2000, juce::roundToInt (period * 2.5f));
        start = first;
        for (int i = last; i >= first; --i) if (buf[i - 1] < -0.004f && buf[i] >= 0.f && i + span < 4096) { start = i; break; }
    }
    float pk = 0.f;
    for (int i = 0; i < span; ++i) pk = juce::jmax (pk, std::abs (buf[start + i]));
    const float target = pk > 0.003f ? juce::jmin (6.f, 0.85f / pk) : 1.f;
    scopeGain += (target - scopeGain) * 0.15f;
    juce::Path p;
    const int nx = (int) scr.getWidth();
    for (int x = 0; x <= nx; ++x)
    {
        const float pos = (float) x / (float) nx * (float) (span - 1);
        const int i0 = (int) pos; const float fr = pos - (float) i0;
        const float v = buf[start + i0] + (buf[start + juce::jmin (i0 + 1, span - 1)] - buf[start + i0]) * fr;
        const float py = scr.getCentreY() - juce::jlimit (-1.f, 1.f, v * scopeGain) * scr.getHeight() * 0.44f;
        if (x == 0) p.startNewSubPath (scr.getX(), py); else p.lineTo (scr.getX() + (float) x, py);
    }
    g.setColour (ph.withAlpha (0.16f)); g.strokePath (p, juce::PathStrokeType (5.f, juce::PathStrokeType::curved));
    g.setColour (ph.withAlpha (0.35f)); g.strokePath (p, juce::PathStrokeType (2.6f, juce::PathStrokeType::curved));
    g.setColour (ph.brighter (0.5f));   g.strokePath (p, juce::PathStrokeType (1.2f, juce::PathStrokeType::curved));
    g.setColour (ph.withAlpha (0.6f));
    g.setFont (font (8.5f));
    g.drawText ("OSC", scr.withTrimmedLeft (5.f).withHeight (12.f).translated (0.f, scr.getHeight() - 13.f), juce::Justification::centredLeft, false);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.07f), scr.getX(), scr.getY(), juce::Colours::transparentWhite, scr.getX(), scr.getCentreY(), false));
    g.fillRect (scr);                                                                  // reflet sur le verre
}

void OrageAudioProcessorEditor::paint (juce::Graphics& g)
{
    const auto& th = look::T();
    g.drawImageAt (panelTex, 0, 0);

    const auto win = lampWindow();                       // une seule fenetre : lampe (chauffe avec la distorsion) ou oscilloscope (clic)
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillRoundedRectangle (win.expanded (3.f).translated (0.f, 2.f), 9.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (th.metalHi), win.getX(), win.getY(), juce::Colour (th.metalLo), win.getRight(), win.getBottom(), false));
    g.fillRoundedRectangle (win.expanded (3.f), 8.f);                                 // lunette metallique
    g.setColour (juce::Colour (0xff040404));
    g.fillRoundedRectangle (win, 6.f);
    const bool scope = proc.headerScope.load() == 1;
    if (scope)
    {
        juce::Graphics::ScopedSaveState ss (g);
        juce::Path clip; clip.addRoundedRectangle (win.reduced (2.f), 5.f);
        g.reduceClipRegion (clip);
        drawScope (g, win.reduced (2.f));
    }
    else
    {
        const float h = heatShown;
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (win.reduced (3.f).toNearestInt());
        drawTube (g, { win.getCentreX() - 22.f, win.getBottom() - 6.f - 60.f, 44.f, 60.f }, juce::jlimit (0.f, 1.f, h), arcShown);
        if (arcShown > 0.35f)
            drawBolt (g, { win.getX() + 60.f + rnd.nextFloat() * 20.f, win.getY() + 14.f }, { win.getRight() - 60.f - rnd.nextFloat() * 20.f, win.getY() + 18.f });
    }
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawRoundedRectangle (win.reduced (0.5f), 6.f, 1.5f);
    g.setColour (look::print.withAlpha (0.85f));
    g.setFont (font (10.f));
    g.drawText (scope ? "SCOPE" : "HEAT", (int) win.getRight() + 10, (int) win.getCentreY() - 14, 60, 14, juce::Justification::centredLeft, false);
    g.setColour (juce::Colour (th.trim).withAlpha (0.8f));
    g.setFont (font (8.5f, false));
    g.drawText ("click to switch", (int) win.getRight() + 10, (int) win.getCentreY(), 74, 12, juce::Justification::centredLeft, false);
}
