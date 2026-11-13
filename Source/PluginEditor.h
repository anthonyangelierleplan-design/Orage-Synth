#pragma once
#include <functional>
#include <map>
#include <tuple>
#include "PluginProcessor.h"

namespace look
{
// Themes "luxe" : meme disposition, finitions differentes (metal brosse, reliefs, LED)
struct Theme
{
    const char* name;
    juce::uint32 panelTop, panelBot, plateTop, plateBot;      // facade et plaques de modules
    juce::uint32 metalHi, metalMid, metalLo;                   // rampe du metal (potards, touches)
    juce::uint32 trim, pointer, led, scope, print;             // filets, repere de potard, LED, ecran, serigraphie
};
inline const Theme kThemes[4] {
    { "CHAMPAGNE", 0xff1f1c19, 0xff100f0d, 0xff1a1816, 0xff0e0d0c, 0xfff3e2b4, 0xffc9a45c, 0xff4a3a1c, 0xffd4b26a, 0xfffff0c8, 0xffff9a3a, 0xffffb84a, 0xffeee6d2 },
    { "ARGENT",    0xff1b1d21, 0xff0e0f11, 0xff17191c, 0xff0b0c0e, 0xfff4f6fa, 0xffb2b9c4, 0xff3a3f48, 0xffc3c9d3, 0xffffffff, 0xff7fd4ff, 0xff7fe0ff, 0xffe8ecf2 },
    { "CUIVRE",    0xff161112, 0xff0a0808, 0xff130f10, 0xff080606, 0xfff2b89a, 0xffb8693f, 0xff3d1c0e, 0xffc97a4e, 0xffffd2b0, 0xffff6a3a, 0xffff7a4a, 0xffefe3d8 },
    { "ORIGINAL",  0xff161617, 0xff0e0e0f, 0xff111112, 0xff0a0a0b, 0xff8a8a90, 0xff3a3a3e, 0xff101011, 0xff8a8a90, 0xfff0ece0, 0xffff7a1a, 0xffff9a3a, 0xffe9e5d8 }
};
inline int themeIdx = 0;
inline const Theme& T() { return kThemes[themeIdx < 0 || themeIdx > 3 ? 0 : themeIdx]; }
inline juce::Colour paint  { 0xff111112 };       // fond des plaques (suit le theme)
inline juce::Colour print  { 0xffe9e5d8 };       // serigraphie (suit le theme)
inline const juce::Colour cvRing { 0xffe0662a }; // anneau orange des prises CV
inline void applyTheme (int i) { themeIdx = juce::jlimit (0, 3, i); paint = juce::Colour (T().plateBot); print = juce::Colour (T().print); }
}

void paintFacade (juce::Image& img, int W, int H, bool frame);     // fond de facade selon le theme

constexpr int kHeaderH = 96;      // hauteur de l'en-tete (une seule lampe)
constexpr int kFxH = 832;         // hauteur de conception de l'editeur d'effets

class OrageAudioProcessorEditor;
class RackView;
class PatchBay;
class PatchPanel;

// ---------------------------------------------------------------- potard use + boites de dialogue sombres
class OrageLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OrageLookAndFeel();
    ~OrageLookAndFeel() override;
    void refresh();                               // reprend les couleurs du theme courant
    static void refreshAll();                     // ... pour toutes les fenetres ouvertes
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
};

// ---------------------------------------------------------------- potard (echelle et nom serigraphies dans paint())
// potard / curseur avec "cran" : un aimant leger retient la valeur au zero (double-clic = valeur par defaut)
class DetentSlider : public juce::Slider
{
public:
    using juce::Slider::Slider;
    double detent = -1e30;                                   // valeur du cran (hors plage = aucun)
    bool showMark = false;                                   // trait orange sur la piste (curseurs lineaires)
    double snapValue (double v, DragMode mode) override
    {
        if (mode == notDragging || detent < getMinimum() || detent > getMaximum()) return v;
        return std::abs (v - detent) < (getMaximum() - getMinimum()) * 0.022 ? detent : v;
    }
    void paintOverChildren (juce::Graphics& g) override
    {
        if (! showMark || detent < getMinimum() || detent > getMaximum()) return;
        const float x = (float) getPositionOfValue (detent);
        g.setColour (juce::Colour (0xffe0662a));
        g.fillRect (x - 1.f, (float) getHeight() * 0.5f - 9.f, 2.f, 18.f);
    }
};

