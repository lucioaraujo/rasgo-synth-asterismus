#include <JuceHeader.h>

#include <atomic>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <optional>

#ifndef RASGO_ASTERISMUS_VERSION
#define RASGO_ASTERISMUS_VERSION "0.0.0"  // vem do CMake (PROJECT_VERSION)
#endif

#include <rasgo/performance/PerformanceConstellationGeometry.hpp>
#include <rasgo/performance/PerformanceAudioMeasurement.hpp>
#include <rasgo/performance/PerformanceModuleTypes.hpp>
#include <rasgo/performance/PerformanceModuleLibrary.hpp>
#include <rasgo/performance/PerformanceLayout.hpp>
#include <rasgo/performance/PerformanceMemory.hpp>
#include <rasgo/performance/PerformanceAlbum.hpp>
#include <rasgo/performance/PerformanceAlbumMaster.hpp>
#include <rasgo/performance/PerformanceSessionController.hpp>
#include <rasgo/performance/PerformanceSky.hpp>
#include <rasgo/performance/PerformanceTerritoryFactory.hpp>

#include "RealtimeAudioThread.h"

namespace {

using namespace rasgo::performance;

namespace branding {

juce::File assetsDirectory() {
    const auto overrideDirectory = juce::SystemStats::getEnvironmentVariable(
        "RASGO_SYNTH_PERFORMANCE_ASSETS_DIR", {});
    if (overrideDirectory.isNotEmpty())
        return juce::File(overrideDirectory);

    const auto executableDirectory = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
        .getParentDirectory();
    const auto besideExecutable = executableDirectory.getChildFile("assets");
    if (besideExecutable.isDirectory()) return besideExecutable;
    // Instalação Linux (FHS): bin/rasgo-synth-asterismus e
    // share/rasgo-synth-asterismus/assets (30/10, empacotamento).
    const auto installed = executableDirectory.getParentDirectory()
        .getChildFile("share/rasgo-synth-asterismus/assets");
    return installed.isDirectory() ? installed : besideExecutable;
}

juce::Typeface::Ptr typefaceFromAsset(const char* relativePath) {
    const auto file = assetsDirectory().getChildFile(relativePath);
    juce::MemoryBlock data;
    if (!file.existsAsFile() || !file.loadFileAsData(data)) return nullptr;
    return juce::Typeface::createSystemTypefaceFor(data.getData(), data.getSize());
}

juce::Typeface::Ptr displayTypeface(bool bold) {
    static auto regular = typefaceFromAsset("fonts/BigShouldersDisplay.ttf");
    if (!bold || regular == nullptr) return regular;
    static auto strong = [] {
        const auto base = typefaceFromAsset("fonts/BigShouldersDisplay.ttf");
        if (base == nullptr) return juce::Typeface::Ptr{};
        const auto configuration = base->getNamedInstanceConfiguration("Bold");
        return configuration.empty() ? juce::Typeface::Ptr{}
            : base->cloneWithVariableSettings(configuration);
    }();
    return strong != nullptr ? strong : regular;
}

juce::Typeface::Ptr monoTypeface(bool bold) {
    static auto regular = typefaceFromAsset("fonts/IBMPlexMono-Regular.ttf");
    static auto strong = typefaceFromAsset("fonts/IBMPlexMono-Bold.ttf");
    return bold ? strong : regular;
}

juce::Font display(float height, int style = juce::Font::plain) {
    const bool bold = (style & juce::Font::bold) != 0;
    const auto typeface = displayTypeface(bold);
    juce::Font font = typeface != nullptr
        ? juce::Font(juce::FontOptions(typeface))
        : juce::Font(juce::FontOptions("Sans", height, style));
    font.setHeight(height);
    return font;
}

juce::Font mono(float height, int style = juce::Font::plain) {
    // Nunca delegar o chrome ao Monospace do sistema: a fonte escolhida por
    // cada sessão pode mudar a métrica e, em particular, faltar acentos ou
    // símbolos de operação. IBM Plex é copiada junto do binário; os fallbacks
    // entram somente para caracteres excepcionalmente ausentes.
    const auto typeface = monoTypeface((style & juce::Font::bold) != 0);
    auto options = typeface != nullptr
        ? juce::FontOptions(typeface)
        : juce::FontOptions("DejaVu Sans Mono", height, style);
    options = options.withFallbacks({"DejaVu Sans Mono", "Noto Sans Symbols 2", "Noto Sans"});
    juce::Font font(options);
    font.setHeight(height);
    return font;
}

class LogoMark final : public juce::Component {
public:
    LogoMark() {
        const auto logo = assetsDirectory().getChildFile("rasgo_logo_2026.svg");
        if (logo.existsAsFile()) {
            drawable_ = juce::Drawable::createFromSVGFile(logo);
            if (drawable_) drawable_->replaceColour(juce::Colours::black, juce::Colour(0xffd18a45));
        }
    }

    void paint(juce::Graphics& graphics) override {
        if (drawable_ != nullptr)
            drawable_->drawWithin(graphics, getLocalBounds().toFloat().reduced(1.0f),
                                  juce::RectanglePlacement::centred, 1.0f);
    }

private:
    std::unique_ptr<juce::Drawable> drawable_;
};

} // namespace branding

juce::Font interfaceFont(float height, int style = juce::Font::plain) {
    return branding::mono(height, style);
}

juce::String utf8(const char* value) {
    // Regra do host: todo texto visível que contenha algo além de ASCII passa
    // por aqui. juce::String(const char*) trata a entrada como ASCII; usar a
    // sobrecarga implícita é a origem clássica de “Â·”/“Ã£”. Novos textos devem
    // usar escapes UTF-8 (por exemplo "\\xc2\\xb7") quando vierem de código.
    return juce::String(juce::CharPointer_UTF8(value));
}

enum class SupportLanguage : std::uint8_t { Portuguese, English, French, Spanish };

// One vocabulary gateway for the user-facing surface.  Performance terms such
// as ALEA, RASGO and CODA stay stable where they are names of the instrument;
// their explanatory text is localized around them.
juce::String uiText(SupportLanguage language, const char* portuguese,
                    const char* english, const char* french,
                    const char* spanish) {
    switch (language) {
    case SupportLanguage::English: return utf8(english);
    case SupportLanguage::French: return utf8(french);
    case SupportLanguage::Spanish: return utf8(spanish);
    default: return utf8(portuguese);
    }
}

const char* supportLanguageCode(SupportLanguage language) noexcept {
    switch (language) {
    case SupportLanguage::English: return "EN";
    case SupportLanguage::French: return "FR";
    case SupportLanguage::Spanish: return "ES";
    default: return "PT";
    }
}

juce::String aboutCopy(SupportLanguage language) {
    switch (language) {
    case SupportLanguage::English:
        return utf8("Generative instrument for composition and performance.\n\n"
                    "Version " RASGO_ASTERISMUS_VERSION " · 2026\n\n"
                    "Concept, artistic direction, music and RASGO identity\nLucio Araújo\n\n"
                    "Original code: GNU GPLv3\nHost: JUCE 9 under AGPLv3\n"
                    "Typefaces: Big Shoulders Display and IBM Plex Mono · SIL OFL 1.1\n\n"
                    "Full credits and provenance: THIRD_PARTY_NOTICES.md");
    case SupportLanguage::French:
        return utf8("Instrument génératif pour la composition et la performance.\n\n"
                    "Version " RASGO_ASTERISMUS_VERSION " · 2026\n\n"
                    "Conception, direction artistique, musique et identité RASGO\nLucio Araújo\n\n"
                    "Code original : GNU GPLv3\nHôte : JUCE 9 sous AGPLv3\n"
                    "Typographies : Big Shoulders Display et IBM Plex Mono · SIL OFL 1.1\n\n"
                    "Crédits et provenance : THIRD_PARTY_NOTICES.md");
    case SupportLanguage::Spanish:
        return utf8("Instrumento generativo para composición y performance.\n\n"
                    "Versión " RASGO_ASTERISMUS_VERSION " · 2026\n\n"
                    "Concepción, dirección artística, música e identidad RASGO\nLucio Araújo\n\n"
                    "Código original: GNU GPLv3\nHost: JUCE 9 bajo AGPLv3\n"
                    "Tipografías: Big Shoulders Display e IBM Plex Mono · SIL OFL 1.1\n\n"
                    "Créditos y procedencia: THIRD_PARTY_NOTICES.md");
    default:
        return utf8("Instrumento generativo para composição e performance.\n\n"
                    "Versão " RASGO_ASTERISMUS_VERSION " · 2026\n\n"
                    "Concepção, direção artística, música e identidade RASGO\nLucio Araújo\n\n"
                    "Código autoral: GNU GPLv3\nHost: JUCE 9 pela via AGPLv3\n"
                    "Tipografia: Big Shoulders Display e IBM Plex Mono · SIL OFL 1.1\n\n"
                    "Créditos e proveniência completos: THIRD_PARTY_NOTICES.md");
    }
}

namespace {
constexpr int kMinimumRotaryKnobDiameter = 88;
constexpr int kMaximumRotaryKnobDiameter = 132;

juce::Rectangle<int> readableKnobBounds(juce::Rectangle<int> cell) {
    const auto available = cell.reduced(2, 0);
    // A rotary control is never silently reduced below the performance
    // minimum. Layout callers must provide a cell for this target or hide the
    // secondary panel at the compact breakpoint; overlap is not a fallback.
    const auto diameter = juce::jlimit(kMinimumRotaryKnobDiameter,
                                       kMaximumRotaryKnobDiameter,
                                       juce::jmin(available.getWidth(),
                                                  available.getHeight() - 18));
    const auto width = juce::jmax(48, diameter + 18);
    const auto height = juce::jmax(42, diameter + 20);
    return {available.getCentreX() - width / 2, available.getY(), width, height};
}

// The stock JUCE TextButton is visually too heavy for a narrow mixer strip.
// These two-state controls are deliberately painted as small flat switches;
// their state colour is still supplied by refreshStemMixVisual().
class CompactMixerButton final : public juce::TextButton {
public:
    using juce::TextButton::TextButton;

private:
    void paintButton(juce::Graphics& graphics, bool highlighted, bool down) override {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);
        auto fill = findColour(juce::TextButton::buttonColourId);
        if (down) fill = fill.brighter(0.18f);
        else if (highlighted) fill = fill.brighter(0.08f);
        graphics.setColour(fill);
        graphics.fillRoundedRectangle(bounds, 2.0f);
        graphics.setColour(findColour(juce::TextButton::textColourOffId).withAlpha(0.7f));
        graphics.drawRoundedRectangle(bounds, 2.0f, 1.0f);
        graphics.setColour(findColour(juce::TextButton::textColourOffId));
        graphics.setFont(interfaceFont(10.0f, juce::Font::bold));
        graphics.drawFittedText(getButtonText(), getLocalBounds().reduced(2),
                                juce::Justification::centred, 1);
    }
};

// The Studio's tutorial is a separate reading surface.  The Performance uses
// the same interaction contract, but owns a real content component so its
// readable area cannot collapse when AlertWindow recalculates its layout.
class PerformanceTutorialContent final : public juce::Component {
public:
    explicit PerformanceTutorialContent(SupportLanguage language) : language_(language) {
        setSize(940, 680);
        text_.setText(language == SupportLanguage::Portuguese ? utf8(
            "1. COMEÇAR · SEED E TERRITÓRIO\n"
            "A seed abre uma possibilidade musical: as estrelas acesas, papéis, matéria, tempo e uma narrativa. "
            "Digite um número e confirme ou use ALEA. A seed também define valores iniciais próprios para os módulos; ela não deve apagar o desenho já existente no Campo.\n\n"
            "2. COMPOR NO CAMPO\n"
            "Arraste os círculos dos módulos para estabelecer posições, vizinhanças e conexões. O conjunto desenhado "
            "pelo performer forma uma configuração própria. Posições são memória autoral: apenas um gesto intencional "
            "deve alterar ou apagar esse desenho.\n\n"
            "3. LER A CONSTELAÇÃO\n"
            "A Constelação é um sistema de orientação, não um navegador de presets. Ela mostra os corpos, suas relações, "
            "fluxos e o caminho percorrido. Distância, conexão, movimento e profundidade devem resultar em mudança musical.\n\n"
            "4. EIXO DE FORÇAS\n"
            "O círculo laranja articula AUTONOMIA, INTERVENÇÃO, MEMÓRIA e RISCO sem pares inversos: ambos podem chegar a zero. O azul articula "
            "DENSIDADE, TENSÃO, ESPAÇO e PROFUNDIDADE. Os dois corpos têm quatro macros próprios, sem arrastar controles um do outro. "
            "A mudança deve ser audível; os percursos autônomos ainda serão desenvolvidos como composição, não decoração. Os botões redondos ao lado são os mesmos oito valores: o eixo é o gesto, os botões o ajuste fino.\n\n"
            "5. CENAS E GESTOS\n"
            "NASCENTE, ESPIRAL, RASGO e BALANÇO transformam o material sem interromper uma performance em PLAY. CAPTURAR guarda "
            "um estado de cena; DESFAZER retorna à captura anterior. HOLD suspende, ABRIR abre um desvio, RASGAR introduz ruptura, REPARAR recompõe, "
            "STUTTER cria recortes e DESLOCAR desloca o pulso e a fase do stem (o andamento não muda). Todos incidem sobre o que já existe.\n\n"
            "6. OUVIR E ORQUESTRAR\n"
            "Cada módulo é uma estrela com stem próprio: até oito corpos no Campo, cada um no seu canal do Campo de Matérias. Uma estrela com presença zero fica apagada e não gasta processamento; subir a presença a acende. As estrelas acesas se relacionam pela trama do céu — respostas, contágios, sombras e hoquetus — sem que o som de uma vaze no canal da outra. "
            "Cada matéria cumpre uma função — pulso, trajetória, linha, campo, ruptura, chamada, memória ou vestígio. Use presença, mix, mute, solo e controles locais para revelar ou recolher essas funções. MASTER define somente o nível final.\n\n"
            "7. O CÉU · TRAMA, FORMA, TEMPO E MOVIMENTO\n"
            "TRAMA liga as relações do céu: a cada compasso o céu escolhe pares em foco (resposta, contágio, sombra, hoquetus, pedal, atrito, cadência) "
            "e a peça percorre uma forma — ABERTURA, DESENVOLVIMENTO, CONTRASTE, RETORNO — mostrada no cabeçalho. O tempo respira (TEMPO 1,04×), "
            "o swing vem de uma órbita (SWING 58%), cada estrela conta o seu ciclo e o compasso se agrupa em 3+3+2 e afins. No desenvolvimento uma "
            "estrela DERIVA contra o pulso; luas calculadas trazem ECLIPSES. Na Constelação as estrelas se movem por física — gravidade, molas, "
            "pêndulos, bilhar, magnetismo, efeito, espuma — e deixam rastros; choques fortes soam. Cada estrela soa no estéreo onde está na Constelação (o grave fica no centro); ESPAÇO abre esse campo. VARIAÇÃO deixa o céu acender e apagar estrelas "
            "no mixer: o fader anda sozinho (cursor laranja) e volta ao seu valor quando você o pega. Com a TRAMA desligada, o instrumento soa exatamente como antes.\n\n"
            "8. A BIBLIOTECA · TROCAR E FIXAR ESTRELAS\n"
            "A seed escolhe as estrelas entre todas as 28 do céu (no máximo uma de CONTROLE por céu); a BIBLIOTECA as reúne por família. Clique numa estrela da Constelação e depois no botão "
            "BIBLIOTECA (▾; sem estrela em foco, ele pergunta qual corpo): o menu mostra a estrela do corpo, FIXAR ESTA ESTRELA e as famílias — TOM, TEXTURA, VOZ, PULSO e CONTROLE. Escolha "
            "outra estrela e o corpo passa a tocá-la; VOLTAR À ESTRELA DA SEED desfaz. Ou arraste da faixa da BIBLIOTECA e solte sobre uma estrela: o mesmo menu abre para ela, também com os 8 lugares cheios. Uma estrela fixada (losango âmbar) não é apagada pela "
            "VARIAÇÃO nem pelos eclipses. As estrelas de CONTROLE não soam: acesas, filtram os ataques das outras — SIEVE (a peneira de "
            "Xenakis), POLYMETER (cada estrela no seu ciclo), HOCKET (um ataque por vez) e RUPTURE (o céu se cala de repente). O MODAL fica "
            "num corpo só.\n\n"
            "9. GRAVAR UMA REALIZAÇÃO\n"
            "PLAY só toca. REC começa a gravar uma tomada WAV estéreo de 24 bits (e começa a tocar, se estiver parado); gravando, a caixa de transporte e a moldura ficam vermelhas, com o tempo da tomada, e o botão vira CODA: ele pede a coda e fecha o arquivo após a cauda — em âmbar, CODA · FINALIZANDO. O caminho do arquivo aparece na linha de status. FAIXA · DURAÇÃO (1, 2, 3 ou 5 MIN): um clique já grava, com contagem regressiva; a forma do céu se encaixa na duração — abertura, desenvolvimento, contraste, retorno — com o PERCURSO trocando as cenas, e a coda chega no fim. "
            "Gravando, a peça também termina sozinha quando você deixa o instrumento seguir sem tocar em nada por um tempo (cerca de 100 a 190 tempos), depois de a seed percorrer algumas viradas; o cabeçalho avisa CODA EM BREVE. "
            "Depois da coda só a gravação fecha: o som continua e uma nova peça abre. STOP é uma interrupção técnica, não a coda. A performance também pode alcançar um epílogo após inatividade.\n\n"
            "REGRA CENTRAL\n"
            "Todo controle e toda figura visível devem produzir uma consequência musical clara. Se a escuta não revelar função, o elemento "
            "deve ser revisto, redesenhado ou retirado.") : tutorialSummary(language), false);
        text_.setReadOnly(true);
        text_.setMultiLine(true, true);
        text_.setScrollbarsShown(true);
        text_.setFont(interfaceFont(14.0f));
        text_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff102d27));
        text_.setColour(juce::TextEditor::textColourId, juce::Colour(0xffe7e3d5));
        text_.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff315e51));
        addAndMakeVisible(text_);
        close_.setButtonText(closeLabel(language_));
        close_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff315e51));
        close_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
        close_.onClick = [this] {
            if (auto* dialog = findParentComponentOfClass<juce::DialogWindow>())
                dialog->exitModalState(0);
        };
        addAndMakeVisible(close_);
    }

    void paint(juce::Graphics& graphics) override {
        graphics.fillAll(juce::Colour(0xff102d27));
        graphics.setColour(juce::Colour(0xffd18a45));
        graphics.setFont(interfaceFont(18.0f, juce::Font::bold));
        graphics.drawText(tutorialTitle(language_), getLocalBounds().reduced(24, 16).removeFromTop(30),
                          juce::Justification::centredLeft);
        graphics.setColour(juce::Colour(0xff8aa99b));
        graphics.setFont(interfaceFont(12.5f));
        graphics.drawText(tutorialSubtitle(language_),
                          getLocalBounds().reduced(24, 48).removeFromTop(20),
                          juce::Justification::centredLeft);
    }

    void resized() override {
        auto area = getLocalBounds().reduced(24, 18);
        area.removeFromTop(64);
        auto footer = area.removeFromBottom(38);
        close_.setBounds(footer.withSizeKeepingCentre(112, 28));
        text_.setBounds(area.withTrimmedBottom(12));
    }

private:
    SupportLanguage language_;

    static juce::String tutorialTitle(SupportLanguage language) {
        switch (language) {
        case SupportLanguage::English: return "GUIDE";
        case SupportLanguage::French: return utf8("TUTORIEL");
        case SupportLanguage::Spanish: return "TUTORIAL";
        default: return "TUTORIAL";
        }
    }

    static juce::String tutorialSubtitle(SupportLanguage language) {
        switch (language) {
        case SupportLanguage::English: return "guidance for listening, composing and recording";
        case SupportLanguage::French: return utf8("repères pour écouter, composer et enregistrer");
        case SupportLanguage::Spanish: return utf8("orientación para escuchar, componer y grabar");
        default: return utf8("orientação para escutar, compor e gravar");
        }
    }

    static juce::String closeLabel(SupportLanguage language) {
        switch (language) {
        case SupportLanguage::English: return "CLOSE";
        case SupportLanguage::French: return "FERMER";
        case SupportLanguage::Spanish: return "CERRAR";
        default: return "FECHAR";
        }
    }

    static juce::String tutorialSummary(SupportLanguage language) {
        switch (language) {
        case SupportLanguage::English:
            return utf8("1. START · SEED AND TERRITORY\nA seed opens a musical possibility: modules, roles, material, tempo and a narrative. Type a number or use ALEA. It also defines initial module values; changing it must not erase the drawing already made in the Field.\n\n"
                        "2. COMPOSE IN THE FIELD\nDrag module circles to establish positions, neighbourhoods and connections. The performer’s drawing is spatial memory and changes only by intentional action.\n\n"
                        "3. READ THE CONSTELLATION\nThe Constellation is orientation, not a preset browser. Distance, connection, movement and depth must produce musical change.\n\n"
                        "4. AXIS OF FORCES\nThe orange body controls AUTONOMY, INTERVENTION, MEMORY and RISK without inverse pairs: both may be zero. The blue body controls DENSITY, TENSION, SPACE and DEPTH. The two bodies have four independent macros each. Every movement must be audible. The round knobs beside it are the same eight values: the axis is the gesture, the knobs the fine adjustment.\n\n"
                        "5. SCENES AND GESTURES\nNASCENTE, ESPIRAL, RASGO and BALANÇO transform the material without interrupting a performance already in PLAY. CAPTURAR stores a scene; DESFAZER returns to the previous capture. HOLD, OPEN, TEAR, REPAIR, STUTTER and DISPLACE (shifts the stem's pulse and phase; the tempo stays) act on existing material.\n\n"
                        "6. LISTEN AND ORCHESTRATE\nEach module is a star with its own stem: up to eight bodies in the Field, each on its own channel in the Matter Field. A star at zero presence is unlit and uses no processing; raising its presence lights it. Lit stars relate through the sky’s web — answers, contagion, shadows and hockets — without one star’s sound leaking into another’s channel. Each material serves a role — pulse, trajectory, line, field, rupture, call, memory or trace. Use presence, mix, mute, solo and local controls to reveal or withdraw those roles. MASTER sets final level only.\n\n"
                        "7. THE SKY · WEB, FORM, TIME AND MOTION\nTRAMA switches on the sky's web: every bar the sky chooses pairs in focus (answer, contagion, shadow, hocket, pedal, friction, cadence) and the piece travels a form — OPENING, DEVELOPMENT, CONTRAST, RETURN — shown in the header. Time breathes (TEMPO 1.04×), swing comes from an orbit, each star counts its own cycle and the bar groups as 3+3+2 and the like. In development one star DRIFTS against the pulse; computed moons bring ECLIPSES. In the Constellation the stars move by physics — gravity, springs, pendulums, billiards, magnetism, spin, foam — and leave trails; strong collisions sound. Each star sounds in the stereo field where it is in the Constellation (the bass stays centred); ESPAÇO opens that field. VARIAÇÃO lets the sky light and dim stars in the mixer: the fader moves by itself (orange thumb) and returns to your value when you grab it. With TRAMA off the instrument sounds exactly as before.\n\n"
                        "8. THE LIBRARY · SWAP AND PIN STARS\nThe seed chooses the stars among all 28 of the sky (at most one CONTROL star per sky); the LIBRARY groups them by family. Click a star in the Constellation, then the LIBRARY button (▾; with no star in focus, it asks which body): the menu shows the body's star, PIN THIS STAR and the families — TONE, TEXTURE, VOICE, PULSE and CONTROL. Choose another star and the body plays it; BACK TO THE SEED'S STAR undoes it. Or drag from the LIBRARY strip and drop onto a star: the same menu opens for it, even with all 8 places full. A pinned star (amber diamond) is not dimmed by VARIATION or by eclipses. CONTROL stars make no sound: lit, they filter the others' attacks — SIEVE (Xenakis's sieve), POLYMETER (each star in its own cycle), HOCKET (one attack at a time) and RUPTURE (the sky falls silent). The MODAL lives in one body only.\n\n"
                        "9. RECORD A REALISATION\nPLAY only plays. REC starts recording a 24-bit stereo WAV take (and starts playing if stopped); while recording, the transport box and the window frame turn red with the take time, and the button becomes CODA: it requests the coda and closes the file after its tail — amber, CODA · FINISHING. The file path appears in the status line. TRACK · DURATION (1, 2, 3 or 5 MIN): one click records at once, with a countdown; the sky's form fits the duration — opening, development, contrast, return — with the PATH switching scenes, and the coda arrives at the end. While recording, the piece also ends by itself when you let the instrument run untouched for a while (about 100 to 190 beats) after the seed has gone through a few turns; the header warns CODA SOON. After the coda only the recording closes: the sound goes on and a new piece opens. STOP is a technical interruption, not a coda.\n\n"
                        "CENTRAL RULE\nEvery visible form and control must have a clear musical consequence.");
        case SupportLanguage::French:
            return utf8("1. COMMENCER · SEED ET TERRITOIRE\nUne seed ouvre une possibilité musicale : modules, rôles, matière, tempo et narration. Entrez un nombre ou utilisez ALEA. Elle définit aussi les valeurs initiales des modules sans effacer le dessin déjà réalisé dans le Champ.\n\n"
                        "2. COMPOSER DANS LE CHAMP\nGlissez les cercles des modules pour établir positions, voisinages et connexions. Le dessin du performeur est une mémoire spatiale et ne change que par une action intentionnelle.\n\n"
                        "3. LIRE LA CONSTELLATION\nLa Constellation est une orientation, pas un navigateur de presets. Distance, connexion, mouvement et profondeur doivent transformer la musique.\n\n"
                        "4. AXE DES FORCES\nLe corps orange contrôle AUTONOMIE, INTERVENTION, MÉMOIRE et RISQUE sans paire inverse : les deux peuvent être à zéro. Le bleu contrôle DENSITÉ, TENSION, ESPACE et PROFONDEUR. Les deux corps ont chacun quatre macros indépendantes. Tout mouvement doit être audible. Les boutons ronds à côté sont les mêmes huit valeurs : l’axe est le geste, les boutons le réglage fin.\n\n"
                        "5. SCÈNES ET GESTES\nNASCENTE, ESPIRAL, RASGO et BALANÇO transforment la matière sans interrompre une performance déjà en PLAY. CAPTURAR mémorise une scène ; DESFAZER revient à la capture précédente. HOLD, OUVRIR, DÉCHIRER, RÉPARER, STUTTER et DÉCALER (décale la pulsation et la phase du stem ; le tempo ne change pas) agissent sur la matière existante.\n\n"
                        "6. ÉCOUTER ET ORCHESTRER\nChaque module est une étoile avec son propre stem : jusqu’à huit corps dans le Champ, chacun sur son canal du Champ de matières. Une étoile à présence nulle est éteinte et ne consomme pas de calcul ; monter sa présence l’allume. Les étoiles allumées se relient par la trame du ciel — réponses, contagions, ombres et hoquets — sans que le son de l’une fuie dans le canal de l’autre. Chaque matière a une fonction — pulsation, trajectoire, ligne, champ, rupture, appel, mémoire ou trace. Utilisez présence, mix, mute, solo et contrôles locaux pour révéler ou retirer ces fonctions. MASTER ne règle que le niveau final.\n\n"
                        "7. LE CIEL · TRAME, FORME, TEMPS ET MOUVEMENT\nTRAMA allume la trame du ciel : à chaque mesure le ciel choisit des paires en focus (réponse, contagion, ombre, hoquet, pédale, friction, cadence) et la pièce parcourt une forme — OUVERTURE, DÉVELOPPEMENT, CONTRASTE, RETOUR — affichée dans l’en-tête. Le temps respire (TEMPO 1,04×), le swing vient d’une orbite, chaque étoile compte son propre cycle et la mesure se groupe en 3+3+2 et ainsi de suite. Dans le développement une étoile DÉRIVE contre la pulsation ; des lunes calculées amènent des ÉCLIPSES. Dans la Constellation les étoiles bougent par la physique — gravité, ressorts, pendules, billard, magnétisme, effet, mousse — et laissent des traînées ; les chocs forts sonnent. Chaque étoile sonne dans la stéréo là où elle est dans la Constellation (le grave reste au centre) ; ESPAÇO ouvre ce champ. VARIAÇÃO laisse le ciel allumer et éteindre des étoiles dans le mixeur : le fader bouge seul (curseur orange) et revient à votre valeur quand vous le prenez. TRAMA éteinte, l’instrument sonne exactement comme avant.\n\n"
                        "8. LA BIBLIOTHÈQUE · CHANGER ET ÉPINGLER DES ÉTOILES\nLa seed choisit les étoiles parmi les 28 du ciel (au plus une de CONTRÔLE par ciel) ; la BIBLIOTHÈQUE les range par famille. Cliquez sur une étoile de la Constellation puis sur le bouton BIBLIOTHÈQUE (▾ ; sans étoile au foyer, il demande quel corps) : le menu montre l’étoile du corps, ÉPINGLER CETTE ÉTOILE et les familles — TON, TEXTURE, VOIX, PULSATION et CONTRÔLE. Choisissez une autre étoile et le corps la joue ; RETOUR À L’ÉTOILE DE LA SEED annule. Ou glissez depuis la bande de la BIBLIOTHÈQUE et déposez sur une étoile : le même menu s’ouvre pour elle, même avec les 8 places occupées. Une étoile épinglée (losange ambre) n’est pas éteinte par la VARIATION ni par les éclipses. Les étoiles de CONTRÔLE ne sonnent pas : allumées, elles filtrent les attaques des autres — SIEVE (le crible de Xenakis), POLYMETER (chaque étoile dans son cycle), HOCKET (une attaque à la fois) et RUPTURE (le ciel se tait d’un coup). Le MODAL vit dans un seul corps.\n\n"
                        "9. ENREGISTRER\nPLAY joue seulement. REC lance l’enregistrement d’une prise WAV stéréo 24 bits (et lance la lecture si elle est arrêtée) ; en enregistrement, la boîte de transport et le cadre deviennent rouges avec le temps de la prise, et le bouton devient CODA : il demande la coda et ferme le fichier après sa queue — en ambre, CODA · FIN EN COURS. Le chemin du fichier apparaît dans la ligne d’état. PISTE · DURÉE (1, 2, 3 ou 5 MIN) : un clic enregistre aussitôt, avec un compte à rebours ; la forme du ciel s’ajuste à la durée — ouverture, développement, contraste, retour — avec le PARCOURS qui change les scènes, et la coda arrive à la fin. En enregistrement, la pièce se termine aussi d’elle-même quand vous laissez l’instrument jouer sans le toucher pendant un moment (environ 100 à 190 temps), après quelques tours de la seed ; l’en-tête annonce CODA PROCHE. Après la coda, seul l’enregistrement se ferme : le son continue et une nouvelle pièce s’ouvre. STOP est une interruption technique, pas une coda.\n\n"
                        "RÈGLE CENTRALE\nToute forme et tout contrôle visibles doivent avoir une conséquence musicale claire.");
        case SupportLanguage::Spanish:
            return utf8("1. COMENZAR · SEED Y TERRITORIO\nUna seed abre una posibilidad musical: módulos, funciones, materia, tempo y narrativa. Escriba un número o use ALEA. También define valores iniciales de los módulos sin borrar el dibujo ya realizado en el Campo.\n\n"
                        "2. COMPONER EN EL CAMPO\nArrastre los círculos de módulos para establecer posiciones, vecindades y conexiones. El dibujo del intérprete es memoria espacial y solo cambia mediante una acción intencional.\n\n"
                        "3. LEER LA CONSTELACIÓN\nLa Constelación es orientación, no un navegador de presets. Distancia, conexión, movimiento y profundidad deben producir cambio musical.\n\n"
                        "4. EJE DE FUERZAS\nEl cuerpo naranja controla AUTONOMÍA, INTERVENCIÓN, MEMORIA y RIESGO sin pares inversos: ambos pueden llegar a cero. El azul controla DENSIDAD, TENSIÓN, ESPACIO y PROFUNDIDAD. Los dos cuerpos tienen cuatro macros independientes cada uno. Todo movimiento debe ser audible. Las perillas redondas al lado son los mismos ocho valores: el eje es el gesto, las perillas el ajuste fino.\n\n"
                        "5. ESCENAS Y GESTOS\nNASCENTE, ESPIRAL, RASGO y BALANÇO transforman el material sin interrumpir una performance ya en PLAY. CAPTURAR guarda una escena y DESFAZER vuelve a la captura anterior. HOLD, ABRIR, RASGAR, REPARAR, STUTTER y DESPLAZAR (desplaza el pulso y la fase del stem; el tempo no cambia) actúan sobre el material existente.\n\n"
                        "6. ESCUCHAR Y ORQUESTAR\nCada módulo es una estrella con stem propio: hasta ocho cuerpos en el Campo, cada uno en su canal del Campo de materias. Una estrella con presencia cero queda apagada y no consume procesamiento; subir la presencia la enciende. Las estrellas encendidas se relacionan por la trama del cielo — respuestas, contagios, sombras y hoquetus — sin que el sonido de una se filtre en el canal de otra. Cada materia cumple una función — pulso, trayectoria, línea, campo, ruptura, llamada, memoria o vestigio. Use presencia, mix, mute, solo y controles locales para revelar o retirar esas funciones. MASTER define solo el nivel final.\n\n"
                        "7. EL CIELO · TRAMA, FORMA, TIEMPO Y MOVIMIENTO\nTRAMA enciende la trama del cielo: en cada compás el cielo elige pares en foco (respuesta, contagio, sombra, hoquetus, pedal, fricción, cadencia) y la pieza recorre una forma — APERTURA, DESARROLLO, CONTRASTE, RETORNO — mostrada en la cabecera. El tiempo respira (TEMPO 1,04×), el swing viene de una órbita, cada estrella cuenta su propio ciclo y el compás se agrupa en 3+3+2 y similares. En el desarrollo una estrella DERIVA contra el pulso; lunas calculadas traen ECLIPSES. En la Constelación las estrellas se mueven por física — gravedad, resortes, péndulos, billar, magnetismo, efecto, espuma — y dejan estelas; los choques fuertes suenan. Cada estrella suena en el estéreo donde está en la Constelación (el grave queda en el centro); ESPAÇO abre ese campo. VARIAÇÃO deja que el cielo encienda y apague estrellas en el mezclador: el fader se mueve solo (cursor naranja) y vuelve a su valor cuando usted lo toma. Con la TRAMA apagada, el instrumento suena exactamente como antes.\n\n"
                        "8. LA BIBLIOTECA · CAMBIAR Y FIJAR ESTRELLAS\nLa seed elige las estrellas entre las 28 del cielo (como máximo una de CONTROL por cielo); la BIBLIOTECA las reúne por familia. Haga clic en una estrella de la Constelación y luego en el botón BIBLIOTECA (▾; sin estrella en foco, pregunta qué cuerpo): el menú muestra la estrella del cuerpo, FIJAR ESTA ESTRELLA y las familias — TONO, TEXTURA, VOZ, PULSO y CONTROL. Elija otra estrella y el cuerpo la toca; VOLVER A LA ESTRELLA DE LA SEED lo deshace. O arrastre desde la franja de la BIBLIOTECA y suelte sobre una estrella: el mismo menú se abre para ella, también con los 8 lugares llenos. Una estrella fijada (rombo ámbar) no se apaga con la VARIACIÓN ni con los eclipses. Las estrellas de CONTROL no suenan: encendidas, filtran los ataques de las otras — SIEVE (la criba de Xenakis), POLYMETER (cada estrella en su ciclo), HOCKET (un ataque a la vez) y RUPTURE (el cielo calla de golpe). El MODAL vive en un solo cuerpo.\n\n"
                        "9. GRABAR UNA REALIZACIÓN\nPLAY solo toca. REC empieza a grabar una toma WAV estéreo de 24 bits (y empieza a tocar si está detenido); grabando, la caja de transporte y el marco se ponen rojos con el tiempo de la toma, y el botón pasa a CODA: solicita la coda y cierra el archivo tras la cola — en ámbar, CODA · FINALIZANDO. La ruta del archivo aparece en la línea de estado. PISTA · DURACIÓN (1, 2, 3 o 5 MIN): un clic graba enseguida, con cuenta regresiva; la forma del cielo se ajusta a la duración — apertura, desarrollo, contraste, retorno — con el RECORRIDO cambiando las escenas, y la coda llega al final. Grabando, la pieza también termina sola cuando deja que el instrumento siga sin tocar nada durante un tiempo (unos 100 a 190 tiempos), tras algunas vueltas de la seed; la cabecera avisa CODA PRÓXIMA. Después de la coda solo se cierra la grabación: el sonido sigue y se abre una nueva pieza. STOP es una interrupción técnica, no una coda.\n\n"
                        "REGLA CENTRAL\nToda forma y control visible debe tener una consecuencia musical clara.");
        default: return {};
        }
    }

    juce::TextEditor text_;
    juce::TextButton close_;
};
}

const char* narrativeMoveLabel(std::uint8_t move) {
    switch (move) {
    case 0U: return "SUSTENTA";
    case 1U: return "APROXIMA";
    case 2U: return "RESPONDE";
    case 3U: return "DISPUTA";
    case 4U: return "RETÉM";
    case 5U: return "RASGA";
    case 6U: return "TROCA";
    default: return "EM CURSO";
    }
}

const char* narrativeStrategyLabel(std::uint8_t strategy) {
    switch (strategy) {
    case 0U: return "SEGUE";
    case 1U: return "TRANSFORMA";
    case 2U: return "CONTRAPONTO";
    case 3U: return "INTERROMPE";
    case 4U: return "OCULTA";
    case 5U: return "NEGOCIA";
    default: return "SEED";
    }
}

juce::String narrativeMoveText(SupportLanguage language, std::uint8_t move) {
    static constexpr std::array<const char*, 7> pt{"SUSTENTA", "APROXIMA", "RESPONDE", "DISPUTA", "RETÉM", "RASGA", "TROCA"};
    static constexpr std::array<const char*, 7> en{"SUSTAINS", "DRAWS NEAR", "RESPONDS", "CONTESTS", "RETAINS", "TEARS", "EXCHANGES"};
    static constexpr std::array<const char*, 7> fr{"SOUTIENT", "RAPPROCHE", "RÉPOND", "CONTESTE", "RETIENT", "DÉCHIRE", "ÉCHANGE"};
    static constexpr std::array<const char*, 7> es{"SOSTIENE", "ACERCA", "RESPONDE", "DISPUTA", "RETIENE", "RASGA", "INTERCAMBIA"};
    const auto index = std::min<std::size_t>(move, pt.size() - 1U);
    return uiText(language, pt[index], en[index], fr[index], es[index]);
}

juce::String narrativeStrategyText(SupportLanguage language, std::uint8_t strategy) {
    static constexpr std::array<const char*, 7> pt{"SEGUE", "TRANSFORMA", "CONTRAPONTO", "INTERROMPE", "OCULTA", "NEGOCIA", "SEED"};
    static constexpr std::array<const char*, 7> en{"FOLLOWS", "TRANSFORMS", "COUNTERPOINT", "INTERRUPTS", "HIDES", "NEGOTIATES", "SEED"};
    static constexpr std::array<const char*, 7> fr{"SUIT", "TRANSFORME", "CONTREPOINT", "INTERROMPT", "CACHE", "NÉGOCIE", "SEED"};
    static constexpr std::array<const char*, 7> es{"SIGUE", "TRANSFORMA", "CONTRAPUNTO", "INTERRUMPE", "OCULTA", "NEGOCIA", "SEED"};
    const auto index = std::min<std::size_t>(strategy, pt.size() - 1U);
    return uiText(language, pt[index], en[index], fr[index], es[index]);
}

juce::String localizedRelationalPhenomenon(SupportLanguage language,
                                           PerformanceRelationalPhenomenon phenomenon) {
    static constexpr std::array<const char*, 23> pt{"ATRAIR", "REPELIR", "ORBITAR", "ÓRBITA ELÍPTICA", "DANÇA BARICÊNTRICA", "ENCONTRO", "FUSIONAR", "SOBREVOAR", "ESPALHAR", "PERSEGUIR", "FUGIR", "PARALELO", "CONVERGIR", "DIVERGIR", "OSCILAR", "ESPIRAL >", "ESPIRAL <", "EQUILÍBRIO", "TRAVAR", "SINCRONIZAR", "RESSOAR", "TROCAR", "LIBERAR"};
    static constexpr std::array<const char*, 23> en{"ATTRACT", "REPEL", "ORBIT", "ELLIPTICAL ORBIT", "BARYCENTRIC DANCE", "ENCOUNTER", "MERGE", "FLYBY", "SCATTER", "CHASE", "ESCAPE", "PARALLEL", "CONVERGE", "DIVERGE", "OSCILLATE", "SPIRAL IN", "SPIRAL OUT", "EQUILIBRIUM", "LOCK", "SYNCHRONIZE", "RESONATE", "SWAP", "RELEASE"};
    static constexpr std::array<const char*, 23> fr{"ATTIRER", "REPOUSSER", "ORBITER", "ORBITE ELLIPTIQUE", "DANSE BARYCENTRIQUE", "RENCONTRE", "FUSIONNER", "SURVOL", "DISPERSER", "POURSUIVRE", "FUIR", "PARALLÈLE", "CONVERGER", "DIVERGER", "OSCILLER", "SPIRALE ENTRANTE", "SPIRALE SORTANTE", "ÉQUILIBRE", "VERROUILLER", "SYNCHRONISER", "RÉSONNER", "ÉCHANGER", "LIBÉRER"};
    static constexpr std::array<const char*, 23> es{"ATRAER", "REPELER", "ORBITAR", "ÓRBITA ELÍPTICA", "DANZA BARICÉNTRICA", "ENCUENTRO", "FUSIONAR", "SOBREVUELO", "DISPERSAR", "PERSEGUIR", "ESCAPAR", "PARALELO", "CONVERGER", "DIVERGIR", "OSCILAR", "ESPIRAL INTERIOR", "ESPIRAL EXTERIOR", "EQUILIBRIO", "BLOQUEAR", "SINCRONIZAR", "RESONAR", "INTERCAMBIAR", "LIBERAR"};
    const auto index = static_cast<std::size_t>(phenomenon);
    if (index >= pt.size()) return uiText(language, "RELACIONAR", "RELATE", "RELATIONNER", "RELACIONAR");
    return uiText(language, pt[index], en[index], fr[index], es[index]);
}

juce::String localizedMotionField(SupportLanguage language, PerformanceMotionField field) {
    switch (field) {
    case PerformanceMotionField::Pulse: return uiText(language, "PULSO", "PULSE", "POULS", "PULSO");
    case PerformanceMotionField::FigureEight: return uiText(language, "FIGURA-8", "FIGURE-8", "FIGURE-8", "FIGURA-8");
    case PerformanceMotionField::Orbit:
    default: return uiText(language, "ÓRBITA", "ORBIT", "ORBITE", "ÓRBITA");
    }
}

juce::String trajectoryLabel(SupportLanguage language, const PerformanceTelemetrySnapshot& telemetry) {
    if (telemetry.panicSafe) return uiText(language, "TRAJETÓRIA · SAFE", "TRAJECTORY · SAFE", "TRAJECTOIRE · SAFE", "TRAYECTORIA · SAFE");
    if (!telemetry.playing) return uiText(language, "TRAJETÓRIA · PARADA", "TRAJECTORY · STOPPED", "TRAJECTOIRE · ARRÊTÉE", "TRAYECTORIA · DETENIDA");
    if (telemetry.narrativeEventValid) {
        return utf8("SEED ") + juce::String(telemetry.narrativeTurn + 1U)
            + utf8(" · ") + narrativeMoveText(language, telemetry.narrativeMove)
            + utf8(" · ") + narrativeStrategyText(language, telemetry.narrativeStrategy);
    }
    if (telemetry.wildMicros > 5000U)
        return uiText(language, "TRAJETÓRIA · SELVAGEM ", "TRAJECTORY · WILD ", "TRAJECTOIRE · SAUVAGE ", "TRAYECTORIA · SALVAJE ")
            + juce::String(static_cast<double>(telemetry.wildMicros) / 1000000.0, 2);
    if (telemetry.repairMicros > 5000U)
        return uiText(language, "TRAJETÓRIA · REPARO ", "TRAJECTORY · REPAIR ", "TRAJECTOIRE · RÉPARATION ", "TRAYECTORIA · REPARACIÓN ")
            + juce::String(static_cast<double>(telemetry.repairMicros) / 1000000.0, 2);
    if (telemetry.scarMicros > 5000U)
        return uiText(language, "TRAJETÓRIA · CICATRIZ ", "TRAJECTORY · SCAR ", "TRAJECTOIRE · CICATRICE ", "TRAYECTORIA · CICATRIZ ")
            + juce::String(static_cast<double>(telemetry.scarMicros) / 1000000.0, 2);
    if (telemetry.holding) return uiText(language, "TRAJETÓRIA · SUSPENSA", "TRAJECTORY · HELD", "TRAJECTOIRE · SUSPENDUE", "TRAYECTORIA · SUSPENDIDA");
    return uiText(language, "TRAJETÓRIA · SEED EM CURSO", "TRAJECTORY · SEED IN PROGRESS", "TRAJECTOIRE · SEED EN COURS", "TRAYECTORIA · SEED EN CURSO");
}

PerformanceSessionDocument makeStartupDocument() {
    // Each launch begins with a fresh aleatory territory. The seed remains
    // visible and editable, so the performer can still make the opening
    // reproducible after the instrument has started.
    const auto seed = static_cast<std::uint32_t>(
        juce::Random::getSystemRandom().nextInt());
    return PerformanceTerritoryFactory::makeForReleasedStems(
        seed, kPerformanceSafeBodyLimit, true);
}

std::array<std::array<double, 3>, 8> initialModuleControls(std::uint32_t seed) {
    std::array<std::array<double, 3>, 8> controls{};
    for (std::size_t index = 0U; index < controls.size(); ++index)
        controls[index] = PerformanceTerritoryFactory::initialModuleControls(
            seed, static_cast<std::uint16_t>(index + 1U));
    return controls;
}

// O trilho ORIGEM ⇄ VARIAÇÃO (3/11, proposta A1 + A2 aprovada pelo autor):
// o trecho de 0 até o valor em âmbar — a cor da VARIAÇÃO no mixer —, o resto
// num verde médio com contorno fino, na espessura dos outros sliders; três
// marcas finas sob ele em 0 · 0,5 · 1. Antes: verde-escuro sobre verde-escuro, quase invisível.
class VariationTrackLookAndFeel final : public juce::LookAndFeel_V4 {
public:
    void drawLinearSlider(juce::Graphics& graphics, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle style, juce::Slider& slider) override {
        if (style != juce::Slider::LinearHorizontal) {
            juce::LookAndFeel_V4::drawLinearSlider(graphics, x, y, width, height, sliderPos,
                                                   minSliderPos, maxSliderPos, style, slider);
            return;
        }
        // As mesmas medidas dos outros sliders (LookAndFeel_V4): o destaque é
        // a cor, o contorno e as marcas, não o tamanho (3/11, o autor achou
        // o primeiro desenho grande demais).
        const float trackHeight = juce::jmin(6.0f, static_cast<float>(height) * 0.25f);
        const auto centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
        const auto left = static_cast<float>(x);
        const auto right = static_cast<float>(x + width);
        const juce::Rectangle<float> track(left, centreY - trackHeight * 0.5f, right - left, trackHeight);
        graphics.setColour(juce::Colour(0xff2a5a4d));
        const float corner = trackHeight * 0.5f;
        graphics.fillRoundedRectangle(track, corner);
        graphics.setColour(juce::Colour(0xff8aa99b).withAlpha(0.55f));
        graphics.drawRoundedRectangle(track, corner, 1.0f);
        graphics.setColour(juce::Colour(0xffd18a45));
        graphics.fillRoundedRectangle(track.withRight(juce::jmax(left + trackHeight, sliderPos)), corner);
        // A2: as marcas 0 · 0,5 · 1, finas, logo abaixo do trilho.
        graphics.setColour(juce::Colour(0xff8aa99b));
        for (const float fraction : {0.0f, 0.5f, 1.0f}) {
            const auto markX = left + (right - left) * fraction;
            graphics.drawVerticalLine(juce::roundToInt(markX), track.getBottom() + 2.0f, track.getBottom() + 5.0f);
        }
        // O JUCE desenha o botão com diâmetro = getSliderThumbRadius.
        const auto thumb = static_cast<float>(getSliderThumbRadius(slider));
        const auto knob = juce::Rectangle<float>(thumb, thumb).withCentre({sliderPos, centreY});
        graphics.setColour(slider.findColour(juce::Slider::thumbColourId));
        graphics.fillEllipse(knob);
        graphics.setColour(juce::Colour(0xfff2e7d5));
        graphics.drawEllipse(knob, 1.0f);
    }
};

class PerformanceField final : public juce::Component,
                               public juce::SettableTooltipClient {
public:
    enum class ChangeSource : std::uint8_t { Orange, Blue, Both };
    PerformanceField() {
        setWantsKeyboardFocus(true);
        setMouseClickGrabsKeyboardFocus(true);
    }

    void setLanguage(SupportLanguage language) noexcept {
        language_ = language;
        repaint();
    }

    // The two bodies are one four-axis field.  Sending their state together
    // prevents the old complementary-pair mapping from quietly overwriting a
    // macro set by the other body.
    std::function<void(double, double, double, double, ChangeSource)> onFieldChanged;

    void setPosition(double x, double y) {
        x_ = juce::jlimit(0.0, 1.0, x);
        y_ = juce::jlimit(0.0, 1.0, y);
        displayX_ = x_;
        displayY_ = y_;
        repaint();
    }

    void setTexturePosition(double x, double y) {
        textureX_ = juce::jlimit(0.0, 1.0, x);
        textureY_ = juce::jlimit(0.0, 1.0, y);
        displayTextureX_ = textureX_;
        displayTextureY_ = textureY_;
        repaint();
    }

    // A narrative turn can temporarily move the live bodies while the four
    // authored coordinates remain untouched. The next manual gesture takes
    // the bodies back immediately and becomes the new anchor.
    void setNarrativePosition(double x, double y, double textureX, double textureY) {
        displayX_ = juce::jlimit(0.0, 1.0, x);
        displayY_ = juce::jlimit(0.0, 1.0, y);
        displayTextureX_ = juce::jlimit(0.0, 1.0, textureX);
        displayTextureY_ = juce::jlimit(0.0, 1.0, textureY);
        repaint();
    }

    // O Eixo mostra o céu (3/11, proposta B1 aprovada pelo autor): com a
    // trama, um anel fino em volta de cada ponto — o raio respira com a
    // energia do movimento das estrelas (velocidade média na Constelação), a
    // cor vai do azul ao âmbar com a tensão harmônica. Sem texto novo.
    void setSkyState(const PerformanceTelemetrySnapshot& telemetry, bool web) {
        const bool visible = web && telemetry.playing && telemetry.skyMotionValid;
        const auto now = juce::Time::getMillisecondCounterHiRes();
        double speed = 0.0;
        if (visible && skyStateMs_ > 0.0 && now > skyStateMs_) {
            const double seconds = (now - skyStateMs_) / 1000.0;
            for (std::size_t index = 0U; index < lastSkyX_.size(); ++index) {
                const double dx = (telemetry.skyPositionX[index] - lastSkyX_[index]) / 65535.0;
                const double dy = (telemetry.skyPositionY[index] - lastSkyY_[index]) / 65535.0;
                speed += std::sqrt(dx * dx + dy * dy) / seconds;
            }
            speed /= static_cast<double>(lastSkyX_.size());
        }
        for (std::size_t index = 0U; index < lastSkyX_.size(); ++index) {
            lastSkyX_[index] = telemetry.skyPositionX[index];
            lastSkyY_[index] = telemetry.skyPositionY[index];
        }
        skyStateMs_ = now;
        // A física anda a até 0,9 campo/s; 0,4 já é um céu agitado.
        const double energyTarget = visible ? juce::jlimit(0.0, 1.0, speed / 0.4) : 0.0;
        skyEnergy_ += (energyTarget - skyEnergy_) * 0.15;
        skyTension_ = juce::jlimit(0.0, 1.0, telemetry.skyTensionMicros / 1000000.0);
        const bool changed = visible != skyRingVisible_ || visible;
        skyRingVisible_ = visible;
        if (changed) repaint();
    }

    void paint(juce::Graphics& graphics) override {
        const auto bounds = getLocalBounds().toFloat().reduced(10.0f);
        graphics.setColour(juce::Colour(0xff102d27));
        graphics.fillRoundedRectangle(bounds, 5.0f);
        graphics.setColour(juce::Colour(0xff315e51));
        for (int division = 1; division < 4; ++division) {
            const auto x = bounds.getX() + bounds.getWidth() * static_cast<float>(division) / 4.0f;
            const auto y = bounds.getY() + bounds.getHeight() * static_cast<float>(division) / 4.0f;
            graphics.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());
            graphics.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());
        }
        graphics.setColour(juce::Colour(0xff8aa99b));
        graphics.setFont(interfaceFont(12.0f));
        graphics.drawText(uiText(language_, "AUTONOMIA", "AUTONOMY", "AUTONOMIE", "AUTONOMÍA"),
                          bounds.withHeight(18.0f), juce::Justification::topLeft);
        graphics.drawText(uiText(language_, "INTERVENÇÃO", "INTERVENTION", "INTERVENTION", "INTERVENCIÓN"),
                          bounds.withHeight(18.0f), juce::Justification::topRight);
        graphics.drawText(uiText(language_, "MEMÓRIA", "MEMORY", "MÉMOIRE", "MEMORIA"),
                          bounds.withTrimmedTop(bounds.getHeight() - 18.0f),
                          juce::Justification::bottomLeft);
        graphics.drawText(uiText(language_, "RUPTURA", "RUPTURE", "RUPTURE", "RUPTURA"),
                          bounds.withTrimmedTop(bounds.getHeight() - 18.0f),
                          juce::Justification::bottomRight);
        const auto travelBounds = circleTravelBounds(bounds, 10.0f);
        const auto textureTravelBounds = circleTravelBounds(bounds, 11.0f);
        const auto point = juce::Point<float>(travelBounds.getX() + travelBounds.getWidth() * static_cast<float>(displayX_),
                                              travelBounds.getY() + travelBounds.getHeight() * static_cast<float>(displayY_));
        graphics.setColour(juce::Colour(0xffd18a45));
        graphics.fillEllipse(point.x - 7.0f, point.y - 7.0f, 14.0f, 14.0f);
        graphics.setColour(juce::Colour(0xfff2e7d5));
        graphics.drawEllipse(point.x - 10.0f, point.y - 10.0f, 20.0f, 20.0f, 1.5f);
        const auto texturePoint = juce::Point<float>(textureTravelBounds.getX() + textureTravelBounds.getWidth() * static_cast<float>(displayTextureX_),
                                                     textureTravelBounds.getY() + textureTravelBounds.getHeight() * static_cast<float>(displayTextureY_));
        graphics.setColour(juce::Colour(0xff5eb6d7));
        graphics.fillEllipse(texturePoint.x - 6.0f, texturePoint.y - 6.0f, 12.0f, 12.0f);
        graphics.drawEllipse(texturePoint.x - 11.0f, texturePoint.y - 11.0f, 22.0f, 22.0f, 1.4f);
        if (skyRingVisible_) {
            const auto ring = juce::Colour(0xff5eb6d7).interpolatedWith(juce::Colour(0xffd18a45),
                                                                         static_cast<float>(skyTension_));
            graphics.setColour(ring.withAlpha(0.75f));
            const auto radius = 15.0f + 14.0f * static_cast<float>(skyEnergy_);
            graphics.drawEllipse(point.x - radius, point.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);
            graphics.drawEllipse(texturePoint.x - radius, texturePoint.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);
        }
        if (hasKeyboardFocus(true)) {
            graphics.setColour(juce::Colour(0xffe1aa59));
            graphics.drawRoundedRectangle(bounds.expanded(3.0f), 7.0f, 1.5f);
        }
    }

    void mouseDown(const juce::MouseEvent& event) override {
        grabKeyboardFocus();
        const auto bounds = getLocalBounds().toFloat().reduced(10.0f);
        const auto textureTravelBounds = circleTravelBounds(bounds, 11.0f);
        const auto texturePoint = juce::Point<float>(textureTravelBounds.getX() + textureTravelBounds.getWidth() * static_cast<float>(displayTextureX_),
                                                     textureTravelBounds.getY() + textureTravelBounds.getHeight() * static_cast<float>(displayTextureY_));
        draggingTexture_ = event.position.getDistanceFrom(texturePoint) <= 20.0f;
        if (draggingTexture_) moveTextureTo(event.position);
        else moveTo(event.position);
    }
    void mouseDrag(const juce::MouseEvent& event) override {
        if (draggingTexture_) moveTextureTo(event.position);
        else moveTo(event.position);
    }

    bool keyPressed(const juce::KeyPress& key) override {
        const auto step = key.getModifiers().isShiftDown() ? 0.10 : 0.02;
        auto nextX = x_;
        auto nextY = y_;
        if (key == juce::KeyPress::leftKey) nextX -= step;
        else if (key == juce::KeyPress::rightKey) nextX += step;
        else if (key == juce::KeyPress::upKey) nextY -= step;
        else if (key == juce::KeyPress::downKey) nextY += step;
        else if (key == juce::KeyPress::homeKey) nextX = nextY = 0.5;
        else return false;
        setPosition(nextX, nextY);
        notifyFieldChanged(ChangeSource::Orange);
        return true;
    }

    void focusGained(juce::Component::FocusChangeType) override { repaint(); }
    void focusLost(juce::Component::FocusChangeType) override { repaint(); }

private:
    void moveTo(juce::Point<float> point) {
        const auto bounds = getLocalBounds().toFloat().reduced(10.0f);
        const auto travelBounds = circleTravelBounds(bounds, 10.0f);
        if (travelBounds.isEmpty()) return;
        x_ = juce::jlimit(0.0, 1.0, static_cast<double>((point.x - travelBounds.getX()) / travelBounds.getWidth()));
        y_ = juce::jlimit(0.0, 1.0, static_cast<double>((point.y - travelBounds.getY()) / travelBounds.getHeight()));
        displayX_ = x_;
        displayY_ = y_;
        notifyFieldChanged(ChangeSource::Orange);
        repaint();
    }

    void moveTextureTo(juce::Point<float> point) {
        const auto bounds = getLocalBounds().toFloat().reduced(10.0f);
        const auto travelBounds = circleTravelBounds(bounds, 11.0f);
        if (travelBounds.isEmpty()) return;
        textureX_ = juce::jlimit(0.0, 1.0, static_cast<double>((point.x - travelBounds.getX()) / travelBounds.getWidth()));
        textureY_ = juce::jlimit(0.0, 1.0, static_cast<double>((point.y - travelBounds.getY()) / travelBounds.getHeight()));
        displayTextureX_ = textureX_;
        displayTextureY_ = textureY_;
        notifyFieldChanged(ChangeSource::Blue);
        repaint();
    }

    static juce::Rectangle<float> circleTravelBounds(juce::Rectangle<float> bounds, float radius) {
        return bounds.reduced(radius);
    }

    void notifyFieldChanged(ChangeSource source) {
        if (onFieldChanged) onFieldChanged(x_, y_, textureX_, textureY_, source);
    }

    double x_ = 0.5;
    double y_ = 0.5;
    double textureX_ = 0.72;
    double textureY_ = 0.72;
    double displayX_ = 0.5;
    double displayY_ = 0.5;
    double displayTextureX_ = 0.72;
    double displayTextureY_ = 0.72;
    bool skyRingVisible_ = false;
    double skyEnergy_ = 0.0;
    double skyTension_ = 0.0;
    double skyStateMs_ = 0.0;
    std::array<std::uint16_t, kPerformanceStemActorCapacity> lastSkyX_{};
    std::array<std::uint16_t, kPerformanceStemActorCapacity> lastSkyY_{};
    bool draggingTexture_ = false;
    SupportLanguage language_ = SupportLanguage::Portuguese;
};

class ConstellationGraph final : public juce::Component,
                                 public juce::SettableTooltipClient {
public:
    using OrbitMode = PerformanceMotionField;

    ConstellationGraph() {
        setWantsKeyboardFocus(true);
        setMouseClickGrabsKeyboardFocus(true);
    }

    void setLanguage(SupportLanguage language) noexcept {
        language_ = language;
        repaint();
    }

    std::function<void(std::size_t, double)> onPresenceChanged;
    std::function<void(std::size_t, bool, double)> onActivationChanged;
    std::function<void()> onLibraryAdd;
    std::function<void(double, double)> onLibraryDrop;
    std::function<void(std::size_t, double, double)> onPositionChanged;
    std::function<void(std::size_t, double, bool)> onRelationChanged;
    std::function<void(std::size_t, PerformanceRelationalPhenomenon)> onRelationalPhenomenonChanged;
    std::function<void(PerformancePopulationPhenomenon, double)> onPopulationPhenomenonChanged;
    std::function<void(std::size_t, bool)> onSelectionChanged;
    std::function<void(std::size_t, OrbitMode)> onOrbitModeChanged;
    std::function<void(std::size_t, PerformanceMotionFamily)> onMotionFamilyChanged;
    std::function<void(PerformanceConstellationMotion)> onConstellationMotionChanged;
    std::function<void(std::size_t)> onStarSwapRequested;
    void setPinnedNodes(const std::array<bool, 8>& pinned) {
        pinnedNodes_ = pinned;
        repaint();
    }
    std::function<void(std::size_t, bool)> onOrbitDirectionChanged;

    void setModuleNames(const std::array<juce::String, 8>& names) {
        for (std::size_t index = 0U; index < names.size(); ++index) moduleNames_[index] = names[index];
        repaint();
    }

    void setOptionalModule(std::size_t index, const juce::String& name,
                           std::uint16_t actorId, double presence = 0.0) {
        if (index >= moduleNames_.size()) return;
        moduleNames_[index] = name;
        actorIds_[index] = actorId;
        presences_[index] = juce::jlimit(0.0, 1.0, presence);
        active_[index] = presences_[index] > 0.025;
        lifeStates_[index] = active_[index] ? ActorLifeState::Active : ActorLifeState::Dormant;
        repaint();
    }

    // O lugar desenhado de cada nó (fração da área do Campo).
    juce::Point<float> nodePosition(std::size_t nodeIndex) const noexcept {
        return nodeIndex < nodes_.size() ? nodes_[nodeIndex] : juce::Point<float>{0.5f, 0.5f};
    }
    std::uint16_t actorIdAt(std::size_t nodeIndex) const noexcept {
        return nodeIndex < actorIds_.size() ? actorIds_[nodeIndex] : kInvalidActorId;
    }

    std::array<juce::Point<float>, 4> spatialDrawing() const noexcept {
        return {{nodes_[0], nodes_[1], nodes_[2], nodes_[3]}};
    }

    std::array<double, 16> relationDrawing() const noexcept { return relations_; }

    PerformanceRelationalPhenomenon relationPhenomenonAt(std::size_t index) const noexcept {
        return index < relationalPhenomena_.size() ? relationalPhenomena_[index]
                                                   : PerformanceRelationalPhenomenon::Attract;
    }

    bool relationPhenomenonEnabledAt(std::size_t index) const noexcept {
        return index < relationalPhenomenaEnabled_.size() && relationalPhenomenaEnabled_[index];
    }

    PerformancePopulationPhenomenon populationPhenomenon() const noexcept {
        return populationPhenomenon_;
    }

    double populationPhenomenonIntensity() const noexcept {
        return populationPhenomenonIntensity_;
    }

    bool populationPhenomenonEnabled() const noexcept {
        return populationPhenomenonEnabled_;
    }

    void restorePopulationPhenomenon(PerformancePopulationPhenomenon phenomenon,
                                     double intensity, bool enabled) noexcept {
        if (!validPopulationPhenomenon(phenomenon)) return;
        populationPhenomenon_ = phenomenon;
        populationPhenomenonIntensity_ = juce::jlimit(0.0, 1.0, intensity);
        populationPhenomenonEnabled_ = enabled && populationPhenomenonIntensity_ > 0.0;
        repaint();
    }

    void restoreSpatialDrawing(const std::array<juce::Point<float>, 4>& drawing) {
        for (std::size_t index = 0U; index < drawing.size(); ++index) nodes_[index] = drawing[index];
        repaint();
    }

    void restoreRelationDrawing(const std::array<double, 16>& drawing) {
        relations_ = drawing;
        repaint();
    }

    void selectNode(std::size_t nodeIndex) {
        selectedNode_ = nodeIndex < nodes_.size() ? nodeIndex : kNoNode;
        selectedRelation_ = kNoNode;
        repaint();
    }

    void setRelationDetails(const std::array<juce::String, 6>& names,
                            const std::array<juce::String, 6>& effects) {
        for (std::size_t index = 0U; index < names.size(); ++index) {
            relationNames_[index] = names[index];
            relationEffects_[index] = effects[index];
        }
        repaint();
    }

    void setPresence(std::size_t actorIndex, double presence) {
        if (actorIndex >= presences_.size()) return;
        presences_[actorIndex] = juce::jlimit(0.0, 1.0, presence);
        active_[actorIndex] = presences_[actorIndex] > 0.025;
        lifeStates_[actorIndex] = active_[actorIndex] ? ActorLifeState::Active
                                                       : ActorLifeState::Dormant;
        repaint();
    }

    // The XY pad is a force field over the same live constellation. It never
    // changes the authored nodes, which remain the recallable drawing-base.
    void setFieldForce(double x, double y) noexcept {
        fieldForceX_ = juce::jlimit(-1.0, 1.0, x * 2.0 - 1.0);
        fieldForceY_ = juce::jlimit(-1.0, 1.0, y * 2.0 - 1.0);
        repaint();
    }

    void setRelation(std::size_t relationIndex, double intensity) {
        if (relationIndex >= relations_.size()) return;
        relations_[relationIndex] = juce::jlimit(0.0, 1.0, intensity);
        projectGeometryFromRelations();
        repaint();
    }

    void setRelations(const std::array<double, 16>& intensities) {
        for (std::size_t index = 0U; index < std::min(relations_.size(), intensities.size()); ++index)
            relations_[index] = juce::jlimit(0.0, 1.0, intensities[index]);
        // Scene/cell synchronisation changes relation values only. The field
        // is authored by the musician and must not be silently projected back
        // to default geometry. Geometry changes require an explicit node,
        // cable or relation-slider gesture.
        repaint();
    }

    void setRelationalPhenomenon(std::size_t relationIndex,
                                 PerformanceRelationalPhenomenon phenomenon,
                                 bool enabled = true) {
        if (relationIndex >= relationalPhenomena_.size()
            || !validRelationalPhenomenon(phenomenon)) return;
        relationalPhenomena_[relationIndex] = phenomenon;
        relationalPhenomenaEnabled_[relationIndex] = enabled;
        repaint();
    }

    juce::String skyActName(RelationKind kind) const {
        switch (kind) {
        case RelationKind::CallResponse: return uiText(language_, "RESPOSTA", "ANSWER", "RÉPONSE", "RESPUESTA");
        case RelationKind::Contagion: return uiText(language_, "CONTÁGIO", "CONTAGION", "CONTAGION", "CONTAGIO");
        case RelationKind::Shadow: return uiText(language_, "SOMBRA", "SHADOW", "OMBRE", "SOMBRA");
        case RelationKind::Hocket: return uiText(language_, "HOQUETUS", "HOCKET", "HOQUET", "HOQUETUS");
        case RelationKind::Support: return uiText(language_, "PEDAL", "PEDAL", "PÉDALE", "PEDAL");
        case RelationKind::Friction: return uiText(language_, "ATRITO", "FRICTION", "FRICTION", "FRICCIÓN");
        case RelationKind::Repair: return uiText(language_, "CADÊNCIA", "CADENCE", "CADENCE", "CADENCIA");
        default: return {};
        }
    }

    void setStemTelemetry(const PerformanceTelemetrySnapshot& telemetry) {
        skyFocusCount_ = std::min<std::size_t>(telemetry.skyFocusPacked & 3U, skyFoci_.size());
        for (std::size_t index = 0U; index < skyFocusCount_; ++index) {
            const auto entry = (telemetry.skyFocusPacked >> (2U + index * 10U)) & 0x3ffU;
            skyFoci_[index] = {static_cast<std::size_t>(entry & 7U), static_cast<std::size_t>((entry >> 3U) & 7U),
                               static_cast<RelationKind>(std::min<std::uint32_t>((entry >> 6U) & 15U,
                                   static_cast<std::uint32_t>(RelationKind::Count) - 1U))};
        }
        skyTension_ = juce::jlimit(0.0, 1.0, telemetry.skyTensionMicros / 1000000.0);
        for (std::size_t index = 0U; index < starBrightness_.size(); ++index)
            starBrightness_[index] = juce::jlimit(0.0, 1.0, telemetry.starBrightnessMicros[index] / 1000000.0);
        skyChorusVisible_ = telemetry.playing && telemetry.skyFocusPacked != 0U;
        for (std::size_t index = 0U; index < chorusPhase_.size(); ++index)
            chorusPhase_[index] = telemetry.skyChorusPhase[index] / 255.0;
        driftNode_ = telemetry.skyDriftSlot;
        eclipseNode_ = telemetry.skyEclipseSlot;
        skyMotion_ = telemetry.skyMotionValid;
        for (std::size_t index = 0U; index < skyPositions_.size(); ++index)
            skyPositions_[index] = {telemetry.skyPositionX[index] / 65535.0f, telemetry.skyPositionY[index] / 65535.0f};
        stemLevels_.fill(0.0);
        stemLayerMix_.fill(0.0);
        readiness_.fill(PerformanceStemReadiness::Unreserved);
        for (std::size_t stem = 0U; stem < telemetry.stemCount; ++stem) {
            for (std::size_t node = 0U; node < actorIds_.size(); ++node) {
                if (telemetry.stemActorIds[stem] != actorIds_[node]) continue;
                stemLevels_[node] = juce::jlimit(0.0, 1.0,
                    static_cast<double>(telemetry.stemPeakMicros[stem]) / 1000000.0);
                stemLayerMix_[node] = juce::jlimit(0.0, 1.0,
                    static_cast<double>(telemetry.stemLayerMixMicros[stem]) / 1000000.0);
                const auto life = telemetry.stemLifeStates[stem];
                lifeStates_[node] = life < static_cast<std::uint8_t>(ActorLifeState::Count)
                    ? static_cast<ActorLifeState>(life) : ActorLifeState::Dormant;
                active_[node] = presences_[node] > 0.025
                    && lifeStates_[node] != ActorLifeState::Dormant;
                const auto readiness = telemetry.stemReadiness[stem];
                readiness_[node] = readiness < static_cast<std::uint8_t>(PerformanceStemReadiness::Count)
                    ? static_cast<PerformanceStemReadiness>(readiness)
                    : PerformanceStemReadiness::Unreserved;
                const auto field = telemetry.stemMotionFields[stem];
                motionFields_[node] = field < static_cast<std::uint8_t>(OrbitMode::Count)
                    ? static_cast<OrbitMode>(field) : OrbitMode::Orbit;
                motionForwards_[node] = telemetry.stemMotionDirections[stem] != 0U;
            }
        }
        wildLevel_ = juce::jlimit(0.0, 1.0,
            static_cast<double>(telemetry.wildMicros) / 1000000.0);
        // This is control-side telemetry from the audio ecology. It makes the
        // body on screen the very body that is moving through the stems and
        // relations; the GUI never advances a competing orbit clock.
        motionPhase_ = juce::jlimit(0.0, 1.0,
            static_cast<double>(telemetry.motionPhaseMicros) / 1000000.0);
        const auto collective = static_cast<OrbitMode>(telemetry.motionField);
        if (validPerformanceMotionField(collective)) collectiveMotionField_ = collective;
        trajectoryText_ = trajectoryLabel(language_, telemetry);
        repaint();
    }

    // Visual orbit only: authored node positions remain the source of truth
    // for hit-testing, relations and recall.
    void setOrbitSeed(std::uint32_t seed) noexcept {
        orbitPhaseOffset_ = static_cast<double>(seed % 37U) * 0.17;
    }

    OrbitMode orbitMode() const noexcept {
        return selectedNode_ < motionFields_.size() ? motionFields_[selectedNode_] : collectiveMotionField_;
    }

    static const char* orbitModeLabel(OrbitMode mode) noexcept {
        switch (mode) {
        case OrbitMode::Pulse: return "PULSO";
        case OrbitMode::FigureEight: return "FIGURA-8";
        case OrbitMode::Orbit:
        default: return "ÓRBITA";
        }
    }

    // Famílias de movimento (30/10). Sem estrela em foco: o campo inteiro,
    // LIVRE → BILHAR → ESPUMA → PÊNDULOS → MAGNÉTICO → GRAVIDADE. Com uma
    // estrela em foco: CAMPO (segue o campo) e depois as seis.
    // A MEMÓRIA (2/11): ler e restaurar a DANÇA e o CÉU guardados; restaurar
    // avisa o motor pelos mesmos callbacks dos cliques.
    PerformanceMotionFamily collectiveFamily() const noexcept { return collectiveFamily_; }
    const std::array<PerformanceMotionFamily, 8>& bodyFamilies() const noexcept { return bodyFamilies_; }
    PerformanceMotionFamily bodyFamily(std::size_t index) const noexcept {
        return index < bodyFamilies_.size() ? bodyFamilies_[index] : PerformanceMotionFamily::Count;
    }
    PerformanceConstellationMotion constellationMotion() const noexcept { return constellationMotion_; }
    void restoreMotion(PerformanceMotionFamily collective, const std::array<PerformanceMotionFamily, 8>& bodies,
                       PerformanceConstellationMotion motion) {
        collectiveFamily_ = collective;
        bodyFamilies_ = bodies;
        constellationMotion_ = motion;
        if (onMotionFamilyChanged) {
            onMotionFamilyChanged(kNoNode, collectiveFamily_);
            for (std::size_t index = 0U; index < bodyFamilies_.size(); ++index)
                onMotionFamilyChanged(index, bodyFamilies_[index]);
        }
        if (onConstellationMotionChanged) onConstellationMotionChanged(constellationMotion_);
        repaint();
    }
    void cycleMotionFamily() noexcept {
        constexpr auto count = static_cast<std::uint8_t>(PerformanceMotionFamily::Count);
        if (selectedNode_ >= bodyFamilies_.size()) {
            collectiveFamily_ = static_cast<PerformanceMotionFamily>(
                (static_cast<std::uint8_t>(collectiveFamily_) + 1U) % count);
            if (onMotionFamilyChanged) onMotionFamilyChanged(kNoNode, collectiveFamily_);
        } else {
            auto& family = bodyFamilies_[selectedNode_];
            family = static_cast<PerformanceMotionFamily>((static_cast<std::uint8_t>(family) + 1U) % (count + 1U));
            if (onMotionFamilyChanged) onMotionFamilyChanged(selectedNode_, family);
        }
        repaint();
    }
    static juce::String localizedConstellationMotion(SupportLanguage language, PerformanceConstellationMotion motion) {
        switch (motion) {
        case PerformanceConstellationMotion::Rotate: return uiText(language, "GIRA", "ROTATES", "TOURNE", "GIRA");
        case PerformanceConstellationMotion::Travel: return uiText(language, "VIAJA", "TRAVELS", "VOYAGE", "VIAJA");
        case PerformanceConstellationMotion::RotateTravel:
            return uiText(language, "GIRA+VIAJA", "ROTATES+TRAVELS", "TOURNE+VOYAGE", "GIRA+VIAJA");
        default: return uiText(language, "FIXA", "STILL", "FIXE", "FIJA");
        }
    }
    static juce::String localizedMotionFamily(SupportLanguage language, PerformanceMotionFamily family) {
        switch (family) {
        case PerformanceMotionFamily::Free: return uiText(language, "LIVRE", "FREE", "LIBRE", "LIBRE");
        case PerformanceMotionFamily::Billiard: return uiText(language, "BILHAR", "BILLIARDS", "BILLARD", "BILLAR");
        case PerformanceMotionFamily::Foam: return uiText(language, "ESPUMA", "FOAM", "MOUSSE", "ESPUMA");
        case PerformanceMotionFamily::Pendulum: return uiText(language, "PÊNDULOS", "PENDULUMS", "PENDULES", "PÉNDULOS");
        case PerformanceMotionFamily::Magnetic: return uiText(language, "MAGNÉTICO", "MAGNETIC", "MAGNÉTIQUE", "MAGNÉTICO");
        case PerformanceMotionFamily::Gravity: return uiText(language, "GRAVIDADE", "GRAVITY", "GRAVITÉ", "GRAVEDAD");
        default: return uiText(language, "CAMPO", "FIELD", "CHAMP", "CAMPO");
        }
    }

    void cycleOrbitMode() noexcept {
        if (selectedNode_ >= motionFields_.size()) {
            const auto next = (static_cast<std::uint8_t>(collectiveMotionField_) + 1U) % 3U;
            collectiveMotionField_ = static_cast<OrbitMode>(next);
            if (onOrbitModeChanged) onOrbitModeChanged(kNoNode, collectiveMotionField_);
            repaint();
            return;
        }
        const auto next = (static_cast<std::uint8_t>(motionFields_[selectedNode_]) + 1U) % 3U;
        motionFields_[selectedNode_] = static_cast<OrbitMode>(next);
        if (onOrbitModeChanged) onOrbitModeChanged(selectedNode_, motionFields_[selectedNode_]);
        repaint();
    }

    void toggleOrbitDirection() noexcept {
        if (selectedNode_ >= motionForwards_.size()) return;
        motionForwards_[selectedNode_] = !motionForwards_[selectedNode_];
        if (onOrbitDirectionChanged)
            onOrbitDirectionChanged(selectedNode_, motionForwards_[selectedNode_]);
        repaint();
    }

    void cycleRelationalPhenomenon() noexcept {
        if (selectedRelation_ >= relationalPhenomena_.size()) return;
        auto& phenomenon = relationalPhenomena_[selectedRelation_];
        if (relationalPhenomenaEnabled_[selectedRelation_])
            phenomenon = nextRelationalPhenomenon(phenomenon);
        relationalPhenomenaEnabled_[selectedRelation_] = true;
        if (onRelationalPhenomenonChanged)
            onRelationalPhenomenonChanged(selectedRelation_, phenomenon);
        repaint();
    }

    void cyclePopulationPhenomenon() noexcept {
        if (populationPhenomenonEnabled_)
            populationPhenomenon_ = static_cast<PerformancePopulationPhenomenon>(
                (static_cast<std::uint8_t>(populationPhenomenon_) + 1U)
                % static_cast<std::uint8_t>(PerformancePopulationPhenomenon::Count));
        populationPhenomenonEnabled_ = true;
        if (onPopulationPhenomenonChanged)
            onPopulationPhenomenonChanged(populationPhenomenon_, populationPhenomenonIntensity_);
        repaint();
    }

    void adjustPopulationPhenomenon(double delta) noexcept {
        if (!populationPhenomenonEnabled_) return;
        populationPhenomenonIntensity_ = juce::jlimit(0.0, 1.0,
                                                       populationPhenomenonIntensity_ + delta);
        if (onPopulationPhenomenonChanged)
            onPopulationPhenomenonChanged(populationPhenomenon_, populationPhenomenonIntensity_);
        repaint();
    }

    void paint(juce::Graphics& graphics) override {
        const auto panel = getLocalBounds().toFloat().reduced(16.0f, 8.0f);
        const auto bounds = interactionBounds();
        graphics.setColour(juce::Colour(0xff102d27));
        graphics.fillRoundedRectangle(panel, 5.0f);
        const auto movementControl = movementControlBounds();
        graphics.setColour(juce::Colour(0xff0b211d));
        graphics.fillRoundedRectangle(movementControl, 3.5f);
        graphics.setColour(juce::Colour(0xffd18a45));
        graphics.drawRoundedRectangle(movementControl, 3.5f, 0.9f);
        graphics.setColour(juce::Colour(0xffe7e3d5));
        graphics.setFont(interfaceFont(7.8f, juce::Font::bold));
        const auto focusedMode = orbitMode();
        const auto directionLabel = selectedNode_ >= motionForwards_.size() ? ""
            : motionForwards_[selectedNode_] ? ">" : "<";
        const auto movementLabel = selectedNode_ < motionFields_.size()
            ? uiText(language_, "DANÇA ", "DANCE ", "DANSE ", "DANZA ")
            : uiText(language_, "CAMPO ", "FIELD ", "CHAMP ", "CAMPO ");
        // Uma estrela que segue o campo mostra a família herdada ("= LIVRE").
        const bool inherits = selectedNode_ < bodyFamilies_.size()
            && bodyFamilies_[selectedNode_] == PerformanceMotionFamily::Count;
        const auto movementValue = skyMotion_
            ? (inherits ? "= " + localizedMotionFamily(language_, collectiveFamily_)
                        : localizedMotionFamily(language_, selectedNode_ < bodyFamilies_.size()
                                                               ? bodyFamilies_[selectedNode_] : collectiveFamily_))
            : localizedMotionField(language_, focusedMode) + " " + directionLabel;
        graphics.drawFittedText(movementLabel + movementValue,
                                movementControl.toNearestInt().reduced(3, 0),
                                juce::Justification::centred, 1);
        if (skyMotion_) {
            const auto turn = constellationMotionBounds();
            graphics.setColour(juce::Colour(0xff0b211d));
            graphics.fillRoundedRectangle(turn, 3.5f);
            graphics.setColour(juce::Colour(0xffd18a45));
            graphics.drawRoundedRectangle(turn, 3.5f, 0.9f);
            graphics.setColour(juce::Colour(0xffe7e3d5));
            graphics.drawFittedText(uiText(language_, "CÉU ", "SKY ", "CIEL ", "CIELO ")
                                        + localizedConstellationMotion(language_, constellationMotion_),
                                    turn.toNearestInt().reduced(3, 0), juce::Justification::centred, 1);
        }
        if (selectedRelation_ < relationalPhenomena_.size()) {
            const auto phenomenonControl = relationalPhenomenonControlBounds();
            graphics.setColour(juce::Colour(0xff0b211d));
            graphics.fillRoundedRectangle(phenomenonControl, 3.5f);
            graphics.setColour(juce::Colour(0xffd18a45));
            graphics.drawRoundedRectangle(phenomenonControl, 3.5f, 0.9f);
            graphics.setColour(juce::Colour(0xffe7e3d5));
            graphics.setFont(interfaceFont(7.8f, juce::Font::bold));
            const auto label = relationalPhenomenaEnabled_[selectedRelation_]
                ? uiText(language_, "FENÔMENO · ", "PHENOMENON · ", "PHÉNOMÈNE · ", "FENÓMENO · ")
                    + localizedRelationalPhenomenon(language_, relationalPhenomena_[selectedRelation_])
                : uiText(language_, "FENÔMENO · ESCOLHER", "PHENOMENON · CHOOSE",
                         "PHÉNOMÈNE · CHOISIR", "FENÓMENO · ELEGIR");
            graphics.drawFittedText(label, phenomenonControl.toNearestInt().reduced(3, 0),
                                    juce::Justification::centred, 1);
        }
        graphics.setFont(interfaceFont(9.5f, juce::Font::bold));
        double collectiveCentreX = 0.0;
        double collectiveCentreY = 0.0;
        std::size_t collectiveCount = 0U;
        for (std::size_t index = 0U; index < nodes_.size(); ++index) {
            if (!visibleConstellationNode(index)) continue;
            collectiveCentreX += nodes_[index].x;
            collectiveCentreY += nodes_[index].y;
            ++collectiveCount;
        }
        collectiveCentreX /= static_cast<double>(std::max<std::size_t>(collectiveCount, 1U));
        collectiveCentreY /= static_cast<double>(std::max<std::size_t>(collectiveCount, 1U));
        std::array<PerformanceMotionPoint, 8> livePoints{};
        for (std::size_t index = 0U; index < livePoints.size(); ++index) {
            const auto direction = motionForwards_[index] ? 1.0 : -1.0;
            const auto collectivePhase = motionPhase_ * juce::MathConstants<double>::twoPi
                + orbitPhaseOffset_;
            const auto collective = performanceCollectiveMotionPoint(nodes_[index].x, nodes_[index].y,
                collectiveCentreX, collectiveCentreY, collectiveMotionField_, collectivePhase);
            livePoints[index] = motionFields_[index] == collectiveMotionField_ && motionForwards_[index]
                ? collective : performanceMotionPoint(collective.x, collective.y,
                motionFields_[index], direction * motionPhase_ * juce::MathConstants<double>::twoPi
                    + orbitPhaseOffset_, index, presences_[index]);
            // Com a trama, o corpo está onde a física do céu o pôs
            // (gravidade, molas, bilhar, magnetismo, spin, espuma, pêndulo,
            // distorção — PerformanceSkyMotion.hpp); a profundidade segue a
            // da órbita.
            if (skyMotion_ && active_[index]) {
                livePoints[index].x = skyPositions_[index].x;
                livePoints[index].y = skyPositions_[index].y;
            }
            livePoints[index].x = juce::jlimit(0.0, 1.0,
                livePoints[index].x + fieldForceX_ * 0.24);
            livePoints[index].y = juce::jlimit(0.0, 1.0,
                livePoints[index].y + fieldForceY_ * 0.24);
        }
        auto populationPoints = livePoints;
        if (populationPhenomenonEnabled_) {
            PerformanceMotionPoint populationCentre{};
            for (std::size_t index = 0U; index < livePoints.size(); ++index) {
                if (!active_[index]) continue;
                populationCentre.x += livePoints[index].x;
                populationCentre.y += livePoints[index].y;
                populationCentre.z += livePoints[index].z;
            }
            const auto count = static_cast<double>(std::max<std::size_t>(collectiveCount, 1U));
            populationCentre.x /= count;
            populationCentre.y /= count;
            populationCentre.z /= count;
            for (std::size_t index = 0U; index < populationPoints.size(); ++index) {
                if (!active_[index]) continue;
                populationPoints[index] = populationMotionPoint(
                    populationPhenomenon_, livePoints[index], populationCentre,
                    motionPhase_ * juce::MathConstants<double>::twoPi + orbitPhaseOffset_,
                    populationPhenomenonIntensity_, index, collectiveCount);
            }
        }
        auto relationalPoints = populationPoints;
        for (std::size_t relation = 0U; relation < relations_.size(); ++relation) {
            const auto source = relationSources_[relation];
            const auto destination = relationDestinations_[relation];
            if (!relationalPhenomenaEnabled_[relation] || !active_[source] || !active_[destination])
                continue;
            const auto displacement = relationalDisplacement(
                relationalPhenomena_[relation], populationPoints[source], populationPoints[destination],
                motionPhase_ * juce::MathConstants<double>::twoPi
                    + orbitPhaseOffset_ + static_cast<double>(relation) * 0.71,
                relations_[relation]);
            relationalPoints[source].x += displacement.sourceX;
            relationalPoints[source].y += displacement.sourceY;
            relationalPoints[destination].x += displacement.destinationX;
            relationalPoints[destination].y += displacement.destinationY;
        }
        const auto liveScreenPoint = [bounds, &relationalPoints](std::size_t index) {
            // The constellation remains drawn on a 2D surface, but z is a
            // real musical coordinate. Near bodies gain scale and slightly
            // pull the projection toward the viewer; far bodies recede.
            const auto& body = relationalPoints[index];
            const auto depth = juce::jlimit(0.0, 1.0, body.z);
            const auto perspective = 0.72 + depth * 0.28;
            const auto x = 0.5 + (juce::jlimit(0.0, 1.0, body.x) - 0.5) * perspective;
            const auto y = 0.5 + (juce::jlimit(0.0, 1.0, body.y) - 0.5) * perspective
                - (depth - 0.5) * 0.055;
            return screenPoint(juce::jlimit(0.0, 1.0, x),
                               juce::jlimit(0.0, 1.0, y), bounds);
        };
        for (std::size_t index = 0; index < relations_.size(); ++index) {
            if (!visibleConstellationNode(relationSources_[index])
                || !visibleConstellationNode(relationDestinations_[index]))
                continue;
            const auto from = liveScreenPoint(relationSources_[index]);
            const auto to = liveScreenPoint(relationDestinations_[index]);
            const auto wildAlpha = static_cast<float>(wildLevel_ * 0.28);
            graphics.setColour((wildLevel_ > 0.005 ? juce::Colour(0xffe1aa59) : juce::Colour(0xffd18a45))
                .withAlpha(static_cast<float>(0.52 + relations_[index] * 0.42) + wildAlpha));
            graphics.drawLine({from, to}, static_cast<float>(1.5 + relations_[index] * 4.5
                + wildLevel_ * 2.5));
        }
        // A trama do céu: as relações em foco, de quem chama para quem
        // responde, com o nome do ato. A cor esquenta com a tensão harmônica.
        for (std::size_t index = 0U; index < skyFocusCount_; ++index) {
            const auto& focus = skyFoci_[index];
            if (!visibleConstellationNode(focus.caller) || !visibleConstellationNode(focus.responder)) continue;
            const auto from = liveScreenPoint(focus.caller);
            const auto to = liveScreenPoint(focus.responder);
            const auto colour = juce::Colour(0xfff2e7d5).interpolatedWith(juce::Colour(0xffe0603a),
                                                                          static_cast<float>(skyTension_));
            graphics.setColour(colour.withAlpha(0.92f));
            juce::Path arrow;
            arrow.addArrow({from, to}, 1.6f, 9.0f, 9.0f);
            graphics.fillPath(arrow);
            graphics.setFont(interfaceFont(8.5f, juce::Font::bold));
            const auto middle = (from + to) * 0.5f;
            graphics.drawText(skyActName(focus.kind),
                              juce::Rectangle<float>(middle.x - 50.0f, middle.y - 16.0f, 100.0f, 12.0f),
                              juce::Justification::centred, false);
        }
        const auto roleLabel = [this](std::size_t index) {
            switch (index) {
            case 0U: return uiText(language_, "CHÃO", "GROUND", "SOL", "SUELO");
            case 1U: return uiText(language_, "ATRITO", "FRICTION", "FRICTION", "FRICCIÓN");
            case 2U: return uiText(language_, "CONTÁGIO", "CONTAGION", "CONTAGION", "CONTAGIO");
            case 3U: return uiText(language_, "RESERVA", "RESERVE", "RÉSERVE", "RESERVA");
            case 4U: return uiText(language_, "MEMÓRIA", "MEMORY", "MÉMOIRE", "MEMORIA");
            case 5U: return uiText(language_, "CORPO", "BODY", "CORPS", "CUERPO");
            case 6U: return uiText(language_, "RASGO", "RASGO", "RASGO", "RASGO");
            default: return uiText(language_, "VESTÍGIO", "TRACE", "TRACE", "VESTIGIO");
            }
        };
        for (std::size_t index = 0; index < moduleNames_.size(); ++index) {
            auto cell = rackCell(index);
            // Desde 2026-10-28 cada corpo tem stem próprio: nenhuma lane é
            // dobrada dentro de outro stem.
            const bool internalFoldedLane = false;
            const bool selected = index == selectedNode_;
            graphics.setColour(active_[index] || internalFoldedLane ? juce::Colour(0xff183b33)
                                                                     : juce::Colour(0xff285348));
            graphics.fillRoundedRectangle(cell, 4.0f);
            // Orange is a focused gesture colour, never a permanent status
            // for folded lanes. Modules 5..8 are available internal matter,
            // not inactive production stems; their border stays neutral until
            // the performer selects one.
            graphics.setColour(selected ? juce::Colour(0xffe1aa59)
                                         : active_[index] || internalFoldedLane
                                             ? juce::Colour(0xff8aa99b)
                                             : juce::Colour(0xff6f9b8d));
            graphics.drawRoundedRectangle(cell, 4.0f, selected ? 1.8f : 1.0f);
            graphics.setColour(juce::Colour(0xffe7e3d5));
            auto identityCell = cell;
            auto statusCell = identityCell.removeFromBottom(identityCell.getHeight() * 0.45f);
            graphics.drawFittedText(juce::String(static_cast<int>(index + 1U)) + " "
                                        + moduleNames_[index] + utf8(" · ") + roleLabel(index),
                                    identityCell.toNearestInt(), juce::Justification::centred, 1);
            graphics.setColour(juce::Colour(0xffb8d8cc));
            const auto routing = internalFoldedLane
                ? uiText(language_, "LANE INTERNA → STEM ", "INTERNAL LANE → STEM ",
                         "LANE INTERNE → STEM ", "LANE INTERNA → STEM ")
                      + juce::String(static_cast<int>(index - 3U))
                : utf8("STEM ") + juce::String(actorIds_[index]);
            const auto laneState = internalFoldedLane
                ? uiText(language_, "ATIVA", "ACTIVE", "ACTIVE", "ACTIVA")
                : lifeLabel(lifeStates_[index]);
            graphics.drawFittedText(routing + utf8(" · ") + laneState + utf8(" / ")
                                        + readinessLabel(readiness_[index]),
                                    statusCell.toNearestInt(), juce::Justification::centred, 1);
        }
        // Rastros (29/10): com a física do céu, cada estrela deixa ~1,5 s do
        // seu caminho, apagando-se — as curvas, balanços e ricochetes ficam
        // legíveis. Amostrado a ~30 Hz, guardado em fração da área.
        const auto nowMs = juce::Time::getMillisecondCounterHiRes();
        const bool sampleTrail = skyMotion_ && nowMs - lastTrailMs_ >= 33.0;
        if (sampleTrail) {
            lastTrailMs_ = nowMs;
            trailHead_ = (trailHead_ + 1U) % kTrailLength;
            trailCount_ = std::min(trailCount_ + 1U, kTrailLength);
        }
        if (!skyMotion_) trailCount_ = 0U;
        for (std::size_t index = 0; index < nodes_.size(); ++index) {
            if (!visibleConstellationNode(index)) continue;
            const auto point = liveScreenPoint(index);
            if (sampleTrail)
                trails_[index][trailHead_] = {(point.x - bounds.getX()) / std::max(1.0f, bounds.getWidth()),
                                              (point.y - bounds.getY()) / std::max(1.0f, bounds.getHeight())};
            if (trailCount_ > 2U && active_[index]) {
                const auto glowTrail = 0.25f + 0.75f * static_cast<float>(starBrightness_[index]);
                for (std::size_t step = 1U; step < trailCount_; ++step) {
                    const auto newer = trails_[index][(trailHead_ + kTrailLength - step + 1U) % kTrailLength];
                    const auto older = trails_[index][(trailHead_ + kTrailLength - step) % kTrailLength];
                    const auto fade = 1.0f - static_cast<float>(step) / static_cast<float>(kTrailLength);
                    graphics.setColour(juce::Colour(0xffe1aa59).withAlpha(0.42f * fade * fade * glowTrail));
                    graphics.drawLine(bounds.getX() + newer.x * bounds.getWidth(), bounds.getY() + newer.y * bounds.getHeight(),
                                      bounds.getX() + older.x * bounds.getWidth(), bounds.getY() + older.y * bounds.getHeight(),
                                      0.6f + 1.4f * fade);
                }
            }
            const auto anchor = screenPoint(nodes_[index].x, nodes_[index].y, bounds);
            const auto trailLength = point.getDistanceFrom(anchor);
            if (trailLength > 1.5f) {
                graphics.setColour(juce::Colour(0xff8aa99b).withAlpha(
                    juce::jlimit(0.10f, 0.46f, trailLength / 90.0f)));
                graphics.drawLine({anchor, point}, 1.0f);
                graphics.fillEllipse(anchor.x - 2.0f, anchor.y - 2.0f, 4.0f, 4.0f);
            }
            const auto energy = static_cast<float>(std::sqrt(stemLevels_[index]));
            const auto layer = static_cast<float>(stemLayerMix_[index]);
            const auto depth = static_cast<float>(juce::jlimit(0.0, 1.0, relationalPoints[index].z));
            const auto depthScale = 0.74f + depth * 0.52f;
            const auto haloRadius = (13.0f + energy * 10.0f) * depthScale;
            graphics.setColour(juce::Colour(0xff5eb6d7).withAlpha(0.12f + energy * 0.62f));
            graphics.drawEllipse(point.x - haloRadius, point.y - haloRadius,
                                 haloRadius * 2.0f, haloRadius * 2.0f,
                                 1.0f + energy * 4.0f);
            // A copper inner orbit makes an internal mix legible without
            // adding another node or pretending that the source body moved.
            if (layer > 0.0005f) {
                const auto layerRadius = haloRadius * (0.56f + layer * 0.32f);
                graphics.setColour(juce::Colour(0xffd18a45).withAlpha(0.22f + layer * 0.68f));
                graphics.drawEllipse(point.x - layerRadius, point.y - layerRadius,
                                     layerRadius * 2.0f, layerRadius * 2.0f,
                                     0.8f + layer * 3.4f);
            }
            if (wildLevel_ > 0.005) {
                const auto wildRadius = haloRadius + 7.0f + static_cast<float>(wildLevel_) * 15.0f;
                graphics.setColour(juce::Colour(0xffe1aa59).withAlpha(
                    static_cast<float>(0.14 + wildLevel_ * 0.42)));
                graphics.drawEllipse(point.x - wildRadius, point.y - wildRadius,
                                     wildRadius * 2.0f, wildRadius * 2.0f,
                                     0.8f + static_cast<float>(wildLevel_) * 2.2f);
            }
            const bool selected = index == selectedNode_;
            const auto nodeRadius = (8.0f + static_cast<float>(presences_[index]) * 5.0f) * depthScale;
            // O brilho da estrela (variação do mixer): apagada, ela continua
            // no céu, esmaecida.
            const auto glow = 0.25f + 0.75f * static_cast<float>(starBrightness_[index]);
            graphics.setColour((index == activeNode_ || selected ? juce::Colour(0xffe1aa59)
                                                                 : juce::Colour(0xff315e51)).withMultipliedAlpha(glow));
            graphics.fillEllipse(point.x - nodeRadius, point.y - nodeRadius,
                                 nodeRadius * 2.0f, nodeRadius * 2.0f);
            graphics.setColour(juce::Colour(0xfff2e7d5).withMultipliedAlpha(glow));
            graphics.drawEllipse(point.x - nodeRadius - 1.5f, point.y - nodeRadius - 1.5f,
                                 nodeRadius * 2.0f + 3.0f, nodeRadius * 2.0f + 3.0f, 1.2f);
            if (index < pinnedNodes_.size() && pinnedNodes_[index]) {
                // Fixada (2/11): um losango cheio acima da estrela, sempre
                // aceso — a VARIAÇÃO não a apaga.
                juce::Path pin;
                const float top = point.y - nodeRadius - 11.0f;
                pin.addQuadrilateral(point.x, top - 4.0f, point.x + 4.0f, top, point.x, top + 4.0f, point.x - 4.0f, top);
                graphics.setColour(juce::Colour(0xfff0b860));
                graphics.fillPath(pin);
            }
            if (skyChorusVisible_) {
                // O coro de fases: um ponteiro mostra onde, dentro do pulso,
                // a estrela ataca (meio-dia = no pulso; uma volta = um passo).
                const auto angle = static_cast<float>(chorusPhase_[index] * juce::MathConstants<double>::twoPi);
                const auto hand = nodeRadius + 5.0f;
                const juce::Point<float> tip(point.x + std::sin(angle) * hand, point.y - std::cos(angle) * hand);
                graphics.setColour(juce::Colour(0xffe1aa59).withMultipliedAlpha(glow));
                graphics.fillEllipse(tip.x - 2.2f, tip.y - 2.2f, 4.4f, 4.4f);
                // O planeta que deriva: um anel a mais, fino.
                if (index == driftNode_) {
                    const auto ring = nodeRadius + 9.0f;
                    graphics.setColour(juce::Colour(0xff8fc9e8).withAlpha(0.75f));
                    graphics.drawEllipse(point.x - ring, point.y - ring, ring * 2.0f, ring * 2.0f, 1.0f);
                }
                // O eclipse: um disco escuro, deslocado como uma lua nova.
                if (index == eclipseNode_) {
                    const auto shift = nodeRadius * 0.45f;
                    graphics.setColour(juce::Colour(0xff06120f).withAlpha(0.88f));
                    graphics.fillEllipse(point.x - nodeRadius + shift, point.y - nodeRadius - shift * 0.4f,
                                         nodeRadius * 2.0f, nodeRadius * 2.0f);
                }
            }
            graphics.setFont(interfaceFont(10.0f, juce::Font::bold));
            graphics.drawText(juce::String(static_cast<int>(index + 1U)),
                              juce::Rectangle<float>(point.x - nodeRadius, point.y - nodeRadius,
                                                     nodeRadius * 2.0f, nodeRadius * 2.0f),
                              juce::Justification::centred);
        }
        // The rack shows the currently active trio; keep the complete adapted
        // library visible in the same panel so the available territories are
        // discoverable without adding fourteen competing controls.
        const auto graphLayout = performanceGraphLayout();
        const auto libraryBounds = panel.withTrimmedTop(panel.getHeight() - graphLayout.libraryTopInset)
                                        .withHeight(graphLayout.libraryHeight).reduced(4.0f, 2.0f);
        // A BIBLIOTECA como botão (5/11, pedido do autor: "do jeito que está
        // é praticamente impossível de um usuário descobrir que é possível
        // abrir uma caixa para escolher os módulos"): borda âmbar e fundo dos
        // botões de GESTOS, acende sob o mouse, e a seta ▾ diz que abre um menu.
        const bool libraryLit = libraryHover_ || paletteDrag_ < nodes_.size();
        graphics.setColour(libraryLit ? juce::Colour(0xff5a4430) : juce::Colour(0xff3b2d22));
        graphics.fillRoundedRectangle(libraryBounds, 4.0f);
        graphics.setColour(libraryLit ? juce::Colour(0xfff0b860) : juce::Colour(0xffd18a45));
        graphics.drawRoundedRectangle(libraryBounds, 4.0f, libraryLit ? 1.6f : 1.2f);
        auto libraryTextBounds = libraryBounds.reduced(10.0f, 2.0f).toNearestInt();
        const auto libraryLineOne = (selectedNode_ < nodes_.size()
            ? uiText(language_, "TROCAR ", "SWAP ", "CHANGER ", "CAMBIAR ")
                + moduleNames_[selectedNode_]
                + uiText(language_, " (CORPO ", " (BODY ", " (CORPS ", " (CUERPO ")
                + juce::String(static_cast<int>(selectedNode_ + 1U)) + ")"
            : uiText(language_, "BIBLIOTECA · ESCOLHER ESTRELAS", "LIBRARY · CHOOSE STARS",
                     "BIBLIOTHÈQUE · CHOISIR DES ÉTOILES", "BIBLIOTECA · ELEGIR ESTRELLAS"))
            + utf8(" ▾");
        const auto libraryLineTwo = juce::String(static_cast<int>(kPerformanceSkyStars.size()))
            + uiText(language_, " ESTRELAS · TOM, TEXTURA, VOZ, PULSO, CONTROLE · OU ARRASTE ATÉ UMA ESTRELA",
                     " STARS · TONE, TEXTURE, VOICE, PULSE, CONTROL · OR DRAG ONTO A STAR",
                     " ÉTOILES · TON, TEXTURE, VOIX, PULSATION, CONTRÔLE · OU GLISSEZ SUR UNE ÉTOILE",
                     " ESTRELLAS · TONO, TEXTURA, VOZ, PULSO, CONTROL · O ARRASTRE SOBRE UNA ESTRELLA");
        graphics.setColour(juce::Colour(0xfff2e7d5));
        graphics.setFont(interfaceFont(10.5f, juce::Font::bold));
        graphics.drawFittedText(libraryLineOne, libraryTextBounds.removeFromTop(19),
                                juce::Justification::centredLeft, 1);
        graphics.setColour(juce::Colour(0xffd8b48c));
        graphics.setFont(interfaceFont(8.5f));
        graphics.drawFittedText(libraryLineTwo, libraryTextBounds,
                                juce::Justification::centredLeft, 1);
        if (paletteDrag_ < nodes_.size()) {
            graphics.setColour(juce::Colour(0xffd18a45).withAlpha(0.42f));
            graphics.fillEllipse(dragGhost_.x - 15.0f, dragGhost_.y - 15.0f, 30.0f, 30.0f);
            graphics.setColour(juce::Colour(0xfff2e7d5));
            graphics.drawEllipse(dragGhost_.x - 16.0f, dragGhost_.y - 16.0f, 32.0f, 32.0f, 1.5f);
        }
        juce::String relationText = trajectoryText_;
        if (selectedRelation_ < relations_.size()) {
            const auto source = relationSources_[selectedRelation_];
            const auto destination = relationDestinations_[selectedRelation_];
            const bool audibleNow = active_[source] && active_[destination];
            relationText = relationNames_[selectedRelation_] + utf8(" · ")
                + (relationalPhenomenaEnabled_[selectedRelation_]
                    ? localizedRelationalPhenomenon(language_, relationalPhenomena_[selectedRelation_])
                    : relationEffects_[selectedRelation_]) + utf8(" · ")
                + juce::String(relations_[selectedRelation_], 2)
                + (audibleNow ? uiText(language_, " · ATUA AGORA", " · ACTIVE NOW",
                                       " · ACTIF MAINTENANT", " · ACTÚA AHORA")
                              : uiText(language_, " · POTENCIAL: TRAGA OS DOIS CORPOS",
                                       " · POTENTIAL: BRING BOTH BODIES", " · POTENTIEL : AMENEZ LES DEUX CORPS",
                                       " · POTENCIAL: TRAIGA AMBOS CUERPOS"));
        } else if (selectedNode_ < nodes_.size() && visibleConstellationNode(selectedNode_)) {
            relationText = moduleNames_[selectedNode_] + " · STEM · "
                + lifeLabel(lifeStates_[selectedNode_])
                + uiText(language_, " · PICO ", " · PEAK ", " · PIC ", " · PICO ")
                + juce::String(stemLevels_[selectedNode_], 2) + " · Z "
                + juce::String(relationalPoints[selectedNode_].z, 2)
                + uiText(language_, " · MISTURA ", " · MIX ", " · MÉLANGE ", " · MEZCLA ")
                + juce::String(stemLayerMix_[selectedNode_], 2);
        }
        graphics.setColour(juce::Colour(0xffd18a45));
        graphics.setFont(interfaceFont(10.0f, juce::Font::bold));
        graphics.drawText(performanceCue(), panel.withTrimmedTop(panel.getHeight() - 36.0f)
                          .withHeight(16.0f), juce::Justification::bottomLeft);
        graphics.setColour(juce::Colour(0xff8aa99b));
        graphics.setFont(interfaceFont(10.0f));
        graphics.drawText(relationText, panel.withTrimmedTop(panel.getHeight() - 18.0f),
                          juce::Justification::bottomLeft);
        if (hasKeyboardFocus(true)) {
            graphics.setColour(juce::Colour(0xffe1aa59));
            graphics.drawRoundedRectangle(panel.expanded(3.0f), 7.0f, 1.2f);
        }
    }

    void mouseDown(const juce::MouseEvent& event) override {
        grabKeyboardFocus();
        const auto panel = getLocalBounds().toFloat().reduced(16.0f, 8.0f);
        const auto graphLayout = performanceGraphLayout();
        const auto libraryBounds = panel.withTrimmedTop(panel.getHeight() - graphLayout.libraryTopInset)
                                        .withHeight(graphLayout.libraryHeight).reduced(4.0f, 2.0f);
        if (skyMotion_ && constellationMotionBounds().contains(event.position)) {
            constellationMotion_ = static_cast<PerformanceConstellationMotion>(
                (static_cast<std::uint8_t>(constellationMotion_) + 1U)
                % static_cast<std::uint8_t>(PerformanceConstellationMotion::Count));
            if (onConstellationMotionChanged) onConstellationMotionChanged(constellationMotion_);
            repaint();
            return;
        }
        if (movementControlBounds().contains(event.position)) {
            // Com a trama, a DANÇA escolhe a família de movimento (30/10).
            if (skyMotion_ && !event.mods.isShiftDown()) cycleMotionFamily();
            else if (event.mods.isShiftDown()) toggleOrbitDirection();
            else cycleOrbitMode();
            return;
        }
        if (relationalPhenomenonControlBounds().contains(event.position)) {
            if (selectedRelation_ < relationalPhenomena_.size()) cycleRelationalPhenomenon();
            return;
        }
        if (libraryBounds.contains(event.position) && selectedNode_ < nodes_.size() && onStarSwapRequested) {
            // Com uma estrela em foco, a BIBLIOTECA troca a estrela daquele
            // corpo (30/10, gestão dos módulos).
            onStarSwapRequested(selectedNode_);
            return;
        }
        if (libraryBounds.contains(event.position)) {
            // The library is a palette: press, drag to the field, release.
            // A click without a field drop retains the old quick-insert path.
            paletteDrag_ = 3U;
            dragGhost_ = event.position;
            // O foco só muda ao soltar (no campo, ou pelo menu dos corpos).
            selectedRelation_ = kNoNode;
            repaint();
            return;
        }
        const auto bounds = interactionBounds();
        float nearest = std::numeric_limits<float>::max();
        activeNode_ = kNoNode;
        for (std::size_t index = 0; index < nodes_.size(); ++index) {
            if (!visibleConstellationNode(index)) continue;
            const auto distance = event.position.getDistanceFrom(screenPoint(index, bounds));
            if (distance < nearest) {
                nearest = distance;
                activeNode_ = index;
            }
        }
        // The visible body can be displaced by its live orbit. Keep a broad,
        // invisible interaction target so the performer can select it without
        // pixel hunting; relation hit-testing only happens after this target.
        if (activeNode_ < nodes_.size() && nearest <= 58.0f) {
            // Lanes 5–8 are internal matter, yet remain real bodies of the
            // Campo. Their positions travel through the realtime queue and
            // shape the folded layer without creating a fifth stem.
            draggedNode_ = activeNode_;
            selectedNode_ = activeNode_;
            selectedRelation_ = kNoNode;
            if (onSelectionChanged) onSelectionChanged(selectedNode_, false);
            mouseDrag(event);
            return;
        }
        // Resolve active nodes before rack cards. The fourth node can be
        // drawn close to its reserve card; the node owns that hit area while
        // active so dragging it back to the rack can perform recollection.
        for (std::size_t index = 0; index < nodes_.size(); ++index) {
            const auto cell = rackCell(index);
            if (!cell.contains(event.position)) continue;
            paletteDrag_ = index;
            dragGhost_ = event.position;
            selectedNode_ = index;
            selectedRelation_ = kNoNode;
            if (onSelectionChanged) onSelectionChanged(selectedNode_, false);
            repaint();
            return;
        }
        activeNode_ = kNoNode;
        const auto relation = closestRelation(event.position, bounds);
        if (relation.first < relations_.size() && relation.second <= 14.0f) {
            activeRelation_ = relation.first;
            selectedRelation_ = relation.first;
            relationDragStartX_ = event.position.x;
            relationDragStartValue_ = relations_[activeRelation_];
            if (onSelectionChanged) onSelectionChanged(selectedRelation_, true);
            return;
        }
        // O clique no vazio é a saída explícita do foco: os três gestos voltam
        // a agir sobre a constelação inteira. Evita um alvo residual invisível.
        selectedNode_ = kNoNode;
        selectedRelation_ = kNoNode;
        if (onSelectionChanged) onSelectionChanged(kNoNode, true);
        repaint();
    }

    // A estrela visível mais próxima do ponto, até `radius` pixels.
    std::size_t nodeUnder(juce::Point<float> point, const juce::Rectangle<float>& bounds,
                          float radius) const {
        std::size_t found = kNoNode;
        float nearest = radius;
        for (std::size_t index = 0; index < nodes_.size(); ++index) {
            if (!visibleConstellationNode(index)) continue;
            const auto distance = point.getDistanceFrom(screenPoint(index, bounds));
            if (distance <= nearest) {
                nearest = distance;
                found = index;
            }
        }
        return found;
    }

    void mouseDrag(const juce::MouseEvent& event) override {
        const auto bounds = interactionBounds();
        if (bounds.isEmpty()) return;
        if (paletteDrag_ < nodes_.size()) {
            dragGhost_ = event.position;
            repaint();
            return;
        }
        if (activeRelation_ < relations_.size()) {
            const auto offset = static_cast<double>((event.position.x - relationDragStartX_) / bounds.getWidth());
            relations_[activeRelation_] = juce::jlimit(0.0, 1.0, relationDragStartValue_ + offset);
            if (onRelationChanged)
                onRelationChanged(activeRelation_, relations_[activeRelation_], false);
            repaint();
            return;
        }
        if (activeNode_ >= nodes_.size()) return;
        // A reserve card is outside the field bounds. Let the pointer cross
        // that boundary; releasing there performs the explicit removal.
        if (overRack(event.position)) {
            dragGhost_ = event.position;
            repaint();
            return;
        }
        auto& node = nodes_[activeNode_];
        node.x = juce::jlimit(0.04f, 0.96f, (event.position.x - bounds.getX()) / bounds.getWidth());
        node.y = juce::jlimit(0.08f, 0.82f, (event.position.y - bounds.getY()) / bounds.getHeight());
        refreshAdjacentRelations(activeNode_);
        if (onPositionChanged) onPositionChanged(activeNode_, node.x, node.y);
        repaint();
    }

    // O retângulo da BIBLIOTECA, o mesmo do paint e do mouseDown.
    juce::Rectangle<float> libraryButtonBounds() const {
        const auto panel = getLocalBounds().toFloat().reduced(16.0f, 8.0f);
        const auto layout = performanceGraphLayout();
        return panel.withTrimmedTop(panel.getHeight() - layout.libraryTopInset)
                    .withHeight(layout.libraryHeight).reduced(4.0f, 2.0f);
    }

    // A BIBLIOTECA acende sob o mouse e o cursor vira a mão (5/11).
    void mouseMove(const juce::MouseEvent& event) override {
        const bool over = libraryButtonBounds().contains(event.position);
        if (over != libraryHover_) {
            libraryHover_ = over;
            setMouseCursor(over ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
            repaint(libraryButtonBounds().expanded(3.0f).toNearestInt());
        }
    }

    void mouseExit(const juce::MouseEvent&) override {
        if (!libraryHover_) return;
        libraryHover_ = false;
        setMouseCursor(juce::MouseCursor::NormalCursor);
        repaint(libraryButtonBounds().expanded(3.0f).toNearestInt());
    }

    void mouseUp(const juce::MouseEvent& event) override {
        const auto bounds = interactionBounds();
        if (paletteDrag_ < nodes_.size()) {
            const auto index = paletteDrag_;
            // Soltar a BIBLIOTECA sobre uma estrela (3/11): abre a troca daquela
            // estrela, sem precisar pô-la em foco antes. Assim a troca funciona
            // também com os 8 lugares cheios; no vazio, segue a inserção.
            if (index == 3U && bounds.contains(event.position) && onStarSwapRequested) {
                const auto target = nodeUnder(event.position, bounds, 30.0f);
                if (target < nodes_.size()) {
                    paletteDrag_ = kNoNode;
                    activeNode_ = kNoNode;
                    selectedNode_ = target;
                    selectedRelation_ = kNoNode;
                    if (onSelectionChanged) onSelectionChanged(selectedNode_, false);
                    repaint();
                    onStarSwapRequested(target);
                    return;
                }
            }
            if (bounds.contains(event.position)) {
                selectedNode_ = index;
                nodes_[index] = {
                    juce::jlimit(0.04f, 0.96f, (event.position.x - bounds.getX()) / bounds.getWidth()),
                    juce::jlimit(0.08f, 0.92f, (event.position.y - bounds.getY()) / bounds.getHeight())};
                if (index == 3U && onLibraryDrop) {
                    onLibraryDrop(nodes_[index].x, nodes_[index].y);
                    paletteDrag_ = kNoNode;
                    activeNode_ = kNoNode;
                    repaint();
                    return;
                }
                active_[index] = true;
                presences_[index] = std::max(presences_[index], 0.68);
                refreshAdjacentRelations(index);
                if (onActivationChanged)
                    onActivationChanged(index, true, presences_[index]);
                if (onPositionChanged)
                    onPositionChanged(index, nodes_[index].x, nodes_[index].y);
            } else if (index == 3U && libraryButtonBounds().contains(event.position) && onStarSwapRequested) {
                // Um clique sem estrela em foco (5/11): o menu pergunta qual
                // corpo trocar, e a troca abre para ele.
                paletteDrag_ = kNoNode;
                activeNode_ = kNoNode;
                juce::PopupMenu menu;
                menu.addSectionHeader(uiText(language_, "TROCAR A ESTRELA DE QUAL CORPO?",
                                             "SWAP THE STAR OF WHICH BODY?",
                                             "CHANGER L’ÉTOILE DE QUEL CORPS ?",
                                             "¿CAMBIAR LA ESTRELLA DE QUÉ CUERPO?"));
                for (std::size_t body = 0U; body < nodes_.size(); ++body)
                    menu.addItem(static_cast<int>(body + 1U),
                                 juce::String(static_cast<int>(body + 1U)) + utf8(" · ") + moduleNames_[body]);
                juce::Component::SafePointer<ConstellationGraph> safe(this);
                menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this)
                                       .withTargetScreenArea(localAreaToGlobal(libraryButtonBounds().toNearestInt())),
                                   [safe](int chosen) {
                    if (safe == nullptr || chosen <= 0) return;
                    const auto body = static_cast<std::size_t>(chosen - 1);
                    if (body >= safe->nodes_.size()) return;
                    safe->selectedNode_ = body;
                    safe->selectedRelation_ = kNoNode;
                    if (safe->onSelectionChanged) safe->onSelectionChanged(body, false);
                    safe->repaint();
                    if (safe->onStarSwapRequested) safe->onStarSwapRequested(body);
                });
                repaint();
                return;
            } else if (index == 3U && onLibraryAdd) {
                onLibraryAdd();
            }
            paletteDrag_ = kNoNode;
        } else if (draggedNode_ < nodes_.size() && overRack(event.position)) {
            const auto index = draggedNode_;
            active_[index] = false;
            presences_[index] = 0.0;
            if (onActivationChanged) onActivationChanged(index, false, 0.0);
        }
        activeNode_ = kNoNode;
        draggedNode_ = kNoNode;
        activeRelation_ = kNoNode;
        repaint();
    }

    bool keyPressed(const juce::KeyPress& key) override {
        const auto character = juce::CharacterFunctions::toLowerCase(key.getTextCharacter());
        if (character >= '1' && character <= '3') {
            selectedNode_ = static_cast<std::size_t>(character - '1');
            selectedRelation_ = kNoNode;
            if (onSelectionChanged) onSelectionChanged(selectedNode_, false);
            repaint();
            return true;
        }
        if (character == 'c') {
            selectedRelation_ = selectedRelation_ < relations_.size()
                ? (selectedRelation_ + 1U) % relations_.size() : 0U;
            if (onSelectionChanged) onSelectionChanged(selectedRelation_, true);
            repaint();
            return true;
        }
        if (character == 'o') {
            if (key.getModifiers().isShiftDown()) toggleOrbitDirection();
            else cycleOrbitMode();
            return true;
        }
        if (character == 'm' && selectedRelation_ < relationalPhenomena_.size()) {
            cycleRelationalPhenomenon();
            return true;
        }
        if (character == 'm' && selectedNode_ >= nodes_.size()) {
            cyclePopulationPhenomenon();
            return true;
        }
        const auto step = key.getModifiers().isShiftDown() ? 0.10f : 0.02f;
        if (key.getModifiers().isAltDown()
            && (key == juce::KeyPress::upKey || key == juce::KeyPress::downKey)
            && selectedNode_ < nodes_.size()) {
            const auto direction = key == juce::KeyPress::upKey ? step : -step;
            presences_[selectedNode_] = juce::jlimit(
                0.0, 1.0, presences_[selectedNode_] + static_cast<double>(direction));
            active_[selectedNode_] = presences_[selectedNode_] > 0.025;
            if (onPresenceChanged) onPresenceChanged(selectedNode_, presences_[selectedNode_]);
            repaint();
            return true;
        }
        if ((key == juce::KeyPress::leftKey || key == juce::KeyPress::rightKey
             || key == juce::KeyPress::upKey || key == juce::KeyPress::downKey)
            && selectedNode_ < nodes_.size()) {
            auto& node = nodes_[selectedNode_];
            if (key == juce::KeyPress::leftKey) node.x -= step;
            if (key == juce::KeyPress::rightKey) node.x += step;
            if (key == juce::KeyPress::upKey) node.y -= step;
            if (key == juce::KeyPress::downKey) node.y += step;
            node.x = juce::jlimit(0.04f, 0.96f, node.x);
            node.y = juce::jlimit(0.08f, 0.82f, node.y);
            refreshAdjacentRelations(selectedNode_);
            if (onPositionChanged) onPositionChanged(selectedNode_, node.x, node.y);
            repaint();
            return true;
        }
        if ((character == '[' || character == ']') && selectedRelation_ < relations_.size()) {
            const auto direction = character == ']' ? step : -step;
            relations_[selectedRelation_] = juce::jlimit(0.0, 1.0, relations_[selectedRelation_] + direction);
            if (onRelationChanged)
                onRelationChanged(selectedRelation_, relations_[selectedRelation_], false);
            repaint();
            return true;
        }
        return false;
    }

    void mouseWheelMove(const juce::MouseEvent& event,
                        const juce::MouseWheelDetails& wheel) override {
        if (selectedRelation_ < relationalPhenomena_.size()
            || selectedNode_ < nodes_.size()
            || !relationalPhenomenonControlBounds().contains(event.position)) return;
        if (std::fabs(wheel.deltaY) > 1.0e-6)
            adjustPopulationPhenomenon(static_cast<double>(wheel.deltaY) * 0.10);
    }
    void focusGained(juce::Component::FocusChangeType) override { repaint(); }
    void focusLost(juce::Component::FocusChangeType) override { activeNode_ = kNoNode; repaint(); }

private:
    static constexpr std::size_t kNoNode = std::numeric_limits<std::size_t>::max();

    juce::String lifeLabel(ActorLifeState life) const {
        switch (life) {
        case ActorLifeState::Emerging: return uiText(language_, "EMERGE", "EMERGING", "ÉMERGENT", "EMERGE");
        case ActorLifeState::Active: return uiText(language_, "ATIVO", "ACTIVE", "ACTIF", "ACTIVO");
        case ActorLifeState::Yielding: return uiText(language_, "CEDE", "YIELDS", "CÈDE", "CEDE");
        case ActorLifeState::Ruptured: return uiText(language_, "RASGADO", "TORN", "DÉCHIRÉ", "RASGADO");
        case ActorLifeState::Recovering: return uiText(language_, "REPARA", "REPAIRS", "RÉPARE", "REPARA");
        default: return uiText(language_, "RESERVA", "RESERVE", "RÉSERVE", "RESERVA");
        }
    }

    juce::String readinessLabel(PerformanceStemReadiness readiness) const {
        switch (readiness) {
        // Por extenso (3/11, R3): eram PREP / SIL / AUD / REJ.
        case PerformanceStemReadiness::Prepared: return uiText(language_, "PRONTO", "READY", "PRÊT", "LISTO");
        case PerformanceStemReadiness::Silent: return uiText(language_, "MUDO", "SILENT", "MUET", "MUDO");
        case PerformanceStemReadiness::Audible: return uiText(language_, "SOA", "SOUNDS", "SONNE", "SUENA");
        case PerformanceStemReadiness::Rejected: return uiText(language_, "RECUSADO", "REFUSED", "REFUSÉ", "RECHAZADO");
        default: return "N/RES";
        }
    }

    std::array<juce::Point<float>, 8> nodes_{{{0.16f, 0.48f}, {0.50f, 0.28f}, {0.84f, 0.58f}, {0.50f, 0.78f}, {0.66f, 0.66f}, {0.33f, 0.69f}, {0.73f, 0.38f}, {0.42f, 0.54f}}};
    std::array<juce::String, 8> moduleNames_{{"ACID", "GENDYN", "DRUMS", "WALSH", "MODAL", "GALO", "ATROPELO", "DEJA VU"}};
    std::array<std::uint16_t, 8> actorIds_{{1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U}};
    std::array<double, 8> stemLevels_{};
    struct SkyFocusView {
        std::size_t caller = 8U;
        std::size_t responder = 8U;
        RelationKind kind = RelationKind::Support;
    };
    std::array<SkyFocusView, 3> skyFoci_{};
    std::size_t skyFocusCount_ = 0U;
    double skyTension_ = 0.0;
    std::array<double, 8> starBrightness_{{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}};
    std::array<bool, 8> pinnedNodes_{};
    std::array<double, 8> chorusPhase_{};
    bool skyMotion_ = false;
    std::array<juce::Point<float>, 8> skyPositions_{};
    PerformanceMotionFamily collectiveFamily_ = PerformanceMotionFamily::Free;
    PerformanceConstellationMotion constellationMotion_ = PerformanceConstellationMotion::Still;
    std::array<PerformanceMotionFamily, 8> bodyFamilies_{{
        PerformanceMotionFamily::Count, PerformanceMotionFamily::Count, PerformanceMotionFamily::Count,
        PerformanceMotionFamily::Count, PerformanceMotionFamily::Count, PerformanceMotionFamily::Count,
        PerformanceMotionFamily::Count, PerformanceMotionFamily::Count}};
    static constexpr std::size_t kTrailLength = 48U;
    std::array<std::array<juce::Point<float>, kTrailLength>, 8> trails_{};
    std::size_t trailHead_ = 0U;
    std::size_t trailCount_ = 0U;
    double lastTrailMs_ = 0.0;
    bool skyChorusVisible_ = false;
    std::size_t driftNode_ = 255U;
    std::size_t eclipseNode_ = 255U;
    std::array<double, 8> stemLayerMix_{};
    double wildLevel_ = 0.0;
    std::array<ActorLifeState, 8> lifeStates_{{ActorLifeState::Active,
        ActorLifeState::Active, ActorLifeState::Dormant, ActorLifeState::Dormant, ActorLifeState::Emerging,
        ActorLifeState::Emerging, ActorLifeState::Emerging, ActorLifeState::Emerging}};
    std::array<PerformanceStemReadiness, 8> readiness_{{
        PerformanceStemReadiness::Unreserved, PerformanceStemReadiness::Unreserved,
        PerformanceStemReadiness::Unreserved, PerformanceStemReadiness::Unreserved,
        PerformanceStemReadiness::Unreserved, PerformanceStemReadiness::Unreserved,
        PerformanceStemReadiness::Unreserved, PerformanceStemReadiness::Unreserved}};
    juce::String trajectoryText_{utf8("TRAJETÓRIA · PARADA")};
    std::array<juce::String, 16> relationNames_{{utf8("MATÉRIA"), "TEMPO", utf8("ESPAÇO"), utf8("ÓRBITA"), utf8("CONTÁGIO"), utf8("VESTÍGIO"), utf8("HERANÇA"), utf8("FRICÇÃO"), utf8("INFILTRAÇÃO"), utf8("ESCAPE"), utf8("DIAGONAL"), utf8("PONTE"), utf8("CRUZAMENTO"), utf8("SALTO"), utf8("FECHO"), utf8("RETORNO")}};
    std::array<juce::String, 16> relationEffects_{{
        utf8("FILTRA E ESPESSA"), utf8("DESLOCA O RELÓGIO"), utf8("MOVE O ESTÉREO"),
        utf8("APROXIMA E SIMBIOSE"), utf8("TRANSMITE PULSO"), utf8("DEIXA MEMÓRIA"),
        utf8("ABRE A COORTE"), utf8("DESGASTA O TEMPO"), utf8("CONTAMINA O ESPAÇO"),
        utf8("LIBERA O CORPO"), utf8("DESVIA A MATÉRIA"), utf8("UNE OS CORPOS"),
        utf8("FRATURA O FLUXO"), utf8("SALTA O VIZINHO"), utf8("FECHA A FORMA"),
        utf8("DEVOLVE O PULSO")}};
    std::array<double, 8> presences_{{0.62, 0.55, 0.0, 0.0, 0.52, 0.50, 0.48, 0.46}};
    std::array<OrbitMode, 8> motionFields_{{OrbitMode::Orbit, OrbitMode::Orbit,
        OrbitMode::Orbit, OrbitMode::Orbit, OrbitMode::Orbit, OrbitMode::Orbit,
        OrbitMode::Orbit, OrbitMode::Orbit}};
    std::array<bool, 8> motionForwards_{{true, true, true, true, true, true, true, true}};
    // A body is visible in the live constellation only after the current
    // scene/seed grants it presence or the performer inserts it explicitly.
    // Do not let the expanded topology turn dormant reserve bodies into faint
    // decorative cables at startup.
    std::array<bool, 8> active_{{true, true, false, false, false, false, false, false}};
    // Topologia canônica da coorte expandida: a reserva não inventa cabos;
    // estes pares correspondem às relações declaradas pela seed/fábrica.
    std::array<double, 16> relations_{{0.52, 0.46, 0.36, 0.42, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};
    std::array<std::size_t, 16> relationSources_{{0U, 1U, 0U, 0U, 1U, 2U, 0U, 4U, 5U, 6U, 0U, 1U, 1U, 2U, 3U, 4U}};
    std::array<std::size_t, 16> relationDestinations_{{1U, 2U, 2U, 3U, 3U, 3U, 4U, 5U, 6U, 7U, 5U, 4U, 5U, 6U, 7U, 6U}};
    std::array<PerformanceRelationalPhenomenon, 16> relationalPhenomena_{{
        PerformanceRelationalPhenomenon::Repel,
        PerformanceRelationalPhenomenon::Synchronize,
        PerformanceRelationalPhenomenon::Orbit,
        PerformanceRelationalPhenomenon::Orbit,
        PerformanceRelationalPhenomenon::Attract,
        PerformanceRelationalPhenomenon::Chase,
        PerformanceRelationalPhenomenon::Attract,
        PerformanceRelationalPhenomenon::Synchronize,
        PerformanceRelationalPhenomenon::SpiralIn,
        PerformanceRelationalPhenomenon::Escape}};
    std::array<bool, 16> relationalPhenomenaEnabled_{};
    PerformancePopulationPhenomenon populationPhenomenon_
        = PerformancePopulationPhenomenon::CollectiveRotate;
    bool populationPhenomenonEnabled_ = false;
    double populationPhenomenonIntensity_ = 0.65;
    std::size_t activeNode_ = kNoNode;
    std::size_t draggedNode_ = kNoNode;
    std::size_t activeRelation_ = kNoNode;
    std::size_t selectedNode_ = 0U;
    std::size_t selectedRelation_ = kNoNode;
    std::size_t paletteDrag_ = kNoNode;
    bool libraryHover_ = false;
    juce::Point<float> dragGhost_{};
    float relationDragStartX_ = 0.0f;
    double relationDragStartValue_ = 0.0;
    double motionPhase_ = 0.0;
    double fieldForceX_ = 0.0;
    double fieldForceY_ = 0.0;
    double orbitPhaseOffset_ = 0.0;
    OrbitMode collectiveMotionField_ = OrbitMode::Orbit;
    SupportLanguage language_ = SupportLanguage::Portuguese;

    juce::Rectangle<float> interactionBounds() const {
        auto bounds = getLocalBounds().toFloat().reduced(16.0f, 8.0f);
        bounds.removeFromTop(42.0f);
        // Reserve the complete lower strip for library, cue and log text.
        // The fourth rack card used to end inside the library box because
        // only the log's 22 px were removed here.
        bounds.removeFromBottom(static_cast<float>(performanceGraphLayout().interactionBottomInset));
        bounds = bounds.reduced(112.0f, 0.0f);
        return bounds;
    }

    juce::Rectangle<float> rackCell(std::size_t index) const {
        const auto panel = getLocalBounds().toFloat().reduced(16.0f, 8.0f);
        const auto field = interactionBounds();
        if (index >= nodes_.size() || field.isEmpty()) return {};
        // Eight module boxes are part of the field's vocabulary. Keep them in
        // two side columns so the centre remains available for authored
        // bodies, cables and collective movement. Every index has a concrete
        // target; returning an empty rectangle here used to hide modules 6–8.
        constexpr float cardWidth = 98.0f;
        constexpr float cardHeight = 44.0f;
        const auto row = index < 4U ? index : index - 4U;
        const auto x = index < 4U ? panel.getX() : panel.getRight() - cardWidth;
        const auto rowHeight = field.getHeight() / 4.0f;
        const auto y = field.getY() + static_cast<float>(row) * rowHeight
            + (rowHeight - cardHeight) * 0.5f;
        return {x, y, cardWidth, cardHeight};
    }

    juce::Rectangle<float> movementControlBounds() const {
        const auto panel = getLocalBounds().toFloat().reduced(16.0f, 8.0f);
        return {panel.getRight() - 108.0f, panel.getY() + 4.0f, 104.0f, 20.0f};
    }
    // Giro e viagem da constelação inteira, à esquerda da DANÇA (só com a
    // trama).
    juce::Rectangle<float> constellationMotionBounds() const {
        const auto movement = movementControlBounds();
        return movement.translated(-movement.getWidth() - 8.0f, 0.0f);
    }

    juce::Rectangle<float> relationalPhenomenonControlBounds() const {
        const auto panel = getLocalBounds().toFloat().reduced(16.0f, 8.0f);
        return {panel.getX() + 4.0f, panel.getY() + 4.0f, 158.0f, 20.0f};
    }

    bool overRack(juce::Point<float> point) const {
        for (std::size_t index = 0; index < nodes_.size(); ++index)
            if (rackCell(index).expanded(18.0f).contains(point)) return true;
        return false;
    }

    // The first four nodes are production stems. The following four are real
    // internal lanes folded into those stems, so their visual presence must
    // not depend on a stem-presence telemetry slot that does not exist.
    bool visibleConstellationNode(std::size_t index) const noexcept {
        return index < active_.size() && (active_[index] || index >= 4U);
    }

    juce::Point<float> screenPoint(std::size_t index, juce::Rectangle<float> bounds) const {
        return screenPoint(nodes_[index].x, nodes_[index].y, bounds);
    }

    static juce::Point<float> screenPoint(double x, double y, juce::Rectangle<float> bounds) {
        return {bounds.getX() + static_cast<float>(x) * bounds.getWidth(),
                bounds.getY() + static_cast<float>(y) * bounds.getHeight()};
    }

    juce::String performanceCue() const {
        const auto modePrefix = uiText(language_, "DANÇA · ", "DANCE · ", "DANSE · ", "DANZA · ")
            + localizedMotionField(language_, orbitMode()) + utf8("   ·   ");
        if (selectedRelation_ < relations_.size()) {
            const auto source = relationSources_[selectedRelation_];
            const auto destination = relationDestinations_[selectedRelation_];
            return modePrefix + uiText(language_, "CABO EM FOCO · ARRASTE HORIZONTALMENTE PARA MUDAR ",
                "CABLE IN FOCUS · DRAG HORIZONTALLY TO CHANGE ",
                "CÂBLE FOCAL · GLISSEZ HORIZONTALEMENT POUR MODIFIER ",
                "CABLE EN FOCO · ARRASTRE HORIZONTALMENTE PARA CAMBIAR ")
                + moduleNames_[source] + utf8(" ↔ ") + moduleNames_[destination];
        }
        for (std::size_t index = 0U; index < active_.size(); ++index) {
            if (!active_[index])
            return modePrefix + uiText(language_, "PRÓXIMO GESTO · ARRASTE ", "NEXT GESTURE · DRAG ",
                "GESTE SUIVANT · GLISSEZ ", "PRÓXIMO GESTO · ARRASTRE ") + moduleNames_[index]
                    + uiText(language_, " DA RESERVA PARA O CAMPO · ELE ENTRA NO SOM",
                        " FROM RESERVE INTO THE FIELD · IT ENTERS THE SOUND",
                        " DE LA RÉSERVE VERS LE CHAMP · IL ENTRE DANS LE SON",
                        " DE LA RESERVA AL CAMPO · ENTRA EN EL SONIDO");
        }
        if (selectedNode_ < nodes_.size() && active_[selectedNode_])
            return modePrefix + uiText(language_, "NÓ EM FOCO · ABRIR, RASGAR E REPARAR ATINGEM ",
                "NODE IN FOCUS · OPEN, TEAR AND REPAIR AFFECT ",
                "NŒUD FOCAL · OUVRIR, DÉCHIRER ET RÉPARER AGISSENT SUR ",
                "NODO EN FOCO · ABRIR, RASGAR Y REPARAR ACTÚAN SOBRE ") + moduleNames_[selectedNode_];
        return modePrefix + uiText(language_, "PRÓXIMO GESTO · ARRASTE UM NÓ PARA REESCREVER AS RELAÇÕES",
            "NEXT GESTURE · DRAG A NODE TO REWRITE RELATIONS",
            "GESTE SUIVANT · GLISSEZ UN NŒUD POUR RÉÉCRIRE LES RELATIONS",
            "PRÓXIMO GESTO · ARRASTRE UN NODO PARA REESCRIBIR LAS RELACIONES");
    }

    void projectGeometryFromRelations() {
        const std::array<double, 3> baseRelations{{relations_[0], relations_[1], relations_[2]}};
        const auto projected = projectConstellationRelations(baseRelations);
        for (std::size_t index = 0U; index < projected.size(); ++index)
            nodes_[index] = {static_cast<float>(projected[index].x),
                             static_cast<float>(projected[index].y)};
    }

    std::pair<std::size_t, float> closestRelation(juce::Point<float> point,
                                                   juce::Rectangle<float> bounds) const {
        std::size_t closest = kNoNode;
        float distance = std::numeric_limits<float>::max();
        for (std::size_t index = 0; index < relations_.size(); ++index) {
            if (!active_[relationSources_[index]] || !active_[relationDestinations_[index]])
                continue;
            const auto from = screenPoint(relationSources_[index], bounds);
            const auto to = screenPoint(relationDestinations_[index], bounds);
            const juce::Line<float> line(from, to);
            juce::Point<float> pointOnLine;
            const auto next = line.getDistanceFromPoint(point, pointOnLine);
            if (next < distance) {
                closest = index;
                distance = next;
            }
        }
        return {closest, distance};
    }

    void refreshAdjacentRelations(std::size_t actorIndex) {
        for (std::size_t relationIndex = 0; relationIndex < relations_.size(); ++relationIndex) {
            if (actorIndex != relationSources_[relationIndex]
                && actorIndex != relationDestinations_[relationIndex]) continue;
            if (!active_[relationSources_[relationIndex]]
                || !active_[relationDestinations_[relationIndex]]) continue;
            const auto distance = nodes_[relationSources_[relationIndex]].getDistanceFrom(
                nodes_[relationDestinations_[relationIndex]]);
            relations_[relationIndex] = constellationRelationFromDistance(distance);
            if (onRelationChanged)
                onRelationChanged(relationIndex, relations_[relationIndex], true);
        }
    }
};

// The audio callback only copies samples into JUCE's bounded ThreadedWriter
// FIFO. Opening/closing the file and all disk writes stay on the control and
// background threads respectively.
class PerformanceMixRecorder final {
public:
    PerformanceMixRecorder() : writerThread_("RasgoSynthPerformanceWriter") {}
    ~PerformanceMixRecorder() {
        stop();
        writerThread_.stopThread(2000);
    }

    bool start(const juce::File& target, double sampleRate) {
        stop();
        if (!std::isfinite(sampleRate) || sampleRate < 8000.0 || !target.getParentDirectory().createDirectory())
            return false;
        std::unique_ptr<juce::OutputStream> stream = target.createOutputStream();
        if (stream == nullptr) return false;
        const auto options = juce::AudioFormatWriter::Options{}
            .withSampleRate(sampleRate).withNumChannels(2).withBitsPerSample(24);
        juce::WavAudioFormat format;
        auto writer = format.createWriterFor(stream, options);
        if (writer == nullptr) return false;
        if (!writerThread_.isThreadRunning()) writerThread_.startThread(juce::Thread::Priority::low);
        const auto fifoSamples = juce::jlimit(8192, 1048576, static_cast<int>(std::ceil(sampleRate * 8.0)));
        writer_ = std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(writer.release(), writerThread_, fifoSamples);
        writer_->setFlushInterval(static_cast<int>(sampleRate));
        currentTake_ = target;
        acceptedSamples_.store(0U, std::memory_order_release);
        droppedBlocks_.store(0U, std::memory_order_release);
        recording_.store(true, std::memory_order_release);
        return true;
    }

    void stop() {
        recording_.store(false, std::memory_order_release);
        while (writersInCallback_.load(std::memory_order_acquire) != 0U)
            juce::Thread::sleep(1);
        writer_.reset(); // destructor flushes the FIFO on the non-audio thread.
    }

    void push(const float* left, const float* right, int samples) noexcept {
        if (!recording_.load(std::memory_order_acquire) || left == nullptr || samples <= 0) return;
        writersInCallback_.fetch_add(1U, std::memory_order_acq_rel);
        if (!recording_.load(std::memory_order_acquire)) {
            writersInCallback_.fetch_sub(1U, std::memory_order_acq_rel);
            return;
        }
        // Dither TPDF na redução para 24 bits (30/10, padrão de saída dos
        // instrumentos RASGO — Navalha 2): ±1 LSB triangular (diferença de
        // dois sorteios uniformes), determinístico, sem alocação.
        const int count = std::min(samples, static_cast<int>(ditherLeft_.size()));
        const float* sourceRight = right != nullptr ? right : left;
        for (int index = 0; index < count; ++index) {
            ditherLeft_[static_cast<std::size_t>(index)] = left[index] + nextTpdf();
            ditherRight_[static_cast<std::size_t>(index)] = sourceRight[index] + nextTpdf();
        }
        const std::array<const float*, 2> channels{{ditherLeft_.data(), ditherRight_.data()}};
        if (writer_ != nullptr && writer_->write(channels.data(), count))
            acceptedSamples_.fetch_add(static_cast<std::uint64_t>(samples), std::memory_order_relaxed);
        else
            droppedBlocks_.fetch_add(1U, std::memory_order_relaxed);
        writersInCallback_.fetch_sub(1U, std::memory_order_acq_rel);
    }

    bool recording() const noexcept { return recording_.load(std::memory_order_acquire); }
    std::uint64_t acceptedSamples() const noexcept { return acceptedSamples_.load(std::memory_order_relaxed); }
    std::uint32_t droppedBlocks() const noexcept { return droppedBlocks_.load(std::memory_order_relaxed); }
    const juce::File& currentTake() const noexcept { return currentTake_; }

private:
    float nextTpdf() noexcept {
        constexpr float lsb = 1.0f / 8388608.0f;  // 24 bits
        ditherState_ = ditherState_ * 1664525U + 1013904223U;
        const float first = static_cast<float>(ditherState_ >> 8U) / 16777216.0f;
        ditherState_ = ditherState_ * 1664525U + 1013904223U;
        const float second = static_cast<float>(ditherState_ >> 8U) / 16777216.0f;
        return (first - second) * lsb;
    }
    std::array<float, 8192> ditherLeft_{};
    std::array<float, 8192> ditherRight_{};
    std::uint32_t ditherState_ = 0x2545F491U;
    juce::TimeSliceThread writerThread_;
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> writer_;
    std::atomic<bool> recording_{false};
    std::atomic<std::uint32_t> writersInCallback_{0U};
    std::atomic<std::uint64_t> acceptedSamples_{0U};
    std::atomic<std::uint32_t> droppedBlocks_{0U};
    juce::File currentTake_;
};

// Um WAV mono de 24 bits por corpo, gravado junto com a mistura (decisão do
// autor de 28/10: um stem por estrela, para mixar e combinar). Cada arquivo é
// o corpo antes da espacialização global; a soma reconstrói a mistura mono.
// Mesmo desenho do gravador da mistura: escrita em thread própria por FIFO,
// nada de disco no callback.
class PerformanceStemRecorder final {
public:
    static constexpr std::size_t kStems = kPerformanceStemActorCapacity;

    PerformanceStemRecorder() : writerThread_("RasgoSynthStemWriter") {}
    ~PerformanceStemRecorder() {
        stop();
        writerThread_.stopThread(2000);
    }

    bool start(const juce::File& mixTake, double sampleRate,
               const std::array<juce::String, kStems>& names) {
        stop();
        if (!std::isfinite(sampleRate) || sampleRate < 8000.0) return false;
        if (!writerThread_.isThreadRunning()) writerThread_.startThread(juce::Thread::Priority::low);
        const auto fifoSamples = juce::jlimit(8192, 1048576, static_cast<int>(std::ceil(sampleRate * 8.0)));
        bool any = false;
        for (std::size_t stem = 0U; stem < kStems; ++stem) {
            auto label = names[stem].toUpperCase().retainCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ")
                             .trim().replaceCharacter(' ', '-');
            if (label.isEmpty()) label = "CORPO";
            const auto file = mixTake.getSiblingFile(mixTake.getFileNameWithoutExtension()
                + "_stem" + juce::String(static_cast<int>(stem + 1U)) + "_" + label + ".wav");
            files_[stem] = file;
            std::unique_ptr<juce::OutputStream> stream = file.createOutputStream();
            if (stream == nullptr) continue;
            const auto options = juce::AudioFormatWriter::Options{}
                .withSampleRate(sampleRate).withNumChannels(1).withBitsPerSample(24);
            juce::WavAudioFormat format;
            auto writer = format.createWriterFor(stream, options);
            if (writer == nullptr) continue;
            writers_[stem] = std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(
                writer.release(), writerThread_, fifoSamples);
            writers_[stem]->setFlushInterval(static_cast<int>(sampleRate));
            any = true;
        }
        droppedBlocks_.store(0U, std::memory_order_release);
        recording_.store(any, std::memory_order_release);
        return any;
    }

    void stop() {
        recording_.store(false, std::memory_order_release);
        while (writersInCallback_.load(std::memory_order_acquire) != 0U)
            juce::Thread::sleep(1);
        for (auto& writer : writers_) writer.reset();
    }

    void push(float* const* stems, int samples) noexcept {
        if (!recording_.load(std::memory_order_acquire) || stems == nullptr || samples <= 0) return;
        writersInCallback_.fetch_add(1U, std::memory_order_acq_rel);
        if (recording_.load(std::memory_order_acquire)) {
            for (std::size_t stem = 0U; stem < kStems; ++stem) {
                if (writers_[stem] == nullptr || stems[stem] == nullptr) continue;
                const float* channel = stems[stem];
                if (!writers_[stem]->write(&channel, samples))
                    droppedBlocks_.fetch_add(1U, std::memory_order_relaxed);
            }
        }
        writersInCallback_.fetch_sub(1U, std::memory_order_acq_rel);
    }

    bool recording() const noexcept { return recording_.load(std::memory_order_acquire); }
    std::uint32_t droppedBlocks() const noexcept { return droppedBlocks_.load(std::memory_order_relaxed); }
    const juce::File& file(std::size_t stem) const noexcept { return files_[stem < kStems ? stem : 0U]; }

private:
    juce::TimeSliceThread writerThread_;
    std::array<std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter>, kStems> writers_;
    std::array<juce::File, kStems> files_;
    std::atomic<bool> recording_{false};
    std::atomic<std::uint32_t> writersInCallback_{0U};
    std::atomic<std::uint32_t> droppedBlocks_{0U};
};

class PerformanceHostComponent final : public juce::Component,
                                       private juce::AudioIODeviceCallback,
                                       private juce::Timer {
public:
    PerformanceHostComponent() {
        setWantsKeyboardFocus(true);
        setMouseClickGrabsKeyboardFocus(true);
#if JUCE_LINUX
        // Avoid an ALSA/JUCE probe crash when the kernel exposes no sound
        // device. The host remains available as a safe waiting screen.
        const bool audioDeviceNodePresent = juce::File("/dev/snd").isDirectory();
#else
        const bool audioDeviceNodePresent = true;
#endif
        if (audioDeviceNodePresent) {
            deviceManager_ = std::make_unique<juce::AudioDeviceManager>();
            auto error = deviceManager_->initialise(0, 2, nullptr, true);
            // PipeWire's default sink is 48 kHz on the development machine.
            // Prefer that rate when the selected device exposes it: keeping
            // JUCE and the graph at the device rate avoids an unnecessary
            // 44.1 -> 48 kHz conversion in the realtime path.  Never make
            // startup depend on this preference; unsupported devices keep
            // their normal rate.
            if (error.isEmpty()) {
                if (auto* device = deviceManager_->getCurrentAudioDevice()) {
                    const auto rates = device->getAvailableSampleRates();
                    if (rates.contains(48000.0) && device->getCurrentSampleRate() != 48000.0) {
                        juce::AudioDeviceManager::AudioDeviceSetup setup;
                        deviceManager_->getAudioDeviceSetup(setup);
                        setup.sampleRate = 48000.0;
                        const auto preferredError = deviceManager_->setAudioDeviceSetup(setup, true);
                        if (preferredError.isNotEmpty())
                            deviceManager_->getAudioDeviceSetup(setup);
                    }
                }
            }
            if (error.isEmpty() && deviceManager_->getCurrentAudioDevice() != nullptr) {
                deviceManager_->addAudioCallback(this);
                audioCallbackRegistered_ = true;
            }
        }
        initialiseControls();
        syncControlsFromScene(1U, true);
        startTimerHz(15);
        // Gancho de teste (29/10): com RASGO_ASTERISMUS_AUTOPLAY=1 o app dá
        // play sozinho 2 s depois de abrir — para conferir a tela tocando
        // numa instância isolada (HOME temporário, saída muda) sem mexer no
        // mouse do autor. Sem a variável, nada muda.
        // Gancho de teste (3/11): com RASGO_ASTERISMUS_AUTOREC=1 o app aperta
        // 1 MIN sozinho — para conferir, numa instância isolada e muda, a
        // forma encaixada na duração e a pasta da tomada. O REC recusa até o
        // dispositivo de áudio estar pronto: tenta a cada segundo, por até
        // 10 s. Sem a variável, nada muda.
        if (juce::SystemStats::getEnvironmentVariable("RASGO_ASTERISMUS_AUTOREC", {}) == "1")
            for (int attempt = 1; attempt <= 10; ++attempt)
                juce::Timer::callAfterDelay(1000 + attempt * 1000,
                    [safe = juce::Component::SafePointer<PerformanceHostComponent>(this)] {
                        if (safe != nullptr && !safe->recorder_.recording() && safe->takeTargetSeconds_ <= 0.0
                            && safe->ready_.load(std::memory_order_acquire))
                            safe->startTimedTake(kTakeDurations[0]);
                    });
        // Gancho de teste (4/11): com RASGO_ASTERISMUS_SNAPSHOT=1 as dicas não
        // aparecem — para os instantâneos do site, sem que o ponteiro do
        // autor (que não é movido) deixe uma dica na imagem. Sem a variável,
        // nada muda.
        if (juce::SystemStats::getEnvironmentVariable("RASGO_ASTERISMUS_SNAPSHOT", {}) == "1")
            tooltipWindow_.setMillisecondsBeforeTipAppears(24 * 60 * 60 * 1000);
        if (juce::SystemStats::getEnvironmentVariable("RASGO_ASTERISMUS_AUTOPLAY", {}) == "1")
            juce::Timer::callAfterDelay(2000, [safe = juce::Component::SafePointer<PerformanceHostComponent>(this)] {
                if (safe != nullptr && !safe->playingRequested_) safe->togglePlaying();
            });
    }
    ~PerformanceHostComponent() override {
        stopTimer();
        if (audioCallbackRegistered_ && deviceManager_ != nullptr)
            deviceManager_->removeAudioCallback(this);
    }

    void paint(juce::Graphics& graphics) override {
        const auto bounds = getLocalBounds();
        graphics.fillAll(juce::Colour(0xff102d27));
        graphics.setColour(juce::Colour(0xffd18a45));
        graphics.drawRect(bounds.reduced(18), 2);
        const auto peakRight = juce::jmax(330, bounds.getWidth() - 950);
        const auto peakLeft = juce::jmax(258, peakRight - 150);
        const auto titleRight = peakLeft - 8;
        if (titleRight > 310)
            drawBrandTitle(graphics, {258, 20, titleRight - 258, 40});
        graphics.setFont(branding::mono(14.0f));
        const auto telemetry = controller_.readTelemetry();
        juce::String status = uiText(supportLanguage_, "HOST AGUARDANDO DISPOSITIVO DE ÁUDIO",
                                     "HOST WAITING FOR AUDIO DEVICE", "HÔTE EN ATTENTE DU PÉRIPHÉRIQUE AUDIO",
                                     "HOST ESPERANDO DISPOSITIVO DE AUDIO");
        juce::String machine;
        if (ready_.load()) {
            const auto sceneId = controller_.currentSceneId();
            const auto cpuPercent = static_cast<double>(cpuMilliPercent_.load()) / 1000.0;
            const bool morphing = telemetry.sceneId != 0U && telemetry.sceneId != sceneId;
            std::size_t preparedStems = 0U;
            for (std::size_t stem = 0U; stem < telemetry.stemCount; ++stem)
                preparedStems += telemetry.stemReadiness[stem]
                    == static_cast<std::uint8_t>(PerformanceStemReadiness::Prepared);
            const auto audibleOrSilentStems = telemetry.stemCount - preparedStems;
            status = (telemetry.playing
                    ? uiText(supportLanguage_, "PLAY · ÁUDIO ATIVO", "PLAY · AUDIO ACTIVE", "PLAY · AUDIO ACTIF", "PLAY · AUDIO ACTIVO")
                    : uiText(supportLanguage_, "STOP · SAÍDA SILENCIOSA", "STOP · SILENT OUTPUT", "STOP · SORTIE SILENCIEUSE", "STOP · SALIDA SILENCIOSA"))
                + uiText(supportLanguage_, "  ·  CENA ", "  ·  SCENE ", "  ·  SCÈNE ", "  ·  ESCENA ") + juce::String(controller_.currentSceneId())
                + utf8(" · ") + sceneName(sceneId)
                + (telemetry.wildMicros > 1000U
                   ? utf8("  ·  SELVAGEM ") + juce::String(static_cast<double>(telemetry.wildMicros) / 1000000.0, 2)
                   : juce::String{})
                + (telemetry.scarMicros > 1000U
                   ? utf8("  ·  CICATRIZ ") + juce::String(static_cast<double>(telemetry.scarMicros) / 1000000.0, 2)
                   : juce::String{})
                + (telemetry.repairMicros > 1000U
                   ? utf8("  ·  REPARO ") + juce::String(static_cast<double>(telemetry.repairMicros) / 1000000.0, 2)
                   : juce::String{})
                + (morphing ? utf8("  ·  MORPH") : juce::String{})
                + (telemetry.eventQueueDepth > 0U
                   ? utf8("  ·  GESTOS ") + juce::String(telemetry.eventQueueDepth) : juce::String{})
                + (telemetry.holding ? utf8("  ·  HOLD") : juce::String{})
                + (telemetry.panicSafe ? utf8("  ·  SAFE") : juce::String{})
                + (sceneDirty_ ? utf8("  ·  MOD") : juce::String{})
                + (recorder_.recording() ? utf8("  ·  REC") : juce::String{});
            // Duas linhas (3/11, R6): a música em cima; a máquina embaixo.
            machine = uiText(supportLanguage_, "CORPOS ", "BODIES ", "CORPS ", "CUERPOS ") + juce::String(telemetry.activeActorCount)
                + utf8("  ·  STEMS ") + juce::String(audibleOrSilentStems)
                + (preparedStems > 0U ? utf8(" +") + juce::String(preparedStems)
                                            + uiText(supportLanguage_, " PRONTOS", " READY", " PRÊTS", " LISTOS")
                                      : juce::String{})
                + uiText(supportLanguage_, "  ·  LIMITE ", "  ·  LIMIT ", "  ·  LIMITE ", "  ·  LÍMITE ") + juce::String(static_cast<int>(kPerformanceSafeBodyLimit))
                + uiText(supportLanguage_, "  ·  PERFIL ", "  ·  PROFILE ", "  ·  PROFIL ", "  ·  PERFIL ")
                + (capacityProfileForCpus(static_cast<std::size_t>(juce::SystemStats::getNumCpus()))
                       == PerformanceCapacityProfile::Performance
                   ? "PERFORMANCE" : uiText(supportLanguage_, "COMPATIBILIDADE", "COMPATIBILITY", "COMPATIBILITÉ", "COMPATIBILIDAD"))
                + uiText(supportLanguage_, "  ·  ATRASOS ", "  ·  XRUNS ", "  ·  XRUNS ", "  ·  XRUNS ") + juce::String(telemetry.xrunCount)
                + (cpuPercent >= 85.0 ? uiText(supportLanguage_, "  ·  SOBRECARGA", "  ·  OVERLOAD", "  ·  SURCHARGE", "  ·  SOBRECARGA")
                   : cpuPercent >= 60.0 ? uiText(supportLanguage_, "  ·  ATENÇÃO", "  ·  ATTENTION", "  ·  ATTENTION", "  ·  ATENCIÓN") : juce::String{});
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
            machine += utf8("  ·  MODAL v1");
            if (telemetry.playing) {
                static constexpr std::array<const char*, 4> sectionsPt{{"ABERTURA", "DESENVOLVIMENTO", "CONTRASTE", "RETORNO"}};
                static constexpr std::array<const char*, 4> sectionsEn{{"OPENING", "DEVELOPMENT", "CONTRAST", "RETURN"}};
                static constexpr std::array<const char*, 4> sectionsFr{{"OUVERTURE", "DÉVELOPPEMENT", "CONTRASTE", "RETOUR"}};
                static constexpr std::array<const char*, 4> sectionsEs{{"APERTURA", "DESARROLLO", "CONTRASTE", "RETORNO"}};
                const auto section = std::min<std::size_t>(telemetry.skySection, 3U);
                status += uiText(supportLanguage_, "  ·  FORMA · ", "  ·  FORM · ", "  ·  FORME · ", "  ·  FORMA · ")
                    + uiText(supportLanguage_, sectionsPt[section], sectionsEn[section], sectionsFr[section], sectionsEs[section]);
                // Gravando, a peça termina sozinha depois de uma pausa longa do
                // performer (e de idade e viradas mínimas); avisar antes
                // (auditoria da interface, 29/10).
                if (recorder_.recording() && !codaRequested_ && telemetry.codaReadinessMicros >= 850000U)
                    status += uiText(supportLanguage_, "  ·  CODA EM BREVE", "  ·  CODA SOON",
                                     "  ·  CODA PROCHE", "  ·  CODA PRÓXIMA");
                if (controller_.skyWeb()) {
                    status += utf8("  ·  TEMPO ") + juce::String(telemetry.skyTempoMillis / 1000.0, 2)
                        + utf8("× · SWING ") + juce::String(static_cast<int>(telemetry.skySwingPercent)) + "%";
                    // O agrupamento do compasso (3+3+2…) e quem deriva.
                    if (telemetry.skyGroupingMask != 0U) {
                        juce::String grouping;
                        int start = 0;
                        for (int step = 1; step <= 16; ++step) {
                            if (step < 16 && (telemetry.skyGroupingMask & (1U << step)) == 0U) continue;
                            grouping += (grouping.isEmpty() ? juce::String{} : juce::String("+")) + juce::String(step - start);
                            start = step;
                        }
                        status += utf8(" · ") + grouping;
                    }
                    if (telemetry.skyDriftBody < static_cast<std::uint8_t>(PerformanceModuleBody::Count))
                        status += uiText(supportLanguage_, " · DERIVA ", " · DRIFT ", " · DÉRIVE ", " · DERIVA ")
                            + moduleDisplayName(static_cast<PerformanceModuleBody>(telemetry.skyDriftBody));
                    if (telemetry.skyEclipseBody < static_cast<std::uint8_t>(PerformanceModuleBody::Count))
                        status += uiText(supportLanguage_, " · ECLIPSE ", " · ECLIPSE ", " · ÉCLIPSE ", " · ECLIPSE ")
                            + moduleDisplayName(static_cast<PerformanceModuleBody>(telemetry.skyEclipseBody));
                }
            }
#endif
            // CPU é o último item variável do cabeçalho: leitura rápida do
            // orçamento do callback sem deixar estados posteriores truncados.
            machine += utf8("  ·  CPU ") + juce::String(cpuPercent, 1) + "%";
        }
        const auto cpuPercent = static_cast<double>(cpuMilliPercent_.load()) / 1000.0;
        // Status is a persistent reading, not an alert lamp. Changing the
        // colour of the whole line at every CPU threshold made it flicker
        // during normal realtime fluctuations; warnings remain explicit in
        // the text (ATENÇÃO, SOBRECARGA, SAFE).
        graphics.setColour(juce::Colour(0xffe7e3d5));
        // y = 62 (4/11): a 2ª linha (máquina) em y = 118 ficava sob a faixa das
        // CENAS; entre o título (até y = 60) e as CENAS há lugar para as duas.
        // A linha de status termina antes da caixa FORMA · PERCURSO / ÁLBUM
        // (5/11), que fica à esquerda de FAIXA · DURAÇÃO.
        auto statusArea = bounds.reduced(34).withTrimmedTop(28).withTrimmedRight(686);
        auto musicLine = statusArea.removeFromTop(18);
        graphics.drawFittedText(status, musicLine, juce::Justification::topLeft, 1);
        if (machine.isNotEmpty()) {
            graphics.setColour(juce::Colour(0xff9fbfb3));
            graphics.setFont(branding::mono(12.0f));
            graphics.drawFittedText(machine, statusArea.removeFromTop(16), juce::Justification::topLeft, 1);
            graphics.setFont(branding::mono(14.0f));
        }
        const auto meter = juce::Rectangle<float>(static_cast<float>(peakLeft), 40.0f,
                                                  static_cast<float>(juce::jmax(64, peakRight - peakLeft)), 12.0f);
        const auto peak = static_cast<float>(telemetry.outputPeakMicros) / 1000000.0f;
        const auto peakDb = peak > 0.000001f ? 20.0f * std::log10(peak) : -60.0f;
        const auto meterLevel = juce::jlimit(0.0f, 1.0f, (peakDb + 60.0f) / 60.0f);
        const bool clipHeld = clipHoldFrames_ > 0;
        graphics.setColour(juce::Colour(0xff183b33));
        graphics.fillRoundedRectangle(meter, 3.0f);
        graphics.setColour(telemetry.panicSafe || clipHeld ? juce::Colour(0xffb95b46) : juce::Colour(0xffd18a45));
        graphics.fillRoundedRectangle(meter.withWidth(meter.getWidth() * meterLevel), 3.0f);
        graphics.setColour(juce::Colour(0xffe7e3d5));
        graphics.setFont(branding::mono(11.0f, juce::Font::bold));
        graphics.drawText("PEAK " + juce::String(peakDb, 1)
                              + (telemetry.panicSafe ? "  SAFE" : clipHeld ? "  CLIP" : ""),
                          meter.translated(0.0f, -14.0f), juce::Justification::topLeft);
        const auto headerLayout = performanceHeaderLayout(getWidth());
        const auto recordingBox = juce::Rectangle<int>(headerLayout.recordingBox.x,
                                                        headerLayout.recordingBox.y,
                                                        headerLayout.recordingBox.width,
                                                        headerLayout.recordingBox.height);
        // A gravação em destaque (2/11, pedido do autor: "não sei quando está
        // gravando; destacar a gravação em curso, e outro destaque para quando
        // ela está em vias de finalizar"). Gravando: vermelho, ponto que pulsa
        // e o tempo da tomada. Coda pedida, ou a peça perto de terminar
        // sozinha: âmbar, pulso mais rápido. A moldura da janela acompanha.
        const bool recording = recorder_.recording();
        const bool codaSoon = recording && !codaRequested_ && telemetry.codaReadinessMicros >= 850000U;
        const bool finishing = recording && (codaRequested_ || codaSoon);
        const double now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
        const float pulse = 0.55f + 0.45f * static_cast<float>(std::sin(now * (finishing ? 9.0 : 4.0)));
        const auto takeSeconds = recording && takeSampleRate_ > 0.0
            ? static_cast<int>(static_cast<double>(recorder_.acceptedSamples()) / takeSampleRate_) : 0;
        // Com duração, o relógio conta o que falta (como no Antitotem).
        const auto shown = takeTargetSeconds_ > 0.0
            ? std::max(0, static_cast<int>(std::ceil(takeTargetSeconds_)) - takeSeconds) : takeSeconds;
        const auto clock = (takeTargetSeconds_ > 0.0 ? juce::String("-") : juce::String{})
            + juce::String(shown / 60).paddedLeft('0', 2) + ":" + juce::String(shown % 60).paddedLeft('0', 2);
        const auto alarm = finishing ? juce::Colour(0xfff0b860) : juce::Colour(0xffe0503c);
        if (recording) {
            graphics.setColour(alarm.withAlpha(0.35f + 0.5f * pulse));
            graphics.drawRect(getLocalBounds().reduced(14), 4);
        }
        graphics.setColour(!recording ? juce::Colour(0xff2c2521)
                                      : finishing ? juce::Colour(0xff4f3814) : juce::Colour(0xff4a1c1a));
        graphics.fillRoundedRectangle(recordingBox.toFloat(), 5.0f);
        graphics.setColour(!recording ? juce::Colour(0xff8b5a36) : alarm.withAlpha(0.5f + 0.5f * pulse));
        graphics.drawRoundedRectangle(recordingBox.toFloat(), 5.0f, recording ? 2.0f : 1.0f);
        {
            const auto durationBox = juce::Rectangle<int>(getWidth() - 444, 56, 208, 50);
            graphics.setColour(juce::Colour(0xff2c2521));
            graphics.fillRoundedRectangle(durationBox.toFloat(), 5.0f);
            graphics.setColour(takeTargetSeconds_ > 0.0 && recording ? alarm.withAlpha(0.5f + 0.5f * pulse)
                                                                     : juce::Colour(0xff8b5a36));
            graphics.drawRoundedRectangle(durationBox.toFloat(), 5.0f, takeTargetSeconds_ > 0.0 && recording ? 2.0f : 1.0f);
            graphics.setColour(juce::Colour(0xffd8b48c));
            graphics.setFont(branding::mono(9.0f, juce::Font::bold));
            graphics.drawText(uiText(supportLanguage_, "FAIXA · DURAÇÃO", "TRACK · DURATION", "PISTE · DURÉE",
                                     "PISTA · DURACIÓN"),
                              durationBox.reduced(8, 0).removeFromTop(13), juce::Justification::centredLeft);
            // FORMA · PERCURSO / ÁLBUM (5/11, pedido do autor: "não quero duas
            // linhas de botões em cenas"): a mesma caixa de FAIXA · DURAÇÃO,
            // à esquerda dela — o percurso e o álbum são da forma e das faixas.
            const auto formBox = juce::Rectangle<int>(getWidth() - 712, 56, 260, 50);
            graphics.setColour(juce::Colour(0xff2c2521));
            graphics.fillRoundedRectangle(formBox.toFloat(), 5.0f);
            graphics.setColour(juce::Colour(0xff8b5a36));
            graphics.drawRoundedRectangle(formBox.toFloat(), 5.0f, 1.0f);
            graphics.setColour(juce::Colour(0xffd8b48c));
            graphics.drawText(uiText(supportLanguage_, "FORMA · PERCURSO / ÁLBUM", "FORM · PATH / ALBUM",
                                     "FORME · PARCOURS / ALBUM", "FORMA · RECORRIDO / ÁLBUM"),
                              formBox.reduced(8, 0).removeFromTop(13), juce::Justification::centredLeft);
        }
        auto titleRow = recordingBox.reduced(8, 0).removeFromTop(13);
        if (recording) {
            const auto dot = titleRow.removeFromLeft(12).toFloat().withSizeKeepingCentre(8.0f, 8.0f);
            graphics.setColour(alarm.withAlpha(pulse));
            if (finishing) graphics.fillRect(dot.reduced(0.5f));  // quadrado: está acabando
            else graphics.fillEllipse(dot);                       // ponto: gravando
        }
        graphics.setColour(recording ? juce::Colour(0xfff2e7d5) : juce::Colour(0xffd8b48c));
        graphics.setFont(branding::mono(9.0f, juce::Font::bold));
        graphics.drawText(!recording ? uiText(supportLanguage_, "TRANSPORTE · REC / CODA", "TRANSPORT · REC / CODA",
                                              "TRANSPORT · REC / CODA", "TRANSPORTE · REC / CODA")
                          : codaRequested_ ? uiText(supportLanguage_, "CODA · FINALIZANDO  ", "CODA · FINISHING  ",
                                                    "CODA · FIN EN COURS  ", "CODA · FINALIZANDO  ") + clock
                          : codaSoon ? uiText(supportLanguage_, "CODA EM BREVE  ", "CODA SOON  ", "CODA PROCHE  ",
                                              "CODA PRÓXIMA  ") + clock
                          : uiText(supportLanguage_, "GRAVANDO  ", "RECORDING  ", "ENREGISTREMENT  ", "GRABANDO  ") + clock,
                          titleRow, juce::Justification::centredLeft);
        drawZone(graphics, sceneArea_, uiText(supportLanguage_, "CENAS · MORPH AJUSTÁVEL", "SCENES · ADJUSTABLE MORPH",
                                               "SCÈNES · MORPH RÉGLABLE", "ESCENAS · MORPH AJUSTABLE"));
        drawZone(graphics, gestureArea_, gestureZoneTitle());
        if (!fieldArea_.isEmpty())
            drawZone(graphics, fieldArea_,
                     uiText(supportLanguage_, "EIXO DE FORÇAS · AUTONOMIA / INTERVENÇÃO · MEMÓRIA / RUPTURA",
                            "AXIS OF FORCES · AUTONOMY / INTERVENTION · MEMORY / RUPTURE",
                            "AXE DES FORCES · AUTONOMIE / INTERVENTION · MÉMOIRE / RUPTURE",
                            "EJE DE FUERZAS · AUTONOMÍA / INTERVENCIÓN · MEMORIA / RUPTURA"));
        if (!constellationArea_.isEmpty())
            drawZone(graphics, constellationArea_, uiText(supportLanguage_, "CAMPO DE MATÉRIAS · ", "MATERIAL FIELD · ",
                                                           "CHAMP DES MATIÈRES · ", "CAMPO DE MATERIAS · ") + moduleRosterText());
        if (!macroArea_.isEmpty())
            drawZone(graphics, macroArea_, uiText(supportLanguage_, "MACROS DE VARIAÇÃO · EVENTOS", "VARIATION MACROS · EVENTS",
                                                   "MACROS DE VARIATION · ÉVÉNEMENTS", "MACROS DE VARIACIÓN · EVENTOS"));
        if (!relationArea_.isEmpty())
            drawZone(graphics, relationArea_, uiText(supportLanguage_, "CAMPO · CONSTELAÇÃO / MÓDULOS / ÓRBITAS / CONEXÕES",
                                                      "FIELD · CONSTELLATION / MODULES / ORBITS / CONNECTIONS",
                                                      "CHAMP · CONSTELLATION / MODULES / ORBITES / CONNEXIONS",
                                                      "CAMPO · CONSTELACIÓN / MÓDULOS / ÓRBITAS / CONEXIONES"));
        graphics.drawFittedText(uiText(supportLanguage_, "última ação · ", "last action · ", "dernière action · ", "última acción · ")
                                + lastAction_ + "  |  seed "
                                + juce::String(startupDocument_.territory.seed)
                                + uiText(supportLanguage_, "  |  memória ≤ ", "  |  memory ≤ ", "  |  mémoire ≤ ", "  |  memoria ≤ ")
                                + juce::String(startupDocument_.territory.identity.memoryLimit, 2)
                                + uiText(supportLanguage_, "  ·  risco ≤ ", "  ·  risk ≤ ", "  ·  risque ≤ ", "  ·  riesgo ≤ ")
                                + juce::String(startupDocument_.territory.identity.riskLimit, 2)
                                + utf8("  |  ") + trajectoryLabel(supportLanguage_, telemetry),
                                lastActionArea_, juce::Justification::centredLeft, 1);
        drawFooter(graphics);
    }

    void resized() override {
        auto area = getLocalBounds().reduced(34);
        logoMark_.setBounds(34, 22, 194, 36);
        const auto headerLayout = performanceHeaderLayout(getWidth());
        const auto setBounds = [](juce::Component& component, const PerformanceLayoutRect& bounds) {
            component.setBounds(bounds.x, bounds.y, bounds.width, bounds.height);
        };
        setBounds(seedLabel_, headerLayout.seedLabel);
        setBounds(seedEditor_, headerLayout.seedEditor);
        setBounds(seedButton_, headerLayout.alea);
        // The title and slider share one compact, exclusive 28 px header
        // target. TEMPO/BPM stays readable without stealing width from ALEA.
        tempoLabel_.setBounds(headerLayout.tempo.x, headerLayout.tempo.y,
                              juce::jmin(130, juce::jmax(92, headerLayout.tempo.width)), 10);
        tempoLabel_.setVisible(true);
        tempoSlider_.setBounds(headerLayout.tempo.x, headerLayout.tempo.y + 11,
                               headerLayout.tempo.width, 17);
        tempoEditor_.setBounds({});
        // Cabeçalho sem faixa vazia: controles principais na primeira linha e
        // a caixa TRANSPORTE / GRAVAÇÃO imediatamente abaixo, sem invadir o
        // instrumento.
        // Não alterar uma faixa sem rever as demais; controles sobrepostos
        // tornam a superfície impossível de aprender e de operar no palco.
        // MASTER uses the same compact stacked target as TEMPO: its caption
        // stays above the slider, never in GRID's exclusive horizontal area.
        masterLabel_.setBounds(headerLayout.master.x, headerLayout.master.y,
                               92, 10);
        masterLabel_.setVisible(true);
        masterLabel_.setInterceptsMouseClicks(false, false);
        masterLabel_.toFront(false);
        masterSlider_.setBounds(headerLayout.master.x, headerLayout.master.y + 11,
                                headerLayout.master.width, 17);
        setBounds(tutorialButton_, headerLayout.tutorial);
        setBounds(aboutButton_, headerLayout.about);
        setBounds(languageButton_, headerLayout.language);
        setBounds(recordButton_, headerLayout.record);
        {
            // FAIXA · 1/2/3/5 MIN, à esquerda da caixa de transporte (a linha de
            // status termina antes, em largura − 444).
            auto row = juce::Rectangle<int>(getWidth() - 440, 74, 200, 28);
            const auto width = row.getWidth() / static_cast<int>(durationButtons_.size());
            for (auto& button : durationButtons_) button.setBounds(row.removeFromLeft(width).reduced(2, 0));
            // ▶ OUVIR na linha de título da caixa FAIXA · DURAÇÃO, à direita.
            listenButton_.setBounds(getWidth() - 318, 57, 74, 15);
            // PERCURSO e ÁLBUM na caixa FORMA, à esquerda (5/11).
            auto form = juce::Rectangle<int>(getWidth() - 708, 74, 252, 28);
            percursoButton_.setBounds(form.removeFromLeft(104).reduced(2, 0));
            albumButton_.setBounds(form.reduced(2, 0));
        }
        setBounds(transportButton_, headerLayout.transport);
        // The header status line ends before the first operating band. Keep a
        // compact reserve here; the former 96 px left an unnecessary dead
        // strip between the header and the performance controls.
        area.removeFromTop(82);
        // Scene and gesture controls share one compact horizontal band. They
        // are two operating surfaces of the same instrument, not separate
        // tabs; the field below receives the recovered height.
        // Cenas e gestos são uma única faixa de ações. Mantê-la alta demais
        // alongava as caixas sem acrescentar área de toque aos botões.
        auto controlBand = area.removeFromTop(72).reduced(0, 0);
        sceneArea_ = controlBand.removeFromLeft(controlBand.getWidth() / 2);
        gestureArea_ = controlBand;
        // Uma única linha operacional: memória de cena e duração do morph
        // permanecem juntos, sem uma segunda faixa de botões desconectada.
        auto sceneRow = sceneArea_.reduced(8, 4);
        // ÁLBUM e PERCURSO saíram daqui para a caixa FORMA do cabeçalho
        // (5/11): as CENAS têm uma linha só de botões.
        sceneRow.removeFromTop(28);
        undoCaptureButton_.setBounds(sceneRow.removeFromLeft(92).reduced(2, 0));
        captureButton_.setBounds(sceneRow.removeFromLeft(96).reduced(2, 0));
        memoryButton_.setBounds(sceneRow.removeFromLeft(92).reduced(2, 0));
        morphLabel_.setBounds(sceneRow.removeFromLeft(74).reduced(1, 4));
        morphSlider_.setBounds(sceneRow.removeFromLeft(96).reduced(2, 0));
        const auto sceneButtonWidth = juce::jlimit(52, 86,
            sceneRow.getWidth() / static_cast<int>(sceneButtons_.size()));
        for (auto& button : sceneButtons_)
            button.setBounds(sceneRow.removeFromLeft(sceneButtonWidth).reduced(2, 1));
        auto gestureButtons = gestureArea_.reduced(12, 4);
        gestureButtons.removeFromTop(28);
        // Larguras por peso (auditoria da interface, 29/10): ABRIR, RASGAR e
        // REPARAR levam o nome do módulo em foco e, com largura igual, o
        // JUCE encolhia a letra deles — os sete gestos ficavam com tamanhos
        // diferentes. Com o peso, todos cabem na mesma letra.
        static constexpr std::array<int, 7> gestureWeights{{100, 150, 162, 176, 110, 126, 120}};
        int weightSum = 0;
        for (const auto weight : gestureWeights) weightSum += weight;
        const double gestureScale = std::min(1.0, static_cast<double>(gestureButtons.getWidth()) / weightSum);
        const auto gestureGroupWidth = static_cast<int>(weightSum * gestureScale);
        gestureButtons = gestureButtons.withTrimmedLeft(
            juce::jmax(0, (gestureButtons.getWidth() - gestureGroupWidth) / 2));
        for (std::size_t index = 0U; index < gestureButtons_.size(); ++index)
            gestureButtons_[index].setBounds(gestureButtons.removeFromLeft(
                static_cast<int>(gestureWeights[index] * gestureScale)).reduced(4));
        // A nota é uma assinatura junto do contorno, não um painel. Uma faixa
        // menor devolve altura útil à Constelação/Mesa sem comprimir controles.
        footerArea_ = area.removeFromBottom(16).translated(0, 12);
        // O rodapé é uma assinatura baixa; os painéis podem chegar quase até
        // ele, sem uma faixa vazia artificial no fim da superfície.
        auto lower = area.reduced(0, 6).withBottom(footerArea_.getY() - 4);
        // The Performance has one surface. The field, constellation, seed,
        // cells and performance controls remain available together; there is
        // no PALCO/CAMPO tab that hides part of the instrument.
        constexpr bool singleSurface = true;
        seedLabel_.setVisible(singleSurface);
        seedEditor_.setVisible(singleSurface);
        seedButton_.setVisible(singleSurface);
        tempoLabel_.setVisible(singleSurface);
        tempoSlider_.setVisible(singleSurface);
        tempoEditor_.setVisible(false);
        captureButton_.setVisible(singleSurface);
        undoCaptureButton_.setVisible(singleSurface);
        performanceField_.setVisible(singleSurface);
        for (auto& component : macroLabels_) component.setVisible(singleSurface);
        for (auto& component : macroSliders_) component.setVisible(singleSurface);
        for (std::size_t index = 0; index < actorLabels_.size(); ++index) {
            const bool productionStem = index < kPerformanceSafeBodyLimit;
            const bool focusedStem = productionStem && modulePanelActive_[index] && index == selectedModuleIndex_;
            actorLabels_[index].setVisible(productionStem && (singleSurface || focusedStem));
            actorSliders_[index].setVisible(productionStem && (singleSurface || focusedStem));
        }
        for (std::size_t index = 0; index < stemGainSliders_.size(); ++index) {
            const bool productionStem = index < kPerformanceSafeBodyLimit;
            const bool visible = productionStem
                && (singleSurface || (modulePanelActive_[index] && index == selectedModuleIndex_));
            stemGainSliders_[index].setVisible(visible);
            stemMuteButtons_[index].setVisible(visible);
            stemSoloButtons_[index].setVisible(visible);
        }
        // The four production stems own presence/gain/mute/solo, but the
        // contextual module inspectors belong to all eight internal lanes.
        // Keeping this loop coupled to the four stem sliders hid the local
        // controls of Modal, Galo, Atropelo and Deja Vu in the normal surface.
        for (std::size_t index = 0; index < moduleControlSliders_.size(); ++index) {
            for (std::size_t control = 0; control < 3U; ++control) {
                const bool focusedPanel = moduleInspectorAvailable(index)
                    && index == selectedModuleIndex_ && index != 3U;
                moduleControlLabels_[index][control].setVisible(focusedPanel);
                moduleControlSliders_[index][control].setVisible(focusedPanel);
            }
        }
        const bool actorSeedVariationVisible = seedVariationInspectorAvailable(selectedModuleIndex_);
        actorSeedVariationLabel_.setVisible(actorSeedVariationVisible);
        actorSeedVariationSlider_.setVisible(actorSeedVariationVisible);
        if (actorSeedVariationVisible)
            actorSeedVariationSlider_.setValue(actorSeedVariationValues_[selectedModuleIndex_],
                                                juce::dontSendNotification);
        // O quarto corpo agora participa da mesma rota genérica de oito
        // inspectors. O painel histórico específico de Walsh fica oculto para
        // não duplicar knobs sobre o mesmo stem.
        const bool walshPanel = selectedModuleIndex_ == 3U
            && selectedModuleIndex_ < modulePanelActive_.size()
            && modulePanelActive_[selectedModuleIndex_];
        for (std::size_t control = 0U; control < walshControlSliders_.size(); ++control) {
            walshControlLabels_[control].setVisible(walshPanel);
            walshControlSliders_[control].setVisible(walshPanel);
        }
        for (auto& component : relationLabels_) component.setVisible(false);
        for (auto& component : relationSliders_) component.setVisible(false);
        // The minimum profile is intentionally a reduced Campo. Below 1280x720,
        // keeping every preparation panel visible would force macro knobs and
        // relation controls into rows smaller than their interaction targets.
        // Preserve the field, mixer and focused module; secondary preparation
        // panels return when the window has enough vertical room.
        const bool compactField = getWidth() < 1280 || getHeight() < 720;
        if (compactField) {
            for (auto& component : macroLabels_) component.setVisible(false);
            for (auto& component : macroSliders_) component.setVisible(false);
            for (auto& component : relationLabels_) component.setVisible(false);
            for (auto& component : relationSliders_) component.setVisible(false);
            // A compact screen cannot carry eight mixer strips and three knobs
            // without overlap: it exposes the selected astro as one compact
            // vertical fader pair.
            for (std::size_t index = 0U; index < stemGainSliders_.size(); ++index) {
                const bool focusedStem = selectedModuleIndex_ == index && modulePanelActive_[index];
                actorLabels_[index].setVisible(focusedStem);
                actorSliders_[index].setVisible(focusedStem);
                stemGainSliders_[index].setVisible(focusedStem);
                stemMuteButtons_[index].setVisible(focusedStem);
                stemSoloButtons_[index].setVisible(focusedStem);
            }

            fieldArea_ = lower.removeFromLeft(lower.getWidth() * 58 / 100).reduced(0, 4);
            layoutField(fieldArea_);
            macroArea_ = {};
            relationArea_ = {};
            constellationArea_ = lower.reduced(0, 4);
            // TRAMA e VARIAÇÃO numa linha própria, abaixo do título (antes
            // cobriam "CAMPO DE MATÉRIAS"; auditoria da interface, 29/10).
            skyWebButton_.setBounds(constellationArea_.getX() + 12, constellationArea_.getY() + 30, 136, 20);
            mixerVariationButton_.setBounds(constellationArea_.getX() + 154, constellationArea_.getY() + 30, 186, 20);
            auto mixer = constellationArea_.reduced(12, 30).withTrimmedTop(24);
            // MESA must expose the spatial instrument itself. Previously the
            // graph was given an empty bounds rectangle here, leaving only
            // stem sliders visible and hiding the reserve cards needed for
            // module insertion/removal.
            auto graphArea = mixer.removeFromTop(mixer.getHeight() * 58 / 100);
            constellationGraph_.setBounds(graphArea.reduced(2));
            auto focusedPanel = mixer.removeFromBottom(210).reduced(2, 2);
            const bool hasFocusedStem = selectedModuleIndex_ < modulePanelActive_.size()
                && modulePanelActive_[selectedModuleIndex_];
            const auto rowHeight = mixer.getHeight() / juce::jmax(1, hasFocusedStem ? 1 : 0);
            for (std::size_t index = 0; index < stemGainSliders_.size(); ++index) {
                if (index != selectedModuleIndex_ || !modulePanelActive_[index]) continue;
                auto row = mixer.removeFromTop(rowHeight).reduced(2, 2);
                actorLabels_[index].setBounds(row.removeFromTop(18));
                auto faders = row.removeFromTop(juce::jmax(44, row.getHeight() - 30));
                const auto faderWidth = faders.getWidth() / 2;
                const auto leftFader = faders.removeFromLeft(faderWidth).reduced(2, 0);
                const auto rightFader = faders.reduced(2, 0);
                actorSliders_[index].setBounds(leftFader);
                stemGainSliders_[index].setBounds(rightFader);
                row.removeFromTop(2);
                const auto buttonHeight = juce::jmin(24, row.getHeight());
                const auto leftButton = juce::Rectangle<int>(leftFader.getX(), row.getY(),
                                                              leftFader.getWidth(), buttonHeight).reduced(2, 1);
                const auto rightButton = juce::Rectangle<int>(rightFader.getX(), row.getY(),
                                                               rightFader.getWidth(), buttonHeight).reduced(2, 1);
                stemMuteButtons_[index].setBounds(leftButton);
                stemSoloButtons_[index].setBounds(rightButton);
            }

            if (seedVariationInspectorAvailable(selectedModuleIndex_)) {
                auto seedVariation = focusedPanel.removeFromTop(26).reduced(6, 2);
                actorSeedVariationLabel_.setBounds(seedVariation.removeFromLeft(128));
                actorSeedVariationSlider_.setBounds(seedVariation.reduced(2, 0));
            }
            if (selectedModuleIndex_ < moduleControlSliders_.size()
                && selectedModuleIndex_ != 3U && moduleInspectorAvailable(selectedModuleIndex_)) {
                auto moduleControls = focusedPanel.reduced(8, 8);
                const auto controlWidth = moduleControls.getWidth() / 3;
                for (std::size_t control = 0; control < 3U; ++control) {
                    moduleControlLabels_[selectedModuleIndex_][control].setVisible(true);
                    moduleControlSliders_[selectedModuleIndex_][control].setVisible(true);
                    auto controlCell = moduleControls.removeFromLeft(controlWidth).reduced(2, 0);
                    moduleControlLabels_[selectedModuleIndex_][control].setBounds(controlCell.removeFromTop(20));
                    moduleControlSliders_[selectedModuleIndex_][control].setBounds(
                        readableKnobBounds(controlCell));
                }
            }
            if (walshPanel) {
                auto moduleControls = focusedPanel.reduced(8, 8);
                const auto controlWidth = moduleControls.getWidth() / 3;
                for (std::size_t control = 0U; control < walshControlSliders_.size(); ++control) {
                    walshControlLabels_[control].setVisible(true);
                    walshControlSliders_[control].setVisible(true);
                    auto controlCell = moduleControls.removeFromLeft(controlWidth).reduced(2, 0);
                    walshControlLabels_[control].setBounds(controlCell.removeFromTop(20));
                    walshControlSliders_[control].setBounds(readableKnobBounds(controlCell));
                }
            }
            lastActionArea_ = {};
            return;
        }
        juce::Rectangle<int> actionArea;
        juce::Rectangle<int> overviewArea;
        // The single surface keeps the force field, macros, active modules and
        // relation graph visible together. No mode or hidden second tab exists.
        actionArea = lower.removeFromLeft(lower.getWidth() * 27 / 100).reduced(0, 4);
        overviewArea = lower.removeFromRight(lower.getWidth() * 32 / 100).reduced(0, 4);
        relationArea_ = lower.reduced(0, 4);
        // The force field remains a living surface. Its circles must receive
        // dynamic and audible behaviour, never remain static decoration.
        fieldArea_ = actionArea.removeFromTop(actionArea.getHeight() * 58 / 100).reduced(0, 4);
        layoutField(fieldArea_);
        // Seven equal columns made the principal musical controls too small.
        // FOCO privileges two broad rows of knobs; Mesa keeps the stems and
        // graph balanced while using the same readable two-row grammar.
        macroArea_ = actionArea.reduced(0, 4);
        layoutMacroControls(macroArea_);
        constellationArea_ = overviewArea.reduced(0, 4);
        // TRAMA e VARIAÇÃO numa linha própria, abaixo do título (antes
        // cobriam "CAMPO DE MATÉRIAS"; auditoria da interface, 29/10).
        skyWebButton_.setBounds(constellationArea_.getX() + 12, constellationArea_.getY() + 30, 136, 20);
        mixerVariationButton_.setBounds(constellationArea_.getX() + 154, constellationArea_.getY() + 30, 186, 20);
        // The constellation is the visible spatial body of the instrument.
        // Give it the broad central panel; the adjacent matter panel keeps
        // the stem/module controls without shrinking the spatial drawing.
        auto actorControls = constellationArea_.reduced(10, 30).withTrimmedTop(24);
        auto focusedPanel = actorControls.removeFromBottom(210).reduced(2, 2);
        const auto actorRowHeight = actorControls.getHeight() / 2;
        for (std::size_t index = 0; index < actorSliders_.size(); ++index) {
            const auto row = static_cast<int>(index / 4U);
            const auto column = static_cast<int>(index % 4U);
            auto control = juce::Rectangle<int>(actorControls.getX() + column * actorControls.getWidth() / 4,
                                                actorControls.getY() + row * actorRowHeight,
                                                actorControls.getWidth() / 4, actorRowHeight).reduced(2, 1);
            actorLabels_[index].setBounds(control.removeFromTop(18));
            auto faders = control.removeFromTop(juce::jmax(44, control.getHeight() - 30));
            const auto faderWidth = faders.getWidth() / 2;
            const auto leftFader = faders.removeFromLeft(faderWidth).reduced(2, 0);
            const auto rightFader = faders.reduced(2, 0);
            actorSliders_[index].setBounds(leftFader);
            stemGainSliders_[index].setBounds(rightFader);
            control.removeFromTop(2);
            const auto buttonHeight = juce::jmin(24, control.getHeight());
            const auto leftButton = juce::Rectangle<int>(leftFader.getX(), control.getY(),
                                                          leftFader.getWidth(), buttonHeight).reduced(2, 1);
            const auto rightButton = juce::Rectangle<int>(rightFader.getX(), control.getY(),
                                                           rightFader.getWidth(), buttonHeight).reduced(2, 1);
            stemMuteButtons_[index].setBounds(leftButton);
            stemSoloButtons_[index].setBounds(rightButton);
        }
        if (seedVariationInspectorAvailable(selectedModuleIndex_)) {
            auto seedVariation = focusedPanel.removeFromTop(26).reduced(6, 2);
            actorSeedVariationLabel_.setBounds(seedVariation.removeFromLeft(128));
            actorSeedVariationSlider_.setBounds(seedVariation.reduced(2, 0));
        }
        if (selectedModuleIndex_ < moduleControlSliders_.size()
            && selectedModuleIndex_ != 3U && moduleInspectorAvailable(selectedModuleIndex_)) {
            auto moduleControls = focusedPanel.reduced(8, 8);
            const auto controlWidth = moduleControls.getWidth() / 3;
            for (std::size_t control = 0; control < 3U; ++control) {
                auto controlCell = moduleControls.removeFromLeft(controlWidth).reduced(2, 0);
                moduleControlLabels_[selectedModuleIndex_][control].setBounds(controlCell.removeFromTop(22));
                moduleControlSliders_[selectedModuleIndex_][control].setBounds(
                    readableKnobBounds(controlCell));
            }
        }
        if (walshPanel) {
            auto moduleControls = focusedPanel.reduced(8, 8);
            const auto controlWidth = moduleControls.getWidth() / 3;
            for (std::size_t control = 0U; control < walshControlSliders_.size(); ++control) {
                auto controlCell = moduleControls.removeFromLeft(controlWidth).reduced(2, 0);
                walshControlLabels_[control].setBounds(controlCell.removeFromTop(22));
                walshControlSliders_[control].setBounds(readableKnobBounds(controlCell));
            }
        }
        auto relationContent = relationArea_.reduced(10, 28);
        lastActionArea_ = relationContent.removeFromBottom(18);
        auto relationControls = relationContent.removeFromBottom(34);
        constellationGraph_.setBounds(relationContent.reduced(2));
        static constexpr std::array<std::uint16_t, 16> relationSources{
            1U, 2U, 1U, 1U, 2U, 3U, 1U, 5U, 6U, 7U, 1U, 2U, 2U, 3U, 4U, 5U};
        static constexpr std::array<std::uint16_t, 16> relationDestinations{
            2U, 3U, 3U, 4U, 4U, 4U, 5U, 6U, 7U, 8U, 6U, 5U, 6U, 7U, 8U, 7U};
        const auto liveRelations = constellationGraph_.relationDrawing();
        std::size_t visibleRelations = 0U;
        for (std::size_t index = 0U; index < relationSliders_.size(); ++index) {
            const auto source = relationSources[index];
            const auto destination = relationDestinations[index];
            const bool endpointsActive = source > 0U && destination > 0U
                && source <= modulePanelActive_.size() && destination <= modulePanelActive_.size()
                && modulePanelActive_[source - 1U] && modulePanelActive_[destination - 1U];
            const bool connected = endpointsActive && liveRelations[index] > 0.025;
            relationLabels_[index].setVisible(connected);
            relationSliders_[index].setVisible(connected);
            if (connected) ++visibleRelations;
        }
        const auto relationWidth = visibleRelations > 0U
            ? relationControls.getWidth() / static_cast<int>(visibleRelations) : 0;
        for (std::size_t index = 0U; index < relationSliders_.size(); ++index) {
            if (!relationSliders_[index].isVisible()) {
                relationLabels_[index].setBounds({});
                relationSliders_[index].setBounds({});
                continue;
            }
            auto control = relationControls.removeFromLeft(relationWidth);
            relationLabels_[index].setBounds(control.removeFromTop(15));
            relationSliders_[index].setBounds(control.reduced(2, 0));
        }
    }

    void mouseDown(const juce::MouseEvent&) override { grabKeyboardFocus(); }

    bool keyPressed(const juce::KeyPress& key) override {
        if (key == juce::KeyPress::spaceKey) {
            togglePlaying();
            return true;
        }

        const auto character = juce::CharacterFunctions::toLowerCase(key.getTextCharacter());
        if (key.getModifiers().isShiftDown() && character == 'z') {
            undoLastCapture();
            return true;
        }
        if (key.getModifiers().isShiftDown() && character >= '1' && character <= '4') {
            captureScene(static_cast<std::uint16_t>(character - '0'));
            return true;
        }
        switch (character) {
        case '1': recallScene(1U); return true;
        case '2': recallScene(2U); return true;
        case '3': recallScene(3U); return true;
        case '4': recallScene(4U); return true;
        case 'q': requestHold(); return true;
        case 'w': pushGesture(GestureType::Mutate); return true;
        case 'e': pushGesture(GestureType::Rasgar); return true;
        case 'r': pushGesture(GestureType::Recall); return true;
        case 's': pushGesture(GestureType::Stutter); return true;
        case 'd': pushGesture(GestureType::TempoShift); return true;
        case 'x': pushGesture(GestureType::Silence, 1.0); return true;
        case 'p': requestPanicSafe(); return true;
        case '[': adjustMorph(-0.1); return true;
        case ']': adjustMorph(0.1); return true;
        case '-': adjustMaster(-0.5); return true;
        case '=': adjustMaster(0.5); return true;
        case ',': adjustTempo(-2.0); return true;
        case '.': adjustTempo(2.0); return true;
        case 'f':
            lastAction_ = uiText(supportLanguage_, "CAMPO único · Eixo, Constelação e Gestos visíveis",
                                 "single FIELD · Axis, Constellation and Gestures visible",
                                 "CHAMP unique · Axe, Constellation et Gestes visibles",
                                 "CAMPO único · Eje, Constelación y Gestos visibles");
            repaint();
            return true;
        case 'v': toggleRecording(); return true;
        default: return false;
        }
    }

private:
    static void drawBrandTitle(juce::Graphics& graphics, juce::Rectangle<int> bounds) {
        const auto font = branding::display(34.0f, juce::Font::bold);
        graphics.setFont(font);
        float x = static_cast<float>(bounds.getX());
        const auto baseline = bounds.getBottom() - 1;
        const auto drawSegment = [&graphics, &font, &x, baseline](const juce::String& text, juce::Colour colour) {
            graphics.setColour(colour);
            graphics.drawSingleLineText(text, static_cast<int>(std::round(x)), baseline,
                                        juce::Justification::left);
            x += juce::GlyphArrangement::getStringWidth(font, text);
        };
        drawSegment("RASGO", juce::Colour(0xffe7e3d5));
        drawSegment("SYNTH", juce::Colour(0xffd18a45));
        drawSegment(juce::CharPointer_UTF8(" \xc2\xb7 ASTERISMUS"), juce::Colour(0xffe7e3d5));
    }

    static juce::String sceneName(std::uint16_t sceneId) {
        switch (sceneId) {
        case 1U: return "NASCENTE";
        case 2U: return "ESPIRAL";
        case 3U: return "RASGO";
        case 4U: return utf8("BALANÇO");
        default: return utf8("—");
        }
    }

    // rightReserve deixa livre a ponta direita do título (o botão VARIAÇÃO
    // ocupa a do Campo de Matérias); o texto que não couber termina em "…".
    static void drawZone(juce::Graphics& graphics, const juce::Rectangle<int>& area,
                         const juce::String& title, int rightReserve = 0) {
        graphics.setColour(juce::Colour(0xff183b33));
        graphics.fillRoundedRectangle(area.toFloat(), 6.0f);
        graphics.setColour(juce::Colour(0xff93683e));
        graphics.drawRoundedRectangle(area.toFloat(), 6.0f, 1.0f);
        graphics.setColour(juce::Colour(0xffd18a45));
        graphics.setFont(interfaceFont(12.0f, juce::Font::bold));
        auto titleArea = area.reduced(12).removeFromTop(20);
        titleArea.removeFromRight(rightReserve);
        graphics.drawText(title, titleArea, juce::Justification::centredLeft, true);
    }

    juce::String focusedModuleName() const {
        return selectedModuleIndex_ < moduleDisplayNames_.size()
            ? moduleDisplayNames_[selectedModuleIndex_] : utf8("COLETIVO");
    }

    juce::String gestureZoneTitle() const {
        return uiText(supportLanguage_, "GESTOS · ", "GESTURES · ", "GESTES · ", "GESTOS · ")
            + focusedModuleName()
            + (selectedActorId_ != kInvalidActorId
                ? uiText(supportLanguage_, " EM FOCO · ABRIR / RASGAR / RECORTAR",
                         " IN FOCUS · OPEN / TEAR / CUT", " AU FOYER · OUVRIR / DÉCHIRER / COUPER",
                         " EN FOCO · ABRIR / RASGAR / CORTAR")
                : uiText(supportLanguage_, " · CONSEQUÊNCIA COLETIVA", " · COLLECTIVE CONSEQUENCE",
                         " · CONSÉQUENCE COLLECTIVE", " · CONSECUENCIA COLECTIVA"));
    }

    void refreshNarrativeGestureLabels() {
        const auto target = focusedModuleName();
        // A ação continua reconhecível pelo verbo, mas o botão declara em qual
        // corpo ela cairá. Não há um seletor escondido nem um sexto botão.
        gestureButtons_[1].setButtonText(uiText(supportLanguage_, "ABRIR · ", "OPEN · ", "OUVRIR · ", "ABRIR · ") + target);
        gestureButtons_[2].setButtonText(uiText(supportLanguage_, "RASGAR · ", "TEAR · ", "DÉCHIRER · ", "RASGAR · ") + target);
        gestureButtons_[3].setButtonText(uiText(supportLanguage_, "REPARAR · ", "REPAIR · ", "RÉPARER · ", "REPARAR · ") + target);
        // TEMPO SHIFT virou DESLOCAR (3/11, R2): é um recorte que desloca o
        // pulso e a fase do stem; o andamento não muda.
        gestureButtons_[5].setButtonText(uiText(supportLanguage_, "DESLOCAR", "DISPLACE", "DÉCALER", "DESPLAZAR"));
        gestureButtons_[5].setTooltip(uiText(supportLanguage_,
            "D · desloca por um momento o pulso e a fase de ", "D · briefly displaces the pulse and phase of ",
            "D · décale un moment la pulsation et la phase de ", "D · desplaza un momento el pulso y la fase de ")
            + target + uiText(supportLanguage_, "; o andamento não muda.", "; the tempo stays.",
                              " ; le tempo ne change pas.", "; el tempo no cambia."));
        gestureButtons_[1].setTooltip(uiText(supportLanguage_, "W · abre uma variação SELVAGEM em ",
            "W · opens a WILD variation on ", "W · ouvre une variation SAUVAGE sur ", "W · abre una variación SALVAJE en ") + target
            + uiText(supportLanguage_, "; com a TRAMA, também a empurra na Constelação.",
                     "; with the WEB, it also pushes it in the Constellation.",
                     " ; avec la TRAME, la pousse aussi dans la Constellation.",
                     "; con la TRAMA, también la empuja en la Constelación."));
        gestureButtons_[2].setTooltip(uiText(supportLanguage_, "E · deposita uma cicatriz em ",
            "E · places a scar on ", "E · dépose une cicatrice sur ", "E · deja una cicatriz en ") + target);
        gestureButtons_[3].setTooltip(uiText(supportLanguage_, "R · recompõe ",
            "R · repairs ", "R · répare ", "R · repara ") + target);
    }

    void synchronizeFocusedActor() {
        // Nunca anunciar um alvo que ainda está na reserva: isso faria o
        // gesto parecer inerte, embora o contrato de ativação diga que o
        // corpo só entra no som após ser arrastado para a Constelação.
        if (selectedModuleIndex_ < modulePanelActive_.size()
            && modulePanelActive_[selectedModuleIndex_]) {
            selectedActorId_ = constellationGraph_.actorIdAt(selectedModuleIndex_);
        } else {
            const auto active = std::find(modulePanelActive_.begin(), modulePanelActive_.end(), true);
            if (active == modulePanelActive_.end()) {
                selectedActorId_ = kInvalidActorId;
                selectedModuleIndex_ = kNoStemIndex;
                constellationGraph_.selectNode(kNoStemIndex);
            } else {
                selectedModuleIndex_ = static_cast<std::size_t>(std::distance(modulePanelActive_.begin(), active));
                selectedActorId_ = constellationGraph_.actorIdAt(selectedModuleIndex_);
                constellationGraph_.selectNode(selectedModuleIndex_);
            }
        }
        refreshNarrativeGestureLabels();
    }

    void drawFooter(juce::Graphics& graphics) const {
        graphics.setColour(juce::Colour(0xff0e2c19));
        graphics.fillRoundedRectangle(footerArea_.toFloat(), 3.0f);
        graphics.setFont(interfaceFont(11.0f));
        graphics.setColour(juce::Colour(0xff8aa99b));
        auto left = footerArea_.reduced(8, 0);
        auto right = left.removeFromRight(left.getWidth() / 2);
        graphics.drawFittedText(audioFooterText_, left, juce::Justification::centredLeft, 1);
        graphics.setColour(juce::Colour(0xff8aa99b).withAlpha(0.78f));
        graphics.drawFittedText(utf8("Rasgo Synth Asterismus v" RASGO_ASTERISMUS_VERSION "  ·  Lucio de Araujo  ·  GPLv3  ·  JUCE AGPLv3  ·  2026"),
                                right, juce::Justification::centredRight, 1);
    }

    void layoutMacroControls(const juce::Rectangle<int>& area) {
        const auto content = area.reduced(10, 32);
        const auto cellWidth = content.getWidth() / 4;
        const auto rowHeight = content.getHeight() / 2;
        const auto dialDiameter = juce::jlimit(kMinimumRotaryKnobDiameter,
            kMaximumRotaryKnobDiameter,
            juce::jmin(cellWidth - 14, rowHeight - 36));
        for (std::size_t index = 0; index < macroSliders_.size(); ++index) {
            const auto row = static_cast<int>(index / 4U);
            const auto column = static_cast<int>(index % 4U);
        const auto bottomOffset = 0;
            const auto cellX = content.getX() + bottomOffset + column * cellWidth;
            const auto cellY = content.getY() + row * rowHeight;
            const juce::Rectangle<int> cell(cellX, cellY, cellWidth, rowHeight);
            macroLabels_[index].setBounds(cell.getX(), cell.getY(), cell.getWidth(), 18);
            const auto sliderWidth = dialDiameter + 18;
            const auto sliderHeight = dialDiameter + 20;
            macroSliders_[index].setBounds(cell.getCentreX() - sliderWidth / 2,
                                           cell.getY() + 18,
                                           sliderWidth, sliderHeight);
        }
    }

    void layoutField(const juce::Rectangle<int>& area) {
        // The whole zone belongs to the force field again. Seed collage was
        // removed because its delayed, indirect trajectory was not audible.
        performanceField_.setBounds(area.reduced(20, 34));
    }

    void initialiseControls() {
        // These values belong to the currently opened seed.  The field must
        // not display a fixed initial volume/origin for module 4 (or any
        // other module) after an ALEA change.
        moduleControlValues_ = initialModuleControls(startupDocument_.territory.seed);
        walshControlValues_ = moduleControlValues_[3U];
        static constexpr std::array<const char*, 4> sceneLabels{"NASCENTE", "ESPIRAL", "RASGO", "BALANÇO"};
        transportButton_.setButtonText("STOP");
        transportButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff813d34));
        transportButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
        transportButton_.setTooltip(utf8("Espaço · INICIAR abre a tomada e PLAY; STOP interrompe e preserva o WAV válido."));
        transportButton_.onClick = [this] {
            togglePlaying();
        };
        addAndMakeVisible(transportButton_);
        // REC starts a take; while it is open the same physical control asks
        // for its composed ending. Naming both states here makes CODA
        // discoverable before the performer has to guess its location.
        recordButton_.setButtonText("REC");
        recordButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff6b5035));
        recordButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
        recordButton_.setTooltip(utf8("V · REC começa a gravar (e a tocar, se estiver parado); gravando, o botão vira CODA: pede o fim composto e fecha o WAV após a cauda. PLAY só toca."));
        recordButton_.onClick = [this] { toggleRecording(); };
        addAndMakeVisible(recordButton_);
        mixerVariationButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
        mixerVariationButton_.onClick = [this] {
            controller_.setMixerVariation(!controller_.mixerVariation());
            refreshMixerVariationButton();
            lastAction_ = mixerVariationButton_.getButtonText();
            repaint();
        };
        addAndMakeVisible(mixerVariationButton_);
        skyWebButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
        skyWebButton_.onClick = [this] {
            controller_.setSkyWeb(!controller_.skyWeb());
            refreshMixerVariationButton();
            lastAction_ = skyWebButton_.getButtonText();
            repaint();
        };
        addAndMakeVisible(skyWebButton_);
        // No app a variação abre ligada; o motor e os testes a começam
        // desligada, e o músico a desliga com um clique.
        controller_.setMixerVariation(true);
        refreshMixerVariationButton();
        addAndMakeVisible(logoMark_);
        tutorialButton_.setButtonText("TUTORIAL");
        tutorialButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff315e51));
        tutorialButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
        tutorialButton_.setTooltip(utf8("Abre um guia curto do Campo, Constelação, seed, gestos e gravação."));
        tutorialButton_.onClick = [this] { showTutorial(); };
        addAndMakeVisible(tutorialButton_);
        aboutButton_.setButtonText("SOBRE");
        aboutButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff564033));
        aboutButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
        aboutButton_.setTooltip(utf8("Créditos, autoria, licença e proveniência do Rasgo Synth Asterismus."));
        aboutButton_.onClick = [this] { showAbout(); };
        addAndMakeVisible(aboutButton_);
        languageButton_.setButtonText("PT");
        languageButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff183b33));
        languageButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffd18a45));
        languageButton_.setTooltip(utf8("Idioma das janelas SOBRE e TUTORIAL · PT, EN, FR, ES."));
        languageButton_.onClick = [this] { showLanguageMenu(); };
        addAndMakeVisible(languageButton_);
        seedLabel_.setText(utf8("SEED / ASTERISMO"), juce::dontSendNotification);
        seedLabel_.setColour(juce::Label::textColourId, juce::Colour(0xffd18a45));
        seedLabel_.setFont(interfaceFont(9.0f, juce::Font::bold));
        seedLabel_.setJustificationType(juce::Justification::centredRight);
        seedLabel_.setTooltip(utf8("Escolha a seed para definir o grupo de três módulos do próximo território."));
        addAndMakeVisible(seedLabel_);
        seedEditor_.setText(juce::String(startupDocument_.territory.seed), false);
        seedEditor_.setInputRestrictions(10, "0123456789");
        // Ao clicar, o número inteiro fica selecionado (4/11): o que se digita o
        // substitui. Antes o cursor parava no ponto do clique e "1" virava
        // "1632995514" (achado na escuta guiada).
        seedEditor_.setSelectAllWhenFocused(true);
        seedEditor_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff183b33));
        seedEditor_.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff315e51));
        seedEditor_.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xffd18a45));
        seedEditor_.setColour(juce::TextEditor::textColourId, juce::Colour(0xffe7e3d5));
        seedEditor_.setFont(interfaceFont(13.0f, juce::Font::bold));
        seedEditor_.setJustification(juce::Justification::centredRight);
        seedEditor_.setTooltip(utf8("Seed do próximo território: 184 = Digital; 185 = Memória. Pare o transporte antes de aplicar."));
        seedEditor_.onReturnKey = [this] { queueSeedTerritory("enter"); };
        // Ao sair do campo, o número digitado vale (4/11): antes só o Enter o
        // aplicava, e na escuta guiada o campo mostrava "1" enquanto tocava
        // outra seed (o autor clicou em ALEA para "aplicar", e o ALEA sorteia).
        seedEditor_.onFocusLost = [this] { applyTypedSeedIfChanged("campo"); };
        addAndMakeVisible(seedEditor_);
        seedButton_.setButtonText("ALEA");
        seedButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff315e51));
        seedButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
        seedButton_.setTooltip(utf8("Lança o dado: sorteia uma seed, mostra o número e desenha um novo território."));
        seedButton_.onClick = [this] { rollAleatorySeed(); };
        addAndMakeVisible(seedButton_);
        actorSeedVariationLabel_.setText(utf8("ORIGEM ↔ VARIAÇÃO"), juce::dontSendNotification);
        actorSeedVariationLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff8aa99b));
        actorSeedVariationLabel_.setFont(interfaceFont(10.0f, juce::Font::bold));
        actorSeedVariationLabel_.setJustificationType(juce::Justification::centredLeft);
        actorSeedVariationLabel_.setTooltip(utf8("Varia somente o astro em foco a partir da SEED VIVA; 0 preserva a origem deste corpo."));
        addAndMakeVisible(actorSeedVariationLabel_);
        actorSeedVariationSlider_.setLookAndFeel(&variationTrackLook_);
        actorSeedVariationSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
        actorSeedVariationSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 68, 20);
        actorSeedVariationSlider_.setRange(0.0, 1.0, 0.01);
        actorSeedVariationSlider_.setTextValueSuffix(" SEED");
        actorSeedVariationSlider_.setColour(juce::Slider::trackColourId, juce::Colour(0xff315e51));
        actorSeedVariationSlider_.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff183b33));
        actorSeedVariationSlider_.setColour(juce::Slider::thumbColourId, juce::Colour(0xffd18a45));
        actorSeedVariationSlider_.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe7e3d5));
        actorSeedVariationSlider_.setTooltip(utf8("0 = origem deste astro; 1 = variação plena. Não move nem substitui módulos."));
        actorSeedVariationSlider_.onValueChange = [this] {
            if (selectedModuleIndex_ < actorSeedVariationValues_.size())
                setActorSeedVariationControl(selectedModuleIndex_, actorSeedVariationSlider_.getValue());
        };
        addAndMakeVisible(actorSeedVariationSlider_);
        tempoEditor_.setText(juce::String(startupDocument_.scenes[0].session.tempoBpm, 0), false);
        tempoEditor_.setInputRestrictions(3, "0123456789");
        tempoEditor_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff183b33));
        tempoEditor_.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff315e51));
        tempoEditor_.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xffd18a45));
        tempoEditor_.setColour(juce::TextEditor::textColourId, juce::Colour(0xffe7e3d5));
        tempoEditor_.setFont(interfaceFont(13.0f, juce::Font::bold));
        tempoEditor_.setTooltip(utf8("TEMPO em BPM · Enter aplica; , e . ajustam 2 BPM."));
        tempoEditor_.onReturnKey = [this] { setTempoFromSlider(); };
        addAndMakeVisible(tempoEditor_);
        tempoSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
        tempoSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 16);
        tempoSlider_.setRange(20.0, 300.0, 1.0);
        tempoSlider_.setValue(startupDocument_.scenes[0].session.tempoBpm, juce::dontSendNotification);
        tempoSlider_.textFromValueFunction = [](double value) {
            return juce::String(value, 0) + " BPM";
        };
        tempoSlider_.setColour(juce::Slider::trackColourId, juce::Colour(0xff315e51));
        tempoSlider_.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff183b33));
        tempoSlider_.setColour(juce::Slider::thumbColourId, juce::Colour(0xffd18a45));
        tempoSlider_.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe7e3d5));
        tempoSlider_.setTooltip(utf8("TEMPO variável em BPM · 20–300. Arraste o slider ou edite o valor numérico."));
        tempoSlider_.onValueChange = [this] { setTempoFromSlider(); };
        addAndMakeVisible(tempoSlider_);
        tempoLabel_.setText(utf8("TEMPO / BPM"), juce::dontSendNotification);
        tempoLabel_.setColour(juce::Label::textColourId, juce::Colour(0xffd18a45));
        tempoLabel_.setFont(interfaceFont(9.0f, juce::Font::bold));
        tempoLabel_.setJustificationType(juce::Justification::centredLeft);
        tempoLabel_.setTooltip(utf8("Velocidade da sessão em batidas por minuto."));
        addAndMakeVisible(tempoLabel_);
        masterLabel_.setText(utf8("MASTER / dB"), juce::dontSendNotification);
        masterLabel_.setColour(juce::Label::textColourId, juce::Colour(0xffd18a45));
        masterLabel_.setFont(interfaceFont(10.0f, juce::Font::bold));
        masterLabel_.setJustificationType(juce::Justification::centredLeft);
        masterSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
        masterSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 16);
        masterSlider_.setRange(-24.0, 0.0, 0.5);
        masterSlider_.setValue(-6.0, juce::dontSendNotification);
        masterSlider_.textFromValueFunction = [](double value) {
            return juce::String(value, 1) + " dB";
        };
        masterSlider_.setColour(juce::Slider::trackColourId, juce::Colour(0xff315e51));
        masterSlider_.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff183b33));
        masterSlider_.setColour(juce::Slider::thumbColourId, juce::Colour(0xffd18a45));
        masterSlider_.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe7e3d5));
        masterSlider_.setTooltip(utf8("MASTER global · −24 a 0 dB. Não é alterado pelo recall de cenas. − e = ajustam 0,5 dB."));
        masterSlider_.onValueChange = [this] {
            const auto requested = masterSlider_.getValue();
            if (!controller_.setOutputTrimDb(requested)) {
                masterSlider_.setValue(masterValue_, juce::dontSendNotification);
                lastAction_ = uiText(supportLanguage_, "MASTER não enfileirado", "MASTER not queued",
                                     "MASTER non mis en file", "MASTER no encolado");
                return;
            }
            masterValue_ = requested;
            lastAction_ = "MASTER " + juce::String(masterValue_, 1) + " dB";
        };
        addAndMakeVisible(masterLabel_);
        addAndMakeVisible(masterSlider_);
        captureButton_.setButtonText("CAPTURAR");
        captureButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff6b5035));
        captureButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
        captureButton_.setTooltip(utf8("Cena guarda BPM, 7 macros, presenças e relações do Campo. Controles locais, MASTER, GRID, Hold e morph permanecem fora. Shift+1–4 captura diretamente."));
        captureButton_.onClick = [this] { captureScene(controller_.currentSceneId()); };
        addAndMakeVisible(captureButton_);
        undoCaptureButton_.setButtonText("DESFAZER");
        undoCaptureButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff564033));
        undoCaptureButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
        undoCaptureButton_.setTooltip(utf8("Shift+Z · restaura a cena anterior à última captura desta sessão."));
        undoCaptureButton_.onClick = [this] { undoLastCapture(); };
        undoCaptureButton_.setEnabled(false);
        addAndMakeVisible(undoCaptureButton_);
        // A MEMÓRIA (2/11): guardar e lembrar sessões com nome.
        memoryButton_.setButtonText(utf8("MEMÓRIA"));
        memoryButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3f5a4a));
        memoryButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
        memoryButton_.onClick = [this] { showMemoryMenu(); };
        addAndMakeVisible(memoryButton_);
        loadMemories();
        albumButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3f5a4a));
        albumButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
        albumButton_.onClick = [this] { showAlbumMenu(); };
        for (std::size_t index = 0U; index < durationButtons_.size(); ++index) {
            auto& button = durationButtons_[index];
            button.setButtonText(juce::String(static_cast<int>(kTakeDurations[index] / 60.0)) + " MIN");
            button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3f5a4a));
            button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffb95b46));
            button.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
            button.onClick = [this, index] { startTimedTake(kTakeDurations[index]); };
            addAndMakeVisible(button);
        }
        addAndMakeVisible(albumButton_);
        listenButton_.setButtonText(uiText(supportLanguage_, "▶ OUVIR", "▶ LISTEN", "▶ ÉCOUTER", "▶ ESCUCHAR"));
        listenButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3f5a4a));
        listenButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
        listenButton_.onClick = [this] { listenToLastTake(); };
        addChildComponent(listenButton_);  // aparece quando uma tomada fecha
        percursoButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
        percursoButton_.onClick = [this] { setPercurso(!percurso_); };
        addAndMakeVisible(percursoButton_);
        refreshAlbumButtons();
        morphLabel_.setText(morphLabelText(), juce::dontSendNotification);
        morphLabel_.setColour(juce::Label::textColourId, juce::Colour(0xffd18a45));
        morphLabel_.setFont(interfaceFont(10.0f, juce::Font::bold));
        morphLabel_.setJustificationType(juce::Justification::centredRight);
        // Mesmo padrão dos sliders horizontais performáticos do Antitotem:
        // trilha limpa, sem caixa numérica comprimindo o gesto; a leitura
        // permanece no rótulo imediatamente à esquerda.
        morphSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
        morphSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        morphSlider_.setRange(0.2, 3.0, 0.1);
        morphSlider_.setValue(morphSeconds_, juce::dontSendNotification);
        morphSlider_.setTextValueSuffix(" s");
        morphSlider_.setColour(juce::Slider::trackColourId, juce::Colour(0xff315e51));
        morphSlider_.setColour(juce::Slider::thumbColourId, juce::Colour(0xffd18a45));
        morphSlider_.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe7e3d5));
        morphSlider_.setTooltip(utf8("Duração do morph de cenas · [ e ] ajustam em 0,1 s."));
        morphSlider_.onValueChange = [this] {
            morphSeconds_ = morphSlider_.getValue();
            morphLabel_.setText(morphLabelText(), juce::dontSendNotification);
            lastAction_ = "MORPH " + juce::String(morphSeconds_, 1) + " s";
        };
        addAndMakeVisible(morphLabel_);
        addAndMakeVisible(morphSlider_);
        for (std::size_t index = 0; index < sceneButtons_.size(); ++index) {
            auto& button = sceneButtons_[index];
            button.setButtonText(utf8(sceneLabels[index]));
            button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff285348));
            button.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe7e3d5));
            button.setTooltip(utf8("Tecla ") + juce::String(static_cast<int>(index + 1U))
                              + utf8(" · recupera esta cena com o morph definido acima."));
            button.onClick = [this, index] { recallScene(static_cast<std::uint16_t>(index + 1U)); };
            addAndMakeVisible(button);
        }
        static constexpr std::array<const char*, 7> gestureLabels{
            "HOLD", "MUTATE", "RASGAR", "RECALL", "STUTTER", "TEMPO SHIFT", "PANIC SAFE"};
        static constexpr std::array<GestureType, 7> gestureTypes{
            GestureType::Hold, GestureType::Mutate, GestureType::Rasgar, GestureType::Recall,
            GestureType::Stutter, GestureType::TempoShift, GestureType::PanicSafe};
        for (std::size_t index = 0; index < gestureButtons_.size(); ++index) {
            auto& button = gestureButtons_[index];
            button.setButtonText(utf8(gestureLabels[index]));
            button.setColour(juce::TextButton::buttonColourId,
                            index == 6U ? juce::Colour(0xff813d34) : juce::Colour(0xff6b5035));
            button.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2e7d5));
            static constexpr std::array<const char*, 7> gestureHints{
                "Q · HOLD congela o pulso e concentra a matéria; Q outra vez libera.",
                "W · abre uma variação SELVAGEM curta: subverte o percurso da seed; Recall a recolhe.",
                "E · injeta RASGAR e deixa uma cicatriz que decai.",
                "R · dispara RECALL: repara cicatriz e suaviza a matéria mesmo em estado limpo.",
                "S · recorta e reitera uma janela rítmica curta no stem em foco; sem mexer no campo.",
                "D · desloca temporariamente o pulso e a fase do stem em foco; BPM e desenho permanecem.",
                "P · ativa PANIC SAFE; pressione outra vez para RESET SAFE."};
            button.setTooltip(utf8(gestureHints[index]));
            button.onClick = [this, type = gestureTypes[index]] {
                if (type == GestureType::Hold) requestHold();
                else if (type == GestureType::PanicSafe) requestPanicSafe();
                else pushGesture(type);
            };
            addAndMakeVisible(button);
        }
        performanceField_.onFieldChanged = [this](double primaryX, double primaryY,
                                                   double textureX, double textureY,
                                                   PerformanceField::ChangeSource source) {
            fieldNarrativeActive_ = false;
            recombineFieldMacros(primaryX, primaryY, textureX, textureY, source);
        };
        performanceField_.setTooltip(utf8("Eixo de forças · laranja: autonomia, intervenção, memória e risco; azul: densidade, tensão, memória e espaço."));
        addAndMakeVisible(performanceField_);
        static constexpr std::array<const char*, 8> macroLabels{
            "AUTO", "INTERV.", "DENS.", "TENS.", "MEM.", "RISCO", "ESPAÇO", "PROF."};
        for (std::size_t index = 0; index < macroSliders_.size(); ++index) {
            auto& slider = macroSliders_[index];
            macroLabels_[index].setText(utf8(macroLabels[index]), juce::dontSendNotification);
            macroLabels_[index].setColour(juce::Label::textColourId, juce::Colour(0xffd18a45));
            macroLabels_[index].setFont(interfaceFont(11.0f, juce::Font::bold));
            macroLabels_[index].setJustificationType(juce::Justification::centred);
            slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 42, 16);
            slider.setRange(0.0, 1.0, 0.01);
            slider.setValue(macroValues_[index], juce::dontSendNotification);
            slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffd18a45));
            slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff315e51));
            slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe7e3d5));
            static constexpr std::array<const char*, 8> macroHints{
                "Autonomia · amplia a deriva tonal e temporal dos atores.",
                "Intervenção · injeta uma cor cruzada no sinal de cada ator.",
                "Densidade de matéria ativa.", "Tensão da sessão.",
                "Memória · deixa um resíduo/eco de fase mais presente.",
                "Risco · aumenta cicatriz, harmônicos e excitação modal.",
                "Espaço · 0 central/mono e dança lenta; 1 abre os dois canais e acelera moderadamente a dança.",
                "Profundidade · eixo Z: afasta corpos com filtragem e presença em planos distintos."};
            slider.setTooltip(utf8(macroHints[index]));
            slider.onValueChange = [this, index] { setMacroControl(index, macroSliders_[index].getValue()); };
            addAndMakeVisible(macroLabels_[index]);
            addAndMakeVisible(slider);
        }
        static constexpr std::array<const char*, 8> actorLabels{
            "ACID · 1 · PRES", "GENDYN · 2 · PRES", "DRUMS · 3 · PRES", "WALSH · 4 · PRES",
            "MODAL · 5 · PRES", "GALO · 6 · PRES", "ATROPELO · 7 · PRES", "DEJA VU · 8 · PRES"};
        static constexpr std::array<const char*, 8> actorHints{
            "Presença do stem Acid · papel Chão / Corpo.",
            "Presença do stem Gendyn · papel Atrito / Vestígio.",
            "Presença do stem Percussões · papel Contágio / Rasgo.",
            "Presença do quarto stem definido pela seed.",
            "Presença do Modal · memória e ressonância.",
            "Presença do Galo · chamada e cauda.",
            "Presença do Atropelo · altura, corpo e sílaba.",
            "Presença do Deja Vu · lembrança, brilho e duração."};
        for (std::size_t index = 0; index < actorSliders_.size(); ++index) {
            actorLabels_[index].setText(utf8(actorLabels[index]), juce::dontSendNotification);
            actorLabels_[index].setColour(juce::Label::textColourId, juce::Colour(0xff8aa99b));
            actorLabels_[index].setFont(interfaceFont(11.0f, juce::Font::bold));
            actorLabels_[index].setJustificationType(juce::Justification::centred);
            actorSliders_[index].setSliderStyle(juce::Slider::LinearVertical);
            actorSliders_[index].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 16);
            actorSliders_[index].setRange(0.0, 1.0, 0.01);
            actorSliders_[index].setValue(index == 0U ? 0.62 : index == 1U ? 0.55 : 0.0,
                                          juce::dontSendNotification);
            actorSliders_[index].setColour(juce::Slider::trackColourId, juce::Colour(0xff315e51));
            actorSliders_[index].setColour(juce::Slider::backgroundColourId, juce::Colour(0xff183b33));
            actorSliders_[index].setColour(juce::Slider::thumbColourId, juce::Colour(0xffd18a45));
            actorSliders_[index].setTooltip(utf8(actorHints[index]));
            actorSliders_[index].onValueChange = [this, index] {
                setActorPresenceControl(index, actorSliders_[index].getValue());
            };
            addAndMakeVisible(actorLabels_[index]);
            addAndMakeVisible(actorSliders_[index]);

            auto& stemGain = stemGainSliders_[index];
            stemGain.setSliderStyle(juce::Slider::LinearVertical);
            stemGain.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 46, 16);
            stemGain.setRange(0.0, 1.0, 0.01);
            stemGain.setValue(1.0, juce::dontSendNotification);
            stemGain.setTextValueSuffix({});  // "1.00 MIX" não cabia na coluna (auditoria da interface, 29/10)
            stemGain.setColour(juce::Slider::trackColourId, juce::Colour(0xff315e51));
            stemGain.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff183b33));
            stemGain.setColour(juce::Slider::thumbColourId, juce::Colour(0xff5eb6d7));
            stemGain.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe7e3d5));
            stemGain.setTooltip(utf8("Ganho do stem · 0 silencia por fade; 1 mantém o nível do ator."));
            stemGain.onValueChange = [this, index] {
                // Só a mão muda o ganho: o movimento da VARIAÇÃO é aplicado
                // sem notificação e nunca vira o valor do performer.
                setStemGainControl(index, stemGainSliders_[index].getValue());
            };
            stemGain.onDragStart = [this, index] {
                stemGainHeld_[index] = true;
                stemGainSliders_[index].setValue(stemGainValues_[index], juce::dontSendNotification);
            };
            stemGain.onDragEnd = [this, index] { stemGainHeld_[index] = false; };
            addAndMakeVisible(stemGain);

            auto& mute = stemMuteButtons_[index];
            mute.setButtonText("MUTE");
            mute.setConnectedEdges(juce::Button::ConnectedOnRight);
            mute.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff285348));
            mute.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff813d34));
            mute.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe7e3d5));
            mute.setColour(juce::TextButton::textColourOnId, juce::Colour(0xfff2e7d5));
            mute.setTooltip(utf8("Silencia somente este stem com fade; não interrompe os outros atores."));
            mute.onClick = [this, index] { toggleStemMute(index); };
            addAndMakeVisible(mute);

            auto& solo = stemSoloButtons_[index];
            solo.setButtonText("SOLO");
            solo.setConnectedEdges(juce::Button::ConnectedOnLeft);
            solo.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff315e51));
            solo.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffb66b39));
            solo.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe7e3d5));
            solo.setColour(juce::TextButton::textColourOnId, juce::Colour(0xfff2e7d5));
            solo.setTooltip(utf8("Isola este stem; clique outra vez para devolver os demais atores."));
            solo.onClick = [this, index] { toggleStemSolo(index); };
            addAndMakeVisible(solo);

            static constexpr std::array<std::array<const char*, 3>, 8> moduleControlNames{{
                {{"CORTE", "RESO", "ARTIC"}},
                {{"PASSO", "TEMPO", "FLUXO"}},
                {{"ATIV.", "FRAG.", "ABERT."}},
                {{"ALTURA", "CORPO", "DERIVA"}},
                {{"MODO", "BRILHO", "CAUDA"}},
                {{"CHAMADA", "REGISTRO", "CAUDA"}},
                {{"ALTURA", "CORPO", "SÍLABA"}},
                {{"LEMBRANÇA", "BRILHO", "DURAÇÃO"}}
            }};
            for (std::size_t control = 0; control < 3U; ++control) {
                auto& label = moduleControlLabels_[index][control];
                label.setText(utf8(moduleControlNames[index][control]), juce::dontSendNotification);
                label.setColour(juce::Label::textColourId, juce::Colour(0xffd18a45));
                label.setFont(interfaceFont(10.0f, juce::Font::bold));
                label.setJustificationType(juce::Justification::centred);
                auto& moduleSlider = moduleControlSliders_[index][control];
                // A gramática é a mesma para os nove controles locais: um
                // valor contínuo, um knob de escala legível, uma leitura abaixo.
                moduleSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
                moduleSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 18);
                moduleSlider.setRotaryParameters(juce::MathConstants<float>::pi * 1.18f,
                                                  juce::MathConstants<float>::pi * 2.82f, true);
                moduleSlider.setRange(0.0, 1.0, 0.01);
                moduleSlider.setValue(moduleControlValues_[index][control], juce::dontSendNotification);
                moduleSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff315e51));
                moduleSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffd18a45));
                moduleSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe7e3d5));
                moduleSlider.onValueChange = [this, index, control] {
                    setModuleControl(index, control,
                                     moduleControlSliders_[index][control].getValue());
                };
                addAndMakeVisible(label);
                addAndMakeVisible(moduleSlider);
            }
        }
        static constexpr std::array<const char*, 3> walshControls{{"ALTURA", "CORPO", "DERIVA"}};
        for (std::size_t control = 0U; control < walshControlSliders_.size(); ++control) {
            walshControlLabels_[control].setText(utf8(walshControls[control]), juce::dontSendNotification);
            walshControlLabels_[control].setColour(juce::Label::textColourId, juce::Colour(0xffd18a45));
            walshControlLabels_[control].setFont(interfaceFont(10.0f, juce::Font::bold));
            walshControlLabels_[control].setJustificationType(juce::Justification::centred);
            auto& slider = walshControlSliders_[control];
            slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 18);
            slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.18f,
                                       juce::MathConstants<float>::pi * 2.82f, true);
            slider.setRange(0.0, 1.0, 0.01);
            slider.setValue(walshControlValues_[control], juce::dontSendNotification);
            slider.setColour(juce::Slider::trackColourId, juce::Colour(0xff315e51));
            slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffd18a45));
            slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe7e3d5));
            slider.setTooltip(optionalModuleDisplayName_ + utf8(" · controle contextual · ")
                              + utf8(walshControls[control]));
            slider.onValueChange = [this, control] {
                const auto value = walshControlSliders_[control].getValue();
                if (controller_.setModuleControl(4U, static_cast<PerformanceModuleControl>(control), value)) {
                    walshControlValues_[control] = value;
                    lastAction_ = optionalModuleDisplayName_ + utf8(" · modular ")
                        + juce::String(static_cast<int>(control + 1U));
                } else {
                    walshControlSliders_[control].setValue(walshControlValues_[control], juce::dontSendNotification);
                }
            };
            addAndMakeVisible(walshControlLabels_[control]);
            addAndMakeVisible(slider);
        }
        refreshStemMixVisual();
        static constexpr std::array<const char*, 16> relationLabels{
            "MATÉRIA ACID → GENDYN", "TEMPO GENDYN → DRUMS", "ESPAÇO ACID → DRUMS",
            "ÓRBITA ACID → WALSH", "CONTÁGIO GENDYN → WALSH", "VESTÍGIO DRUMS → WALSH",
            "MEMÓRIA ACID → MODAL", "PULSO MODAL → GALO", "RISCO GALO → ATROPELO",
            "ESPAÇO ATROPELO → DEJA VU", "DIAGONAL ACID → ATROPELO",
            "PONTE GENDYN → MODAL", "CRUZAMENTO GENDYN → ATROPELO",
            "SALTO DRUMS → GALO", "FECHO WALSH → DEJA VU", "RETORNO MODAL → ATROPELO"};
        static constexpr std::array<double, 16> relationDefaults{
            0.52, 0.46, 0.36, 0.42, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        for (std::size_t index = 0; index < relationSliders_.size(); ++index) {
            relationLabels_[index].setText(utf8(relationLabels[index]), juce::dontSendNotification);
            relationLabels_[index].setColour(juce::Label::textColourId, juce::Colour(0xff8aa99b));
            relationLabels_[index].setFont(interfaceFont(10.0f, juce::Font::bold));
            relationLabels_[index].setJustificationType(juce::Justification::centred);
            // Com o ato da trama ("· ATRITO") o rótulo ficava cortado (4/11):
            // encolher até 65 % antes de cortar; o texto inteiro vai na dica.
            relationLabels_[index].setMinimumHorizontalScale(0.65f);
            auto& slider = relationSliders_[index];
            slider.setSliderStyle(juce::Slider::LinearBar);
            slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 42, 16);
            slider.setRange(0.0, 1.0, 0.01);
            slider.setValue(relationDefaults[index], juce::dontSendNotification);
            slider.setColour(juce::Slider::trackColourId, juce::Colour(0xff315e51));
            slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffd18a45));
            slider.setTooltip(utf8("Cabo ativo · intensidade e fenômeno alteram o comportamento compartilhado."));
            slider.onValueChange = [this, index] {
                setRelationControl(index, relationSliders_[index].getValue());
            };
            addAndMakeVisible(relationLabels_[index]);
            addAndMakeVisible(slider);
        }
        constellationGraph_.setTooltip(utf8("Este é o gráfico da CONSTELAÇÃO dentro do CAMPO. Clique no vazio para foco coletivo. Clique num cabo para escolher FENÔMENO e arraste-o horizontalmente para intensidade. Com astro em foco, DANÇA/O alterna órbita, pulso e figura-8; Shift inverte o sentido."));
        constellationGraph_.onLibraryAdd = [this] {
            if (modulePanelActive_[3U]) {
                lastAction_ = optionalModuleDisplayName_ + utf8(" já está na constelação");
                repaint();
                return;
            }
            if (!optionalCapacityAvailable()) {
                lastAction_ = uiText(supportLanguage_, "inserção bloqueada · perfil COMPATIBILIDADE",
                    "insertion blocked · COMPATIBILITY profile", "insertion bloquée · profil COMPATIBILITÉ",
                    "inserción bloqueada · perfil COMPATIBILIDAD");
                repaint();
                return;
            }
            constexpr double insertedPresence = 0.68;
            const auto moduleId = performanceStarLibraryId(
                performanceInitialAsterism(startupDocument_.territory.seed)[3]);
            if (moduleId.empty()
                || !controller_.addLibraryModule(4U, moduleId, 0.68, 0.50, insertedPresence)) {
                lastAction_ = optionalModuleDisplayName_ + utf8(" não disponível nesta seed");
                repaint();
                return;
            }
            constellationGraph_.setPresence(3U, insertedPresence);
            modulePanelActive_[3U] = true;
            selectedModuleIndex_ = 3U;
            lastAction_ = optionalModuleDisplayName_ + utf8(" inserido pela biblioteca · campo preservado");
            resized();
            repaint();
        };
        constellationGraph_.onLibraryDrop = [this](double x, double y) {
            if (modulePanelActive_[3U]) {
                lastAction_ = optionalModuleDisplayName_ + utf8(" já está na constelação");
                repaint();
                return;
            }
            if (!optionalCapacityAvailable()) {
                lastAction_ = uiText(supportLanguage_, "inserção bloqueada · perfil COMPATIBILIDADE",
                    "insertion blocked · COMPATIBILITY profile", "insertion bloquée · profil COMPATIBILITÉ",
                    "inserción bloqueada · perfil COMPATIBILIDAD");
                repaint();
                return;
            }
            const auto moduleId = performanceStarLibraryId(
                performanceInitialAsterism(startupDocument_.territory.seed)[3]);
            constexpr double insertedPresence = 0.68;
            if (moduleId.empty()
                || !controller_.addLibraryModule(4U, moduleId, x, y, insertedPresence)) {
                lastAction_ = optionalModuleDisplayName_ + utf8(" não disponível nesta seed");
                repaint();
                return;
            }
            constellationGraph_.setPresence(3U, insertedPresence);
            modulePanelActive_[3U] = true;
            selectedModuleIndex_ = 3U;
            lastAction_ = optionalModuleDisplayName_ + utf8(" inserido no ponto escolhido · campo preservado");
            resized();
            repaint();
        };
        constellationGraph_.onPresenceChanged = [this](std::size_t index, double presence) {
            if (index == 3U) {
                if (!optionalCapacityAvailable()) {
                    constellationGraph_.setPresence(3U, 0.0);
                    lastAction_ = uiText(supportLanguage_, "quarto corpo recolhido · perfil COMPATIBILIDADE",
                        "fourth body withdrawn · COMPATIBILITY profile", "quatrième corps retiré · profil COMPATIBILITÉ",
                        "cuarto cuerpo retirado · perfil COMPATIBILIDAD");
                    repaint();
                    return;
                }
                const auto boundedPresence = juce::jlimit(0.0, 1.0, presence);
                const bool changed = boundedPresence > 0.025
                    ? controller_.setActorPresence(4U, boundedPresence)
                    : controller_.removeDeskModule(4U);
                if (changed) {
                    modulePanelActive_[3U] = presence > 0.025;
                    lastAction_ = optionalModuleDisplayName_ + utf8(" presença ")
                        + juce::String(juce::jlimit(0.0, 1.0, presence), 2);
                    repaint();
                }
                return;
            }
            if (setActorPresenceControl(index, presence)) resized();
        };
        constellationGraph_.onActivationChanged = [this](std::size_t index, bool active, double presence) {
            const auto bounded = active ? juce::jlimit(0.05, 1.0, presence) : 0.0;
            if (index == 3U) {
                if (active && !optionalCapacityAvailable()) {
                    constellationGraph_.setPresence(3U, 0.0);
                    lastAction_ = uiText(supportLanguage_, "quarto corpo bloqueado · perfil COMPATIBILIDADE",
                        "fourth body blocked · COMPATIBILITY profile", "quatrième corps bloqué · profil COMPATIBILITÉ",
                        "cuarto cuerpo bloqueado · perfil COMPATIBILIDAD");
                    repaint();
                    return;
                }
                const bool changed = active
                    ? controller_.setActorPresence(4U, bounded)
                    : controller_.removeDeskModule(4U);
                if (changed) {
                    modulePanelActive_[3U] = active;
                    if (active) selectedModuleIndex_ = 3U;
                    else if (selectedModuleIndex_ == 3U) selectedModuleIndex_ = kNoStemIndex;
                    lastAction_ = optionalModuleDisplayName_ + utf8(" ")
                        + (active ? utf8("entrou na constelação") : utf8("voltou à reserva"));
                    repaint();
                } else {
                    lastAction_ = optionalModuleDisplayName_ + utf8(" não entrou · seed/capacidade/fila");
                    constellationGraph_.setPresence(3U, active ? 0.0 : presence);
                    repaint();
                }
                return;
            }
            if (index >= actorSliders_.size()) return;
            if (setActorPresenceControl(index, bounded)) {
                if (active) selectedModuleIndex_ = index;
                else if (selectedModuleIndex_ == index) selectedModuleIndex_ = kNoStemIndex;
                synchronizeFocusedActor();
                lastAction_ = moduleDisplayNames_[index]
                    + (active ? utf8(" entrou na constelação") : utf8(" voltou à reserva"));
                resized();
                repaint();
            } else {
                lastAction_ = moduleDisplayNames_[index]
                    + utf8(" não entrou · território ou fila realtime");
                constellationGraph_.setPresence(index, 0.0);
                repaint();
            }
        };
        constellationGraph_.onPositionChanged = [this](std::size_t index, double x, double y) {
            const auto actorId = constellationGraph_.actorIdAt(index);
            if (actorId == kInvalidActorId || !controller_.setActorPosition(actorId, x, y)) return;
            lastAction_ = (index == 3U ? optionalModuleDisplayName_ : moduleDisplayNames_[index])
                + utf8(" posição ") + juce::String(x, 2) + utf8("/") + juce::String(y, 2);
            appendFieldTrace(actorId, x, y);
            notePerformerActivity();
            sceneDirty_ = true;
        };
        constellationGraph_.onRelationChanged = [this](std::size_t index, double intensity,
                                                        bool fromNodeGeometry) {
            if (index < relationSliders_.size()) {
                setRelationControl(index, intensity, !fromNodeGeometry);
                return;
            }
            static constexpr std::array<std::uint16_t, 16> sources{1U, 2U, 1U, 1U, 2U, 3U, 1U, 5U, 6U, 7U, 1U, 2U, 2U, 3U, 4U, 5U};
            static constexpr std::array<std::uint16_t, 16> destinations{2U, 3U, 3U, 4U, 4U, 4U, 5U, 6U, 7U, 8U, 6U, 5U, 6U, 7U, 8U, 7U};
            if (index >= sources.size()
                || !controller_.setRelationIntensity(sources[index], destinations[index], intensity)) return;
            if (index == 3U) {
                walshRelationValue_ = intensity;
            }
            lastAction_ = uiText(supportLanguage_, "conexão escolhida · ", "selected connection · ",
                                 "connexion choisie · ", "conexión elegida · ") + juce::String(intensity, 2);
            notePerformerActivity();
            sceneDirty_ = true;
        };
        constellationGraph_.onRelationalPhenomenonChanged = [this](
            std::size_t index, PerformanceRelationalPhenomenon phenomenon) {
            static constexpr std::array<std::uint16_t, 16> sources{1U, 2U, 1U, 1U, 2U, 3U, 1U, 5U, 6U, 7U, 1U, 2U, 2U, 3U, 4U, 5U};
            static constexpr std::array<std::uint16_t, 16> destinations{2U, 3U, 3U, 4U, 4U, 4U, 5U, 6U, 7U, 8U, 6U, 5U, 6U, 7U, 8U, 7U};
            if (index >= sources.size()
                || !controller_.setRelationPhenomenon(
                    sources[index], destinations[index], phenomenon)) {
                lastAction_ = uiText(supportLanguage_, "fenômeno não entrou na fila realtime",
                    "phenomenon did not enter the realtime queue", "le phénomène n’est pas entré dans la file temps réel",
                    "el fenómeno no entró en la cola de tiempo real");
                repaint();
                return;
            }
            lastAction_ = uiText(supportLanguage_, "FENÔMENO · ", "PHENOMENON · ", "PHÉNOMÈNE · ", "FENÓMENO · ")
                + localizedRelationalPhenomenon(supportLanguage_, phenomenon)
                + uiText(supportLanguage_, " · intensidade no próprio cabo", " · intensity on its own cable",
                    " · intensité sur son propre câble", " · intensidad en su propio cable");
            notePerformerActivity();
            sceneDirty_ = true;
            repaint();
        };
        constellationGraph_.onSelectionChanged = [this](std::size_t index, bool relation) {
            if (index == kNoStemIndex) {
                selectedActorId_ = kInvalidActorId;
                selectedModuleIndex_ = kNoStemIndex;
                lastAction_ = uiText(supportLanguage_, "foco coletivo", "collective focus", "focus collectif", "foco colectivo");
                refreshNarrativeGestureLabels();
                resized();
                repaint();
                return;
            }
            if (index == 3U && !relation) {
                selectedActorId_ = 4U;
                selectedModuleIndex_ = modulePanelActive_[3U] ? 3U : kNoStemIndex;
            lastAction_ = uiText(supportLanguage_, "nó ", "node ", "nœud ", "nodo ") + optionalModuleDisplayName_
                + (modulePanelActive_[3U]
                    ? uiText(supportLanguage_, " · inspetor contextual", " · contextual inspector", " · inspecteur contextuel", " · inspector contextual")
                    : uiText(supportLanguage_, " · reserva", " · reserve", " · réserve", " · reserva"));
                resized();
                repaint();
                return;
            }
            if (index >= moduleDisplayNames_.size()) return;
            const bool internalFoldedLane = false;
            if (!relation && !modulePanelActive_[index] && !internalFoldedLane) {
                selectedActorId_ = kInvalidActorId;
                selectedModuleIndex_ = kNoStemIndex;
                lastAction_ = moduleDisplayNames_[index] + utf8(" na reserva · arraste para entrar no som");
            } else {
                selectedActorId_ = relation || internalFoldedLane
                    ? kInvalidActorId : constellationGraph_.actorIdAt(index);
                selectedModuleIndex_ = relation ? kNoStemIndex : index;
                // The graph has ten declared relations while the dedicated
                // lower mixer exposes only six names. Never index the six
                // name slots with relation 7–10: that stale UI assumption
                // caused a String crash when the performer selected a later
                // cable.
                const auto relationName = index < relationDisplayNames_.size()
                    ? relationDisplayNames_[index]
                    : utf8("RELAÇÃO ") + juce::String(static_cast<int>(index + 1U));
                lastAction_ = relation ? utf8("cabo ") + relationName
                                       : utf8("nó ") + moduleDisplayNames_[index];
            }
            refreshNarrativeGestureLabels();
            resized();
            repaint();
        };
        constellationGraph_.onStarSwapRequested = [this](std::size_t node) { showStarSwapMenu(node); };
        constellationGraph_.onConstellationMotionChanged = [this](PerformanceConstellationMotion motion) {
            if (!controller_.setConstellationMotion(motion)) {
                lastAction_ = uiText(supportLanguage_, "CÉU não entrou na fila realtime", "SKY not queued",
                                     "CIEL non mis en file", "CIELO no encolado");
                return;
            }
            lastAction_ = uiText(supportLanguage_, "CÉU · ", "SKY · ", "CIEL · ", "CIELO · ")
                + ConstellationGraph::localizedConstellationMotion(supportLanguage_, motion);
            repaint();
        };
        constellationGraph_.onMotionFamilyChanged = [this](std::size_t index, PerformanceMotionFamily family) {
            const auto actorId = index < 8U ? constellationGraph_.actorIdAt(index) : kInvalidActorId;
            if (!controller_.setMotionFamily(actorId, family)) {
                lastAction_ = uiText(supportLanguage_, "DANÇA não entrou na fila realtime", "DANCE not queued",
                                     "DANSE non mise en file", "DANZA no encolada");
                return;
            }
            lastAction_ = uiText(supportLanguage_, "DANÇA · ", "DANCE · ", "DANSE · ", "DANZA · ")
                + (index < 8U ? moduleDisplayNames_[index] : uiText(supportLanguage_, "CAMPO", "FIELD", "CHAMP", "CAMPO"))
                + " · " + ConstellationGraph::localizedMotionFamily(supportLanguage_, family);
            repaint();
        };
        constellationGraph_.onOrbitModeChanged = [this](std::size_t index, ConstellationGraph::OrbitMode mode) {
            if (index == kNoStemIndex) {
                if (!controller_.setMotionField(mode)) {
                    lastAction_ = uiText(supportLanguage_, "DANÇA coletiva não entrou na fila realtime", "collective DANCE not queued",
                                         "DANSE collective non mise en file", "DANZA colectiva no encolada");
                } else {
                    lastAction_ = uiText(supportLanguage_, "CAMPO · ", "FIELD · ", "CHAMP · ", "CAMPO · ")
                        + localizedMotionField(supportLanguage_, mode)
                        + uiText(supportLanguage_, " · transforma a constelação inteira", " · transforms the whole constellation",
                                  " · transforme toute la constellation", " · transforma toda la constelación");
                }
                repaint();
                return;
            }
            const auto actorId = constellationGraph_.actorIdAt(index);
            if (actorId == kInvalidActorId || !controller_.setActorMotionField(actorId, mode)) {
                lastAction_ = uiText(supportLanguage_, "DANÇA do corpo não entrou na fila realtime", "body DANCE not queued",
                                     "DANSE du corps non mise en file", "DANZA del cuerpo no encolada");
            } else {
                const auto name = index == 3U ? optionalModuleDisplayName_ : moduleDisplayNames_[index];
                lastAction_ = uiText(supportLanguage_, "DANÇA · ", "DANCE · ", "DANSE · ", "DANZA · ") + name + " · "
                    + localizedMotionField(supportLanguage_, mode)
                    + uiText(supportLanguage_, " · muda este corpo sem mover o desenho", " · changes this body without moving the drawing",
                              " · change ce corps sans déplacer le dessin", " · cambia este cuerpo sin mover el dibujo");
            }
            repaint();
        };
        constellationGraph_.onOrbitDirectionChanged = [this](std::size_t index, bool forward) {
            const auto actorId = constellationGraph_.actorIdAt(index);
            if (actorId == kInvalidActorId || !controller_.setActorMotionDirection(actorId, forward)) {
                lastAction_ = uiText(supportLanguage_, "SENTIDO da dança não entrou na fila realtime", "DANCE direction not queued",
                                     "sens de DANSE non mis en file", "dirección de DANZA no encolada");
            } else {
                const auto name = index == 3U ? optionalModuleDisplayName_ : moduleDisplayNames_[index];
                lastAction_ = uiText(supportLanguage_, "SENTIDO · ", "DIRECTION · ", "SENS · ", "DIRECCIÓN · ") + name
                    + (forward ? uiText(supportLanguage_, " · AVANÇA", " · FORWARD", " · AVANCE", " · AVANZA")
                               : uiText(supportLanguage_, " · RETORNA", " · RETURN", " · RETOUR", " · RETORNA"));
            }
            repaint();
        };
        addAndMakeVisible(constellationGraph_);
        refreshHoldVisual();
        refreshSafetyVisual();
        refreshCohortIdentity(startupDocument_.territory.seed);
        refreshInterfaceLanguage();
    }

    static juce::String moduleDisplayName(PerformanceModuleBody body) {
        switch (body) {
        case PerformanceModuleBody::Acid: return "ACID";
        case PerformanceModuleBody::Gendyn: return "GENDYN";
        case PerformanceModuleBody::Percussion: return "DRUMS";
        case PerformanceModuleBody::FmFlute: return "FM FLUTE";
        case PerformanceModuleBody::Theremin: return "THEREMIN";
        case PerformanceModuleBody::Scat: return "SCAT";
        case PerformanceModuleBody::Choir: return "CHOIR";
        case PerformanceModuleBody::Vosim: return "VOSIM";
        case PerformanceModuleBody::Walsh: return "WALSH";
        case PerformanceModuleBody::HardSync: return "HARD SYNC";
        case PerformanceModuleBody::AtariPunk: return "ATARI PUNK";
        case PerformanceModuleBody::DejaVu: return "DEJA VU";
        case PerformanceModuleBody::Atropelo: return "ATROPELO";
        case PerformanceModuleBody::Galo: return "GALO";
        case PerformanceModuleBody::Tuba: return "TUBA";
        case PerformanceModuleBody::Footnote: return "FOOTNOTE";
        case PerformanceModuleBody::Bluff: return "BLUFF";
        case PerformanceModuleBody::Spectral: return "SPECTRAL";
        case PerformanceModuleBody::Sieve: return "SIEVE";
        case PerformanceModuleBody::Polymeter: return "POLYMETER";
        case PerformanceModuleBody::Hocket: return "HOCKET";
        case PerformanceModuleBody::Rupture: return "RUPTURE";
        case PerformanceModuleBody::Harmonaig: return "HARMONAIG";
        case PerformanceModuleBody::Bloom: return "BLOOM";
        case PerformanceModuleBody::ThermalStatic: return "THERMAL";
        case PerformanceModuleBody::Core: return "CORE";
        case PerformanceModuleBody::Texture: return "TEXTURE";
        case PerformanceModuleBody::BassFallback: return "BASS";
        default: return utf8("MÓDULO");
        }
    }

    static const char* roleArchiveName(PerformanceRole role) noexcept {
        switch (role) {
        case PerformanceRole::Chao: return "chao";
        case PerformanceRole::Corpo: return "corpo";
        case PerformanceRole::Atrito: return "atrito";
        case PerformanceRole::Contagio: return "contagio";
        case PerformanceRole::Rasgo: return "rasgo";
        case PerformanceRole::Vestigio: return "vestigio";
        case PerformanceRole::Count: break;
        }
        return "corpo";
    }

    static const char* codaGestureArchiveName(std::uint8_t gesture) noexcept {
        switch (gesture) {
        case 0U: return "gather";
        case 1U: return "relay";
        case 2U: return "memory";
        case 3U: return "disperse";
        default: return "gather";
        }
    }

    juce::String moduleRosterText() const {
        juce::String roster = uiText(supportLanguage_, "ASTERISMO · ", "ASTERISM · ",
                                     "ASTÉRISME · ", "ASTERISMO · ");
        bool first = true;
        for (std::size_t index = 0U; index < moduleDisplayNames_.size(); ++index) {
            if (!modulePanelActive_[index]) continue;
            if (!first) roster += utf8(" / ");
            roster += moduleDisplayNames_[index];
            first = false;
        }
        return first ? roster + uiText(supportLanguage_, "nenhum corpo ativo",
                                       "no active body", "aucun corps actif",
                                       "ningún cuerpo activo") : roster;
    }

    bool seedVariationInspectorAvailable(std::size_t index) const noexcept {
        // Modules 5..8 are audible internal lanes folded into the first four
        // production stems. They expose origin/variation but no fifth output.
        return index < actorSeedVariationValues_.size()
            && (modulePanelActive_[index] || index >= 4U);
    }

    bool moduleInspectorAvailable(std::size_t index) const noexcept {
        return index < moduleControlValues_.size()
            && (modulePanelActive_[index] || index >= 4U);
    }

    juce::String localizedMacroName(std::size_t index) const {
        switch (index) {
        case 0U: return uiText(supportLanguage_, "AUTONOMIA", "AUTONOMY", "AUTONOMIE", "AUTONOMÍA");
        case 1U: return uiText(supportLanguage_, "INTERVENÇÃO", "INTERVENTION", "INTERVENTION", "INTERVENCIÓN");
        case 2U: return uiText(supportLanguage_, "DENSIDADE", "DENSITY", "DENSITÉ", "DENSIDAD");
        case 3U: return uiText(supportLanguage_, "TENSÃO", "TENSION", "TENSION", "TENSIÓN");
        case 4U: return uiText(supportLanguage_, "MEMÓRIA", "MEMORY", "MÉMOIRE", "MEMORIA");
        case 5U: return uiText(supportLanguage_, "RISCO", "RISK", "RISQUE", "RIESGO");
        case 6U: return uiText(supportLanguage_, "ESPAÇO", "SPACE", "ESPACE", "ESPACIO");
        case 7U: return uiText(supportLanguage_, "PROFUNDIDADE", "DEPTH", "PROFONDEUR", "PROFUNDIDAD");
        default: return "MACRO";
        }
    }

    juce::String localizedModuleControlName(const char* source) const {
        const auto name = utf8(source);
        if (name == "CORTE") return uiText(supportLanguage_, "CORTE", "CUT", "COUPE", "CORTE");
        if (name == "RESO") return uiText(supportLanguage_, "RESO", "RESO", "RÉSO", "RESO");
        if (name == "ARTIC") return uiText(supportLanguage_, "ARTIC", "ARTIC", "ARTIC", "ARTIC");
        if (name == "PASSO") return uiText(supportLanguage_, "PASSO", "STEP", "PAS", "PASO");
        if (name == "FLUXO") return uiText(supportLanguage_, "FLUXO", "FLOW", "FLUX", "FLUJO");
        if (name == "ATIV.") return uiText(supportLanguage_, "ATIV.", "ACTIV.", "ACTIV.", "ACTIV.");
        if (name == "FRAG.") return uiText(supportLanguage_, "FRAG.", "FRAG.", "FRAG.", "FRAG.");
        if (name == "ABERT.") return uiText(supportLanguage_, "ABERT.", "OPEN.", "OUVR.", "ABIER.");
        if (name == "ALTURA") return uiText(supportLanguage_, "ALTURA", "PITCH", "HAUTEUR", "ALTURA");
        if (name == "CORPO") return uiText(supportLanguage_, "CORPO", "BODY", "CORPS", "CUERPO");
        if (name == "DERIVA") return uiText(supportLanguage_, "DERIVA", "DRIFT", "DÉRIVE", "DERIVA");
        if (name == "MODO") return uiText(supportLanguage_, "MODO", "MODE", "MODE", "MODO");
        if (name == "BRILHO") return uiText(supportLanguage_, "BRILHO", "BRIGHT", "BRILL.", "BRILLO");
        if (name == "CAUDA") return uiText(supportLanguage_, "CAUDA", "TAIL", "QUEUE", "COLA");
        if (name == "CHAMADA") return uiText(supportLanguage_, "CHAMADA", "CALL", "APPEL", "LLAMADA");
        if (name == "REGISTRO") return uiText(supportLanguage_, "REGISTRO", "REGISTER", "REGISTRE", "REGISTRO");
        if (name == "SÍLABA") return uiText(supportLanguage_, "SÍLABA", "SYLLABLE", "SYLLABE", "SÍLABA");
        if (name == "LEMBRANÇA") return uiText(supportLanguage_, "LEMBRANÇA", "REMEMBER", "SOUVENIR", "RECUERDO");
        if (name == "DURAÇÃO") return uiText(supportLanguage_, "DURAÇÃO", "DURATION", "DURÉE", "DURACIÓN");
        return name;
    }

    // As pontas de cada um dos 16 cabos fixos (corpo de saída → de chegada).
    static constexpr std::array<std::pair<std::size_t, std::size_t>, 16> kRelationEnds{{
        {0U, 1U}, {1U, 2U}, {0U, 2U}, {0U, 3U}, {1U, 3U},
        {2U, 3U}, {0U, 4U}, {4U, 5U}, {5U, 6U}, {6U, 7U},
        {0U, 6U}, {1U, 4U}, {1U, 6U}, {2U, 5U}, {3U, 7U}, {4U, 6U}}};

    // Com a trama (3/11, R1), o rótulo do cabo diz também o ato que ele pede
    // ao diretor: o do fenômeno ligado, senão o ato natural do par — o mesmo
    // cálculo do diretor. No foco, o ato pode variar (seção, estratégia).
    juce::String cableActSuffix(std::size_t index) const {
        return cableActSuffix(index, displayedAsterism(identitySeed_));
    }

    juce::String cableActSuffix(std::size_t index, const std::array<PerformanceModuleBody, 8>& stars) const {
        if (!controller_.skyWeb() || index >= kRelationEnds.size()) return {};
        const auto [a, b] = kRelationEnds[index];
        if (a >= stars.size() || b >= stars.size()) return {};
        if (stars[a] == PerformanceModuleBody::Count || stars[b] == PerformanceModuleBody::Count) return {};
        const auto natural = PerformanceSkyDirector::naturalKind(identitySeed_, a, b, stars[a], stars[b]);
        const auto kind = constellationGraph_.relationPhenomenonEnabledAt(index)
            ? PerformanceSkyDirector::phenomenonKind(
                  static_cast<std::uint8_t>(constellationGraph_.relationPhenomenonAt(index)), natural)
            : natural;
        const auto name = constellationGraph_.skyActName(kind);
        return name.isEmpty() ? juce::String{} : utf8(" · ") + name;
    }

    // Chamado pelo timer: só reescreve o rótulo que mudou (trama, troca de
    // estrela, fenômeno, seed).
    void refreshCableActs() {
        const auto stars = displayedAsterism(identitySeed_);
        for (std::size_t index = 0U; index < relationLabels_.size(); ++index) {
            const auto text = relationDisplayNames_[index] + cableActSuffix(index, stars);
            if (relationLabels_[index].getText() != text) {
                relationLabels_[index].setText(text, juce::dontSendNotification);
                relationLabels_[index].setTooltip(text);
            }
        }
    }

    void refreshRelationInterfaceText() {
        static constexpr std::array<const char*, 16> prefixes{
            "MATÉRIA", "TEMPO", "ESPAÇO", "ÓRBITA", "CONTÁGIO", "VESTÍGIO",
            "MEMÓRIA", "PULSO", "RISCO", "ESPAÇO", "DIAGONAL", "PONTE",
            "CRUZAMENTO", "SALTO", "FECHO", "RETORNO"};
        const auto prefix = [this](std::size_t index) {
            const auto name = utf8(prefixes[index]);
            if (name == "MATÉRIA") return uiText(supportLanguage_, "MATÉRIA", "MATERIAL", "MATIÈRE", "MATERIA");
            if (name == "ESPAÇO") return uiText(supportLanguage_, "ESPAÇO", "SPACE", "ESPACE", "ESPACIO");
            if (name == "ÓRBITA") return uiText(supportLanguage_, "ÓRBITA", "ORBIT", "ORBITE", "ÓRBITA");
            if (name == "CONTÁGIO") return uiText(supportLanguage_, "CONTÁGIO", "CONTAGION", "CONTAGION", "CONTAGIO");
            if (name == "VESTÍGIO") return uiText(supportLanguage_, "VESTÍGIO", "TRACE", "TRACE", "VESTIGIO");
            if (name == "MEMÓRIA") return uiText(supportLanguage_, "MEMÓRIA", "MEMORY", "MÉMOIRE", "MEMORIA");
            if (name == "PULSO") return uiText(supportLanguage_, "PULSO", "PULSE", "POULS", "PULSO");
            if (name == "RISCO") return uiText(supportLanguage_, "RISCO", "RISK", "RISQUE", "RIESGO");
            if (name == "DIAGONAL") return uiText(supportLanguage_, "DIAGONAL", "DIAGONAL", "DIAGONALE", "DIAGONAL");
            if (name == "PONTE") return uiText(supportLanguage_, "PONTE", "BRIDGE", "PONT", "PUENTE");
            if (name == "CRUZAMENTO") return uiText(supportLanguage_, "CRUZAMENTO", "CROSSING", "CROISEMENT", "CRUCE");
            if (name == "SALTO") return uiText(supportLanguage_, "SALTO", "LEAP", "SAUT", "SALTO");
            if (name == "FECHO") return uiText(supportLanguage_, "FECHO", "CLOSURE", "CLÔTURE", "CIERRE");
            if (name == "RETORNO") return uiText(supportLanguage_, "RETORNO", "RETURN", "RETOUR", "RETORNO");
            return name;
        };
        const auto& ends = kRelationEnds;
        for (std::size_t index = 0U; index < relationDisplayNames_.size(); ++index)
            relationDisplayNames_[index] = prefix(index) + " " + moduleDisplayNames_[ends[index].first]
                + " → " + moduleDisplayNames_[ends[index].second];
        for (std::size_t index = 0U; index < relationLabels_.size(); ++index) {
            relationLabels_[index].setText(relationDisplayNames_[index] + cableActSuffix(index),
                                           juce::dontSendNotification);
            relationSliders_[index].setTooltip(relationDisplayNames_[index] + uiText(supportLanguage_,
                " · proximidade e intensidade alteram o comportamento compartilhado.",
                " · proximity and intensity change shared behaviour.",
                " · proximité et intensité modifient le comportement partagé.",
                " · proximidad e intensidad alteran el comportamiento compartido."));
        }
        constellationGraph_.setRelationDetails({{
            relationDisplayNames_[0], relationDisplayNames_[1], relationDisplayNames_[2],
            relationDisplayNames_[3], relationDisplayNames_[4], relationDisplayNames_[5]}}, {{
            uiText(supportLanguage_, "FILTRA E ESPESSA", "FILTERS AND THICKENS", "FILTRE ET ÉPAISSIT", "FILTRA Y ENGROSA"),
            uiText(supportLanguage_, "DESLOCA O RELÓGIO", "SHIFTS THE CLOCK", "DÉPLACE L’HORLOGE", "DESPLAZA EL RELOJ"),
            uiText(supportLanguage_, "MOVE O ESTÉREO", "MOVES STEREO", "DÉPLACE LE STÉRÉO", "MUEVE EL ESTÉREO"),
            uiText(supportLanguage_, "APROXIMA E SIMBIOSE", "DRAWS TOGETHER AND SYMBIOSIS", "RAPPROCHE ET SYMBIOSE", "ACERCA Y SIMBIOSIS"),
            uiText(supportLanguage_, "TRANSMITE PULSO", "TRANSMITS PULSE", "TRANSMET LE POULS", "TRANSMITE PULSO"),
            uiText(supportLanguage_, "DEIXA MEMÓRIA", "LEAVES MEMORY", "LAISSE MÉMOIRE", "DEJA MEMORIA")}});
    }

    void refreshLocalizedModuleControlText() {
        const auto asterism = displayedAsterism(startupDocument_.territory.seed);
        for (std::size_t index = 0U; index < 3U; ++index) {
            const auto starControls = performanceStarControlNames(asterism[index]);
            for (std::size_t control = 0U; control < moduleControlLabels_[index].size(); ++control) {
                const auto localized = localizedModuleControlName(starControls[control]);
                moduleControlLabels_[index][control].setText(localized, juce::dontSendNotification);
                moduleControlSliders_[index][control].setTooltip(moduleDisplayNames_[index]
                    + uiText(supportLanguage_, " · controle audível ", " · audible control ",
                             " · contrôle audible ", " · control audible ") + localized);
            }
        }
        const auto optionalControls = performanceStarControlNames(asterism[3]);
        for (std::size_t control = 0U; control < optionalControls.size(); ++control) {
            const auto localized = localizedModuleControlName(optionalControls[control]);
            moduleControlLabels_[3U][control].setText(localized, juce::dontSendNotification);
            walshControlLabels_[control].setText(localized, juce::dontSendNotification);
            moduleControlSliders_[3U][control].setTooltip(moduleDisplayNames_[3U]
                + uiText(supportLanguage_, " · controle audível ", " · audible control ",
                         " · contrôle audible ", " · control audible ") + localized);
        }
    }

    // O asterismo mostrado: o da seed com as trocas do performer (30/10).
    std::array<PerformanceModuleBody, 8> displayedAsterism(std::uint32_t seed) const {
        auto asterism = performanceInitialAsterism(seed);
        for (std::size_t index = 0U; index < asterism.size(); ++index)
            if (starSwaps_[index] != PerformanceModuleBody::Count) asterism[index] = starSwaps_[index];
        return asterism;
    }

    // O menu da troca de estrela: as estrelas que o céu sabe tocar, por
    // função; a atual marcada; e a volta à estrela da seed.
    void showStarSwapMenu(std::size_t node) {
        const auto actorId = constellationGraph_.actorIdAt(node);
        if (actorId == kInvalidActorId || node >= starSwaps_.size()) return;
        const auto shown = displayedAsterism(identitySeed_);
        const auto current = shown[node];
        // O menu da troca (2/11, gestão de 28 estrelas): a estrela atual e o
        // FIXAR no alto; as famílias em submenus, com quantas estrelas cada
        // uma tem; a volta à estrela da seed no fim.
        juce::PopupMenu menu;
        menu.addSectionHeader(uiText(supportLanguage_, "CORPO ", "BODY ", "CORPS ", "CUERPO ")
                              + juce::String(static_cast<int>(node + 1U)) + " · " + moduleDisplayName(current));
        menu.addItem(1100, uiText(supportLanguage_, "FIXAR ESTA ESTRELA (a VARIAÇÃO não a apaga)",
                                  "PIN THIS STAR (VARIATION won't dim it)",
                                  "ÉPINGLER CETTE ÉTOILE (la VARIATION ne l’éteint pas)",
                                  "FIJAR ESTA ESTRELLA (la VARIACIÓN no la apaga)"),
                     true, starPinned_[node]);
        menu.addSeparator();
        menu.addSectionHeader(uiText(supportLanguage_, "TROCAR POR", "SWAP FOR", "REMPLACER PAR", "CAMBIAR POR"));
        static constexpr std::array<PerformanceStarFunction, 5> functions{{
            PerformanceStarFunction::Tone, PerformanceStarFunction::Texture,
            PerformanceStarFunction::Voice, PerformanceStarFunction::Pulse, PerformanceStarFunction::Control}};
        for (std::size_t group = 0U; group < functions.size(); ++group) {
            juce::PopupMenu family;
            int count = 0;
            bool holdsCurrent = false;
            if (group == 0U) {
                // O Modal (30/10): um ressonador, um corpo — se outro corpo já
                // o toca, o item mostra qual e fica desligado.
                std::size_t holder = shown.size();
                for (std::size_t index = 0U; index < shown.size(); ++index)
                    if (shown[index] == PerformanceModuleBody::ModalResonator) { holder = index; break; }
                const bool elsewhere = holder < shown.size() && holder != node;
                family.addItem(900, moduleDisplayName(PerformanceModuleBody::ModalResonator)
                                   + (elsewhere ? uiText(supportLanguage_, "  · no corpo ", "  · in body ",
                                                         "  · dans le corps ", "  · en el cuerpo ")
                                                      + juce::String(static_cast<int>(holder + 1U))
                                                : juce::String{}),
                               !elsewhere, current == PerformanceModuleBody::ModalResonator);
                ++count;
                holdsCurrent = current == PerformanceModuleBody::ModalResonator;
            }
            if (functions[group] == PerformanceStarFunction::Control)
                family.addSectionHeader(uiText(supportLanguage_, "não soam: filtram os ataques das outras",
                                               "silent: they filter the others' attacks",
                                               "muettes : elles filtrent les attaques des autres",
                                               "no suenan: filtran los ataques de las otras"));
            for (std::size_t index = 0U; index < kPerformanceSkyStars.size(); ++index) {
                const auto star = kPerformanceSkyStars[index];
                if (performanceStarFunction(star) != functions[group]) continue;
                // Desde 3/11 a seed escolhe entre todas: não há mais "· biblioteca".
                constexpr bool library = false;
                family.addItem(static_cast<int>(index) + 1, moduleDisplayName(star)
                                   + (library ? uiText(supportLanguage_, "  · biblioteca", "  · library",
                                                       "  · bibliothèque", "  · biblioteca") : juce::String{}),
                               true, star == current);
                ++count;
                holdsCurrent = holdsCurrent || star == current;
            }
            menu.addSubMenu(familyName(functions[group]) + "  (" + juce::String(count) + ")", family, true,
                            nullptr, holdsCurrent);
        }
        menu.addSeparator();
        menu.addItem(1000, uiText(supportLanguage_, "VOLTAR À ESTRELA DA SEED", "BACK TO THE SEED'S STAR",
                                  "RETOUR À L’ÉTOILE DE LA SEED", "VOLVER A LA ESTRELLA DE LA SEED"),
                     starSwaps_[node] != PerformanceModuleBody::Count);
        menu.showMenuAsync(juce::PopupMenu::Options{}, [this, node, actorId](int result) {
            if (result == 0) return;
            if (result == 1100) {
                const bool pin = !starPinned_[node];
                if (!controller_.setActorPinned(actorId, pin)) {
                    lastAction_ = uiText(supportLanguage_, "fixar não entrou na fila realtime", "pin not queued",
                                         "épinglage non mis en file", "fijar no encolado");
                    return;
                }
                starPinned_[node] = pin;
                constellationGraph_.setPinnedNodes(starPinned_);
                lastAction_ = uiText(supportLanguage_, "CORPO ", "BODY ", "CORPS ", "CUERPO ")
                    + juce::String(static_cast<int>(node + 1U))
                    + (pin ? uiText(supportLanguage_, " · fixado", " · pinned", " · épinglé", " · fijado")
                           : uiText(supportLanguage_, " · solto", " · unpinned", " · libéré", " · suelto"));
                repaint();
                return;
            }
            const auto star = result == 1000 ? PerformanceModuleBody::Count
                : result == 900 ? PerformanceModuleBody::ModalResonator
                                : kPerformanceSkyStars[static_cast<std::size_t>(result - 1)];
            if (!controller_.setActorStar(actorId, star)) {
                lastAction_ = uiText(supportLanguage_, "troca de estrela não entrou na fila realtime",
                                     "star swap not queued", "changement d’étoile non mis en file",
                                     "cambio de estrella no encolado");
                return;
            }
            starSwaps_[node] = star;
            refreshCohortIdentity(identitySeed_);
            lastAction_ = uiText(supportLanguage_, "CORPO ", "BODY ", "CORPS ", "CUERPO ")
                + juce::String(static_cast<int>(node + 1U)) + " · "
                + moduleDisplayName(displayedAsterism(identitySeed_)[node]);
            repaint();
        });
    }

    juce::String familyName(PerformanceStarFunction function) const {
        switch (function) {
        case PerformanceStarFunction::Tone: return uiText(supportLanguage_, "TOM", "TONE", "TON", "TONO");
        case PerformanceStarFunction::Texture:
            return uiText(supportLanguage_, "TEXTURA", "TEXTURE", "TEXTURE", "TEXTURA");
        case PerformanceStarFunction::Voice: return uiText(supportLanguage_, "VOZ", "VOICE", "VOIX", "VOZ");
        case PerformanceStarFunction::Pulse: return uiText(supportLanguage_, "PULSO", "PULSE", "PULSATION", "PULSO");
        case PerformanceStarFunction::Control:
            return uiText(supportLanguage_, "CONTROLE", "CONTROL", "CONTRÔLE", "CONTROL");
        }
        return {};
    }

    void refreshCohortIdentity(std::uint32_t seed) {
        identitySeed_ = seed;
        // Os corpos 1..3 recebem as estrelas que o primeiro asterismo da seed
        // acende ali (PerformanceSky.hpp); não há mais coortes fixas.
        const auto openingAsterism = displayedAsterism(seed);
        for (std::size_t index = 0U; index < 3U; ++index) {
            const auto star = openingAsterism[index];
            const auto starControls = performanceStarControlNames(star);
            moduleDisplayNames_[index] = moduleDisplayName(star);
            const auto role = performanceStarRole(star);
            const auto function = juce::String::fromUTF8(role.data(), static_cast<int>(role.size()));
            actorLabels_[index].setText(moduleDisplayNames_[index] + utf8(" · ")
                + juce::String(static_cast<int>(index + 1U)) + utf8(" · PRES"),
                juce::dontSendNotification);
            actorSliders_[index].setTooltip(moduleDisplayNames_[index] + " · " + function
                + uiText(supportLanguage_, " · presença do stem na constelação.",
                          " · stem presence in the constellation.",
                          " · présence du stem dans la constellation.",
                          " · presencia del stem en la constelación."));
            for (std::size_t control = 0U; control < 3U; ++control) {
                moduleControlLabels_[index][control].setText(localizedModuleControlName(starControls[control]),
                                                             juce::dontSendNotification);
                moduleControlSliders_[index][control].setTooltip(moduleDisplayNames_[index]
                    + uiText(supportLanguage_, " · controle audível ", " · audible control ",
                             " · contrôle audible ", " · control audible ")
                    + localizedModuleControlName(starControls[control]));
            }
        }
        constellationGraph_.setOrbitSeed(seed);
        optionalModuleDisplayName_ = moduleDisplayName(openingAsterism[3]);
        const auto optionalControlNames = performanceStarControlNames(openingAsterism[3]);
        moduleDisplayNames_[3U] = optionalModuleDisplayName_;
        actorLabels_[3U].setText(moduleDisplayNames_[3U] + utf8(" · 4 · PRES"),
                                 juce::dontSendNotification);
        actorSliders_[3U].setTooltip(moduleDisplayNames_[3U]
            + uiText(supportLanguage_, " · quarto corpo preparado pela seed na constelação.",
                      " · fourth body prepared by the seed in the constellation.",
                      " · quatrième corps préparé par la seed dans la constellation.",
                      " · cuarto cuerpo preparado por la seed en la constelación."));
        for (std::size_t control = 0U; control < optionalControlNames.size(); ++control) {
            moduleControlLabels_[3U][control].setText(localizedModuleControlName(optionalControlNames[control]),
                                                       juce::dontSendNotification);
            moduleControlSliders_[3U][control].setTooltip(moduleDisplayNames_[3U]
                + uiText(supportLanguage_, " · controle audível ", " · audible control ",
                          " · contrôle audible ", " · control audible ")
                + localizedModuleControlName(optionalControlNames[control]));
            walshControlLabels_[control].setText(localizedModuleControlName(optionalControlNames[control]),
                                                 juce::dontSendNotification);
        }
        // Corpos 6..8: a estrela que o primeiro asterismo da seed acende ali
        // (ou a que o performer trocou).
        const auto asterism = displayedAsterism(seed);
        for (std::size_t index = 5U; index < asterism.size(); ++index) {
            moduleDisplayNames_[index] = moduleDisplayName(asterism[index]);
            actorLabels_[index].setText(moduleDisplayNames_[index] + utf8(" · ")
                + juce::String(static_cast<int>(index + 1U)) + utf8(" · PRES"),
                juce::dontSendNotification);
            actorSliders_[index].setTooltip(moduleDisplayNames_[index]
                + uiText(supportLanguage_, " · presença do stem na constelação.",
                          " · stem presence in the constellation.",
                          " · présence du stem dans la constellation.",
                          " · presencia del stem en la constelación."));
            const auto names = performanceStarControlNames(asterism[index]);
            for (std::size_t control = 0U; control < 3U; ++control) {
                moduleControlLabels_[index][control].setText(localizedModuleControlName(names[control]),
                                                             juce::dontSendNotification);
                moduleControlSliders_[index][control].setTooltip(moduleDisplayNames_[index]
                    + uiText(supportLanguage_, " · controle audível ", " · audible control ",
                             " · contrôle audible ", " · control audible ")
                    + localizedModuleControlName(names[control]));
            }
        }
        constellationGraph_.setModuleNames(moduleDisplayNames_);
        constellationGraph_.setOptionalModule(3U, optionalModuleDisplayName_, 4U, 0.0);
        selectedActorId_ = kInvalidActorId;
        selectedModuleIndex_ = kNoStemIndex;
        refreshNarrativeGestureLabels();
        relationDisplayNames_[0U] = utf8("MATÉRIA ") + moduleDisplayNames_[0U]
            + utf8(" → ") + moduleDisplayNames_[1U];
        relationDisplayNames_[1U] = utf8("TEMPO ") + moduleDisplayNames_[1U]
            + utf8(" → ") + moduleDisplayNames_[2U];
        relationDisplayNames_[2U] = utf8("ESPAÇO ") + moduleDisplayNames_[0U]
            + utf8(" → ") + moduleDisplayNames_[2U];
        relationDisplayNames_[3U] = utf8("ÓRBITA ") + moduleDisplayNames_[0U]
            + utf8(" → ") + optionalModuleDisplayName_;
        relationDisplayNames_[4U] = utf8("CONTÁGIO ") + moduleDisplayNames_[1U]
            + utf8(" → ") + optionalModuleDisplayName_;
        relationDisplayNames_[5U] = utf8("VESTÍGIO ") + moduleDisplayNames_[2U]
            + utf8(" → ") + optionalModuleDisplayName_;
        relationDisplayNames_[6U] = utf8("MEMÓRIA ") + moduleDisplayNames_[0U]
            + utf8(" → ") + moduleDisplayNames_[4U];
        relationDisplayNames_[7U] = utf8("PULSO ") + moduleDisplayNames_[4U]
            + utf8(" → ") + moduleDisplayNames_[5U];
        relationDisplayNames_[8U] = utf8("RISCO ") + moduleDisplayNames_[5U]
            + utf8(" → ") + moduleDisplayNames_[6U];
        relationDisplayNames_[9U] = utf8("ESPAÇO ") + moduleDisplayNames_[6U]
            + utf8(" → ") + moduleDisplayNames_[7U];
        refreshRelationInterfaceText();
        constellationGraph_.setTooltip(uiText(supportLanguage_, "RESERVA AO REDOR: arraste ",
            "RESERVE AROUND: drag ", "RÉSERVE AUTOUR : glissez ", "RESERVA ALREDEDOR: arrastre ")
            + moduleDisplayNames_[0U] + ", " + moduleDisplayNames_[1U]
            + uiText(supportLanguage_, " ou ", " or ", " ou ", " o ")
            + moduleDisplayNames_[2U]
            + uiText(supportLanguage_, " para dentro. Os pares possíveis ficam disponíveis: arraste um cabo para abrir ou modular sua relação; a seed não redesenha o campo.",
                " inward. The available pairs can be used: drag a cable to open or modulate its relation; the seed does not redraw the Field.",
                " vers l’intérieur. Les paires disponibles peuvent être utilisées : glissez un câble pour ouvrir ou moduler sa relation ; la seed ne redessine pas le Champ.",
                " hacia dentro. Los pares disponibles pueden usarse: arrastre un cable para abrir o modular su relación; la seed no redibuja el Campo."));
        refreshLocalizedModuleControlText();
        repaint();
    }

    juce::String moduleDisplayNameForIndex(std::size_t index) const {
        return index < moduleDisplayNames_.size() ? moduleDisplayNames_[index]
            : optionalModuleDisplayName_;
    }

    void recallScene(std::uint16_t sceneId) {
        // Scenes carry musical state, not ownership of the live drawing. Keep
        // the performer's spatial composition across every scene/cell recall.
        const auto drawing = constellationGraph_.spatialDrawing();
        const auto cables = constellationGraph_.relationDrawing();
        if (controller_.recallScene(sceneId, morphSeconds_)) {
            syncControlsFromScene(sceneId);
            constellationGraph_.restoreSpatialDrawing(drawing);
            constellationGraph_.restoreRelationDrawing(cables);
            sceneDirty_ = false;
            lastAction_ = uiText(supportLanguage_, "cena ", "scene ", "scène ", "escena ") + juce::String(sceneId)
                + " · morph "
                + juce::String(morphSeconds_, 1) + " s";
        }
    }

    // ---- O PERCURSO (2/11/2026) ----------------------------------------
    // As cenas seguem a forma do céu: ABERTURA → NASCENTE, DESENVOLVIMENTO →
    // ESPIRAL, CONTRASTE → RASGO, RETORNO → BALANÇO, com o MORPH escolhido.
    // Gravando, quando o retorno acaba (a forma volta ao desenvolvimento), a
    // tomada pede a coda: uma faixa é uma volta da forma. O performer pode
    // intervir a qualquer momento; o PERCURSO só chama a cena da seção nova.
    void setPercurso(bool on) {
        percurso_ = on;
        percursoSection_ = 255U;
        if (on && !controller_.skyWeb() && !controller_.mixerVariation()) {
            // A forma só anda com a TRAMA ou a VARIAÇÃO.
            controller_.setSkyWeb(true);
            refreshMixerVariationButton();
        }
        if (on) takePercurso_ = takePercurso_ || recorder_.recording();
        refreshAlbumButtons();
        lastAction_ = on ? uiText(supportLanguage_, "PERCURSO · as cenas seguem a forma do céu",
                                  "PATH · scenes follow the sky's form", "PARCOURS · les scènes suivent la forme du ciel",
                                  "RECORRIDO · las escenas siguen la forma del cielo")
                         : uiText(supportLanguage_, "PERCURSO desligado", "PATH off", "PARCOURS coupé", "RECORRIDO apagado");
        repaint();
    }
    void followPercurso(const PerformanceTelemetrySnapshot& telemetry) {
        if (!percurso_ || !telemetry.playing) return;
        const auto section = static_cast<std::uint8_t>(std::min<unsigned>(telemetry.skySection, 3U));
        if (section == percursoSection_) return;
        const bool returnEnded = percursoSection_ == 3U && section == 1U;
        // Numa faixa com duração, a forma termina no retorno: se o tempo
        // elástico do céu a fizer acabar antes do alvo, fica no BALANÇO até a
        // coda (no app, 3/11, ela recomeçava o desenvolvimento nos últimos
        // segundos).
        if (returnEnded && takeTargetSeconds_ > 0.0 && recorder_.recording()) return;
        percursoSection_ = section;
        recallScene(static_cast<std::uint16_t>(section + 1U));
        appendTakeTrace("percurso:" + juce::String(static_cast<int>(section)));
        if (returnEnded && recorder_.recording() && takeTargetSeconds_ <= 0.0)
            requestMorceauCoda("coda:percurso", uiText(supportLanguage_, "PERCURSO · o retorno acabou · CODA",
                                                       "PATH · the return ended · CODA", "PARCOURS · le retour est fini · CODA",
                                                       "RECORRIDO · el retorno terminó · CODA"));
    }
    void refreshAlbumButtons() {
        percursoButton_.setButtonText(percurso_ ? uiText(supportLanguage_, "PERCURSO · ON", "PATH · ON", "PARCOURS · ON",
                                                         "RECORRIDO · ON")
                                                : uiText(supportLanguage_, "PERCURSO", "PATH", "PARCOURS", "RECORRIDO"));
        percursoButton_.setColour(juce::TextButton::buttonColourId,
                                  percurso_ ? juce::Colour(0xff9b6a3f) : juce::Colour(0xff3f5a4a));
        albumButton_.setButtonText(album_.active()
            ? uiText(supportLanguage_, "ÁLBUM · FAIXA ", "ALBUM · TRACK ", "ALBUM · PISTE ", "ÁLBUM · PISTA ")
                + juce::String(static_cast<int>(album_.current())) + "/" + juce::String(static_cast<int>(album_.tracks()))
            : uiText(supportLanguage_, "ÁLBUM", "ALBUM", "ALBUM", "ÁLBUM"));
    }

    // ---- O ÁLBUM (2/11/2026) -------------------------------------------
    // Conceito do Modo Álbum do Studio (ver PerformanceAlbum.hpp). A pasta:
    // ~/Music/Rasgo Synth Performance/albuns/<data> <nome>/, com as tomadas
    // FAIXA_nn_… e o registro album.json.
    void showAlbumMenu() {
        juce::PopupMenu menu;
        if (!album_.active()) {
            menu.addItem(1, uiText(supportLanguage_, "NOVO ÁLBUM…", "NEW ALBUM…", "NOUVEL ALBUM…", "NUEVO ÁLBUM…"));
        } else {
            const auto current = album_.current();
            menu.addSectionHeader(juce::String::fromUTF8(album_.name().c_str()) + "  ·  "
                                  + uiText(supportLanguage_, "faixa ", "track ", "piste ", "pista ")
                                  + juce::String(static_cast<int>(current)) + "/" + juce::String(static_cast<int>(album_.tracks())));
            menu.addItem(2, uiText(supportLanguage_, "PRÓXIMA FAIXA", "NEXT TRACK", "PISTE SUIVANTE", "PISTA SIGUIENTE")
                                + (current < album_.tracks() ? "  ·  " + trackSummary(current + 1U) : juce::String{}),
                         current < album_.tracks());
            menu.addItem(3, uiText(supportLanguage_, "FAIXA ANTERIOR", "PREVIOUS TRACK", "PISTE PRÉCÉDENTE", "PISTA ANTERIOR"),
                         current > 1U);
            menu.addItem(4, uiText(supportLanguage_, "RECOMEÇAR ESTA FAIXA", "RESTART THIS TRACK", "RECOMMENCER CETTE PISTE",
                                   "REINICIAR ESTA PISTA"));
            juce::PopupMenu tracks;
            for (std::size_t track = 1U; track <= album_.tracks(); ++track)
                tracks.addItem(100 + static_cast<int>(track), juce::String(static_cast<int>(track)) + ".  " + trackSummary(track),
                               true, track == current);
            menu.addSubMenu(uiText(supportLanguage_, "IR PARA A FAIXA", "GO TO TRACK", "ALLER À LA PISTE", "IR A LA PISTA"), tracks);
            menu.addSeparator();
            menu.addItem(7, uiText(supportLanguage_, "MASTERIZAR O ÁLBUM (só ganho, −16 LUFS, −1 dBTP)",
                                   "MASTER THE ALBUM (gain only, −16 LUFS, −1 dBTP)",
                                   "MASTERISER L’ALBUM (gain seul, −16 LUFS, −1 dBTP)",
                                   "MASTERIZAR EL ÁLBUM (solo ganancia, −16 LUFS, −1 dBTP)"),
                         !album_.takes().empty() && !albumMastering_.load());
            menu.addItem(5, uiText(supportLanguage_, "ABRIR A PASTA DO ÁLBUM", "OPEN THE ALBUM FOLDER",
                                   "OUVRIR LE DOSSIER DE L’ALBUM", "ABRIR LA CARPETA DEL ÁLBUM"));
            menu.addItem(6, uiText(supportLanguage_, "ENCERRAR O ÁLBUM", "END THE ALBUM", "TERMINER L’ALBUM", "TERMINAR EL ÁLBUM"));
        }
        menu.showMenuAsync(juce::PopupMenu::Options{}.withTargetComponent(&albumButton_), [this](int result) {
            if (result == 1) askNewAlbum();
            else if (result == 2) goToAlbumTrack(album_.current() + 1U);
            else if (result == 3) goToAlbumTrack(album_.current() - 1U);
            else if (result == 4) goToAlbumTrack(album_.current());
            else if (result == 5) albumDirectory_.startAsProcess();
            else if (result == 7) masterAlbum();
            else if (result == 6) {
                writeAlbumRegister();
                album_.end();
                refreshAlbumButtons();
                lastAction_ = uiText(supportLanguage_, "álbum encerrado · registro em ", "album ended · register in ",
                                     "album terminé · registre dans ", "álbum terminado · registro en ")
                    + albumDirectory_.getChildFile("album.json").getFullPathName();
                repaint();
            } else if (result > 100) goToAlbumTrack(static_cast<std::size_t>(result - 100));
        });
    }
    juce::String trackSummary(std::size_t track) const {
        const auto takes = album_.takesOf(track);
        return "seed " + juce::String(static_cast<juce::int64>(album_.trackSeed(track))) + "  ·  "
            + juce::String(album_.trackBpm(track), 1) + " BPM"
            + (takes > 0U ? "  ·  " + juce::String(static_cast<int>(takes))
                                + uiText(supportLanguage_, " tomada(s)", " take(s)", " prise(s)", " toma(s)")
                          : juce::String{});
    }
    void askNewAlbum() {
        auto* window = new juce::AlertWindow(uiText(supportLanguage_, "NOVO ÁLBUM", "NEW ALBUM", "NOUVEL ALBUM", "NUEVO ÁLBUM"),
            uiText(supportLanguage_,
                   "A seed do álbum deriva a seed de cada faixa; o andamento das faixas segue o contorno, em torno do TEMPO atual.",
                   "The album seed derives each track's seed; track tempos follow the contour around the current TEMPO.",
                   "La seed de l’album dérive la seed de chaque piste ; les tempos suivent le contour autour du TEMPO actuel.",
                   "La seed del álbum deriva la seed de cada pista; los tempos siguen el contorno alrededor del TEMPO actual."),
            juce::MessageBoxIconType::NoIcon, this);
        window->addTextEditor("nome", "Asterismus " + juce::Time::getCurrentTime().formatted("%d-%m"),
                              uiText(supportLanguage_, "Nome / tema", "Name / theme", "Nom / thème", "Nombre / tema"));
        window->addTextEditor("faixas", "8", uiText(supportLanguage_, "Faixas (1–40)", "Tracks (1–40)", "Pistes (1–40)",
                                                    "Pistas (1–40)"));
        window->addTextEditor("seed", juce::String(static_cast<juce::int64>(
                                  static_cast<std::uint32_t>(juce::Random::getSystemRandom().nextInt()))),
                              uiText(supportLanguage_, "Seed do álbum", "Album seed", "Seed de l’album", "Seed del álbum"));
        window->addComboBox("contorno", {uiText(supportLanguage_, "equilibrado", "balanced", "équilibré", "equilibrado"),
                                         uiText(supportLanguage_, "contrastante", "contrasting", "contrasté", "contrastante"),
                                         uiText(supportLanguage_, "ascendente", "rising", "ascendant", "ascendente"),
                                         uiText(supportLanguage_, "arco", "arch", "arche", "arco")},
                            uiText(supportLanguage_, "Contorno do andamento", "Tempo contour", "Contour du tempo",
                                   "Contorno del tempo"));
        window->addTextEditor("abertura", "15", uiText(supportLanguage_, "Abertura do andamento (0–35 %)",
                                                       "Tempo spread (0–35 %)", "Écart de tempo (0–35 %)",
                                                       "Apertura del tempo (0–35 %)"));
        window->addButton(uiText(supportLanguage_, "COMEÇAR", "START", "COMMENCER", "EMPEZAR"), 1,
                          juce::KeyPress(juce::KeyPress::returnKey));
        window->addButton(uiText(supportLanguage_, "CANCELAR", "CANCEL", "ANNULER", "CANCELAR"), 0,
                          juce::KeyPress(juce::KeyPress::escapeKey));
        juce::Component::SafePointer<PerformanceHostComponent> safe(this);
        window->enterModalState(true, juce::ModalCallbackFunction::create([safe, window](int result) {
            if (safe == nullptr || result != 1) return;
            const auto name = window->getTextEditorContents("nome").trim();
            const auto tracks = window->getTextEditorContents("faixas").getIntValue();
            const auto seed = window->getTextEditorContents("seed").getLargeIntValue();
            const auto spread = window->getTextEditorContents("abertura").getDoubleValue();
            const auto contour = static_cast<PerformanceAlbumContour>(
                std::max(0, window->getComboBoxComponent("contorno")->getSelectedItemIndex()));
            safe->beginAlbum(name, tracks, seed, contour, spread);
        }), true);
    }
    void beginAlbum(const juce::String& name, int tracks, juce::int64 seed, PerformanceAlbumContour contour, double spread) {
        const auto created = juce::Time::getCurrentTime();
        if (!performanceMemoryNameValid(name.toStdString()) || tracks < 1 || tracks > static_cast<int>(kPerformanceAlbumMaxTracks)
            || seed < 0 || seed > static_cast<juce::int64>(std::numeric_limits<std::uint32_t>::max())
            || !album_.begin(name.toStdString(), static_cast<std::uint32_t>(seed), static_cast<std::size_t>(tracks),
                             tempoSlider_.getValue(), contour, spread, created.toISO8601(true).toStdString())) {
            lastAction_ = uiText(supportLanguage_, "álbum inválido (nome 1–40, faixas 1–40, seed de 32 bits)",
                                 "invalid album (name 1–40, tracks 1–40, 32-bit seed)",
                                 "album invalide (nom 1–40, pistes 1–40, seed 32 bits)",
                                 "álbum inválido (nombre 1–40, pistas 1–40, seed de 32 bits)");
            repaint();
            return;
        }
        // Nome de pasta sem caracteres que os sistemas recusam.
        const auto folder = created.formatted("%Y-%m-%d_%H-%M ") + juce::File::createLegalFileName(name);
        albumDirectory_ = juce::File::getSpecialLocation(juce::File::userMusicDirectory)
            .getChildFile("Rasgo Synth Performance").getChildFile("albuns").getChildFile(folder);
        if (!albumDirectory_.createDirectory()) {
            album_.end();
            lastAction_ = uiText(supportLanguage_, "a pasta do álbum não pôde ser criada", "album folder could not be created",
                                 "le dossier de l’album n’a pas pu être créé", "no se pudo crear la carpeta del álbum");
            repaint();
            return;
        }
        writeAlbumRegister();
        goToAlbumTrack(1U);
    }
    // Uma faixa: a seed derivada entra ao vivo (como uma seed nova) com o
    // andamento da faixa em todas as cenas, para o PERCURSO não o desfazer.
    void goToAlbumTrack(std::size_t track) {
        if (!album_.goTo(track)) return;
        const auto seed = album_.trackSeed(track);
        const auto bpm = album_.trackBpm(track);
        auto document = PerformanceTerritoryFactory::makeForReleasedStems(seed, kPerformanceSafeBodyLimit);
        for (std::size_t index = 0U; index < document.sceneCount; ++index) document.scenes[index].session.tempoBpm = bpm;
        if (!controller_.queueTerritory(document)) {
            lastAction_ = uiText(supportLanguage_, "faixa não enfileirada", "track not queued", "piste non mise en file",
                                 "pista no encolada");
            repaint();
            return;
        }
        adoptDocument(document, 1U);
        seedEditor_.setText(juce::String(static_cast<juce::int64>(seed)), false);
        tempoSlider_.setValue(bpm, juce::sendNotificationSync);
        refreshAlbumButtons();
        lastAction_ = uiText(supportLanguage_, "FAIXA ", "TRACK ", "PISTE ", "PISTA ") + juce::String(static_cast<int>(track))
            + "/" + juce::String(static_cast<int>(album_.tracks())) + "  ·  " + trackSummary(track)
            + uiText(supportLanguage_, "  ·  REC grava na pasta do álbum", "  ·  REC records into the album folder",
                     "  ·  REC enregistre dans le dossier de l’album", "  ·  REC graba en la carpeta del álbum");
        repaint();
    }
    // MASTERIZAR: a tomada mais recente de cada faixa, numa thread de
    // trabalho; as cópias vão para master/ com master.json (ganhos e níveis
    // antes e depois). As tomadas originais não mudam.
    void masterAlbum() {
        if (albumMastering_.exchange(true)) return;
        std::vector<std::pair<std::size_t, juce::File>> latest;
        for (std::size_t track = 1U; track <= album_.tracks(); ++track) {
            const PerformanceAlbumTake* last = nullptr;
            for (const auto& take : album_.takes()) if (take.track == track) last = &take;
            if (last != nullptr) latest.emplace_back(track, albumDirectory_.getChildFile(juce::String::fromUTF8(last->file.c_str())));
        }
        const auto folder = albumDirectory_.getChildFile("master");
        lastAction_ = uiText(supportLanguage_, "masterizando o álbum…", "mastering the album…", "mastérisation de l’album…",
                             "masterizando el álbum…");
        repaint();
        juce::Component::SafePointer<PerformanceHostComponent> safe(this);
        const auto language = supportLanguage_;
        juce::Thread::launch(juce::Thread::Priority::background, [safe, latest, folder, language] {
            std::vector<PerformanceTrackLevel> levels;
            bool ok = folder.createDirectory();
            for (const auto& [track, file] : latest) {
                const auto level = measureWavLevel(file.getFullPathName().toStdString());
                ok = ok && level.has_value();
                levels.push_back(level.value_or(PerformanceTrackLevel{}));
            }
            const auto plan = planAlbumMaster(levels);
            juce::String report = "{\n  \"schema\": \"rasgo-asterismus-album-master\",\n  \"targetLufs\": "
                + juce::String(plan.targetLufs, 1) + ",\n  \"ceilingDbtp\": " + juce::String(plan.ceilingDbtp, 1)
                + ",\n  \"medianLufsBefore\": " + juce::String(plan.medianLufs, 2) + ",\n  \"albumGainDb\": "
                + juce::String(plan.albumGainDb, 2)
                + ",\n  \"method\": \"só ganho, sem limitador; cada faixa em direção à mediana (−3,1…+2,9 dB, o limite do Studio), depois um ganho comum\",\n  \"tracks\": [";
            for (std::size_t index = 0U; ok && index < latest.size(); ++index) {
                const auto target = folder.getChildFile(latest[index].second.getFileNameWithoutExtension() + "_master.wav");
                ok = writeMasteredWav(latest[index].second.getFullPathName().toStdString(), target.getFullPathName().toStdString(),
                                      plan.gainDb[index], static_cast<std::uint32_t>(index + 1U) * 0x9E3779B9U);
                const auto after = measureWavLevel(target.getFullPathName().toStdString()).value_or(PerformanceTrackLevel{});
                report += juce::String(index > 0U ? "," : "") + "\n    {\"track\": " + juce::String(static_cast<int>(latest[index].first))
                    + ", \"source\": \"" + latest[index].second.getFileName() + "\", \"master\": \"" + target.getFileName()
                    + "\", \"lufsBefore\": " + juce::String(levels[index].integratedLufs, 2) + ", \"gainDb\": "
                    + juce::String(plan.gainDb[index], 2) + ", \"lufsAfter\": " + juce::String(after.integratedLufs, 2)
                    + ", \"truePeakAfterDbtp\": " + juce::String(after.maxTruePeakDbtp, 2) + "}";
            }
            report += "\n  ]\n}\n";
            if (ok) ok = folder.getChildFile("master.json").replaceWithText(report);
            juce::MessageManager::callAsync([safe, ok, folder, count = latest.size(), language] {
                if (safe == nullptr) return;
                safe->albumMastering_.store(false);
                safe->lastAction_ = ok ? uiText(language, "álbum masterizado · ", "album mastered · ", "album mastérisé · ",
                                                "álbum masterizado · ")
                                             + juce::String(static_cast<int>(count))
                                             + uiText(language, " faixa(s) em ", " track(s) in ", " piste(s) dans ", " pista(s) en ")
                                             + folder.getFullPathName()
                                       : uiText(language, "masterização falhou (arquivo ausente ou ilegível)",
                                                "mastering failed (missing or unreadable file)",
                                                "échec de la mastérisation (fichier absent ou illisible)",
                                                "la masterización falló (archivo ausente o ilegible)");
                safe->repaint();
            });
        });
    }

    void writeAlbumRegister() {
        if (albumDirectory_ == juce::File{}) return;
        const auto text = album_.registerJson();
        const auto file = albumDirectory_.getChildFile("album.json");
        const auto temporary = albumDirectory_.getChildFile("album.json.novo");
        if (!(temporary.replaceWithData(text.data(), text.size()) && temporary.moveFileTo(file)))
            lastAction_ = uiText(supportLanguage_, "o registro do álbum não pôde ser gravado", "album register could not be written",
                                 "le registre de l’album n’a pas pu être écrit", "no se pudo escribir el registro del álbum");
    }

    // ---- A MEMÓRIA (2/11/2026) ----------------------------------------
    // Oito lugares com nome, guardados em arquivos .asterismus na pasta de
    // dados do app; nada é carregado sozinho ao abrir (o app sempre abre algo
    // novo). Conceito do banco de cenas do Aquorbium (ver PerformanceMemory.hpp).
    static juce::File memoryDirectory() {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("RASGO").getChildFile("Rasgo Synth Asterismus").getChildFile("memorias");
    }
    static juce::File memoryFile(std::size_t slot) {
        return memoryDirectory().getChildFile("memoria-" + juce::String(static_cast<int>(slot + 1U)) + ".asterismus");
    }
    static std::optional<PerformanceMemory> readMemoryFile(const juce::File& file) {
        if (!file.existsAsFile() || file.isSymbolicLink()
            || file.getSize() > static_cast<juce::int64>(kPerformanceMemoryMaximumBytes)) return std::nullopt;
        juce::MemoryBlock data;
        if (!file.loadFileAsData(data)) return std::nullopt;
        PerformanceMemory memory;
        if (parsePerformanceMemory(std::string_view(static_cast<const char*>(data.getData()), data.getSize()), memory)
            != PerformanceMemoryError::None) return std::nullopt;
        return memory;
    }
    static bool writeMemoryFile(const juce::File& file, const PerformanceMemory& memory) {
        const auto text = serializePerformanceMemory(memory);
        if (!file.getParentDirectory().createDirectory()) return false;
        // Escreve ao lado e troca: um arquivo pela metade nunca substitui o bom.
        const auto temporary = file.getSiblingFile(file.getFileName() + ".novo");
        return temporary.replaceWithData(text.data(), text.size()) && temporary.moveFileTo(file);
    }
    void loadMemories() {
        for (std::size_t slot = 0U; slot < memories_.size(); ++slot) memories_[slot] = readMemoryFile(memoryFile(slot));
    }
    juce::String memoryLabel(std::size_t slot) const {
        const auto number = juce::String(static_cast<int>(slot + 1U)) + ". ";
        if (!memories_[slot]) return number + uiText(supportLanguage_, "(vazia)", "(empty)", "(vide)", "(vacía)");
        return number + juce::String::fromUTF8(memories_[slot]->name.c_str()) + "  ·  "
            + juce::String::fromUTF8(memories_[slot]->created.c_str());
    }

    PerformanceMemory captureMemory(const juce::String& name) const {
        PerformanceMemory memory;
        memory.name = name.toStdString();
        memory.created = juce::Time::getCurrentTime().formatted("%Y-%m-%d %H:%M").toStdString();
        auto document = startupDocument_;
        const auto sceneId = controller_.currentSceneId();
        for (std::size_t index = 0U; index < document.sceneCount; ++index)
            if (document.scenes[index].id == sceneId) fillSceneFromPerformance(document.scenes[index]);
        memory.sceneId = sceneId;
        memory.document = serializeSessionDocument(document);
        memory.stemGain = stemGainValues_;
        memory.stemMuted = stemMuted_;
        memory.solo = soloStemIndex_ < 8U ? static_cast<int>(soloStemIndex_) : -1;
        memory.moduleControls = moduleControlValues_;
        memory.seedVariation = actorSeedVariationValues_;
        memory.masterDb = masterSlider_.getValue();
        memory.skyWeb = controller_.skyWeb();
        memory.mixerVariation = controller_.mixerVariation();
        memory.collectiveFamily = static_cast<std::uint8_t>(constellationGraph_.collectiveFamily());
        for (std::size_t index = 0U; index < 8U; ++index) {
            memory.bodyFamily[index] = static_cast<std::uint8_t>(constellationGraph_.bodyFamily(index));
            memory.starSwap[index] = static_cast<std::uint8_t>(starSwaps_[index]);
        }
        memory.constellationMotion = static_cast<std::uint8_t>(constellationGraph_.constellationMotion());
        memory.pinned = starPinned_;
        return memory;
    }

    void showMemoryMenu() {
        juce::PopupMenu menu;
        menu.addSectionHeader(uiText(supportLanguage_, "MEMÓRIA · o app sempre abre algo novo",
                                     "MEMORY · the app always opens something new",
                                     "MÉMOIRE · l’app ouvre toujours du neuf", "MEMORIA · la app siempre abre algo nuevo"));
        juce::PopupMenu store, recall, rename, erase;
        bool any = false;
        for (std::size_t slot = 0U; slot < memories_.size(); ++slot) {
            const auto id = static_cast<int>(slot);
            const bool full = memories_[slot].has_value();
            any = any || full;
            store.addItem(100 + id, memoryLabel(slot));
            if (full) {
                recall.addItem(200 + id, memoryLabel(slot));
                rename.addItem(300 + id, memoryLabel(slot));
                erase.addItem(400 + id, memoryLabel(slot));
            }
        }
        menu.addSubMenu(uiText(supportLanguage_, "GUARDAR ESTA SESSÃO EM", "STORE THIS SESSION IN",
                               "GARDER CETTE SESSION DANS", "GUARDAR ESTA SESIÓN EN"), store);
        menu.addSubMenu(uiText(supportLanguage_, "LEMBRAR", "RECALL", "RAPPELER", "RECORDAR"), recall, any);
        menu.addSeparator();
        menu.addSubMenu(uiText(supportLanguage_, "RENOMEAR", "RENAME", "RENOMMER", "RENOMBRAR"), rename, any);
        menu.addSubMenu(uiText(supportLanguage_, "APAGAR", "DELETE", "EFFACER", "BORRAR"), erase, any);
        menu.addItem(700, uiText(supportLanguage_, "DESFAZER A ÚLTIMA MUDANÇA NA MEMÓRIA", "UNDO THE LAST MEMORY CHANGE",
                                 "ANNULER LE DERNIER CHANGEMENT", "DESHACER EL ÚLTIMO CAMBIO"),
                     memoryUndo_.has_value());
        menu.addSeparator();
        // SALVAR / ABRIR SESSÃO (4/11, proposta aprovada): no lugar de
        // EXPORTAR (que pedia um lugar) e IMPORTAR (que só guardava num lugar).
        // Salvar grava o que está soando agora; abrir já toca. Para pôr uma
        // sessão aberta num dos 8 lugares: GUARDAR ESTA SESSÃO EM.
        menu.addItem(800, uiText(supportLanguage_, "SALVAR SESSÃO…", "SAVE SESSION…", "ENREGISTRER LA SESSION…",
                                 "GUARDAR SESIÓN…"));
        menu.addItem(801, uiText(supportLanguage_, "ABRIR SESSÃO…", "OPEN SESSION…", "OUVRIR UNE SESSION…",
                                 "ABRIR SESIÓN…"));
        menu.showMenuAsync(juce::PopupMenu::Options{}.withTargetComponent(&memoryButton_), [this](int result) {
            if (result <= 0) return;
            const auto slot = static_cast<std::size_t>(result % 100);
            if (result == 800) chooseSessionFile(true);
            else if (result == 801) chooseSessionFile(false);
            else if (result == 700) undoMemoryChange();
            else if (result < 200) askMemoryName(slot, false);
            else if (result < 300) recallMemory(slot);
            else if (result < 400) askMemoryName(slot, true);
            else if (result < 500) changeMemory(slot, std::nullopt,
                                                uiText(supportLanguage_, "memória apagada", "memory deleted",
                                                       "mémoire effacée", "memoria borrada"));
        });
    }

    // A última pasta usada para salvar ou abrir (3/11, pedido do autor:
    // "sempre parte de Documentos; melhor do último diretório utilizado").
    // Guardada num arquivo de texto ao lado das memórias, para valer também
    // na próxima vez que o app abrir; se a pasta sumiu, volta a Documentos.
    static juce::File lastMemoryFolderFile() {
        return memoryDirectory().getParentDirectory().getChildFile("ultima_pasta_de_memoria.txt");
    }
    static juce::File lastMemoryFolder() {
        const auto saved = lastMemoryFolderFile().loadFileAsString().trim();
        if (saved.isNotEmpty() && juce::File::isAbsolutePath(saved)) {
            const juce::File folder(saved);
            if (folder.isDirectory()) return folder;
        }
        return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    }
    static void rememberMemoryFolder(const juce::File& folder) {
        if (!folder.isDirectory()) return;
        const auto file = lastMemoryFolderFile();
        if (file.getParentDirectory().createDirectory()) file.replaceWithText(folder.getFullPathName());
    }

    // SALVAR SESSÃO grava o estado vivo (o mesmo que GUARDAR, mas num arquivo
    // .asterismus onde o músico quiser); ABRIR SESSÃO lê e toca — como
    // LEMBRAR, sem ocupar um dos 8 lugares. O seletor abre na última pasta.
    void chooseSessionFile(bool saving) {
        if (memoryChooser_) return;
        const auto seed = juce::String(startupDocument_.territory.seed);
        const auto initial = saving
            ? lastMemoryFolder().getChildFile("sessao seed " + seed + " "
                  + juce::Time::getCurrentTime().formatted("%Y-%m-%d %H-%M") + ".asterismus")
            : lastMemoryFolder();
        memoryChooser_ = std::make_unique<juce::FileChooser>(
            saving ? uiText(supportLanguage_, "Salvar a sessão", "Save the session", "Enregistrer la session",
                            "Guardar la sesión")
                   : uiText(supportLanguage_, "Abrir uma sessão", "Open a session", "Ouvrir une session",
                            "Abrir una sesión"),
            initial, "*.asterismus");
        const auto flags = juce::FileBrowserComponent::canSelectFiles
            | (saving ? juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting
                      : juce::FileBrowserComponent::openMode);
        juce::Component::SafePointer<PerformanceHostComponent> safe(this);
        memoryChooser_->launchAsync(flags, [safe, saving, seed](const juce::FileChooser& chooser) {
            if (safe == nullptr) return;
            const auto file = chooser.getResult();
            safe->memoryChooser_.reset();
            if (file == juce::File{}) return;
            rememberMemoryFolder(file.getParentDirectory());
            if (saving) {
                const auto target = file.hasFileExtension(".asterismus") ? file : file.withFileExtension(".asterismus");
                const auto memory = safe->captureMemory(target.getFileNameWithoutExtension().substring(0, 40));
                safe->lastAction_ = writeMemoryFile(target, memory)
                    ? uiText(safe->supportLanguage_, "sessão salva · ", "session saved · ", "session enregistrée · ",
                             "sesión guardada · ") + target.getFullPathName()
                    : uiText(safe->supportLanguage_, "a sessão não pôde ser salva em ", "the session could not be saved to ",
                             "la session n’a pas pu être enregistrée dans ", "la sesión no pudo guardarse en ")
                          + target.getFullPathName();
                safe->repaint();
                return;
            }
            const auto memory = readMemoryFile(file);
            if (!memory) {
                safe->lastAction_ = uiText(safe->supportLanguage_, "arquivo de sessão inválido · ", "invalid session file · ",
                                           "fichier de session invalide · ", "archivo de sesión inválido · ")
                    + file.getFileName();
                safe->repaint();
                return;
            }
            safe->playMemory(*memory, uiText(safe->supportLanguage_, "SESSÃO ", "SESSION ", "SESSION ", "SESIÓN ")
                                          + file.getFileName());
        });
    }

    // Guardar (ou renomear): pede o nome, sugerindo o atual ou "seed · data".
    void askMemoryName(std::size_t slot, bool renaming) {
        if (slot >= memories_.size()) return;
        const auto suggestion = renaming && memories_[slot]
            ? juce::String::fromUTF8(memories_[slot]->name.c_str())
            : "seed " + juce::String(startupDocument_.territory.seed) + " · "
                + juce::Time::getCurrentTime().formatted("%d/%m %H:%M");
        auto* window = new juce::AlertWindow(
            renaming ? uiText(supportLanguage_, "RENOMEAR A MEMÓRIA", "RENAME THE MEMORY", "RENOMMER LA MÉMOIRE",
                              "RENOMBRAR LA MEMORIA")
                     : uiText(supportLanguage_, "GUARDAR NA MEMÓRIA ", "STORE IN MEMORY ", "GARDER EN MÉMOIRE ",
                              "GUARDAR EN LA MEMORIA ") + juce::String(static_cast<int>(slot + 1U)),
            uiText(supportLanguage_, "Nome (até 40 caracteres):", "Name (up to 40 characters):",
                   "Nom (40 caractères au plus) :", "Nombre (hasta 40 caracteres):"),
            juce::MessageBoxIconType::NoIcon, this);
        window->addTextEditor("nome", suggestion);
        window->addButton(renaming ? uiText(supportLanguage_, "RENOMEAR", "RENAME", "RENOMMER", "RENOMBRAR")
                                   : uiText(supportLanguage_, "GUARDAR", "STORE", "GARDER", "GUARDAR"),
                          1, juce::KeyPress(juce::KeyPress::returnKey));
        window->addButton(uiText(supportLanguage_, "CANCELAR", "CANCEL", "ANNULER", "CANCELAR"), 0,
                          juce::KeyPress(juce::KeyPress::escapeKey));
        juce::Component::SafePointer<PerformanceHostComponent> safe(this);
        window->enterModalState(true, juce::ModalCallbackFunction::create([safe, window, slot, renaming](int result) {
            if (safe == nullptr || result != 1) return;
            const auto name = window->getTextEditorContents("nome").trim();
            if (!performanceMemoryNameValid(name.toStdString())) {
                safe->lastAction_ = uiText(safe->supportLanguage_, "nome inválido (1 a 40 caracteres, uma linha)",
                                           "invalid name (1 to 40 characters, one line)",
                                           "nom invalide (1 à 40 caractères, une ligne)",
                                           "nombre inválido (1 a 40 caracteres, una línea)");
                safe->repaint();
                return;
            }
            auto memory = renaming && safe->memories_[slot] ? *safe->memories_[slot] : safe->captureMemory(name);
            memory.name = name.toStdString();
            safe->changeMemory(slot, memory, renaming
                ? uiText(safe->supportLanguage_, "memória renomeada", "memory renamed", "mémoire renommée",
                         "memoria renombrada")
                : uiText(safe->supportLanguage_, "sessão guardada", "session stored", "session gardée",
                         "sesión guardada"));
        }), true);
    }

    // Toda mudança num lugar passa por aqui: grava (ou apaga) o arquivo e
    // guarda o conteúdo anterior para o DESFAZER.
    void changeMemory(std::size_t slot, const std::optional<PerformanceMemory>& replacement, const juce::String& done) {
        if (slot >= memories_.size()) return;
        const auto file = memoryFile(slot);
        const bool ok = replacement ? writeMemoryFile(file, *replacement) : (!file.existsAsFile() || file.deleteFile());
        if (!ok) {
            lastAction_ = uiText(supportLanguage_, "a memória não pôde ser gravada em ", "memory could not be written to ",
                                 "la mémoire n’a pas pu être écrite dans ", "la memoria no pudo escribirse en ")
                + file.getFullPathName();
            repaint();
            return;
        }
        memoryUndo_ = MemoryUndo{slot, memories_[slot]};
        memories_[slot] = replacement;
        lastAction_ = juce::String(static_cast<int>(slot + 1U)) + " · " + done
            + (replacement ? " · " + juce::String::fromUTF8(replacement->name.c_str()) : juce::String{});
        repaint();
    }
    void undoMemoryChange() {
        if (!memoryUndo_) return;
        const auto undo = *memoryUndo_;
        changeMemory(undo.slot, undo.before, uiText(supportLanguage_, "memória restaurada", "memory restored",
                                                    "mémoire restaurée", "memoria restaurada"));
        memoryUndo_.reset();
    }

    // Lembrar: o território e a cena entram ao vivo (como uma seed nova); o
    // estado do performer é reaplicado logo depois, quando o motor já trocou
    // de território.
    void recallMemory(std::size_t slot) {
        if (slot >= memories_.size() || !memories_[slot]) return;
        playMemory(*memories_[slot], uiText(supportLanguage_, "MEMÓRIA ", "MEMORY ", "MÉMOIRE ", "MEMORIA ")
                                         + juce::String(static_cast<int>(slot + 1U)));
    }

    // Toca uma memória (de um lugar ou de um arquivo de sessão): o território
    // entra ao vivo e o estado do performer é reaplicado 300 ms depois.
    void playMemory(const PerformanceMemory& memory, const juce::String& label) {
        PerformanceSessionDocument document{};
        if (!parseSessionDocument(memory.document, document).ok()
            || !controller_.queueTerritory(document, memory.sceneId)) {
            lastAction_ = uiText(supportLanguage_, "memória não enfileirada", "memory not queued",
                                 "mémoire non mise en file", "memoria no encolada");
            repaint();
            return;
        }
        adoptDocument(document, memory.sceneId);
        lastRecalledMemory_ = memory.name;
        seedEditor_.setText(juce::String(document.territory.seed), false);
        juce::Component::SafePointer<PerformanceHostComponent> safe(this);
        juce::Timer::callAfterDelay(300, [safe, memory, label] {
            if (safe != nullptr) safe->applyMemoryPerformer(memory, label);
        });
    }
    void applyMemoryPerformer(const PerformanceMemory& memory, const juce::String& label) {
        std::size_t refused = 0U;
        for (std::size_t index = 0U; index < 8U; ++index) {
            const auto actorId = static_cast<std::uint16_t>(index + 1U);
            const auto star = static_cast<PerformanceModuleBody>(memory.starSwap[index]);
            if (star != PerformanceModuleBody::Count) {
                if (controller_.setActorStar(actorId, star)) starSwaps_[index] = star;
                else ++refused;
            }
            if (memory.pinned[index]) {
                if (controller_.setActorPinned(actorId, true)) starPinned_[index] = true;
                else ++refused;
            }
            setStemGainControl(index, memory.stemGain[index]);
            if (memory.stemMuted[index] != stemMuted_[index]) toggleStemMute(index);
            for (std::size_t control = 0U; control < 3U; ++control) {
                if (!setModuleControl(index, control, memory.moduleControls[index][control])) continue;
                moduleControlSliders_[index][control].setValue(memory.moduleControls[index][control],
                                                               juce::dontSendNotification);
            }
            setActorSeedVariationControl(index, memory.seedVariation[index]);
        }
        if (memory.solo >= 0 && static_cast<std::size_t>(memory.solo) != soloStemIndex_)
            toggleStemSolo(static_cast<std::size_t>(memory.solo));
        masterSlider_.setValue(memory.masterDb, juce::sendNotificationSync);
        if (controller_.skyWeb() != memory.skyWeb) controller_.setSkyWeb(memory.skyWeb);
        if (controller_.mixerVariation() != memory.mixerVariation) controller_.setMixerVariation(memory.mixerVariation);
        refreshMixerVariationButton();
        std::array<PerformanceMotionFamily, 8> bodies{};
        for (std::size_t index = 0U; index < 8U; ++index)
            bodies[index] = static_cast<PerformanceMotionFamily>(memory.bodyFamily[index]);
        constellationGraph_.restoreMotion(static_cast<PerformanceMotionFamily>(memory.collectiveFamily), bodies,
                                          static_cast<PerformanceConstellationMotion>(memory.constellationMotion));
        constellationGraph_.setPinnedNodes(starPinned_);
        refreshCohortIdentity(identitySeed_);
        refreshStemMixVisual();
        lastAction_ = label + " · " + juce::String::fromUTF8(memory.name.c_str())
            + (refused > 0U ? uiText(supportLanguage_, " · comandos recusados: ", " · refused commands: ",
                                     " · commandes refusées : ", " · comandos rechazados: ") + juce::String(static_cast<int>(refused))
                            : juce::String{});
        repaint();
    }


    // O estado da cena que o performer tem nas mãos (BPM, macros, presenças,
    // relações): usado por CAPTURAR e pela MEMÓRIA.
    void fillSceneFromPerformance(PerformanceScene& scene) const {
        scene.session.autonomy = macroValues_[0U];
        scene.session.intervention = macroValues_[1U];
        scene.session.density = macroValues_[2U];
        scene.session.tension = macroValues_[3U];
        scene.session.memory = macroValues_[4U];
        scene.session.risk = macroValues_[5U];
        scene.session.space = macroValues_[6U];
        scene.session.depth = macroValues_[7U];
        scene.session.tempoBpm = tempoSlider_.getValue();
        scene.session.populationPhenomenon = constellationGraph_.populationPhenomenon();
        scene.session.populationPhenomenonIntensity = constellationGraph_.populationPhenomenonIntensity();
        scene.session.populationPhenomenonEnabled = constellationGraph_.populationPhenomenonEnabled();
        for (std::size_t actorIndex = 0; actorIndex < scene.constellation.actorCount; ++actorIndex) {
            auto& actor = scene.constellation.actors[actorIndex];
            if (actor.actorId >= 1U && actor.actorId <= actorSliders_.size())
                actor.presence = actorSliders_[actor.actorId - 1U].getValue();
        }
        const auto cables = constellationGraph_.relationDrawing();
        static constexpr std::array<std::uint16_t, 16> sources{1U, 2U, 1U, 1U, 2U, 3U, 1U, 5U, 6U, 7U, 1U, 2U, 2U, 3U, 4U, 5U};
        static constexpr std::array<std::uint16_t, 16> destinations{2U, 3U, 3U, 4U, 4U, 4U, 5U, 6U, 7U, 8U, 6U, 5U, 6U, 7U, 8U, 7U};
        for (std::size_t relationIndex = 0; relationIndex < scene.constellation.relationCount; ++relationIndex) {
            auto& relation = scene.constellation.relations[relationIndex];
            for (std::size_t cableIndex = 0U; cableIndex < cables.size(); ++cableIndex) {
                if (relation.sourceActorId == sources[cableIndex]
                    && relation.destinationActorId == destinations[cableIndex]) {
                    relation.intensity = cables[cableIndex];
                    relation.phenomenon = constellationGraph_.relationPhenomenonAt(cableIndex);
                    relation.phenomenonEnabled
                        = constellationGraph_.relationPhenomenonEnabledAt(cableIndex);
                    break;
                }
            }
        }
    }

    void captureScene(std::uint16_t sceneId) {
        for (std::size_t index = 0; index < startupDocument_.sceneCount; ++index) {
            auto& scene = startupDocument_.scenes[index];
            if (scene.id != sceneId) continue;
            const auto previousScene = scene;
            const auto wasDirty = sceneDirty_;
            fillSceneFromPerformance(scene);
            if (controller_.captureScene(scene)) {
                lastCapturedScene_ = previousScene;
                hasUndoCapture_ = true;
                undoCaptureButton_.setEnabled(true);
                sceneDirty_ = false;
                lastAction_ = uiText(supportLanguage_, "cena ", "scene ", "scène ", "escena ")
                    + juce::String(sceneId) + uiText(supportLanguage_, " · captura enfileirada", " · capture queued",
                        " · capture en file", " · captura en cola");
            } else {
                scene = previousScene;
                sceneDirty_ = wasDirty;
                lastAction_ = uiText(supportLanguage_, "captura não enfileirada", "capture not queued",
                                     "capture non mise en file", "captura no encolada");
            }
            return;
        }
    }

    void undoLastCapture() {
        if (!hasUndoCapture_) return;
        if (!controller_.captureScene(lastCapturedScene_)) {
            lastAction_ = uiText(supportLanguage_, "desfazer não enfileirado", "undo not queued",
                                 "annulation non mise en file", "deshacer no encolado");
            return;
        }
        for (std::size_t index = 0; index < startupDocument_.sceneCount; ++index) {
            if (startupDocument_.scenes[index].id == lastCapturedScene_.id) {
                startupDocument_.scenes[index] = lastCapturedScene_;
                break;
            }
        }
        hasUndoCapture_ = false;
        undoCaptureButton_.setEnabled(false);
        sceneDirty_ = false;
        lastAction_ = uiText(supportLanguage_, "cena ", "scene ", "scène ", "escena ")
            + juce::String(lastCapturedScene_.id)
            + uiText(supportLanguage_, " · restauração enfileirada", " · restoration queued",
                      " · restauration mise en file", " · restauración en cola");
    }

    void syncControlsFromScene(std::uint16_t sceneId, bool resetFieldAnchors = false) {
        const PerformanceScene* scene = nullptr;
        for (std::size_t index = 0; index < startupDocument_.sceneCount; ++index) {
            if (startupDocument_.scenes[index].id == sceneId) {
                scene = &startupDocument_.scenes[index];
                break;
            }
        }
        if (scene == nullptr) return;

        const std::array<double, 8> values{
            scene->session.autonomy, scene->session.intervention, scene->session.density,
            scene->session.tension, scene->session.memory, scene->session.risk, scene->session.space,
            scene->session.depth};
        macroValues_ = values;
        constellationGraph_.restorePopulationPhenomenon(scene->session.populationPhenomenon,
            scene->session.populationPhenomenonIntensity, scene->session.populationPhenomenonEnabled);
        tempoSlider_.setValue(scene->session.tempoBpm, juce::dontSendNotification);
        // These two controls belong to the seed territory. Their physical
        // range therefore communicates the available budget before the
        // musician reaches an invisible clamp in the engine.
        macroSliders_[4U].setRange(0.0, startupDocument_.territory.identity.memoryLimit, 0.01);
        macroSliders_[5U].setRange(0.0, startupDocument_.territory.identity.riskLimit, 0.01);
        for (std::size_t index = 0; index < macroSliders_.size(); ++index)
            macroSliders_[index].setValue(values[index], juce::dontSendNotification);
        if (resetFieldAnchors) {
            fieldPrimaryX_ = scene->session.autonomy;
            fieldPrimaryY_ = scene->session.intervention;
            fieldTextureX_ = scene->session.density;
            fieldTextureY_ = scene->session.tension;
            performanceField_.setPosition(fieldPrimaryX_, fieldPrimaryY_);
            performanceField_.setTexturePosition(fieldTextureX_, fieldTextureY_);
        }
        // Scene recall changes the macro values, not the live drawing. The
        // field is authored by the performer and moves only through an
        // explicit field gesture or macro action.

        // Clear reserve bodies before applying the scene. Without this reset,
        // a previous expanded scene could leave its actors active in the
        // graph after recall of a smaller seed, producing stale cables.
        for (std::size_t index = 0; index < actorSliders_.size(); ++index) {
            constellationGraph_.setPresence(index, 0.0);
            modulePanelActive_[index] = false;
        }
        for (std::size_t index = 0; index < actorSliders_.size(); ++index) {
            const auto* actor = scene->constellation.findActor(static_cast<std::uint16_t>(index + 1U));
            if (actor != nullptr) {
                actorPresenceValues_[index] = actor->presence;
                actorSliders_[index].setValue(actor->presence, juce::dontSendNotification);
                constellationGraph_.setPresence(index, actor->presence);
                modulePanelActive_[index] = actor->presence > 0.025;
            }
        }
        std::array<double, 16> sceneRelations = constellationGraph_.relationDrawing();
        for (std::size_t index = 0; index < scene->constellation.relationCount; ++index) {
            const auto& relation = scene->constellation.relations[index];
            if (relation.sourceActorId == 1U && relation.destinationActorId == 2U) {
                relationValues_[0U] = relation.intensity;
                relationSliders_[0].setValue(relation.intensity, juce::dontSendNotification);
            } else if (relation.sourceActorId == 2U && relation.destinationActorId == 3U) {
                relationValues_[1U] = relation.intensity;
                relationSliders_[1].setValue(relation.intensity, juce::dontSendNotification);
            } else if (relation.sourceActorId == 1U && relation.destinationActorId == 3U) {
                relationValues_[2U] = relation.intensity;
                relationSliders_[2].setValue(relation.intensity, juce::dontSendNotification);
            } else if (relation.sourceActorId == 1U && relation.destinationActorId == 4U) {
                walshRelationValue_ = relation.intensity;
                sceneRelations[3U] = relation.intensity;
            } else if (relation.sourceActorId == 2U && relation.destinationActorId == 4U) {
                sceneRelations[4U] = relation.intensity;
            } else if (relation.sourceActorId == 3U && relation.destinationActorId == 4U) {
                sceneRelations[5U] = relation.intensity;
            } else if (relation.sourceActorId == 1U && relation.destinationActorId == 5U) {
                sceneRelations[6U] = relation.intensity;
            } else if (relation.sourceActorId == 5U && relation.destinationActorId == 6U) {
                sceneRelations[7U] = relation.intensity;
            } else if (relation.sourceActorId == 6U && relation.destinationActorId == 7U) {
                sceneRelations[8U] = relation.intensity;
            } else if (relation.sourceActorId == 7U && relation.destinationActorId == 8U) {
                sceneRelations[9U] = relation.intensity;
            }
        }
        // Recalling a cell/scene may update relation values, but never
        // overwrites the live spatial field drawn by the musician.
        sceneRelations[0U] = relationValues_[0U];
        sceneRelations[1U] = relationValues_[1U];
        sceneRelations[2U] = relationValues_[2U];
        constellationGraph_.setRelations(sceneRelations);
        synchronizeFocusedActor();
        resized();
    }

    bool pushGesture(GestureType type, double intensity = 0.72) {
        PerformanceGestureEvent event;
        event.type = type;
        event.intensity = type == GestureType::PanicSafe ? 1.0 : intensity;
        if (type == GestureType::Mutate || type == GestureType::Stutter
            || type == GestureType::TempoShift) {
            event.variationToken = static_cast<std::uint32_t>(juce::Random::getSystemRandom().nextInt());
            event.targetActorId = selectedActorId_;
        } else if (type == GestureType::Rasgar || type == GestureType::Recall) {
            event.targetActorId = selectedActorId_;
        }
        if (controller_.pushGesture(event)) {
            notePerformerActivity();
            appendTakeTrace("gesture:" + gestureName(type));
            lastAction_ = type == GestureType::Mutate || type == GestureType::Rasgar
                    || type == GestureType::Recall || type == GestureType::Stutter
                    || type == GestureType::TempoShift
                ? (type == GestureType::Mutate ? utf8("MUTATE · SELVAGEM")
                   : type == GestureType::Rasgar ? utf8("RASGAR")
                   : type == GestureType::Recall ? utf8("RECALL")
                   : type == GestureType::Stutter ? utf8("STUTTER")
                   : uiText(supportLanguage_, "DESLOCAR", "DISPLACE", "DÉCALER", "DESPLAZAR"))
                    + (selectedModuleIndex_ < moduleDisplayNames_.size()
                        ? utf8(" · ") + moduleDisplayNames_[selectedModuleIndex_]
                        : juce::String{})
                : gestureName(type);
            return true;
        }
        lastAction_ = gestureName(type) + uiText(supportLanguage_, " não enfileirado", " not queued",
                                                  " non mis en file", " no encolado");
        return false;
    }

    static juce::String gestureName(GestureType type) {
        switch (type) {
        case GestureType::Hold: return "HOLD";
        case GestureType::Release: return "RELEASE";
        case GestureType::Mutate: return "MUTATE";
        case GestureType::Rasgar: return "RASGAR";
        case GestureType::Stutter: return "STUTTER";
        case GestureType::TempoShift: return "TEMPO SHIFT";
        case GestureType::Recall: return "RECALL";
        case GestureType::Silence: return utf8("SILÊNCIO");
        case GestureType::PanicSafe: return "PANIC SAFE";
        default: return "GESTO";
        }
    }

    void requestPanicSafe() {
        if (controller_.readTelemetry().panicSafe) {
            if (controller_.setPanicSafe(false)) lastAction_ = uiText(supportLanguage_, "RESET SAFE enfileirado", "RESET SAFE queued",
                                                                       "RESET SAFE mis en file", "RESET SAFE encolado");
            else lastAction_ = uiText(supportLanguage_, "RESET SAFE não enfileirado", "RESET SAFE not queued",
                                      "RESET SAFE non mis en file", "RESET SAFE no encolado");
            return;
        }
        pushGesture(GestureType::PanicSafe, 1.0);
    }

    void requestHold() {
        const auto requested = !holdRequested_;
        if (!pushGesture(requested ? GestureType::Hold : GestureType::Release)) return;
        holdRequested_ = requested;
        refreshHoldVisual();
    }

    // PLAY só toca (2/11, decisão do autor: "a gravação começa quando o
    // usuário define fazer a gravação"); a tomada abre pelo REC.
    void togglePlaying(bool openedTake = false) {
        if (!playingRequested_) applyTypedSeedIfChanged("antes-de-tocar");
        const auto requested = !playingRequested_;
        if (!controller_.setPlaying(requested)) {
            if (openedTake) closeRecording(uiText(supportLanguage_, "tomada cancelada", "take cancelled",
                                                  "prise annulée", "toma cancelada"));
            lastAction_ = uiText(supportLanguage_, "transporte não enfileirado", "transport not queued",
                                 "transport non mis en file", "transporte no encolado");
            return;
        }
        if (!requested) appendTakeTrace("transport:STOP");
        // Keep the writer open until the next audio callback has written the
        // engine's 8 ms technical de-click. STOP remains interruption, never
        // a coda, but the WAV must not end on a discontinuity.
        if (!requested && recorder_.recording()) stopClosePending_ = true;
        playingRequested_ = requested;
        codaRequested_ = false;
        lastAction_ = playingRequested_ ? (openedTake ? "PLAY  ·  " + lastAction_ : juce::String("PLAY")) : juce::String("STOP");
        transportButton_.setButtonText(playingRequested_ ? "STOP" : "PLAY");
        transportButton_.setColour(juce::TextButton::buttonColourId,
                                   playingRequested_ ? juce::Colour(0xff813d34) : juce::Colour(0xff285348));
    }

    // Uma faixa de 1/2/3/5 min (como no Antitotem: um clique arma e grava, com
    // contagem regressiva). A forma do céu se encaixa na duração — abertura
    // 15 %, desenvolvimento 35 %, contraste 25 %, retorno 25 % dos compassos
    // até a coda, como o arco do Studio se ajusta à duração pedida — e o
    // PERCURSO troca as cenas com ela. A coda vem 5,2 s antes do alvo.
    void startTimedTake(double seconds) {
        if (recorder_.recording()) {
            lastAction_ = uiText(supportLanguage_, "já está gravando · CODA ou STOP antes de outra faixa",
                                 "already recording · CODA or STOP before another track",
                                 "enregistrement en cours · CODA ou STOP avant une autre piste",
                                 "ya está grabando · CODA o STOP antes de otra pista");
            repaint();
            return;
        }
        const double bpm = std::max(20.0, tempoSlider_.getValue());
        const double bars = std::max(4.0, (seconds - kCodaLeadSeconds) * bpm / 240.0);
        std::array<std::uint32_t, 4> plan{{
            static_cast<std::uint32_t>(std::max(1.0, std::round(bars * 0.15))),
            static_cast<std::uint32_t>(std::max(1.0, std::round(bars * 0.35))),
            static_cast<std::uint32_t>(std::max(1.0, std::round(bars * 0.25))), 0U}};
        plan[3] = static_cast<std::uint32_t>(std::max(1.0, bars - plan[0] - plan[1] - plan[2]));
        if (!controller_.setFormPlan(plan)) {
            lastAction_ = uiText(supportLanguage_, "a forma não entrou na fila", "form not queued", "forme non mise en file",
                                 "forma no encolada");
            repaint();
            return;
        }
        takeTargetSeconds_ = seconds;
        if (!percurso_) setPercurso(true);
        percursoSection_ = 255U;
        takeStartedWithPlay_ = !playingRequested_;
        if (!beginRecording()) {
            controller_.setFormPlan({{0U, 0U, 0U, 0U}});
            takeTargetSeconds_ = 0.0;
            return;
        }
        for (std::size_t index = 0U; index < durationButtons_.size(); ++index)
            durationButtons_[index].setToggleState(kTakeDurations[index] == seconds, juce::dontSendNotification);
        appendTakeTrace("duracao:" + juce::String(seconds, 0) + "s:compassos:" + juce::String(static_cast<int>(plan[0])) + "/"
                        + juce::String(static_cast<int>(plan[1])) + "/" + juce::String(static_cast<int>(plan[2])) + "/"
                        + juce::String(static_cast<int>(plan[3])));
        if (!playingRequested_) togglePlaying(true);
        lastAction_ = uiText(supportLanguage_, "FAIXA de ", "TRACK of ", "PISTE de ", "PISTA de ")
            + juce::String(static_cast<int>(seconds / 60.0)) + " min  ·  " + lastAction_;
        repaint();
    }

    void toggleRecording() {
        if (recorder_.recording()) {
            if (!playingRequested_) {
                closeRecording(uiText(supportLanguage_, "tomada encerrada", "take closed", "prise fermée", "toma cerrada"));
                return;
            }
            requestMorceauCoda("coda:requested", uiText(supportLanguage_, "CODA solicitada · a gravação fecha após a cauda",
                "CODA requested · recording closes after the tail", "CODA demandée · l’enregistrement ferme après la queue",
                "CODA solicitada · la grabación se cierra tras la cola"));
            return;
        }
        // REC abre a tomada. Parado: a música começa junto, e a tomada começa
        // com a peça. Tocando: grava a partir daqui.
        takeStartedWithPlay_ = !playingRequested_;
        if (!beginRecording()) return;
        if (!playingRequested_) togglePlaying(true);
    }

    bool requestMorceauCoda(const juce::String& trace, const juce::String& action) {
        if (codaRequested_ || !controller_.beginMorceauCoda()) return false;
        codaRequested_ = true;
        appendTakeTrace(trace);
        // Keep this control ASCII-only: some local font/fallback paths render
        // the Unicode ellipsis as mojibake in the compact transport box.
        recordButton_.setButtonText("CODA");
        recordButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff9b6a3f));
        lastAction_ = action;
        return true;
    }

    void notePerformerActivity() noexcept {
        lastPerformerActivityMs_ = juce::Time::getMillisecondCounterHiRes();
    }

    bool beginRecording() {
        // A tomada nunca começa com uma seed diferente da que o campo mostra.
        applyTypedSeedIfChanged("antes-de-gravar");
        const auto rate = sampleRate_.load(std::memory_order_acquire);
        if (!ready_.load(std::memory_order_acquire) || rate < 8000.0) {
            lastAction_ = uiText(supportLanguage_, "INICIAR requer dispositivo de áudio pronto", "START requires a ready audio device",
                                 "DÉMARRER requiert un périphérique audio prêt", "INICIAR requiere un dispositivo de audio listo");
            return false;
        }
        // Uma pasta por tomada (3/11, pedido do autor: "organize as pastas"):
        // recordings/AAAA-MM-DD/HH-MM-SS seed N/, com a mistura, os stems e o
        // manifesto juntos; num álbum, <álbum>/FAIXA_nn HH-MM-SS/. A pasta de
        // dados mantém o nome antigo ("Rasgo Synth Performance", 27/10).
        const auto now = juce::Time::getCurrentTime();
        const auto seed = controller_.document() != nullptr ? controller_.document()->territory.seed : 0U;
        const auto trackTag = "FAIXA_" + juce::String(static_cast<int>(album_.current())).paddedLeft('0', 2);
        const auto directory = album_.active()
            ? albumDirectory_.getChildFile(trackTag + " " + now.formatted("%H-%M-%S"))
            : juce::File::getSpecialLocation(juce::File::userMusicDirectory)
                  .getChildFile("Rasgo Synth Performance").getChildFile("recordings")
                  .getChildFile(now.formatted("%Y-%m-%d"))
                  .getChildFile(now.formatted("%H-%M-%S") + " seed " + juce::String(static_cast<juce::int64>(seed)));
        const auto take = directory.getChildFile((album_.active() ? trackTag + "_" : juce::String("RASGO_ASTERISMUS_"))
            + now.formatted("%Y-%m-%d_%H-%M-%S") + ".wav");
        if (!recorder_.start(take, rate)) {
            lastAction_ = uiText(supportLanguage_, "REC não pôde abrir o arquivo", "REC could not open the file",
                                 "REC n’a pas pu ouvrir le fichier", "REC no pudo abrir el archivo");
            return false;
        }
        stemRecorder_.start(take, rate, moduleDisplayNames_);
        // Onde está gravando (2/11): na linha de status e na dica do REC.
        recordButton_.setTooltip(uiText(supportLanguage_, "Gravando em: ", "Recording to: ", "Enregistrement dans : ",
                                        "Grabando en: ") + take.getFullPathName());
        // A seed que a interface adotou (a fila do território pode ainda não
        // ter chegado ao motor neste instante).
        takeSeed_ = startupDocument_.territory.seed;
        takeStartedAt_ = juce::Time::getCurrentTime().toISO8601(true);
        takeSampleRate_ = rate;
        takeCodaGesture_ = "";
        takePercurso_ = percurso_;
        notePerformerActivity();
        stopClosePending_ = false;
        takeTrace_.clear();
        takeFieldTraceCount_ = 0U;
        lastFieldTraceMs_ = 0.0;
        lastTracedNarrativeTurn_ = std::numeric_limits<std::uint32_t>::max();
        takeTrace_.add("start:seed=" + juce::String(takeSeed_));
        recordButton_.setButtonText("CODA");
        recordButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffb95b46));
        lastAction_ = uiText(supportLanguage_, "GRAVANDO em ", "RECORDING to ", "ENREGISTREMENT dans ", "GRABANDO en ")
            + take.getFullPathName();
        return true;
    }

    static bool measureTakeAudio(const juce::File& take, PerformanceAudioMeasurement& result) {
        juce::WavAudioFormat format;
        std::unique_ptr<juce::InputStream> stream(take.createInputStream());
        if (stream == nullptr) return false;
        std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(stream.release(), true));
        if (reader == nullptr || reader->lengthInSamples <= 0) return false;
        PerformanceAudioMeasurementAccumulator accumulator;
        juce::AudioBuffer<float> buffer(2, 4096);
        for (juce::int64 position = 0; position < reader->lengthInSamples; position += buffer.getNumSamples()) {
            const auto count = static_cast<int>(std::min<juce::int64>(buffer.getNumSamples(), reader->lengthInSamples - position));
            buffer.clear();
            if (!reader->read(&buffer, 0, count, position, true, true)) return false;
            accumulator.push(buffer.getReadPointer(0), buffer.getReadPointer(1), static_cast<std::size_t>(count));
        }
        result = accumulator.result();
        return result.samplesPerChannel > 0U;
    }

    void closeRecording(const juce::String& prefix) {
        if (!recorder_.recording()) return;
        const auto acceptedSamples = recorder_.acceptedSamples();
        const auto droppedBlocks = recorder_.droppedBlocks();
        const auto take = recorder_.currentTake();
        const auto activeRate = sampleRate_.load(std::memory_order_acquire);
        const auto rate = takeSampleRate_ >= 8000.0 ? takeSampleRate_ : activeRate;
        recorder_.stop();
        const auto stemsRecorded = stemRecorder_.recording();
        const auto stemDroppedBlocks = stemRecorder_.droppedBlocks();
        stemRecorder_.stop();
        juce::String stemsJson = "[";
        if (stemsRecorded) {
            for (std::size_t stem = 0U; stem < PerformanceStemRecorder::kStems; ++stem) {
                if (stem > 0U) stemsJson += ", ";
                stemsJson += "{\"body\": " + juce::String(static_cast<int>(stem + 1U))
                    + ", \"wav\": \"" + stemRecorder_.file(stem).getFileName() + "\"}";
            }
        }
        stemsJson += "]";
        juce::String asterismJson = "[";
        for (std::size_t body = 0U; body < moduleDisplayNames_.size(); ++body) {
            if (body > 0U) asterismJson += ", ";
            asterismJson += "{\"body\": " + juce::String(static_cast<int>(body + 1U))
                + ", \"star\": \"" + moduleDisplayNames_[body] + "\"}";
        }
        asterismJson += "]";
        PerformanceAudioMeasurement measurement;
        const bool measurementAvailable = measureTakeAudio(take, measurement);
        // A loudness da tomada: a do motor quando a tomada começou com o PLAY
        // (a medição integrada recomeça no PLAY); a do próprio arquivo quando
        // o REC entrou com a música já tocando.
        PerformanceTrackLevel takeLevel;
        {
            const auto telemetry = controller_.readTelemetry();
            takeLevel.integratedLufs = telemetry.integratedCentiLufs / 100.0;
            takeLevel.maxTruePeakDbtp = 20.0 * std::log10(std::max(1.0e-6, telemetry.maxTruePeakMicros / 1000000.0));
            if (!takeStartedWithPlay_)
                if (const auto measured = measureWavLevel(take.getFullPathName().toStdString())) takeLevel = *measured;
        }
        const auto state = codaRequested_ ? "finalizado" : "interrompido";
        const auto manifest = take.withFileExtension(".json");
        const auto seconds = rate > 0.0 ? static_cast<double>(acceptedSamples) / rate : 0.0;
        const auto jsonString = [](juce::String value) {
            value = value.replace("\\", "\\\\").replace("\"", "\\\"").replace("\n", "\\n");
            return "\"" + value + "\"";
        };
        juce::String traceJson;
        for (int index = 0; index < takeTrace_.size(); ++index) {
            if (index > 0) traceJson += ", ";
            traceJson += jsonString(takeTrace_[index]);
        }
        juce::String modulesJson{"["};
        juce::String relationsJson{"["};
        if (const auto* document = controller_.document(); document != nullptr) {
            const auto sceneId = controller_.currentSceneId();
            const PerformanceScene* scene = nullptr;
            for (std::size_t index = 0U; index < document->sceneCount; ++index) {
                if (document->scenes[index].id == sceneId) {
                    scene = &document->scenes[index];
                    break;
                }
            }
            if (scene == nullptr && document->sceneCount > 0U) scene = &document->scenes[0];
            if (scene != nullptr) {
                for (std::size_t index = 0U; index < scene->constellation.actorCount; ++index) {
                    const auto& actor = scene->constellation.actors[index];
                    if (index > 0U) modulesJson += ", ";
                    const auto moduleIndex = actor.actorId > 0U ? static_cast<std::size_t>(actor.actorId - 1U) : 0U;
                    modulesJson += "{\"id\":" + juce::String(actor.actorId)
                        + ",\"module\":" + jsonString(moduleDisplayNameForIndex(moduleIndex))
                        + ",\"role\":" + jsonString(roleArchiveName(actor.role))
                        + ",\"presence\":" + juce::String(actor.presence, 4) + "}";
                }
                for (std::size_t index = 0U; index < scene->constellation.relationCount; ++index) {
                    const auto& relation = scene->constellation.relations[index];
                    if (relation.intensity <= 0.005) continue;
                    if (relationsJson.length() > 1) relationsJson += ", ";
                    relationsJson += "{\"source\":" + juce::String(relation.sourceActorId)
                        + ",\"destination\":" + juce::String(relation.destinationActorId)
                        + ",\"intensity\":" + juce::String(relation.intensity, 4)
                        + ",\"phenomenon\":"
                        + jsonString(relationalPhenomenonLabel(relation.phenomenon)) + "}";
                }
            }
        }
        modulesJson += "]";
        relationsJson += "]";
        const auto manifestText = juce::String("{\n")
            + "  \"format\": \"RASGO_PERFORMANCE_TAKE\",\n"
            + "  \"version\": 1,\n"
            + "  \"startedAt\": \"" + takeStartedAt_ + "\",\n"
            + "  \"wav\": \"" + take.getFileName() + "\",\n"
            + "  \"seed\": " + juce::String(takeSeed_) + ",\n"
            + "  \"sampleRate\": " + juce::String(rate, 1) + ",\n"
            + "  \"channels\": 2,\n"
            + "  \"acceptedSamples\": " + juce::String(static_cast<juce::int64>(acceptedSamples)) + ",\n"
            + "  \"durationSeconds\": " + juce::String(seconds, 6) + ",\n"
            + "  \"targetSeconds\": " + (takeTargetSeconds_ > 0.0 ? juce::String(takeTargetSeconds_, 0) : juce::String("null")) + ",\n"
            + "  \"droppedBlocks\": " + juce::String(droppedBlocks) + ",\n"
            + "  \"asterism\": " + asterismJson + ",\n"
            + "  \"mixerVariation\": " + (controller_.mixerVariation() ? "true" : "false") + ",\n"
            + "  \"starCapacity\": " + juce::String(static_cast<int>(controller_.starCapacity())) + ",\n"
            // A medição da peça (BS.1770, desde o PLAY; docs/SAIDA_DE_AUDIO.md).
            + "  \"loudness\": {\"integratedLufs\": " + juce::String(takeLevel.integratedLufs, 2)
                + ", \"maxTruePeakDbtp\": " + juce::String(takeLevel.maxTruePeakDbtp, 2)
                + ", \"measuredFrom\": \"" + (takeStartedWithPlay_ ? "engine-since-play" : "take-file") + "\""
                + ", \"limiterMaxReductionDb\": "
                + juce::String(controller_.readTelemetry().limiterMaxReductionCentiDb / 100.0, 2)
                + ", \"stage\": \"post-safety\"},\n"
            + "  \"stems\": " + stemsJson + ",\n"
            + "  \"stemsNote\": \"um WAV mono 24-bit por corpo, antes da espacialização global; a soma reconstrói a mistura mono\",\n"
            + "  \"stemDroppedBlocks\": " + juce::String(stemDroppedBlocks) + ",\n"
            + "  \"analysis\": " + (measurementAvailable ? juce::String("{\"samplePeakDbfs\": ")
                    + juce::String(measurement.dbfs(measurement.samplePeak), 3)
                    + ", \"estimatedInterSamplePeakDbfs\": "
                    + juce::String(measurement.dbfs(measurement.estimatedInterSamplePeak), 3)
                    + ", \"integratedRmsDbfs\": "
                    + juce::String(measurement.dbfs(measurement.integratedRms), 3) + "}"
                : juce::String("null")) + ",\n"
            + "  \"state\": \"" + state + "\",\n"
            + "  \"codaGesture\": " + (takeCodaGesture_.isNotEmpty() ? jsonString(takeCodaGesture_) : "null") + ",\n"
            + "  \"modules\": " + modulesJson + ",\n"
            + "  \"relations\": " + relationsJson + ",\n"
            + "  \"trace\": [" + traceJson + "]\n}\n";
        const bool manifestWritten = manifest.replaceWithText(manifestText);
        if (album_.active() && take.isAChildOf(albumDirectory_)) {
            // O registro de faixas (album.json): a tomada entra com a faixa, a
            // seed, o andamento, a memória lembrada, a duração e a loudness.
            PerformanceAlbumTake entry;
            entry.track = album_.current();
            entry.seed = takeSeed_;
            entry.bpm = tempoSlider_.getValue();
            entry.file = take.getRelativePathFrom(albumDirectory_).toStdString();
            entry.memory = lastRecalledMemory_;
            entry.started = takeStartedAt_.toStdString();
            entry.seconds = seconds;
            entry.integratedLufs = takeLevel.integratedLufs;
            entry.maxTruePeakDbtp = takeLevel.maxTruePeakDbtp;
            entry.percurso = takePercurso_;
            album_.addTake(entry);
            writeAlbumRegister();
            refreshAlbumButtons();
        }
        recordButton_.setButtonText("REC");
        recordButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff6b5035));
        if (takeTargetSeconds_ > 0.0) {
            // A faixa acabou: a forma volta aos compassos da seed.
            controller_.setFormPlan({{0U, 0U, 0U, 0U}});
            takeTargetSeconds_ = 0.0;
            for (auto& button : durationButtons_) button.setToggleState(false, juce::dontSendNotification);
        }
        lastAction_ = prefix + utf8(" · ")
            + juce::String(rate > 0.0 ? static_cast<double>(acceptedSamples) / rate : 0.0, 1)
            + utf8(" s · ") + take.getFileName()
            + (droppedBlocks > 0U ? utf8(" · fila cheia ") + juce::String(droppedBlocks) : juce::String{})
            + (manifestWritten ? utf8(" · manifesto") : utf8(" · manifesto falhou"));
        // ▶ OUVIR (4/11, proposta aprovada): a tomada que acabou de fechar,
        // no player do sistema.
        if (take.existsAsFile()) {
            lastTakeFile_ = take;
            listenButton_.setTooltip(uiText(supportLanguage_, "Abre a última tomada no player do sistema · ",
                                            "Opens the last take in the system player · ",
                                            "Ouvre la dernière prise dans le lecteur du système · ",
                                            "Abre la última toma en el reproductor del sistema · ")
                                     + take.getFullPathName());
            listenButton_.setVisible(true);
        }
    }

    void listenToLastTake() {
        if (!lastTakeFile_.existsAsFile()) {
            listenButton_.setVisible(false);
            lastAction_ = uiText(supportLanguage_, "a última tomada não está mais lá", "the last take is gone",
                                 "la dernière prise n’est plus là", "la última toma ya no está");
            repaint();
            return;
        }
        if (!lastTakeFile_.startAsProcess())
            lastAction_ = uiText(supportLanguage_, "nenhum player abriu a tomada · ", "no player opened the take · ",
                                 "aucun lecteur n’a ouvert la prise · ", "ningún reproductor abrió la toma · ")
                + lastTakeFile_.getFullPathName();
        repaint();
    }

    // O campo SEED e a seed que toca nunca divergem: o que estiver digitado e
    // for diferente da seed atual é aplicado (ao sair do campo e antes de gravar).
    void applyTypedSeedIfChanged(const char* source) {
        const auto text = seedEditor_.getText().trim();
        if (text.isEmpty()) {
            seedEditor_.setText(juce::String(startupDocument_.territory.seed), false);
            return;
        }
        if (text.getLargeIntValue() != static_cast<juce::int64>(startupDocument_.territory.seed))
            queueSeedTerritory(source);
    }

    void queueSeedTerritory(const char* source = "enter") {
        const auto entered = seedEditor_.getText().getLargeIntValue();
        if (entered < 0 || entered > static_cast<juce::int64>(std::numeric_limits<std::uint32_t>::max())) {
            lastAction_ = uiText(supportLanguage_, "semente inválida", "invalid seed", "seed invalide", "seed inválida");
            return;
        }
        auto territory = PerformanceTerritoryFactory::makeForReleasedStems(
            static_cast<std::uint32_t>(entered),
            kPerformanceSafeBodyLimit);
        if (!controller_.queueTerritory(territory)) {
            lastAction_ = uiText(supportLanguage_, "território não enfileirado", "territory not queued",
                                 "territoire non mis en file", "territorio no encolado");
            return;
        }
        adoptDocument(territory, 1U);
        // Toda troca de seed fica no diagnóstico, com a origem.
        if (diagnosticLog_ != nullptr)
            diagnosticLog_->writeText("seed," + juce::Time::getCurrentTime().formatted("%H:%M:%S") + ","
                + juce::String(static_cast<juce::int64>(startupDocument_.territory.seed)) + "," + juce::String(source)
                + "\n", false, false, nullptr);
        lastAction_ = uiText(supportLanguage_, "território ", "territory ", "territoire ", "territorio ")
            + juce::String(startupDocument_.territory.seed)
            + uiText(supportLanguage_, " enfileirado · transição ao vivo", " queued · live transition",
                      " mis en file · transition en direct", " encolado · transición en vivo");
    }

    // Um documento novo (seed nova ou memória lembrada): a superfície do
    // performer volta ao começo — mix, mute/solo, controles locais, variação
    // da seed — e as trocas e fixações de estrela se desfazem (2/11: uma seed
    // nova é uma peça nova; a memória reaplica as suas depois).
    void adoptDocument(const PerformanceSessionDocument& document, std::uint16_t sceneId) {
        startupDocument_ = document;
        lastRecalledMemory_.clear();
        percursoSection_ = 255U;
        notePerformerActivity();
        for (std::size_t index = 0U; index < starSwaps_.size(); ++index) {
            const auto actorId = static_cast<std::uint16_t>(index + 1U);
            if (starSwaps_[index] != PerformanceModuleBody::Count) controller_.setActorStar(actorId, PerformanceModuleBody::Count);
            if (starPinned_[index]) controller_.setActorPinned(actorId, false);
        }
        starSwaps_.fill(PerformanceModuleBody::Count);
        starPinned_.fill(false);
        constellationGraph_.setPinnedNodes(starPinned_);
        refreshCohortIdentity(startupDocument_.territory.seed);
        syncControlsFromScene(sceneId, true);
        seedEditor_.setText(juce::String(startupDocument_.territory.seed), false);
        stemGainValues_.fill(1.0);
        for (std::size_t index = 0U; index < stemGainSliders_.size(); ++index)
            stemGainSliders_[index].setValue(1.0, juce::dontSendNotification);
        stemMuted_.fill(false);
        soloStemIndex_ = kNoStemIndex;
        moduleControlValues_ = initialModuleControls(startupDocument_.territory.seed);
        actorSeedVariationValues_.fill(0.5);
        selectedModuleIndex_ = kNoStemIndex;
        walshControlValues_ = moduleControlValues_[3U];
        for (std::size_t control = 0U; control < walshControlSliders_.size(); ++control)
            walshControlSliders_[control].setValue(walshControlValues_[control], juce::dontSendNotification);
        for (std::size_t index = 0U; index < moduleControlSliders_.size(); ++index)
            for (std::size_t control = 0U; control < moduleControlSliders_[index].size(); ++control)
                moduleControlSliders_[index][control].setValue(
                    moduleControlValues_[index][control], juce::dontSendNotification);
        refreshStemMixVisual();
        hasUndoCapture_ = false;
        undoCaptureButton_.setEnabled(false);
        sceneDirty_ = false;
    }

    void appendTakeTrace(const juce::String& event) {
        if (recorder_.recording() && takeTrace_.size() < 256) takeTrace_.add(event);
    }

    void appendFieldTrace(std::uint16_t actorId, double x, double y) {
        if (!recorder_.recording() || takeFieldTraceCount_ >= 64U) return;
        const auto now = juce::Time::getMillisecondCounterHiRes();
        if (now - lastFieldTraceMs_ < 250.0) return;
        lastFieldTraceMs_ = now;
        ++takeFieldTraceCount_;
        appendTakeTrace("field:" + juce::String(actorId) + ":"
            + juce::String(x, 3) + ":" + juce::String(y, 3));
    }

    void rollAleatorySeed() {
        // ALEA is a control-side dice roll. It never runs in the audio callback:
        // the resulting seed is displayed first and then follows the same
        // bounded territory queue as a manually entered reproducible seed.
        const auto seed = static_cast<std::uint32_t>(juce::Random::getSystemRandom().nextInt());
        seedEditor_.setText(juce::String(static_cast<juce::int64>(seed)), false);
        queueSeedTerritory("alea");
    }

    bool setActorSeedVariationControl(std::size_t index, double amount) {
        if (index >= actorSeedVariationValues_.size()) return false;
        const auto bounded = juce::jlimit(0.0, 1.0, amount);
        if (!controller_.setActorSeedVariation(static_cast<std::uint16_t>(index + 1U), bounded)) {
            actorSeedVariationSlider_.setValue(actorSeedVariationValues_[index],
                                                juce::dontSendNotification);
            lastAction_ = uiText(supportLanguage_, "variação local não entrou na fila realtime",
                                 "local variation did not enter the realtime queue",
                                 "la variation locale n’est pas entrée dans la file temps réel",
                                 "la variación local no entró en la cola de tiempo real");
            return false;
        }
        actorSeedVariationValues_[index] = bounded;
        actorSeedVariationSlider_.setValue(bounded, juce::dontSendNotification);
        lastAction_ = moduleDisplayNames_[index] + uiText(supportLanguage_, " · SEED VIVA ", " · LIVE SEED ",
            " · SEED VIVANTE ", " · SEED VIVA ") + juce::String(bounded, 2)
            + uiText(supportLanguage_, " · Campo preservado", " · Field preserved",
                      " · Champ préservé", " · Campo preservado");
        return true;
    }

    void setTempoFromSlider() {
        const auto entered = tempoSlider_.getValue();
        if (entered < 20.0 || entered > 300.0 || !controller_.setTempoBpm(entered)) {
            lastAction_ = uiText(supportLanguage_, "tempo inválido ou não enfileirado",
                                 "invalid tempo or not queued", "tempo invalide ou non mis en file",
                                 "tempo no válido o no encolado");
            return;
        }
        lastAction_ = "TEMPO " + juce::String(entered, 0) + " BPM";
        sceneDirty_ = true;
    }

    void adjustTempo(double delta) {
        const auto current = tempoSlider_.getValue();
        tempoSlider_.setValue(juce::jlimit(20.0, 300.0, current + delta));
    }

    void showAbout() {
        juce::AlertWindow::showMessageBoxAsync(
            juce::MessageBoxIconType::NoIcon,
            utf8("RASGO SYNTH · ASTERISMUS · ")
                + uiText(supportLanguage_, "SOBRE", "ABOUT", "À PROPOS", "ACERCA DE"),
            aboutCopy(supportLanguage_) + libraryAboutLine());
    }

    // As contagens da biblioteca que saíram da área de tocar (3/11).
    juce::String libraryAboutLine() const {
        std::size_t adapted = 0U, candidates = 0U, audit = 0U;
        for (const auto& entry : kPerformanceModuleLibrary) {
            adapted += entry.state == PerformanceLibraryState::ActiveCohort;
            candidates += entry.state == PerformanceLibraryState::RealtimeCandidate;
            audit += entry.state == PerformanceLibraryState::NeedsAudit;
        }
        const auto n = [](std::size_t value) { return juce::String(static_cast<int>(value)); };
        return utf8("\n\n") + uiText(supportLanguage_, "Biblioteca: ", "Library: ", "Bibliothèque : ", "Biblioteca: ")
            + n(kPerformanceModuleLibrary.size())
            + uiText(supportLanguage_, " módulos — ", " modules — ", " modules — ", " módulos — ") + n(adapted)
            + uiText(supportLanguage_, " estrelas tocáveis (a seed escolhe entre todas), ",
                     " playable stars (the seed chooses among all), ",
                     " étoiles jouables (la seed choisit parmi toutes), ",
                     " estrellas tocables (la seed elige entre todas), ")
            + n(candidates)
            + uiText(supportLanguage_, " com adaptador próprio (Modal), ", " with its own adapter (Modal), ",
                     " avec son propre adaptateur (Modal), ", " con adaptador propio (Modal), ")
            + n(audit) + uiText(supportLanguage_, " em auditoria.", " under audit.", " en audit.", " en auditoría.");
    }

    void showTutorial() {
        juce::DialogWindow::LaunchOptions options;
        options.dialogTitle = uiText(supportLanguage_, "TUTORIAL", "GUIDE", "TUTORIEL", "TUTORIAL");
        options.dialogBackgroundColour = juce::Colour(0xff102d27);
        options.content.setOwned(new PerformanceTutorialContent(supportLanguage_));
        options.componentToCentreAround = this;
        options.escapeKeyTriggersCloseButton = true;
        options.useNativeTitleBar = false;
        options.resizable = true;
        options.useBottomRightCornerResizer = true;
        options.launchAsync();
    }

    void showLanguageMenu() {
        juce::PopupMenu menu;
        // PopupMenu recebe texto UTF-8 por outra rota do renderer JUCE em
        // alguns desktops e exibia o byte do separador como “Â”. O menu usa
        // hífen ASCII para não introduzir ruído de codificação.
        menu.addItem(1, utf8("PT - Português"), true, supportLanguage_ == SupportLanguage::Portuguese);
        menu.addItem(2, "EN - English", true, supportLanguage_ == SupportLanguage::English);
        menu.addItem(3, utf8("FR - Français"), true, supportLanguage_ == SupportLanguage::French);
        menu.addItem(4, utf8("ES - Español"), true, supportLanguage_ == SupportLanguage::Spanish);
        juce::Component::SafePointer<PerformanceHostComponent> safeThis(this);
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&languageButton_),
            [safeThis](int result) {
                if (auto* host = safeThis.getComponent()) {
                    if (result < 1 || result > 4) return;
                    host->setSupportLanguage(static_cast<SupportLanguage>(result - 1));
                }
            });
    }

    // Variação do mixer: a seed e o movimento da Constelação acendem, apagam
    // e misturam as estrelas; desligada, o mixer é só do músico.
    void refreshMixerVariationButton() {
        const bool web = controller_.skyWeb();
        skyWebButton_.setButtonText(web
            ? uiText(supportLanguage_, "TRAMA · LIGADA", "WEB · ON", "TRAME · ACTIVE", "TRAMA · ACTIVA")
            : uiText(supportLanguage_, "TRAMA · DESLIGADA", "WEB · OFF", "TRAME · COUPÉE", "TRAMA · APAGADA"));
        skyWebButton_.setColour(juce::TextButton::buttonColourId,
                                web ? juce::Colour(0xff8a5a2e) : juce::Colour(0xff285348));
        skyWebButton_.setTooltip(uiText(supportLanguage_,
            "Trama do céu · ligada: as estrelas acesas se relacionam — respostas com atraso pela distância, contágio, sombra, hoquetus e pedal, com poucas relações em foco por compasso. Desligada: cada estrela segue só a sua própria frase.",
            "Sky web · on: lit stars relate — answers delayed by distance, contagion, shadow, hocket and pedal, with few relations in focus per bar. Off: each star follows only its own phrase.",
            "Trame du ciel · active : les étoiles allumées se relient — réponses retardées par la distance, contagion, ombre, hoquet et pédale, peu de relations en focus par mesure. Coupée : chaque étoile suit seulement sa propre phrase.",
            "Trama del cielo · activa: las estrellas encendidas se relacionan — respuestas con retardo por la distancia, contagio, sombra, hoquetus y pedal, con pocas relaciones en foco por compás. Apagada: cada estrella sigue solo su propia frase."));
        const bool on = controller_.mixerVariation();
        mixerVariationButton_.setButtonText(on
            ? uiText(supportLanguage_, "VARIAÇÃO · LIGADA", "VARIATION · ON", "VARIATION · ACTIVE", "VARIACIÓN · ACTIVA")
            : uiText(supportLanguage_, "VARIAÇÃO · DESLIGADA", "VARIATION · OFF", "VARIATION · COUPÉE", "VARIACIÓN · APAGADA"));
        mixerVariationButton_.setColour(juce::TextButton::buttonColourId,
                                        on ? juce::Colour(0xff8a5a2e) : juce::Colour(0xff285348));
        mixerVariationButton_.setTooltip(uiText(supportLanguage_,
            "Variação do mixer · ligada: a seed escolhe quais estrelas abrem acesas; ao longo da peça elas perdem o brilho, apagam e reacendem, e a mistura acompanha o movimento da Constelação (quem se aproxima do centro da figura sobe). Seus faders continuam valendo: subir um fader acende a estrela. Desligada: o mixer é só seu.",
            "Mixer variation · on: the seed chooses which stars open lit; during the piece they fade, go out and re-ignite, and the mix follows the Constellation's movement (stars nearing the figure's centre rise). Your faders still apply: raising one lights its star. Off: the mixer is yours alone.",
            "Variation du mixeur · active : la seed choisit quelles étoiles s’ouvrent allumées ; au fil de la pièce elles pâlissent, s’éteignent et se rallument, et le mélange suit le mouvement de la Constellation (celles qui approchent du centre de la figure montent). Vos faders restent valables : en monter un allume son étoile. Coupée : le mixeur est à vous seul.",
            "Variación del mezclador · activa: la seed elige qué estrellas abren encendidas; durante la pieza pierden brillo, se apagan y se reencienden, y la mezcla sigue el movimiento de la Constelación (las que se acercan al centro de la figura suben). Sus faders siguen valiendo: subir uno enciende su estrella. Apagada: el mezclador es solo suyo."));
    }

    void refreshInterfaceLanguage() {
        listenButton_.setButtonText(uiText(supportLanguage_, "▶ OUVIR", "▶ LISTEN", "▶ ÉCOUTER", "▶ ESCUCHAR"));
        const auto l = supportLanguage_;
        refreshMixerVariationButton();
        performanceField_.setLanguage(l);
        constellationGraph_.setLanguage(l);
        refreshLocalizedModuleControlText();
        refreshRelationInterfaceText();
        transportButton_.setTooltip(uiText(l,
            "Espaço · INICIAR abre a tomada e PLAY; STOP interrompe e preserva o WAV válido.",
            "Space · START opens the take and PLAY; STOP interrupts while preserving a valid WAV.",
            "Espace · DÉMARRER ouvre la prise et PLAY ; STOP interrompt en préservant un WAV valide.",
            "Espacio · INICIAR abre la toma y PLAY; STOP interrumpe y conserva un WAV válido."));
        recordButton_.setTooltip(uiText(l,
            "V · REC começa a gravar (e a tocar, se estiver parado); gravando, o botão vira CODA e pede o fim composto. PLAY só toca.",
            "V · REC starts recording (and playing, if stopped); while recording the button becomes CODA and requests the composed ending. PLAY only plays.",
            "V · REC lance l’enregistrement (et la lecture, si arrêtée) ; en enregistrement le bouton devient CODA et demande la fin composée. PLAY joue seulement.",
            "V · REC empieza a grabar (y a tocar, si está detenido); grabando, el botón pasa a CODA y solicita el final compuesto. PLAY solo toca."));
        tutorialButton_.setButtonText(uiText(l, "TUTORIAL", "GUIDE", "TUTORIEL", "TUTORIAL"));
        tutorialButton_.setTooltip(uiText(l,
            "Abre um guia do Campo, Constelação, seed, gestos e gravação.",
            "Opens a guide to the Field, Constellation, seed, gestures and recording.",
            "Ouvre un guide du Champ, de la Constellation, de la seed, des gestes et de l’enregistrement.",
            "Abre una guía del Campo, la Constelación, la seed, los gestos y la grabación."));
        aboutButton_.setButtonText(uiText(l, "SOBRE", "ABOUT", "À PROPOS", "ACERCA DE"));
        aboutButton_.setTooltip(uiText(l,
            "Créditos, autoria, licença e proveniência do Rasgo Synth Asterismus.",
            "Credits, authorship, licence and provenance of Rasgo Synth Asterismus.",
            "Crédits, paternité, licence et provenance de Rasgo Synth Asterismus.",
            "Créditos, autoría, licencia y procedencia de Rasgo Synth Asterismus."));
        languageButton_.setTooltip(uiText(l,
            "Idioma da interface, Sobre e Tutorial · PT, EN, FR, ES.",
            "Language for the interface, About and Guide · PT, EN, FR, ES.",
            "Langue de l’interface, À propos et Tutoriel · PT, EN, FR, ES.",
            "Idioma de la interfaz, Acerca de y Tutorial · PT, EN, FR, ES."));
        seedLabel_.setText(uiText(l, "SEED / ASTERISMO", "SEED / ASTERISM", "SEED / ASTÉRISME", "SEED / ASTERISMO"), juce::dontSendNotification);
        seedLabel_.setTooltip(uiText(l,
            "Escolha a seed que define o próximo território.", "Choose the seed that defines the next territory.",
            "Choisissez la seed qui définit le prochain territoire.", "Elija la seed que define el próximo territorio."));
        seedEditor_.setTooltip(uiText(l,
            "Seed do próximo território. Pare o transporte antes de aplicar.",
            "Seed for the next territory. Stop transport before applying it.",
            "Seed du prochain territoire. Arrêtez le transport avant de l’appliquer.",
            "Seed del próximo territorio. Detenga el transporte antes de aplicarla."));
        seedButton_.setTooltip(uiText(l,
            "Lança o dado: sorteia uma seed e desenha um novo território.",
            "Rolls the dice: chooses a seed and draws a new territory.",
            "Lance le dé : choisit une seed et dessine un nouveau territoire.",
            "Lanza el dado: elige una seed y dibuja un nuevo territorio."));
        actorSeedVariationLabel_.setText(uiText(l, "ORIGEM ↔ VARIAÇÃO", "ORIGIN ↔ VARIATION",
            "ORIGINE ↔ VARIATION", "ORIGEN ↔ VARIACIÓN"), juce::dontSendNotification);
        actorSeedVariationLabel_.setTooltip(uiText(l,
            "Varia somente o astro em foco a partir da seed viva; 0 preserva sua origem.",
            "Varies only the focused body from the live seed; 0 preserves its origin.",
            "Varie seulement l’astre focal depuis la seed vivante ; 0 préserve son origine.",
            "Varía solo el astro en foco desde la seed viva; 0 preserva su origen."));
        actorSeedVariationSlider_.setTooltip(uiText(l,
            "0 = origem deste astro; 1 = variação plena. Não move nem substitui módulos.",
            "0 = this body’s origin; 1 = full variation. It neither moves nor replaces modules.",
            "0 = origine de cet astre ; 1 = variation complète. Ne déplace ni ne remplace les modules.",
            "0 = origen de este astro; 1 = variación plena. No mueve ni sustituye módulos."));
        performanceField_.setTooltip(uiText(l,
            "Eixo de forças · laranja: autonomia, intervenção, memória e risco; azul: densidade, tensão, espaço e profundidade.",
            "Axis of forces · orange: autonomy, intervention, memory and risk; blue: density, tension, space and depth.",
            "Axe des forces · orange : autonomie, intervention, mémoire et risque ; bleu : densité, tension, espace et profondeur.",
            "Eje de fuerzas · naranja: autonomía, intervención, memoria y riesgo; azul: densidad, tensión, espacio y profundidad."));
        constellationGraph_.setTooltip(uiText(l,
            "Constelação no Campo · clique no vazio para foco coletivo; clique num cabo para escolher fenômeno e arraste-o para intensidade.",
            "Constellation in the Field · click empty space for collective focus; click a cable to choose a phenomenon and drag it for intensity.",
            "Constellation dans le Champ · cliquez dans le vide pour le focus collectif ; cliquez un câble pour choisir un phénomène et le glisser pour son intensité.",
            "Constelación en el Campo · haga clic en el vacío para foco colectivo; haga clic en un cable para elegir fenómeno y arrástrelo para su intensidad."));
        for (std::size_t index = 0U; index < stemGainSliders_.size(); ++index) {
            stemGainSliders_[index].setTooltip(uiText(l,
                "Ganho do stem · 0 silencia por fade; 1 mantém o nível do ator.",
                "Stem gain · 0 fades to silence; 1 keeps the actor level.",
                "Gain du stem · 0 réduit au silence par fondu ; 1 conserve le niveau de l’acteur.",
                "Ganancia del stem · 0 silencia con fundido; 1 mantiene el nivel del actor."));
            stemMuteButtons_[index].setTooltip(uiText(l,
                "Silencia somente este stem com fade; não interrompe os outros atores.",
                "Mutes only this stem with a fade; does not interrupt other actors.",
                "Coupe seulement ce stem avec un fondu ; n’interrompt pas les autres acteurs.",
                "Silencia solo este stem con fundido; no interrumpe los demás actores."));
            stemSoloButtons_[index].setTooltip(uiText(l,
                "Isola este stem; clique outra vez para devolver os demais atores.",
                "Isolates this stem; click again to return the other actors.",
                "Isole ce stem ; cliquez à nouveau pour rendre les autres acteurs.",
                "Aísla este stem; haga clic de nuevo para devolver los demás actores."));
        }
        tempoLabel_.setText(uiText(l, "TEMPO / BPM", "TEMPO / BPM", "TEMPO / BPM", "TEMPO / BPM"), juce::dontSendNotification);
        tempoEditor_.setTooltip(uiText(l,
            "TEMPO em BPM · Enter aplica; , e . ajustam 2 BPM.",
            "TEMPO in BPM · Enter applies; , and . adjust 2 BPM.",
            "TEMPO en BPM · Entrée applique ; , et . ajustent 2 BPM.",
            "TEMPO en BPM · Enter aplica; , y . ajustan 2 BPM."));
        tempoSlider_.setTooltip(uiText(l,
            "TEMPO variável em BPM · 20–300. Arraste o slider ou edite o valor.",
            "Variable TEMPO in BPM · 20–300. Drag the slider or edit the value.",
            "TEMPO variable en BPM · 20–300. Glissez le slider ou éditez la valeur.",
            "TEMPO variable en BPM · 20–300. Arrastre el slider o edite el valor."));
        masterLabel_.setText(uiText(l, "MASTER / dB", "MASTER / dB", "MASTER / dB", "MASTER / dB"), juce::dontSendNotification);
        masterSlider_.setTooltip(uiText(l,
            "MASTER global · −24 a 0 dB. Cenas não o alteram.",
            "Global MASTER · −24 to 0 dB. Scenes do not alter it.",
            "MASTER global · −24 à 0 dB. Les scènes ne le modifient pas.",
            "MASTER global · −24 a 0 dB. Las escenas no lo alteran."));
        static constexpr std::array<const char*, 8> macroPt{
            "AUTO", "INTERV.", "DENS.", "TENS.", "MEM.", "RISCO", "ESPAÇO", "PROF."};
        static constexpr std::array<const char*, 8> macroEn{
            "AUTO", "INTERV.", "DENS.", "TENS.", "MEM.", "RISK", "SPACE", "DEPTH"};
        static constexpr std::array<const char*, 8> macroFr{
            "AUTO", "INTERV.", "DENS.", "TENS.", "MÉM.", "RISQUE", "ESPACE", "PROF."};
        static constexpr std::array<const char*, 8> macroEs{
            "AUTO", "INTERV.", "DENS.", "TENS.", "MEM.", "RIESGO", "ESPACIO", "PROF."};
        static constexpr std::array<const char*, 8> macroHintPt{
            "Autonomia: amplia a deriva tonal e temporal dos atores.",
            "Intervenção: injeta cor cruzada no sinal de cada ator.",
            "Densidade de matéria ativa.", "Tensão da sessão.",
            "Memória: deixa um resíduo de fase mais presente.",
            "Risco: aumenta cicatriz, harmônicos e excitação modal.",
            "Espaço: abre os canais e altera o desenho espacial.",
            "Profundidade: eixo Z, planos de filtragem e presença."};
        static constexpr std::array<const char*, 8> macroHintEn{
            "Autonomy: expands tonal and temporal drift of actors.",
            "Intervention: injects cross-colour into each actor signal.",
            "Density of active material.", "Session tension.",
            "Memory: leaves a more present phase residue.",
            "Risk: increases scars, harmonics and modal excitation.",
            "Space: opens channels and changes the spatial drawing.",
            "Depth: Z axis, filtering planes and presence."};
        static constexpr std::array<const char*, 8> macroHintFr{
            "Autonomie : élargit la dérive tonale et temporelle des acteurs.",
            "Intervention : injecte une couleur croisée dans chaque signal.",
            "Densité de matière active.", "Tension de session.",
            "Mémoire : laisse un résidu de phase plus présent.",
            "Risque : augmente cicatrice, harmoniques et excitation modale.",
            "Espace : ouvre les canaux et change le dessin spatial.",
            "Profondeur : axe Z, plans de filtrage et présence."};
        static constexpr std::array<const char*, 8> macroHintEs{
            "Autonomía: amplía la deriva tonal y temporal de los actores.",
            "Intervención: inyecta color cruzado en cada señal.",
            "Densidad de materia activa.", "Tensión de la sesión.",
            "Memoria: deja un residuo de fase más presente.",
            "Riesgo: aumenta cicatriz, armónicos y excitación modal.",
            "Espacio: abre canales y cambia el dibujo espacial.",
            "Profundidad: eje Z, planos de filtrado y presencia."};
        for (std::size_t index = 0U; index < macroLabels_.size(); ++index) {
            macroLabels_[index].setText(uiText(l, macroPt[index], macroEn[index], macroFr[index], macroEs[index]),
                                        juce::dontSendNotification);
            macroSliders_[index].setTooltip(uiText(l, macroHintPt[index], macroHintEn[index],
                                                   macroHintFr[index], macroHintEs[index]));
        }
        captureButton_.setButtonText(uiText(l, "CAPTURAR", "CAPTURE", "CAPTURER", "CAPTURAR"));
        undoCaptureButton_.setButtonText(uiText(l, "DESFAZER", "UNDO", "ANNULER", "DESHACER"));
        memoryButton_.setButtonText(uiText(l, "MEMÓRIA", "MEMORY", "MÉMOIRE", "MEMORIA"));
        refreshAlbumButtons();
        for (std::size_t index = 0U; index < durationButtons_.size(); ++index)
            durationButtons_[index].setTooltip(uiText(l, "Grava uma faixa de ", "Records a track of ",
                                                      "Enregistre une piste de ", "Graba una pista de ")
                + juce::String(static_cast<int>(kTakeDurations[index] / 60.0))
                + uiText(l, " min: o REC começa já; a forma do céu se encaixa na duração (abertura, desenvolvimento, contraste, retorno) e a coda chega no fim.",
                         " min: REC starts at once; the sky's form fits the duration (opening, development, contrast, return) and the coda arrives at the end.",
                         " min : REC démarre aussitôt ; la forme du ciel s’ajuste à la durée et la coda arrive à la fin.",
                         " min: REC empieza ya; la forma del cielo se ajusta a la duración y la coda llega al final."));
        percursoButton_.setTooltip(uiText(l,
            "As cenas seguem a forma do céu: NASCENTE (abertura), ESPIRAL (desenvolvimento), RASGO (contraste), BALANÇO (retorno). Gravando, o fim do retorno pede a coda.",
            "Scenes follow the sky's form: NASCENTE (opening), ESPIRAL (development), RASGO (contrast), BALANÇO (return). While recording, the end of the return asks for the coda.",
            "Les scènes suivent la forme du ciel : NASCENTE (ouverture), ESPIRAL (développement), RASGO (contraste), BALANÇO (retour). En enregistrement, la fin du retour demande la coda.",
            "Las escenas siguen la forma del cielo: NASCENTE (apertura), ESPIRAL (desarrollo), RASGO (contraste), BALANÇO (retorno). Grabando, el fin del retorno pide la coda."));
        albumButton_.setTooltip(uiText(l,
            "Um álbum: a seed do álbum deriva a seed e o andamento de cada faixa; cada tomada gravada entra no registro (album.json).",
            "An album: the album seed derives each track's seed and tempo; each recorded take enters the register (album.json).",
            "Un album : la seed de l’album dérive la seed et le tempo de chaque piste ; chaque prise entre au registre (album.json).",
            "Un álbum: la seed del álbum deriva la seed y el tempo de cada pista; cada toma grabada entra en el registro (album.json)."));
        memoryButton_.setTooltip(uiText(l,
            "Guarda a sessão que você quiser, com nome, e a reabre quando quiser. O app sempre abre algo novo.",
            "Stores the session you want, with a name, and reopens it when you want. The app always opens something new.",
            "Garde la session voulue, avec un nom, et la rouvre quand vous voulez. L’app ouvre toujours du neuf.",
            "Guarda la sesión que quiera, con nombre, y la reabre cuando quiera. La app siempre abre algo nuevo."));
        captureButton_.setTooltip(uiText(l,
            "Guarda BPM, macros, presenças e relações da cena atual.",
            "Stores BPM, macros, presences and relations of the current scene.",
            "Enregistre BPM, macros, présences et relations de la scène actuelle.",
            "Guarda BPM, macros, presencias y relaciones de la escena actual."));
        undoCaptureButton_.setTooltip(uiText(l,
            "Restaura a cena anterior à última captura desta sessão.",
            "Restores the scene before the last capture in this session.",
            "Restaure la scène antérieure à la dernière capture de cette session.",
            "Restaura la escena anterior a la última captura de esta sesión."));
        for (std::size_t index = 0U; index < sceneButtons_.size(); ++index)
            sceneButtons_[index].setTooltip(uiText(l, "Tecla ", "Key ", "Touche ", "Tecla ")
                + juce::String(static_cast<int>(index + 1U))
                + uiText(l, " · recupera esta cena sem interromper PLAY.",
                           " · recalls this scene without interrupting PLAY.",
                           " · rappelle cette scène sans interrompre PLAY.",
                           " · recupera esta escena sin interrumpir PLAY."));
        refreshNarrativeGestureLabels();
        refreshAudioFooter();
        repaint();
    }

    void setSupportLanguage(SupportLanguage language) {
        supportLanguage_ = language;
        languageButton_.setButtonText(supportLanguageCode(language));
        refreshInterfaceLanguage();
        lastAction_ = uiText(language, "IDIOMA ", "LANGUAGE ", "LANGUE ", "IDIOMA ")
            + supportLanguageCode(language);
    }

    void adjustMaster(double delta) {
        masterSlider_.setValue(juce::jlimit(-24.0, 0.0, masterSlider_.getValue() + delta));
    }

    juce::String morphLabelText() const {
        return "MORPH " + juce::String(morphSeconds_, 1) + " s";
    }

    void adjustMorph(double delta) {
        morphSlider_.setValue(juce::jlimit(0.2, 3.0, morphSlider_.getValue() + delta));
    }

    static bool optionalCapacityAvailable() noexcept {
        return kPerformanceSafeBodyLimit >= 4U;
    }

    bool setActorPresenceControl(std::size_t index, double presence) {
        if (index >= actorSliders_.size()) return false;
        const auto bounded = juce::jlimit(0.0, 1.0, presence);
        if (!controller_.setActorPresence(static_cast<std::uint16_t>(index + 1U), bounded)) {
            actorSliders_[index].setValue(actorPresenceValues_[index], juce::dontSendNotification);
            constellationGraph_.setPresence(index, actorPresenceValues_[index]);
            lastAction_ = uiText(supportLanguage_, "presença não enfileirada", "presence not queued",
                                 "présence non mise en file", "presencia no encolada");
            return false;
        }
        actorPresenceValues_[index] = bounded;
        notePerformerActivity();
        actorSliders_[index].setValue(bounded, juce::dontSendNotification);
        constellationGraph_.setPresence(index, bounded);
        modulePanelActive_[index] = bounded > 0.025;
        lastAction_ = moduleDisplayNames_[index] + uiText(supportLanguage_, " presença ", " presence ",
            " présence ", " presencia ")
            + juce::String(bounded, 2);
        sceneDirty_ = true;
        return true;
    }

    bool setRelationControl(std::size_t index, double intensity, bool projectGeometry = true) {
        static constexpr std::array<std::uint16_t, 16> sources{
            1U, 2U, 1U, 1U, 2U, 3U, 1U, 5U, 6U, 7U, 1U, 2U, 2U, 3U, 4U, 5U};
        static constexpr std::array<std::uint16_t, 16> destinations{
            2U, 3U, 3U, 4U, 4U, 4U, 5U, 6U, 7U, 8U, 6U, 5U, 6U, 7U, 8U, 7U};
        if (index >= relationSliders_.size()) return false;
        const auto bounded = juce::jlimit(0.0, 1.0, intensity);
        if (!controller_.setRelationIntensity(sources[index], destinations[index], bounded)) {
            relationSliders_[index].setValue(relationValues_[index], juce::dontSendNotification);
            constellationGraph_.setRelation(index, relationValues_[index]);
            lastAction_ = uiText(supportLanguage_, "relação não enfileirada", "relation not queued",
                                 "relation non mise en file", "relación no encolada");
            return false;
        }
        relationValues_[index] = bounded;
        notePerformerActivity();
        relationSliders_[index].setValue(bounded, juce::dontSendNotification);
        if (projectGeometry) constellationGraph_.setRelation(index, bounded);
        lastAction_ = relationDisplayNames_[index] + uiText(supportLanguage_, " relação ", " relation ",
            " relation ", " relación ")
            + juce::String(bounded, 2);
        sceneDirty_ = true;
        return true;
    }

    bool setModuleControl(std::size_t index, std::size_t control, double value) {
        if (index >= moduleControlSliders_.size()
            || control >= moduleControlSliders_[index].size()) return false;
        const auto bounded = juce::jlimit(0.0, 1.0, value);
        if (!controller_.setModuleControl(static_cast<std::uint16_t>(index + 1U),
                                          static_cast<PerformanceModuleControl>(control), bounded)) {
            moduleControlSliders_[index][control].setValue(
                moduleControlValues_[index][control], juce::dontSendNotification);
            lastAction_ = uiText(supportLanguage_, "controle local não enfileirado", "local control not queued",
                                 "contrôle local non mis en file", "control local no encolado");
            return false;
        }
        moduleControlValues_[index][control] = bounded;
        notePerformerActivity();
        moduleControlSliders_[index][control].setValue(bounded, juce::dontSendNotification);
        lastAction_ = moduleDisplayNames_[index] + uiText(supportLanguage_, " · modular ", " · module ",
            " · module ", " · módulo ") + juce::String(static_cast<int>(control + 1U)) + " · "
            + juce::String(bounded, 2);
        return true;
    }

    void setStemGainControl(std::size_t index, double gain) {
        if (index >= stemGainSliders_.size()) return;
        const auto bounded = juce::jlimit(0.0, 1.0, gain);
        if (!controller_.setStemGain(static_cast<std::uint16_t>(index + 1U), bounded)) {
            stemGainSliders_[index].setValue(stemGainValues_[index], juce::dontSendNotification);
            lastAction_ = uiText(supportLanguage_, "stem não enfileirado", "stem not queued",
                                 "stem non mis en file", "stem no encolado");
            return;
        }
        stemGainValues_[index] = bounded;
        notePerformerActivity();
        stemGainSliders_[index].setValue(bounded, juce::dontSendNotification);
        lastAction_ = moduleDisplayNames_[index] + utf8(" · mix ") + juce::String(bounded, 2);
    }

    void toggleStemMute(std::size_t index) {
        if (index >= stemMuted_.size()) return;
        const auto requested = !stemMuted_[index];
        if (!controller_.setStemMuted(static_cast<std::uint16_t>(index + 1U), requested)) {
            lastAction_ = uiText(supportLanguage_, "mute não enfileirado", "mute not queued",
                                 "mute non mis en file", "mute no encolado");
            return;
        }
        stemMuted_[index] = requested;
        notePerformerActivity();
        refreshStemMixVisual();
        lastAction_ = moduleDisplayNames_[index] + (requested ? " · MUTE" : uiText(supportLanguage_, " · retorno",
            " · return", " · retour", " · retorno"));
    }

    void toggleStemSolo(std::size_t index) {
        if (index >= stemSoloButtons_.size()) return;
        const auto actorId = static_cast<std::uint16_t>(index + 1U);
        const bool requested = soloStemIndex_ != index;
        if (!controller_.setStemSolo(actorId, requested)) {
            lastAction_ = uiText(supportLanguage_, "solo não enfileirado", "solo not queued",
                                 "solo non mis en file", "solo no encolado");
            return;
        }
        soloStemIndex_ = requested ? index : kNoStemIndex;
        notePerformerActivity();
        refreshStemMixVisual();
        lastAction_ = moduleDisplayNames_[index] + (requested ? " · SOLO" : uiText(supportLanguage_, " · conjunto",
            " · ensemble", " · ensemble", " · conjunto"));
    }

    void refreshStemMixVisual() {
        for (std::size_t index = 0U; index < stemMuteButtons_.size(); ++index) {
            const bool muted = stemMuted_[index];
            const bool soloed = soloStemIndex_ == index;
            stemMuteButtons_[index].setColour(juce::TextButton::buttonColourId,
                muted ? juce::Colour(0xff813d34) : juce::Colour(0xff285348));
            stemSoloButtons_[index].setColour(juce::TextButton::buttonColourId,
                soloed ? juce::Colour(0xffb66b39) : juce::Colour(0xff315e51));
        }
    }

    void setMacroControl(std::size_t index, double value) {
        if (index >= macroValues_.size()) return;
        const auto macroName = localizedMacroName(index);
        const auto* document = controller_.document();
        const auto& territory = document != nullptr ? document->territory : startupDocument_.territory;
        double ceiling = 1.0;
        if (index == static_cast<std::size_t>(PerformanceMacro::Memory))
            ceiling = territory.identity.memoryLimit;
        else if (index == static_cast<std::size_t>(PerformanceMacro::Risk))
            ceiling = territory.identity.riskLimit;
        const auto bounded = std::min(juce::jlimit(0.0, 1.0, value), ceiling);
        if (!controller_.setMacro(static_cast<PerformanceMacro>(index), bounded)) {
            macroSliders_[index].setValue(macroValues_[index], juce::dontSendNotification);
            lastAction_ = macroName + uiText(supportLanguage_, " não enfileirado", " not queued",
                " non mis en file", " no encolado");
            return;
        }
        macroValues_[index] = bounded;
        notePerformerActivity();
        macroSliders_[index].setValue(bounded, juce::dontSendNotification);
        lastAction_ = macroName + " " + juce::String(macroValues_[index], 2)
            + (bounded < value ? uiText(supportLanguage_, " · limite do território", " · territory limit",
                " · limite du territoire", " · límite del territorio") : "");
        sceneDirty_ = true;
    }

    void recombineFieldMacros(double primaryX, double primaryY,
                              double textureX, double textureY,
                              PerformanceField::ChangeSource source = PerformanceField::ChangeSource::Both,
                              bool fromNarrative = false) {
        if (!fromNarrative) notePerformerActivity();
        fieldPrimaryX_ = juce::jlimit(0.0, 1.0, primaryX);
        fieldPrimaryY_ = juce::jlimit(0.0, 1.0, primaryY);
        fieldTextureX_ = juce::jlimit(0.0, 1.0, textureX);
        fieldTextureY_ = juce::jlimit(0.0, 1.0, textureY);

        // Cada corpo deve comandar quatro macros legíveis. Laranja: autonomia,
        // intervenção, memória e risco. Azul: densidade, tensão, espaço e
        // profundidade. Não há macro compartilhado entre os dois corpos.
        const auto autoValue = fieldPrimaryX_;
        const auto interventionValue = fieldPrimaryY_;
        const auto densityValue = fieldTextureX_;
        const auto tensionValue = fieldTextureY_;
        // Os eixos derivados também precisam percorrer 0..1. As fórmulas
        // anteriores tinham pisos e tetos escondidos: o músico movia o corpo
        // inteiro e via MEMÓRIA/RISCO/ESPAÇO pararem antes do limite.
        const auto memoryValue = (fieldPrimaryX_ + fieldPrimaryY_) * 0.5;
        const auto riskValue = std::abs(fieldPrimaryX_ - fieldPrimaryY_);
        const auto spaceValue = (fieldTextureX_ + fieldTextureY_) * 0.5;
        const auto depthValue = std::abs(fieldTextureX_ - fieldTextureY_);

        constellationGraph_.setFieldForce(fieldPrimaryX_, fieldPrimaryY_);
        // Nunca reescrever os oito valores só porque um dos corpos se moveu:
        // isso fazia MEMÓRIA/RISCO do laranja arrastarem visualmente ESPAÇO.
        // Cada conjunto de quatro pertence exclusivamente ao seu corpo.
        if (source != PerformanceField::ChangeSource::Blue) {
            setMacroControl(0U, autoValue);
            setMacroControl(1U, interventionValue);
            setMacroControl(4U, memoryValue);
            setMacroControl(5U, riskValue);
        }
        if (source != PerformanceField::ChangeSource::Orange) {
            setMacroControl(2U, densityValue);
            setMacroControl(3U, tensionValue);
            setMacroControl(6U, spaceValue);
            setMacroControl(7U, depthValue);
        }
        if (!fromNarrative) lastAction_ = source == PerformanceField::ChangeSource::Orange
            ? utf8("EIXO LARANJA · 4 MACROS")
            : source == PerformanceField::ChangeSource::Blue
                ? utf8("EIXO AZUL · 4 MACROS") : utf8("EIXO · 8 MACROS RECOMBINADOS");
    }

    static double narrativeUnit(std::uint32_t seed, std::uint32_t turn,
                                std::uint32_t lane) noexcept {
        auto value = seed ^ (turn * 0x9e3779b9U) ^ (lane * 0x85ebca6bU);
        value ^= value >> 16U;
        value *= 0x7feb352dU;
        value ^= value >> 15U;
        return static_cast<double>(value & 0xffffU) / 65535.0;
    }

    void updateFieldNarrative(const PerformanceTelemetrySnapshot& telemetry) {
        if (!telemetry.playing || macroValues_[0U] <= 0.025) {
            if (fieldNarrativeActive_) {
                fieldNarrativeActive_ = false;
                performanceField_.setNarrativePosition(fieldPrimaryX_, fieldPrimaryY_,
                                                       fieldTextureX_, fieldTextureY_);
                recombineFieldMacros(fieldPrimaryX_, fieldPrimaryY_,
                                     fieldTextureX_, fieldTextureY_,
                                     PerformanceField::ChangeSource::Both, true);
            }
            return;
        }
        if (telemetry.narrativeTurn != displayedNarrativeTurn_) {
            displayedNarrativeTurn_ = telemetry.narrativeTurn;
            // The target comes from the opening seed and turn ordinal. It is
            // bounded around the performer’s four anchors: a narrative turn
            // cannot redraw the field or strand a macro at a hidden value.
            const auto amplitude = 0.06 + 0.14 * macroValues_[0U];
            const auto textureAmplitude = 0.05 + 0.13 * macroValues_[6U];
            const auto target = [seed = startupDocument_.territory.seed,
                                 turn = telemetry.narrativeTurn](double anchor,
                                                                  double amount,
                                                                  std::uint32_t lane) {
                return juce::jlimit(0.0, 1.0,
                    anchor + (narrativeUnit(seed, turn, lane) * 2.0 - 1.0) * amount);
            };
            narrativePrimaryX_ = target(fieldPrimaryX_, amplitude, 1U);
            narrativePrimaryY_ = target(fieldPrimaryY_, amplitude, 2U);
            narrativeTextureX_ = target(fieldTextureX_, textureAmplitude, 3U);
            narrativeTextureY_ = target(fieldTextureY_, textureAmplitude, 4U);
            fieldNarrativeStartedMs_ = juce::Time::getMillisecondCounterHiRes();
            fieldNarrativeActive_ = true;
            lastAction_ = uiText(supportLanguage_, "SEED · DESVIO NO EIXO", "SEED · AXIS DEVIATION",
                                 "SEED · DÉVIATION DE L’AXE", "SEED · DESVÍO EN EL EJE");
        }
        if (!fieldNarrativeActive_) return;
        constexpr double kNarrativeSeconds = 3.6;
        const auto elapsed = (juce::Time::getMillisecondCounterHiRes()
                              - fieldNarrativeStartedMs_) / 1000.0;
        const auto progress = juce::jlimit(0.0, 1.0, elapsed / kNarrativeSeconds);
        const auto envelope = std::sin(progress * juce::MathConstants<double>::pi);
        const auto blend = [envelope](double anchor, double target) {
            return anchor + (target - anchor) * envelope;
        };
        const auto primaryX = blend(fieldPrimaryX_, narrativePrimaryX_);
        const auto primaryY = blend(fieldPrimaryY_, narrativePrimaryY_);
        const auto textureX = blend(fieldTextureX_, narrativeTextureX_);
        const auto textureY = blend(fieldTextureY_, narrativeTextureY_);
        performanceField_.setNarrativePosition(primaryX, primaryY, textureX, textureY);
        // Control updates are quantised to perceptible steps. This keeps a
        // 15 Hz UI timer from becoming a continuous automation stream while
        // preserving a clearly audible arrival and return.
        const auto changed = std::abs(primaryX - appliedNarrativePrimaryX_) > 0.012
            || std::abs(primaryY - appliedNarrativePrimaryY_) > 0.012
            || std::abs(textureX - appliedNarrativeTextureX_) > 0.012
            || std::abs(textureY - appliedNarrativeTextureY_) > 0.012
            || progress >= 1.0;
        if (changed) {
            appliedNarrativePrimaryX_ = primaryX;
            appliedNarrativePrimaryY_ = primaryY;
            appliedNarrativeTextureX_ = textureX;
            appliedNarrativeTextureY_ = textureY;
            recombineFieldMacros(primaryX, primaryY, textureX, textureY,
                                 PerformanceField::ChangeSource::Both, true);
        }
        if (progress >= 1.0) fieldNarrativeActive_ = false;
    }

    void timerCallback() override {
        refreshAudioFooter();
        const auto telemetry = controller_.readTelemetry();
        watchAudioFlow();
        refreshEffectiveTempoLabel(telemetry);
        refreshCableActs();
        // A física do céu repousa onde a tela desenha cada nó: a cada ~2 s
        // (e logo ao abrir) o app reenvia os oito lugares como âncoras — as
        // vozes são refeitas quando a seed muda. Não muda a posição da
        // ecologia; arrastar um nó continua mudando as duas.
        if (skyAnchorTicks_-- == 0U) {
            skyAnchorTicks_ = 30U;
            for (std::size_t index = 0U; index < 8U; ++index) {
                const auto actorId = constellationGraph_.actorIdAt(index);
                const auto place = constellationGraph_.nodePosition(index);
                if (actorId != kInvalidActorId)
                    controller_.setSkyAnchor(actorId, juce::jlimit(0.0, 1.0, static_cast<double>(place.x)),
                                             juce::jlimit(0.0, 1.0, static_cast<double>(place.y)));
            }
        }
        writeDiagnostics(telemetry);
        // The audio-device callback never touches the writer or filesystem.
        // It only raises this flag; the message thread finalizes the partial
        // take and its manifest once the device has stopped.
        if (recorder_.recording() && deviceStopPending_.exchange(false, std::memory_order_acq_rel)) {
            codaRequested_ = false;
            stopClosePending_ = false;
            appendTakeTrace("device:stopped");
            closeRecording(utf8("tomada interrompida · dispositivo"));
            playingRequested_ = false;
        }
        if (recorder_.recording() && telemetry.panicSafe) {
            codaRequested_ = false;
            stopClosePending_ = false;
            appendTakeTrace("safety:PANIC_SAFE");
            closeRecording(utf8("tomada interrompida · PANIC SAFE"));
            controller_.setPlaying(false);
            playingRequested_ = false;
        }
        if (recorder_.recording() && stopClosePending_ && !telemetry.playing) {
            stopClosePending_ = false;
            closeRecording(utf8("tomada interrompida"));
        }
        if (recorder_.recording() && playingRequested_ && !codaRequested_
            && telemetry.codaReadinessMicros >= 1000000U) {
            requestMorceauCoda("coda:seed-narrative",
                               utf8("CODA da seed · percurso autônomo completo"));
        }
        if (recorder_.recording() && telemetry.narrativeEventValid
            && telemetry.narrativeTurn != lastTracedNarrativeTurn_) {
            lastTracedNarrativeTurn_ = telemetry.narrativeTurn;
            appendTakeTrace("seed-turn:" + juce::String(telemetry.narrativeTurn));
        }
        if (recorder_.recording() && codaRequested_)
            takeCodaGesture_ = codaGestureArchiveName(telemetry.codaGesture);
        if (recorder_.recording()
            && controller_.morceauState() == PerformanceMorceauState::Finalized) {
            closeRecording(utf8("morceau finalizado"));
            // Só a gravação termina (5/11, pedido do autor): o som segue,
            // uma nova peça abre sem STOP.
            controller_.continueAfterMorceau();
            codaRequested_ = false;
        }
        constellationGraph_.setStemTelemetry(telemetry);
        performanceField_.setSkyState(telemetry, controller_.skyWeb());
        followPercurso(telemetry);
        // A faixa com duração: a coda chega 5,2 s antes do alvo (coda 4 s +
        // cauda 1,2 s), e o arquivo fecha em torno do tempo escolhido.
        if (takeTargetSeconds_ > 0.0 && recorder_.recording() && !codaRequested_ && takeSampleRate_ > 0.0
            && static_cast<double>(recorder_.acceptedSamples()) / takeSampleRate_ >= takeTargetSeconds_ - kCodaLeadSeconds)
            requestMorceauCoda("coda:duracao", uiText(supportLanguage_, "FAIXA · o tempo chegou · CODA",
                                                      "TRACK · time is up · CODA", "PISTE · le temps est écoulé · CODA",
                                                      "PISTA · se acabó el tiempo · CODA"));
        // A VARIAÇÃO no mixer: o fader de mix anda sozinho (decisão do autor,
        // 2/11 — antes, 30/10, era uma barra ao lado). Com a VARIAÇÃO ligada e
        // tocando, o cursor mostra o ganho efetivo do stem: o seu ganho vezes
        // o brilho que o céu dá à estrela (cursor laranja). Ao pegar o fader,
        // ele volta ao seu valor e obedece à mão; ao soltar, volta a andar.
        // Desligada ou parada, o fader fica no seu valor (cursor azul).
        {
            const bool show = controller_.mixerVariation() && telemetry.playing;
            for (std::size_t index = 0U; index < shownBrightness_.size(); ++index) {
                const double brightness = show ? juce::jlimit(0.0, 1.0,
                    telemetry.starBrightnessMicros[index] / 1000000.0) : -1.0;
                shownBrightness_[index] = brightness;
                auto& fader = stemGainSliders_[index];
                const bool moving = brightness >= 0.0 && !stemGainHeld_[index];
                fader.setColour(juce::Slider::thumbColourId,
                                moving ? juce::Colour(0xfff0b860) : juce::Colour(0xff5eb6d7));
                const double shown = moving ? stemGainValues_[index] * brightness : stemGainValues_[index];
                if (!stemGainHeld_[index] && std::fabs(fader.getValue() - shown) > 0.004)
                    fader.setValue(shown, juce::dontSendNotification);
            }
        }
        updateFieldNarrative(telemetry);
        if (telemetry.playing != playingRequested_) {
            playingRequested_ = telemetry.playing;
            transportButton_.setButtonText(playingRequested_ ? "STOP" : "PLAY");
            transportButton_.setColour(juce::TextButton::buttonColourId,
                                       playingRequested_ ? juce::Colour(0xff813d34)
                                                          : juce::Colour(0xff285348));
        }
        if (telemetry.outputPeakMicros >= 980000U) clipHoldFrames_ = 45;
        else if (clipHoldFrames_ > 0) --clipHoldFrames_;
        if (telemetry.panicSafe != panicSafeDisplayed_) {
            panicSafeDisplayed_ = telemetry.panicSafe;
            if (panicSafeDisplayed_) holdRequested_ = false;
            refreshSafetyVisual();
            refreshHoldVisual();
        }
        const auto currentScene = controller_.currentSceneId();
        if (currentScene != displayedSceneId_) {
            displayedSceneId_ = currentScene;
            for (std::size_t index = 0; index < sceneButtons_.size(); ++index) {
                const bool active = currentScene == static_cast<std::uint16_t>(index + 1U);
                sceneButtons_[index].setColour(juce::TextButton::buttonColourId,
                                                active ? juce::Colour(0xffb66b39) : juce::Colour(0xff285348));
                sceneButtons_[index].setColour(juce::TextButton::textColourOffId,
                                                active ? juce::Colour(0xfffff3ea) : juce::Colour(0xffe7e3d5));
            }
        }
        // Parado, a janela inteira só é redesenhada quando algo visível muda
        // (e ao menos uma vez por segundo); tocando, a cada tique, como antes.
        // Antes eram 15 redesenhos completos por segundo mesmo em STOP.
        const auto signature = juce::String(telemetry.outputPeakMicros / 2000U) + "|"
            + juce::String(telemetry.xrunCount) + "|" + juce::String(telemetry.narrativeTurn) + "|"
            + juce::String(telemetry.motionPhaseMicros / 2000U) + "|" + lastAction_ + "|"
            + audioFooterText_ + "|" + juce::String(static_cast<int>(selectedModuleIndex_)) + "|"
            + juce::String(static_cast<int>(recorder_.recording())) + "|"
            + juce::String(static_cast<int>(controller_.mixerVariation())) + "|"
            + juce::String(static_cast<int>(controller_.skyWeb()));
        const bool idle = !telemetry.playing && signature == lastRepaintSignature_ && ++idleRepaintTicks_ < 15U;
        if (idle) return;
        idleRepaintTicks_ = 0U;
        lastRepaintSignature_ = signature;
        repaint();
    }

    // O andamento que soa (3/11, R8): com a trama tocando, o tempo respira;
    // o rótulo mostra o BPM efetivo ao lado do de base ("TEMPO / BPM → 124").
    void refreshEffectiveTempoLabel(const PerformanceTelemetrySnapshot& telemetry) {
        juce::String text = "TEMPO / BPM";
        if (telemetry.playing && controller_.skyWeb() && telemetry.skyTempoMillis > 0U) {
            const auto effective = static_cast<int>(std::lround(
                tempoSlider_.getValue() * static_cast<double>(telemetry.skyTempoMillis) / 1000.0));
            if (effective != static_cast<int>(std::lround(tempoSlider_.getValue())))
                text += utf8(" → ") + juce::String(effective);
        }
        if (tempoLabel_.getText() != text) tempoLabel_.setText(text, juce::dontSendNotification);
    }

    void refreshHoldVisual() {
        auto& holdButton = gestureButtons_.front();
        holdButton.setButtonText(holdRequested_ ? "RELEASE" : "HOLD");
        holdButton.setColour(juce::TextButton::buttonColourId,
                             holdRequested_ ? juce::Colour(0xffb66b39) : juce::Colour(0xff6b5035));
    }

    void refreshSafetyVisual() {
        auto& panicButton = gestureButtons_.back();
        panicButton.setButtonText(panicSafeDisplayed_ ? "RESET SAFE" : "PANIC SAFE");
        panicButton.setColour(juce::TextButton::buttonColourId,
                              panicSafeDisplayed_ ? juce::Colour(0xffb66b39) : juce::Colour(0xff813d34));
    }

    void refreshAudioFooter() {
        auto* device = deviceManager_ != nullptr ? deviceManager_->getCurrentAudioDevice() : nullptr;
        if (!ready_.load(std::memory_order_acquire) || device == nullptr) {
            audioFooterText_ = uiText(supportLanguage_, "ÁUDIO · aguardando dispositivo",
                                      "AUDIO · waiting for device", "AUDIO · en attente du périphérique",
                                      "AUDIO · esperando dispositivo");
            return;
        }
        const auto sampleRateKhz = device->getCurrentSampleRate() / 1000.0;
        audioFooterText_ = uiText(supportLanguage_, "ÁUDIO · ", "AUDIO · ", "AUDIO · ", "AUDIO · ")
            + device->getName() + utf8("  ·  ")
            + juce::String(sampleRateKhz, 1) + utf8(" kHz  ·  ")
            + juce::String(device->getCurrentBufferSizeSamples()) + utf8(" samples  ·  ")
            + juce::String(device->getActiveOutputChannels().countNumberOfSetBits())
            + uiText(supportLanguage_, " saídas", " outs", " sorties", " salidas")
            + utf8("  ·  ") + realtimeFooterText()
            + uiText(supportLanguage_, "  ·  PIOR BLOCO ", "  ·  WORST BLOCK ", "  ·  PIRE BLOC ", "  ·  PEOR BLOQUE ")
            + juce::String(worstShownMs_, 1) + " ms"
            + uiText(supportLanguage_, "  ·  CAPACIDADE ", "  ·  CAPACITY ", "  ·  CAPACITÉ ", "  ·  CAPACIDAD ")
            + juce::String(static_cast<int>(controller_.starCapacity()))
            + loudnessFooterText()
            + (audioRestarts_ > 0U ? uiText(supportLanguage_, "  ·  ÁUDIO REABERTO ", "  ·  AUDIO REOPENED ",
                                            "  ·  AUDIO ROUVERT ", "  ·  AUDIO REABIERTO ")
                                         + juce::String(static_cast<int>(audioRestarts_)) + "×"
                                   : juce::String{});
    }

    // Loudness de curto prazo (3 s) e true peak da saída, BS.1770 (30/10,
    // padrão de medição dos instrumentos RASGO; docs/SAIDA_DE_AUDIO.md).
    juce::String loudnessFooterText() {
        const auto telemetry = controller_.readTelemetry();
        if (!telemetry.playing) return {};
        const double shortTerm = telemetry.shortTermCentiLufs / 100.0;
        const double peak = telemetry.truePeakMicros / 1000000.0;
        const double peakDb = peak > 1.0e-6 ? 20.0 * std::log10(peak) : -120.0;
        const double integrated = telemetry.integratedCentiLufs / 100.0;
        const double reduction = telemetry.limiterMaxReductionCentiDb / 100.0;
        return utf8("  ·  LUFS ") + (shortTerm <= -69.9 ? juce::String("—") : juce::String(shortTerm, 1))
            + utf8(" (I ") + (integrated <= -69.9 ? juce::String("—") : juce::String(integrated, 1)) + ")"
            + utf8("  ·  TP ") + (peakDb <= -119.0 ? juce::String("—") : juce::String(peakDb, 1)) + " dBTP"
            // A marca persistente de que o limitador agiu desde o PLAY
            // (clip latch da arquitetura comum de saída).
            + (reduction >= 0.5 ? uiText(supportLanguage_, "  ·  LIMITADOR −", "  ·  LIMITER −", "  ·  LIMITEUR −",
                                         "  ·  LIMITADOR −") + juce::String(reduction, 1) + " dB"
                                : juce::String{});
    }

    juce::String realtimeFooterText() const {
        const auto priority = realtimePriority_.load(std::memory_order_acquire);
        if (priority > 0) return utf8("RT ") + juce::String(priority);
        if (!realtimeAsked_.load(std::memory_order_acquire))
            return uiText(supportLanguage_, "RT …", "RT …", "RT …", "RT …");
        return uiText(supportLanguage_, "sem tempo real", "no realtime", "sans temps réel", "sin tiempo real");
    }

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override {
        if (device != nullptr) audioEverStarted_.store(true, std::memory_order_release);
        const auto outputs = device != nullptr
            ? device->getActiveOutputChannels().countNumberOfSetBits() : 0;
        const auto channels = static_cast<std::uint32_t>(juce::jlimit(1, 2, outputs));
        // Buffers dos stems, alocados aqui (fora do callback) e entregues ao motor.
        const auto blockCapacity = static_cast<std::uint32_t>(device != nullptr
            ? juce::jmax(1, device->getCurrentBufferSizeSamples()) : 1);
        for (std::size_t stem = 0U; stem < stemBuffers_.size(); ++stem) {
            stemBuffers_[stem].assign(blockCapacity, 0.0f);
            stemPointers_[stem] = stemBuffers_[stem].data();
        }
        controller_.setStemTap(stemPointers_.data(), stemPointers_.size(), blockCapacity);
        // Numa reabertura, o documento vivo (cena atual, macros, presenças,
        // relações, BPM), não o de partida.
        PerformanceSessionDocument restoreDocument{};
        const bool restoring = audioRestore_.has_value()
            && parseSessionDocument(audioRestore_->document, restoreDocument).ok();
        ready_.store(device != nullptr
            && controller_.prepare(device->getCurrentSampleRate(),
                                   static_cast<std::uint32_t>(device->getCurrentBufferSizeSamples()), channels)
            && (restoring ? controller_.loadDocument(restoreDocument, audioRestore_->sceneId)
                          : controller_.loadDocument(startupDocument_)), std::memory_order_release);
        if (restoring) {
            audioRestore_.reset();
            juce::Component::SafePointer<PerformanceHostComponent> safe(this);
            juce::MessageManager::callAsync([safe] {
                if (safe != nullptr) safe->reapplyLiveState();
            });
        }
        sampleRate_.store(device != nullptr ? device->getCurrentSampleRate() : 0.0,
                          std::memory_order_release);
        cpuMilliPercent_.store(0U, std::memory_order_release);
        realtimePriority_.store(0, std::memory_order_release);
        realtimeAsked_.store(false, std::memory_order_release);
    }

    void audioDeviceStopped() override {
        if (recorder_.recording())
            deviceStopPending_.store(true, std::memory_order_release);
        ready_.store(false, std::memory_order_release);
        sampleRate_.store(0.0, std::memory_order_release);
        cpuMilliPercent_.store(0U, std::memory_order_release);
        realtimePriority_.store(0, std::memory_order_release);
        realtimeAsked_.store(false, std::memory_order_release);
    }

    void audioDeviceIOCallbackWithContext(const float* const*, int,
                                          float* const* outputChannelData, int numOutputChannels,
                                          int numSamples, const juce::AudioIODeviceCallbackContext&) override {
        // Números desnormalizados zerados no bloco (FTZ/DAZ; 4/11): nas caudas
        // e na coda os filtros e ressonadores decaem para ~1e-38 e o motor
        // ficava 3–4× mais lento (média 7 ms, picos de 34–65 ms em 10,67 ms) —
        // os atrasos da escuta guiada. Com eles zerados, ~2 ms. Inaudível.
        const juce::ScopedNoDenormals noDenormals;
        // The JUCE/ALSA audio thread starts as SCHED_OTHER; promote it once,
        // from inside itself, so the GUI cannot preempt the block deadline.
        if (!realtimeAsked_.exchange(true, std::memory_order_acq_rel))
            realtimePriority_.store(rasgo::performance::app::promoteCurrentThreadToRealtime(),
                                    std::memory_order_release);
        for (int channel = 0; channel < numOutputChannels; ++channel)
            if (outputChannelData[channel] != nullptr)
                juce::FloatVectorOperations::clear(outputChannelData[channel], numSamples);
        if (!ready_.load(std::memory_order_acquire) || numOutputChannels < 1 || numSamples <= 0
            || outputChannelData[0] == nullptr) return;
        const auto startTicks = juce::Time::getHighResolutionTicks();
        lastCallbackMs_.store(juce::Time::getMillisecondCounterHiRes(), std::memory_order_relaxed);
        controller_.processBlock(outputChannelData[0],
                                 numOutputChannels > 1 ? outputChannelData[1] : nullptr,
                                 static_cast<std::uint32_t>(numSamples));
        const auto engineTicks = juce::Time::getHighResolutionTicks() - startTicks;
        stemRecorder_.push(stemPointers_.data(), numSamples);
        recorder_.push(outputChannelData[0],
                       numOutputChannels > 1 ? outputChannelData[1] : nullptr, numSamples);
        const auto elapsedTicks = juce::Time::getHighResolutionTicks() - startTicks;
        const auto sampleRate = sampleRate_.load(std::memory_order_relaxed);
        if (sampleRate > 0.0) {
            const auto elapsedSeconds = juce::Time::highResolutionTicksToSeconds(elapsedTicks);
            const auto blockSeconds = static_cast<double>(numSamples) / sampleRate;
            const auto milliPercent = static_cast<std::uint32_t>(juce::jlimit(
                0.0, 100000.0, (elapsedSeconds / blockSeconds) * 100000.0));
            cpuMilliPercent_.store(milliPercent, std::memory_order_release);
            // Diagnóstico (2026-10-29): pior bloco e cada atraso, com o tempo
            // do motor separado do tempo das gravações. Só atômicos e um anel
            // fixo; a interface grava o arquivo.
            const auto totalMicros = static_cast<std::uint32_t>(std::min(elapsedSeconds * 1.0e6, 4.0e9));
            const auto engineMicros = static_cast<std::uint32_t>(std::min(
                juce::Time::highResolutionTicksToSeconds(engineTicks) * 1.0e6, 4.0e9));
            // A média do segundo (4/11): a guarda olha a carga real, não só o
            // pior bloco (um soluço do sistema não é sobrecarga).
            cpuWorkMicros_.fetch_add(totalMicros, std::memory_order_relaxed);
            cpuBlockMicros_.fetch_add(static_cast<std::uint32_t>(std::min(blockSeconds * 1.0e6, 4.0e9)),
                                      std::memory_order_relaxed);
            auto worst = worstBlockMicros_.load(std::memory_order_relaxed);
            while (totalMicros > worst
                   && !worstBlockMicros_.compare_exchange_weak(worst, totalMicros, std::memory_order_relaxed)) {}
            if (elapsedSeconds > blockSeconds) {
                controller_.reportDeadlineMiss();
                const auto slot = missWrite_.load(std::memory_order_relaxed) % missRing_.size();
                missRing_[slot] = {juce::Time::getMillisecondCounterHiRes(), totalMicros, engineMicros,
                                   static_cast<std::uint32_t>(numSamples)};
                missWrite_.fetch_add(1U, std::memory_order_release);
            }
        }
    }

    struct MissEvent {
        double ms = 0.0;
        std::uint32_t totalMicros = 0U;
        std::uint32_t engineMicros = 0U;
        std::uint32_t samples = 0U;
    };
    std::atomic<std::uint32_t> worstBlockMicros_{0U};
    std::atomic<std::uint64_t> cpuWorkMicros_{0U};   // tempo de callback somado no segundo
    std::atomic<std::uint64_t> cpuBlockMicros_{0U};  // prazo somado no segundo
    std::uint32_t lastXrunsSeen_ = 0U;
    std::array<MissEvent, 256> missRing_{};
    std::atomic<std::uint32_t> missWrite_{0U};
    std::uint32_t missRead_ = 0U;
    std::unique_ptr<juce::FileOutputStream> diagnosticLog_;
    double diagnosticLastMs_ = 0.0;
    double worstShownMs_ = 0.0;
    bool diagnosticUnavailable_ = false;
    std::atomic<double> lastCallbackMs_{0.0};
    std::uint32_t skyAnchorTicks_ = 0U;
    // As trocas de estrela do performer, por corpo (Count = a da seed).
    std::array<PerformanceModuleBody, 8> starSwaps_{{
        PerformanceModuleBody::Count, PerformanceModuleBody::Count, PerformanceModuleBody::Count,
        PerformanceModuleBody::Count, PerformanceModuleBody::Count, PerformanceModuleBody::Count,
        PerformanceModuleBody::Count, PerformanceModuleBody::Count}};
    std::uint32_t identitySeed_ = 0U;
    // As estrelas fixadas pelo performer (2/11), por corpo.
    std::array<bool, 8> starPinned_{};
    std::array<double, 8> shownBrightness_{{-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0}};
    double lastAudioRestartMs_ = 0.0;
    std::uint32_t audioRestarts_ = 0U;
    juce::String lastRepaintSignature_;
    std::uint32_t idleRepaintTicks_ = 0U;
    std::uint32_t overloadSeconds_ = 0U;
    std::uint32_t calmSeconds_ = 0U;

    // Arquivo de diagnóstico (interface, nunca o áudio): uma linha por
    // segundo e uma por atraso, em CSV, ao lado das gravações.
    void writeDiagnostics(const PerformanceTelemetrySnapshot& telemetry) {
        const auto now = juce::Time::getMillisecondCounterHiRes();
        if (diagnosticLog_ == nullptr && !diagnosticUnavailable_) {
            diagnosticUnavailable_ = true;
            const auto directory = juce::File::getSpecialLocation(juce::File::userMusicDirectory)
                .getChildFile("Rasgo Synth Performance").getChildFile("diagnostico");
            const auto file = directory.getChildFile("ASTERISMUS_DIAGNOSTICO_"
                + juce::Time::getCurrentTime().formatted("%Y-%m-%d_%H-%M-%S") + ".csv");
            if (directory.createDirectory()) diagnosticLog_ = std::make_unique<juce::FileOutputStream>(file);
            if (diagnosticLog_ != nullptr && diagnosticLog_->failedToOpen()) diagnosticLog_.reset();
            if (diagnosticLog_ != nullptr) diagnosticUnavailable_ = false;
            if (diagnosticLog_ != nullptr) diagnosticLog_->writeText("tipo,hora,ms_desde_inicio,pior_bloco_ms,motor_ms,amostras,cpu_bloco_pct,"
                                      "tocando,gravando,stems_soando,virada_seed,atrasos_total,capacidade,cpu_media_pct\n", false, false, nullptr);
            diagnosticLastMs_ = now;
        }
        const auto stamp = juce::Time::getCurrentTime().formatted("%H:%M:%S");
        const auto write = missWrite_.load(std::memory_order_acquire);
        if (write - missRead_ > missRing_.size()) missRead_ = write - static_cast<std::uint32_t>(missRing_.size());
        for (; missRead_ != write; ++missRead_) {
            const auto& event = missRing_[missRead_ % missRing_.size()];
            if (diagnosticLog_ != nullptr) diagnosticLog_->writeText("atraso," + stamp + "," + juce::String(event.ms, 1) + ","
                + juce::String(event.totalMicros / 1000.0, 2) + "," + juce::String(event.engineMicros / 1000.0, 2) + ","
                + juce::String(event.samples) + ",,,,,," + juce::String(telemetry.xrunCount) + "\n",
                false, false, nullptr);
        }
        if (now - diagnosticLastMs_ < 1000.0) return;
        diagnosticLastMs_ = now;
        std::uint32_t sounding = 0U;
        for (std::size_t stem = 0U; stem < telemetry.stemCount && stem < telemetry.stemPeakMicros.size(); ++stem)
            sounding += telemetry.stemPeakMicros[stem] > 1000U ? 1U : 0U;
        const auto worstMs = worstBlockMicros_.exchange(0U, std::memory_order_relaxed) / 1000.0;
        worstShownMs_ = worstMs;
        const auto workMicros = cpuWorkMicros_.exchange(0U, std::memory_order_relaxed);
        const auto blockMicros = cpuBlockMicros_.exchange(0U, std::memory_order_relaxed);
        const double averagePercent = blockMicros > 0U ? 100.0 * static_cast<double>(workMicros)
                                                             / static_cast<double>(blockMicros) : 0.0;
        const bool newXruns = telemetry.xrunCount > lastXrunsSeen_;
        lastXrunsSeen_ = telemetry.xrunCount;
        updateStarCapacity(worstMs, averagePercent, newXruns, telemetry.playing);
        if (diagnosticLog_ == nullptr) return;
        diagnosticLog_->writeText("segundo," + stamp + "," + juce::String(now, 1) + ","
            + juce::String(worstMs, 2) + ",,,"
            + juce::String(cpuMilliPercent_.load(std::memory_order_relaxed) / 1000.0, 1) + ","
            + (telemetry.playing ? "1" : "0") + "," + (recorder_.recording() ? "1" : "0") + ","
            + juce::String(sounding) + "," + juce::String(telemetry.narrativeTurn) + ","
            + juce::String(telemetry.xrunCount) + "," + juce::String(static_cast<int>(controller_.starCapacity()))
            + "," + juce::String(averagePercent, 1) + "\n", false, false, nullptr);
        diagnosticLog_->flush();
    }

    // Guarda do fluxo de áudio (29/10). Numa sessão do autor, depois de 5 h
    // aberto, o sistema de som passou a chamar o app só a cada ~2 s (metade
    // dos segundos sem nenhum bloco; o motor levava ~1 ms) e o instrumento
    // "travou". Se o dispositivo está aberto e nenhum bloco chega por mais de
    // 400 ms, o app reabre o dispositivo (no máximo uma vez a cada 10 s),
    // registra no diagnóstico e avisa no rodapé.
    // Depois de uma reabertura do áudio: manda de novo ao motor o que a
    // interface mostra (o motor recomeçou do zero). Trama, variação e
    // capacidade vivem no controlador e já sobrevivem.
    void reapplyLiveState() {
        if (!ready_.load(std::memory_order_acquire)) return;
        std::size_t refused = 0U;
        for (std::size_t index = 0U; index < 8U; ++index) {
            const auto actorId = static_cast<std::uint16_t>(index + 1U);
            if (starSwaps_[index] != PerformanceModuleBody::Count)
                refused += !controller_.setActorStar(actorId, starSwaps_[index]);
            if (starPinned_[index]) refused += !controller_.setActorPinned(actorId, true);
            refused += !controller_.setStemGain(actorId, stemGainValues_[index]);
            if (stemMuted_[index]) refused += !controller_.setStemMuted(actorId, true);
            for (std::size_t control = 0U; control < 3U; ++control)
                refused += !controller_.setModuleControl(actorId, static_cast<PerformanceModuleControl>(control),
                                                         moduleControlValues_[index][control]);
            refused += !controller_.setActorSeedVariation(actorId, actorSeedVariationValues_[index]);
        }
        if (soloStemIndex_ < 8U) refused += !controller_.setStemSolo(static_cast<std::uint16_t>(soloStemIndex_ + 1U), true);
        refused += !controller_.setOutputTrimDb(masterSlider_.getValue());
        constellationGraph_.restoreMotion(constellationGraph_.collectiveFamily(), constellationGraph_.bodyFamilies(),
                                          constellationGraph_.constellationMotion());
        if (audioRestorePlaying_) refused += !controller_.setPlaying(true);
        lastAction_ = uiText(supportLanguage_, "ÁUDIO REABERTO · a peça continua de onde estava",
                             "AUDIO REOPENED · the piece goes on from where it was",
                             "AUDIO ROUVERT · la pièce continue là où elle était",
                             "AUDIO REABIERTO · la pieza sigue desde donde estaba")
            + (refused > 0U ? uiText(supportLanguage_, " · comandos recusados: ", " · refused commands: ",
                                     " · commandes refusées : ", " · comandos rechazados: ")
                                  + juce::String(static_cast<int>(refused))
                            : juce::String{});
        repaint();
    }

    void watchAudioFlow() {
        // O dispositivo sumiu (3/11): interface desligada, servidor de som
        // reiniciado. A cada 5 s, tentar o último dispositivo e, se não
        // houver, o padrão do sistema; ao voltar, o estado vivo é reaplicado.
        if (deviceManager_ != nullptr && deviceManager_->getCurrentAudioDevice() == nullptr
            && audioEverStarted_.load(std::memory_order_acquire)) {
            const auto now = juce::Time::getMillisecondCounterHiRes();
            if (now - lastAudioRestartMs_ < 5000.0) return;
            lastAudioRestartMs_ = now;
            if (!audioRestore_) {
                audioRestore_ = captureMemory("dispositivo perdido");
                audioRestorePlaying_ = playingRequested_;
            }
            ++audioRestarts_;
            if (diagnosticLog_ != nullptr)
                diagnosticLog_->writeText("dispositivo_perdido," + juce::Time::getCurrentTime().formatted("%H:%M:%S")
                    + ",tentando reabrir\n", false, false, nullptr);
            deviceManager_->restartLastAudioDevice();
            if (deviceManager_->getCurrentAudioDevice() == nullptr)
                deviceManager_->initialise(0, 2, nullptr, true);
            if (deviceManager_->getCurrentAudioDevice() == nullptr)
                lastAction_ = uiText(supportLanguage_, "ÁUDIO PERDIDO · procurando o dispositivo a cada 5 s",
                                     "AUDIO LOST · looking for the device every 5 s",
                                     "AUDIO PERDU · recherche du périphérique toutes les 5 s",
                                     "AUDIO PERDIDO · buscando el dispositivo cada 5 s");
            return;
        }
        if (deviceManager_ == nullptr || deviceManager_->getCurrentAudioDevice() == nullptr
            || !ready_.load(std::memory_order_acquire)) return;
        const auto now = juce::Time::getMillisecondCounterHiRes();
        const auto last = lastCallbackMs_.load(std::memory_order_relaxed);
        if (last <= 0.0 || now - last < 400.0 || now - lastAudioRestartMs_ < 10000.0) return;
        lastAudioRestartMs_ = now;
        ++audioRestarts_;
        if (diagnosticLog_ != nullptr)
            diagnosticLog_->writeText("reinicio_audio," + juce::Time::getCurrentTime().formatted("%H:%M:%S") + ","
                + juce::String(now, 1) + "," + juce::String((now - last) / 1000.0, 2) + " s sem bloco\n",
                false, false, nullptr);
        lastCallbackMs_.store(now, std::memory_order_relaxed);
        // A peça atravessa a reabertura (3/11): o prepare do motor recomeça
        // tudo e o loadDocument voltava ao documento de partida — cena 1,
        // trocas, fixações, mix e PLAY perdidos. Guardar o estado vivo como
        // uma memória e reaplicá-lo depois que o dispositivo volta.
        audioRestore_ = captureMemory("reabertura");
        audioRestorePlaying_ = playingRequested_;
        deviceManager_->closeAudioDevice();
        deviceManager_->restartLastAudioDevice();
        lastAction_ = uiText(supportLanguage_, "ÁUDIO REABERTO · o sistema de som parou de chamar o instrumento",
                             "AUDIO REOPENED · the sound system stopped calling the instrument",
                             "AUDIO ROUVERT · le système audio a cessé d’appeler l’instrument",
                             "AUDIO REABIERTO · el sistema de sonido dejó de llamar al instrumento");
    }

    // Guarda de capacidade (auditoria 4.4; lista L2519/L2522; revista em
    // 4/11). Tira uma estrela (mínimo 3) depois de 2 s seguidos de
    // sobrecarga REAL: um atraso (bloco que estourou o prazo), a média do
    // segundo acima de 70 % ou um pico acima de 95 % do prazo. Antes bastava
    // um bloco acima de 75 % — um soluço do sistema sem atraso nenhum — e,
    // numa escuta de 4/11, o céu caiu de 8 para 3 estrelas com a média em
    // 16–28 % e 0 atrasos (o motor sozinho, 8 corpos, blocos de 512: média
    // 2 ms, pior 4,8 ms em 10,67). Devolve uma estrela depois de 20 s calmos
    // (média < 45 %, pico < 85 %, nenhum atraso). A variação do mixer
    // respeita a capacidade; os faders não são bloqueados.
    void updateStarCapacity(double worstMs, double averagePercent, bool newXruns, bool playing) {
        const auto rate = sampleRate_.load(std::memory_order_acquire);
        auto* device = deviceManager_ != nullptr ? deviceManager_->getCurrentAudioDevice() : nullptr;
        if (rate <= 0.0 || device == nullptr) return;
        const double budgetMs = device->getCurrentBufferSizeSamples() / rate * 1000.0;
        auto capacity = controller_.starCapacity();
        const bool overload = newXruns || averagePercent > 70.0 || worstMs > budgetMs * 0.95;
        if (overload) {
            calmSeconds_ = 0U;
            if (++overloadSeconds_ >= 2U && capacity > 3U) {
                controller_.setStarCapacity(capacity - 1U);
                overloadSeconds_ = 0U;
                lastAction_ = uiText(supportLanguage_, "CAPACIDADE · ", "CAPACITY · ", "CAPACITÉ · ", "CAPACIDAD · ")
                    + juce::String(static_cast<int>(capacity - 1U))
                    + uiText(supportLanguage_, " estrelas acesas · a máquina pediu folga",
                             " lit stars · the machine asked for room",
                             " étoiles allumées · la machine a demandé de la marge",
                             " estrellas encendidas · la máquina pidió margen");
            }
        } else {
            overloadSeconds_ = 0U;
            if (playing && averagePercent < 45.0 && worstMs < budgetMs * 0.85
                && ++calmSeconds_ >= 20U && capacity < 8U) {
                controller_.setStarCapacity(capacity + 1U);
                calmSeconds_ = 0U;
            }
        }
    }

    std::unique_ptr<juce::AudioDeviceManager> deviceManager_;
    juce::TooltipWindow tooltipWindow_{this, 650};
    PerformanceSessionController controller_;
    PerformanceSessionDocument startupDocument_{makeStartupDocument()};
    PerformanceMixRecorder recorder_;
    PerformanceStemRecorder stemRecorder_;
    std::array<std::vector<float>, kPerformanceStemActorCapacity> stemBuffers_;
    std::array<float*, kPerformanceStemActorCapacity> stemPointers_{};
    std::atomic<bool> ready_{false};
    std::atomic<bool> deviceStopPending_{false};
    std::optional<PerformanceMemory> audioRestore_;  // o estado vivo durante uma reabertura
    bool audioRestorePlaying_ = false;
    std::atomic<bool> audioEverStarted_{false};  // só procurar de novo um dispositivo que já existiu
    bool audioCallbackRegistered_ = false;
    std::atomic<double> sampleRate_{0.0};
    std::atomic<std::uint32_t> cpuMilliPercent_{0U};
    std::atomic<int> realtimePriority_{0};
    std::atomic<bool> realtimeAsked_{false};
    bool playingRequested_ = true;
    bool holdRequested_ = false;
    bool panicSafeDisplayed_ = false;
    bool sceneDirty_ = false;
    int clipHoldFrames_ = 0;
    std::uint16_t displayedSceneId_ = std::numeric_limits<std::uint16_t>::max();
    std::array<juce::TextButton, 4> sceneButtons_;
    branding::LogoMark logoMark_;
    std::array<juce::TextButton, 7> gestureButtons_;
    juce::TextButton transportButton_;
    juce::TextButton recordButton_;
    juce::TextButton mixerVariationButton_;
    juce::TextButton skyWebButton_;
    juce::TextButton tutorialButton_;
    juce::TextButton aboutButton_;
    juce::TextButton languageButton_;
    SupportLanguage supportLanguage_ = SupportLanguage::Portuguese;
    bool codaRequested_ = false;
    bool stopClosePending_ = false;
    double takeSampleRate_ = 0.0;
    juce::String takeCodaGesture_;
    double lastPerformerActivityMs_ = 0.0;
    std::uint32_t takeSeed_ = 0U;
    juce::String takeStartedAt_;
    juce::StringArray takeTrace_;
    std::uint32_t takeFieldTraceCount_ = 0U;
    double lastFieldTraceMs_ = 0.0;
    std::uint32_t lastTracedNarrativeTurn_ = std::numeric_limits<std::uint32_t>::max();
    juce::TextButton captureButton_;
    juce::TextButton undoCaptureButton_;
    juce::TextButton memoryButton_;
    std::array<std::optional<PerformanceMemory>, kPerformanceMemorySlots> memories_{};
    struct MemoryUndo { std::size_t slot = kPerformanceMemorySlots; std::optional<PerformanceMemory> before; };
    std::optional<MemoryUndo> memoryUndo_;
    std::unique_ptr<juce::FileChooser> memoryChooser_;
    // O ÁLBUM e o PERCURSO (2/11).
    PerformanceAlbum album_;
    juce::File albumDirectory_;
    juce::TextButton albumButton_;
    juce::TextButton listenButton_;  // ▶ OUVIR a última tomada
    juce::File lastTakeFile_;
    juce::TextButton percursoButton_;
    bool percurso_ = false;
    std::uint8_t percursoSection_ = 255U;
    bool takePercurso_ = false;
    bool takeStartedWithPlay_ = true;
    // A FAIXA com duração (2/11): 1/2/3/5 min, como no Antitotem; 0 = sem
    // limite (o REC de sempre).
    static constexpr std::array<double, 4> kTakeDurations{{60.0, 120.0, 180.0, 300.0}};
    static constexpr double kCodaLeadSeconds = 5.2;  // coda 4 s + cauda 1,2 s
    std::array<juce::TextButton, 4> durationButtons_;
    double takeTargetSeconds_ = 0.0;
    std::atomic<bool> albumMastering_{false};
    std::string lastRecalledMemory_;
    juce::Label seedLabel_;
    juce::TextEditor seedEditor_;
    juce::TextButton seedButton_;
    juce::Label actorSeedVariationLabel_;
    VariationTrackLookAndFeel variationTrackLook_;  // antes do slider: vive mais que ele
    juce::Slider actorSeedVariationSlider_;
    juce::Label tempoLabel_;
    juce::TextEditor tempoEditor_;
    juce::Slider tempoSlider_;
    juce::Slider masterSlider_;
    juce::Label masterLabel_;
    juce::Slider morphSlider_;
    juce::Label morphLabel_;
    std::array<juce::Slider, 8> macroSliders_;
    std::array<juce::Label, 8> macroLabels_;
    std::array<juce::Slider, 8> actorSliders_;
    std::array<juce::Label, 8> actorLabels_;
    std::array<juce::Slider, 8> stemGainSliders_;
    std::array<CompactMixerButton, 8> stemMuteButtons_;
    std::array<CompactMixerButton, 8> stemSoloButtons_;
    std::array<std::array<juce::Slider, 3>, 8> moduleControlSliders_;
    std::array<std::array<juce::Label, 3>, 8> moduleControlLabels_;
    std::array<juce::Slider, 3> walshControlSliders_;
    std::array<juce::Label, 3> walshControlLabels_;
    std::array<juce::String, 8> moduleDisplayNames_{{
        "ACID", "GENDYN", "DRUMS", "WALSH", "MODAL", "GALO", "ATROPELO", "DEJA VU"}};
    juce::String optionalModuleDisplayName_{utf8("WALSH")};
    std::array<juce::String, 16> relationDisplayNames_{{
        utf8("MATÉRIA ACID → GENDYN"), utf8("TEMPO GENDYN → DRUMS"),
        utf8("ESPAÇO ACID → DRUMS"), utf8("ÓRBITA ACID → WALSH"),
        utf8("CONTÁGIO GENDYN → WALSH"), utf8("VESTÍGIO DRUMS → WALSH"),
        utf8("MEMÓRIA ACID → MODAL"), utf8("PULSO MODAL → GALO"),
        utf8("RISCO GALO → ATROPELO"), utf8("ESPAÇO ATROPELO → DEJA VU"),
        utf8("DIAGONAL ACID → ATROPELO"), utf8("PONTE GENDYN → MODAL"),
        utf8("CRUZAMENTO GENDYN → ATROPELO"), utf8("SALTO DRUMS → GALO"),
        utf8("FECHO WALSH → DEJA VU"), utf8("RETORNO MODAL → ATROPELO")}};
    std::array<bool, 8> modulePanelActive_{{true, true, false, false, true, true, true, true}};
    std::array<juce::Slider, 16> relationSliders_;
    std::array<juce::Label, 16> relationLabels_;
    ConstellationGraph constellationGraph_;
    std::array<double, 8> macroValues_{0.50, 0.50, 0.50, 0.50, 0.50, 0.50, 0.50, 0.50};
    double fieldPrimaryX_ = 0.50;
    double fieldPrimaryY_ = 0.50;
    double fieldTextureX_ = 0.72;
    double fieldTextureY_ = 0.72;
    std::uint32_t displayedNarrativeTurn_ = 0U;
    double fieldNarrativeStartedMs_ = 0.0;
    double narrativePrimaryX_ = 0.50;
    double narrativePrimaryY_ = 0.50;
    double narrativeTextureX_ = 0.72;
    double narrativeTextureY_ = 0.72;
    double appliedNarrativePrimaryX_ = 0.50;
    double appliedNarrativePrimaryY_ = 0.50;
    double appliedNarrativeTextureX_ = 0.72;
    double appliedNarrativeTextureY_ = 0.72;
    bool fieldNarrativeActive_ = false;
    std::array<double, 8> actorSeedVariationValues_{};
    std::array<double, 8> actorPresenceValues_{};
    std::array<double, 16> relationValues_{0.52, 0.46, 0.36, 0.42, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    // Intensidade da órbita entre os corpos 1 e 4, alternada por um gesto
    // (o slider exclusivo que a mostrava saiu na auditoria de 28/10, item 3.4).
    double walshRelationValue_ = 0.42;
    std::array<double, 8> stemGainValues_{{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}};
    std::array<bool, 8> stemGainHeld_{};
    std::array<std::array<double, 3>, 8> moduleControlValues_
        = initialModuleControls(181U);
    std::array<double, 3> walshControlValues_{{0.48, 0.50, 0.42}};
    double masterValue_ = -6.0;
    double morphSeconds_ = 0.8;
    PerformanceScene lastCapturedScene_{};
    bool hasUndoCapture_ = false;
    static constexpr std::size_t kNoStemIndex = std::numeric_limits<std::size_t>::max();
    std::array<bool, 8> stemMuted_{};
    std::size_t soloStemIndex_ = kNoStemIndex;
    std::uint16_t selectedActorId_ = kInvalidActorId;
    std::size_t selectedModuleIndex_ = kNoStemIndex;
    PerformanceField performanceField_;
    juce::Rectangle<int> sceneArea_;
    juce::Rectangle<int> gestureArea_;
    juce::Rectangle<int> fieldArea_;
    juce::Rectangle<int> macroArea_;
    juce::Rectangle<int> constellationArea_;
    juce::Rectangle<int> relationArea_;
    juce::Rectangle<int> lastActionArea_;
    juce::Rectangle<int> footerArea_;
    juce::String audioFooterText_{utf8("ÁUDIO · aguardando dispositivo")};
    juce::String lastAction_{utf8("COMECE · clique num corpo ativo e experimente ABRIR")};
};

class PerformanceMainWindow final : public juce::DocumentWindow {
public:
    explicit PerformanceMainWindow(juce::JUCEApplication& application)
        : juce::DocumentWindow(application.getApplicationName(), juce::Colour(0xff102d27),
                               juce::DocumentWindow::allButtons), application_(application) {
        setUsingNativeTitleBar(true);
        setResizable(true, true);
        setResizeLimits(1280, 720, 10000, 10000);
        setContentOwned(new PerformanceHostComponent(), true);

        // Choose the primary display explicitly. Calling JUCE setFullScreen on
        // X11 delegates monitor selection to the window manager, which can
        // restore the last-used secondary monitor. Use the primary user area as
        // the actual maximised geometry instead.
        const auto& displays = juce::Desktop::getInstance().getDisplays();
        juce::Rectangle<int> primaryBounds;
        if (const auto* primaryDisplay = displays.getPrimaryDisplay()) {
            primaryBounds = primaryDisplay->userBounds.toNearestInt();
            setBounds(primaryBounds);
        } else {
            // Do not call centreWithSize without a display: on the affected
            // X11 startup path JUCE dereferenced an unavailable peer and the
            // process terminated before the window became visible.
            setBounds(48, 48, 1440, 900);
        }

        setVisible(true);
        // Re-apply after mapping: some X11 window managers restore the last
        // monitor during map and otherwise override the pre-map geometry.
        if (!primaryBounds.isEmpty()) setBounds(primaryBounds);
    }

    void closeButtonPressed() override { application_.systemRequestedQuit(); }

private:
    juce::JUCEApplication& application_;
};

class RasgoSynthPerformanceApplication final : public juce::JUCEApplication {
public:
    const juce::String getApplicationName() override { return "Rasgo Synth Asterismus"; }
    const juce::String getApplicationVersion() override { return "0.1.6-dev"; }

    void initialise(const juce::String&) override {
        window_ = std::make_unique<PerformanceMainWindow>(*this);
    }

    void shutdown() override { window_.reset(); }

private:
    std::unique_ptr<PerformanceMainWindow> window_;
};

} // namespace

START_JUCE_APPLICATION(RasgoSynthPerformanceApplication)
