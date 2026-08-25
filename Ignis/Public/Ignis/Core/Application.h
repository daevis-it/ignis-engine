#pragma once

#include "Ignis/Core/Base.h"
#include "Ignis/Core/LayerStack.h"
#include "Ignis/Core/Window.h"
#include "Ignis/Events/ApplicationEvent.h"
#include <chrono>
#include <memory>
#include <string>

namespace Ignis {

    // Definita in Private/: che sotto ci sia GLFW è un dettaglio interno
    // dell'engine, non qualcosa che un client debba sapere.
    class GLFWContext;

    // Dichiarata in avanti e non inclusa: qui serve solo tenerne un PUNTATORE.
    // Includere "Ignis/ImGui/ImGuiLayer.h" farebbe entrare ImGui nella catena di
    // ogni client che include Application.h, per un tipo di cui basta il nome.
    // Chi vuole chiamarne i metodi (GetImGuiLayer) include quell'header da sé.
    class ImGuiLayer;

    // Gli argomenti della riga di comando, passati dall'engine al client.
    // Oggi nessuno li usa davvero: sono la GIUNTURA per il Launcher della Fase 5,
    // che avvierà l'editor come `IgnisEditor /percorso/progetto`. Metterla adesso
    // costa niente; aggiungerla dopo vorrebbe dire riscrivere l'avvio del motore.
    struct ApplicationCommandLineArgs
    {
        int    Count = 0;
        char** Args  = nullptr;

        const char* operator[](int index) const
        {
            IGNIS_CORE_ASSERT(index >= 0 && index < Count,
                              "Indice {} fuori dai {} argomenti disponibili", index, Count);
            return Args[index];
        }
    };

    struct ApplicationSpecification
    {
        std::string Name = "Ignis Application";

        // Titolo, dimensioni e VSync della finestra.
        WindowProps Window{};

        // false = l'ImGuiLayer non viene nemmeno creato. Un gioco non paga contesto,
        // atlas dei font e un NewFrame/Render a vuoto per ogni frame.
        // NOTA: toglie il costo a RUNTIME, non i simboli dal binario — per quello
        // servirebbe separare ImGui a livello di build (vedi ROADMAP, Fase 5).
        bool EnableImGui = true;

        // Se non vuota, l'engine ci si sposta all'avvio. È l'altra metà della
        // giuntura per il Launcher: aprire un progetto significa lavorare dentro
        // la sua cartella.
        std::string WorkingDirectory;

        ApplicationCommandLineArgs CommandLineArgs;
    };

    class Application {
    public:
        explicit Application(ApplicationSpecification specification = {});
        virtual ~Application();

        void Run();
        void OnEvent(Event& e);

        // Chiude ordinatamente l'applicazione. QUANDO chiuderla è una decisione del
        // client: nessun gioco vero si chiude con ESC, quindi quella scelta non può
        // stare cablata nel game loop del motore.
        void Close() { m_Running = false; }

        const ApplicationSpecification& GetSpecification() const { return m_Specification; }

        // Lo stack possiede il layer; il puntatore restituito è un osservatore.
        Layer* PushLayer(std::unique_ptr<Layer> layer);
        Layer* PushOverlay(std::unique_ptr<Layer> overlay);

        Window& GetWindow() { return *m_Window; }

        // Può essere nullptr: con EnableImGui = false l'overlay non viene creato.
        // Serve alla Fase 4, quando il viewport dell'editor vorrà SetBlockEvents(false).
        ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
        static Application& Get() { return *s_Instance; }

    private:
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

        // ══ L'ORDINE DI QUESTI DUE MEMBRI È LA CORREZIONE DEL TASK 04 ══
        // I membri si distruggono in ordine INVERSO di dichiarazione. Il contesto
        // GLFW, dichiarato per primo, è quindi l'ULTIMO a morire — dopo la finestra.
        // Prima glfwTerminate() stava nel corpo di ~Application e girava PRIMA di
        // ~Window, cioè si distruggeva una finestra già liberata.
        // NON riordinare: il compilatore non protesterebbe, il bug tornerebbe.
        ApplicationSpecification m_Specification;

        std::unique_ptr<GLFWContext> m_GLFWContext;
        std::unique_ptr<Window>      m_Window;

        // DOPO m_Window, e per lo stesso motivo per cui m_GLFWContext sta prima:
        // i layer possiederanno risorse GPU (shader, texture, framebuffer) e devono
        // morire MENTRE il contesto OpenGL è ancora vivo. Distruggendosi per primo,
        // lo stack rilascia tutto prima che la finestra — e con lei il contesto —
        // se ne vada. Invertirli darebbe glDelete* su un contesto morto, in silenzio.
        LayerStack m_LayerStack;

        // NON è più un membro concreto: l'ImGuiLayer è un overlay POSSEDUTO dallo
        // stack. Questo è solo un riferimento osservatore, perché Application::Run
        // deve poter chiamare Begin()/End() a delimitare il frame — cosa che lo
        // stack, che sa solo di Layer generici, non può fare.
        ImGuiLayer* m_ImGuiLayer = nullptr;

        // steady_clock e NON system_clock: quest'ultimo può saltare all'indietro per
        // una sincronizzazione NTP o un cambio d'ora, e un delta time negativo fa
        // cose molto strane in una simulazione. steady_clock è monotono per contratto.
        std::chrono::steady_clock::time_point m_LastFrameTime{};

        // Tetto al delta time. Se metti un breakpoint e riprendi dopo trenta secondi,
        // il frame successivo avrebbe ts = 30.0 e qualunque cosa si muova
        // attraverserebbe la mappa — con la fisica, esploderebbe. 0.1s equivale a
        // 10 FPS: sotto quella soglia il tempo rallenta invece di saltare.
        // Non è un default silenzioso: quando scatta viene loggato.
        static constexpr float s_MaxTimestepSeconds = 0.1f;
        bool m_Running = true;

        static Application* s_Instance;
    };

    // Funzione da definire nel CLIENT (Editor o Gioco Standalone).
    // Riceve gli argomenti della riga di comando: è il client a decidere cosa farne.
    Application* CreateApplication(ApplicationCommandLineArgs args);

}