class KnobUnit : public juce::Component
{
public:
    KnobUnit (juce::AudioProcessorValueTreeState&, const juce::String& id, const juce::String& text,
              const juce::String& lo, const juce::String& hi, int cvIdx = -1);
    void paint (juce::Graphics&) override;
    void resized() override;
    void setCvActive (bool);

    DetentSlider slider;
    juce::String text, loText, hiText;
    int cvIndex;                                     // destination CV associee (ou -1)
    bool cvActive = false;
private:
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
};

// ---------------------------------------------------------------- boutons-poussoirs (parametre "choice")
class SegmentSelector : public juce::Component
{
public:
    SegmentSelector (juce::AudioProcessorValueTreeState&, const juce::String& id, juce::StringArray names);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
private:
    juce::StringArray labels;
    int selected = 0;
    juce::ParameterAttachment attachment;
};

class LedToggle : public juce::ToggleButton
{
public:
    void paintButton (juce::Graphics&, bool over, bool down) override;
};

class FootSwitch : public juce::ToggleButton
{
public:
    void paintButton (juce::Graphics&, bool over, bool down) override;
};

// ---------------------------------------------------------------- vumeter a aiguille (reduction de gain du compresseur)
class VuMeter : public juce::Component
{
public:
    void setValue (float grDb) { target = grDb; }
    void tick();
    void paint (juce::Graphics&) override;
private:
    float target = 0.f, shown = 0.f, lastDrawn = -1.f;
};

// ---------------------------------------------------------------- voyant (gate ouvert)
class LedLamp : public juce::Component
{
public:
    explicit LedLamp (juce::String t) : text (std::move (t)) {}
    void set (bool on) { if (on != lit) { lit = on; repaint(); } }
    void paint (juce::Graphics&) override;
private:
    juce::String text;
    bool lit = true;
};

// ---------------------------------------------------------------- prise jack du patchbay
enum class JackKind { AudioOut, AudioIn, CvOut, CvIn };

class Jack : public juce::Component
{
public:
    Jack (PatchBay&, JackKind, int index);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    juce::Point<float> socketCentre() const;

    JackKind kind;
    int index;
    bool highlight = false;
private:
    PatchBay& owner;
};

// ---------------------------------------------------------------- calque des cables : transparent, recouvre toute la fenetre.
// Les prises vivent sur la face de chaque module ; ce calque dessine les cables fins par-dessus et gere le glisser-deposer.
class PatchBay : public juce::Component
{
public:
    explicit PatchBay (OrageAudioProcessor&);
    Jack* makeJack (juce::Component& parent, JackKind, int index);
    void paint (juce::Graphics&) override;
    bool hitTest (int, int) override { return false; }
    void beginDrag (Jack&);
    void dragMoved (juce::Point<int>);
    void endDrag (juce::Point<int>);
    void clearJack (Jack&);
    void hover (Jack* j) { if (hovered != j) { hovered = j; repaint(); } }
    uint32_t stateHash() const;
    void repaintLeds();
    juce::Point<float> centreOf (const Jack&) const;

    bool cablesVisible = true;
    OrageAudioProcessor& proc;
private:
    bool usable (const Jack*) const;
    void tryConnect (Jack&, Jack&);

    std::vector<std::unique_ptr<Jack>> jacks;
    Jack* audioOutJ[orage::kNumNodes] {};
    Jack* audioInJ[orage::kNumNodes] {};
    Jack* cvOutJ[orage::kNumSrc] {};
    Jack* cvInJ[orage::kNumCv] {};
    Jack* dragFrom = nullptr;
    Jack* hovered = nullptr;
    juce::Point<float> dragPos;
};

// ---------------------------------------------------------------- panneau patchbay (toutes les prises, separe des modules)
class PatchPanel : public juce::Component
{
public:
    explicit PatchPanel (OrageAudioProcessorEditor& e) : owner (e) {}
    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void layoutJacks();                                         // relit le rack : lignes audio dans l'ordre, prises des modules absents masquees

    Jack* audioOut[orage::kNumNodes] {};
    Jack* audioIn[orage::kNumNodes] {};
    Jack* cvOut[orage::kNumSrc] {};
    Jack* cvIn[orage::kNumCv] {};
private:
    struct Label { juce::String text; juce::Rectangle<int> r; bool dim; bool cv; };
    std::vector<Label> labels;
    OrageAudioProcessorEditor& owner;
};

// ---------------------------------------------------------------- un module (rack ou bandeau de modulation)
class ModuleUnit : public juce::Component
{
public:
    ModuleUnit (RackView* rack, int node, const juce::String& title, int fullHeight, bool fixed);
    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

    void addJack (Jack*, int col, int row, const juce::String& label);
    int jackStripWidth() const { return jackCols > 0 ? jackCols * 58 + 8 : 0; }

    int node;
    juce::String title;
    int fullHeight;
    bool fixed, collapsed = false;
    std::function<void (juce::Rectangle<int>)> layoutFn;
    std::function<void()> onRemove;
    int topTrim = 28;                                // hauteur reservee au bandeau de titre avant la zone des commandes
    std::function<bool()> isActive;                  // false : module contourne (bypass)
    std::function<void()> onToggleActive;
    int currentHeight() const { return collapsed ? 26 : fullHeight; }
private:
    struct JackSlot { Jack* jack; int col, row; juce::String label; };
    std::vector<JackSlot> jackSlots;
    int jackCols = 0;
    RackView* rackView;
    enum Region { None, Handle, Power, Collapse, Remove } pressed = None;
    Region regionAt (juce::Point<int>) const;
    int grabOffset = 0;
};

// ---------------------------------------------------------------- le rack (contenu du Viewport)
class RackView : public juce::Component, private juce::Timer
{
public:
    explicit RackView (OrageAudioProcessorEditor& e) : owner (e) {}
    void paint (juce::Graphics&) override;
    void resized() override { relayout(); }
    void setUnits (std::vector<ModuleUnit*> inRackOrder);       // modules visibles, dans l'ordre
    void layoutUnits (const std::vector<ModuleUnit*>& list, ModuleUnit* dragged = nullptr, int draggedY = 0);
    void relayout() { layoutUnits (order); }
    void dragStart (ModuleUnit*, int grabOffsetY);
    void dragMove (ModuleUnit*, int pointerY, int grabOffsetY);
    void dragEnd (ModuleUnit*);
private:
    void timerCallback() override;                              // defilement automatique pendant un deplacement de module
    OrageAudioProcessorEditor& owner;
    std::vector<ModuleUnit*> order, temp;
    ModuleUnit* dragU = nullptr;
    int lastPointerY = 0, dragGrab = 0;
};

// ---------------------------------------------------------------- panneaux lateraux
class PlatePanel : public juce::Component
{
public:
    explicit PlatePanel (juce::String t) : title (std::move (t)) {}
    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    juce::String title;
    std::vector<std::pair<juce::String, int>> subtitles;        // (texte, y) serigraphies supplementaires
    std::vector<std::tuple<juce::String, int, int>> notes;      // (texte centre, x, y) etiquettes de prises
};

// ---------------------------------------------------------------- boite de dialogue integree (plaque metal, assortie au design)
class OrageDialog : public juce::Component
{
public:
    OrageDialog (juce::String title, juce::String message, bool askText, juce::String initialText, juce::String okText,
                 std::function<void (bool, const juce::String&)> callback);
    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress&) override;
    void focus();

    std::function<void()> onFinished;
private:
    void finish (bool ok);
    juce::String titleText, messageText;
    bool hasText;
    juce::TextEditor editor;
    juce::TextButton okBtn, cancelBtn;
    std::function<void (bool, const juce::String&)> cb;
    juce::Rectangle<int> box;
    bool done = false;
};

// ---------------------------------------------------------------- editeur
class OrageAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit OrageAudioProcessorEditor (OrageAudioProcessor&, juce::String title = "ORAGE FX",
                                        juce::String subtitle = "SEMI-MODULAR HIGH VOLTAGE DISTORTION");
    ~OrageAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;

    // appels du rack
    void rackChangedByUser (const std::vector<int>& newOrder);
    void refreshRack();                                         // reconstruit rack, patchbay et tuiles depuis le moteur

    OrageAudioProcessor& proc;
    juce::String titleText, subText;
    juce::Component* dialogHost = nullptr;                      // si non nul : composant qui porte les dialogues (fenetre complete)

private:
    void timerCallback() override;
    void buildUnits();
    void rebuildFacade();
    void cycleTheme();
    void drawScope (juce::Graphics&, juce::Rectangle<float> screen);
    void presetMenu();
    void afterPreset();
    void saveDialog();
    void showDialog (const juce::String& title, const juce::String& message, bool askText, const juce::String& initial,
                     const juce::String& okText, std::function<void (bool, const juce::String&)> cb);
    void stepPreset (int dir);
    void confirm (const juce::String& title, const juce::String& message, std::function<void()> action);
    KnobUnit* mk (juce::Component& parent, const char* id, const juce::String& text, const char* lo = "0", const char* hi = "10", int cv = -1);
    void drawTube (juce::Graphics&, juce::Rectangle<float>, float glow, float arc);
    void drawBolt (juce::Graphics&, juce::Point<float> from, juce::Point<float> to);
    ModuleUnit* newUnit (int node, const juce::String& title, int height, bool fixed);

    OrageLookAndFeel laf;
    juce::Image panelTex;
    juce::Random rnd;
    float heatShown = 0.f, arcShown = 0.f;
    uint32_t lastPatchHash = 0, lastRackHash = 0, lastBypass = 0;

    // en-tete
    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, nameBtn, saveBtn { "SAVE" };
    juce::Rectangle<int> headerArea;

    // composants
    RackView rackView { *this };
    juce::Viewport viewport;
    PlatePanel side { "MODULES" };
    std::vector<std::unique_ptr<juce::TextButton>> tiles;       // une par module audio
    PatchBay patchBay;
    PatchPanel patchPanel { *this };
    std::unique_ptr<OrageDialog> dialog;
    Jack *jackIn = nullptr, *jackOut = nullptr, *jackMix = nullptr;
    LedToggle cablesBtn;

    std::vector<std::unique_ptr<KnobUnit>> knobs;
    std::map<std::string, KnobUnit*> knobById;
    std::vector<std::unique_ptr<juce::Component>> owned;        // selecteurs, toggles, vumeter...
    std::vector<std::unique_ptr<juce::ComboBox>> combos;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAtt;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAtt;
    struct SyncLink { juce::ComboBox* box; KnobUnit* rate; bool lastFree = true; };
    std::vector<SyncLink> syncLinks;
    VuMeter* vu = nullptr;
    LedLamp* gateLamp = nullptr;

    std::map<int, std::unique_ptr<ModuleUnit>> rackUnits;       // modules audio (node -> unit)
    std::vector<std::unique_ptr<ModuleUnit>> stripUnits;        // LFO1, LFO2, S&H, ENV

    FootSwitch foot;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> footAtt;
    juce::TextButton clearRackBtn, clearPatchBtn, resetBtn, themeBtn { "LOOK" };
    float scopeGain = 1.f;
public:
    std::function<void()> onThemeChanged;                        // pour que la fenetre parente repeigne ses propres fonds
private:
    static juce::Rectangle<float> lampWindow() { return { 470.f, 12.f, 220.f, 72.f }; }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OrageAudioProcessorEditor)
};
