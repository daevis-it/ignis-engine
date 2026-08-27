#include "Ignis/Core/Application.h"
#include "Ignis/Core/Base.h"
#include "Ignis/Core/GLFWContext.h"
#include "Ignis/Core/Input.h"
#include "Ignis/Core/Logger.h"

// Application.h dichiara ImGuiLayer solo in avanti, per non far entrare ImGui nella
// catena di ogni client. Qui serve il tipo completo (make_unique, Begin/End), e qui
// è giusto averlo: siamo dentro l'engine.
#include "Ignis/ImGui/ImGuiLayer.h"

#include <format>
#include <stdexcept>

// Nessun header OpenGL qui dentro, dal task 13: le tre chiamate GL che stavano in
// questo file vivono ora in RenderCommand, cioè in Private/Ignis/Renderer/ (D17).
#include "Ignis/Renderer/RenderCommand.h"

#include <filesystem>
#include <utility>

namespace Ignis {

    Application* Application::s_Instance = nullptr;

    Application::Application(ApplicationSpecification specification)
        : m_Specification(std::move(specification))
    {
        s_Instance = this;

        // Prima di tutto il resto: se il client ha chiesto una directory di lavoro,
        // ci si sposta ORA, altrimenti ogni percorso relativo usato dopo (asset,
        // configurazioni) verrebbe risolto rispetto alla cartella sbagliata.
        if (!m_Specification.WorkingDirectory.empty())
        {
            std::error_code ec;
            std::filesystem::current_path(m_Specification.WorkingDirectory, ec);
            if (ec)
            {
                // Non è un assert: una cartella che non esiste è una condizione del
                // mondo reale, e deve fermare l'avvio anche in Release.
                throw std::runtime_error(std::format(
                    "Impossibile spostarsi nella directory di lavoro \"{}\": {}",
                    m_Specification.WorkingDirectory, ec.message()));
            }
            IGNIS_CORE_INFO("Directory di lavoro: {}", std::filesystem::current_path().string());
        }

        // Se GLFW non parte, il costruttore di GLFWContext lancia e l'avvio si
        // ferma qui: non c'è nessun percorso in cui si prosegue con una GLFW morta.
        m_GLFWContext = std::make_unique<GLFWContext>();

        m_Window = std::make_unique<Window>(m_Specification.Window);
        m_Window->SetEventCallback([this](Event& e) { this->OnEvent(e); });

        // Colore di sfondo di default. STA QUI E NON IN Run() PER UN MOTIVO PRECISO:
        // il costruttore della classe base gira PRIMA del corpo del costruttore del
        // client, quindi un gioco che chiami SetClearColor nel proprio costruttore
        // sovrascrive questo default. Se la stessa riga stesse in Run(), l'engine
        // ricablerebbe il colore dopo la scelta del client, e la scelta del client
        // non avrebbe alcun effetto — senza un solo messaggio d'errore.
        //
        // IMPALCATURA DICHIARATA: che sia l'engine a pulire lo schermo è provvisorio.
        // Al task 19 sarà la scena a decidere cosa c'è dietro. Non lo togliamo adesso
        // perché un client che si dimenticasse di pulire vedrebbe spazzatura, ed è
        // esattamente il tipo di default silenzioso che non vogliamo.
        RenderCommand::SetClearColor({ 0.2f, 0.3f, 0.3f, 1.0f });

        if (m_Specification.EnableImGui)
        {
            // Overlay: essendo in cima allo stack riceve gli eventi per primo e può
            // consumarli quando riguardano l'interfaccia.
            auto imguiLayer = std::make_unique<ImGuiLayer>();
            m_ImGuiLayer = imguiLayer.get();
            PushOverlay(std::move(imguiLayer));
        }
        else
        {
            IGNIS_CORE_INFO("ImGui disattivato dalla ApplicationSpecification.");
        }
    }

    Application::~Application() {
        // Niente Shutdown esplicito: ImGuiLayer::OnDetach lo fa, e lo chiama il
        // LayerStack distruggendosi — che avviene PRIMA di ~Window, quindi con il
        // contesto OpenGL ancora vivo. Vedi l'ordine dei membri in Application.h.
        // glfwTerminate() NON va più qui: la chiama ~GLFWContext, che per ordine di
        // dichiarazione dei membri gira DOPO ~Window. Vedi Application.h.
    }

    Layer* Application::PushLayer(std::unique_ptr<Layer> layer) {
        return m_LayerStack.PushLayer(std::move(layer));
    }

    Layer* Application::PushOverlay(std::unique_ptr<Layer> overlay) {
        return m_LayerStack.PushOverlay(std::move(overlay));
    }

    void Application::OnEvent(Event& e) {
        // Debug purpose only: per stampare i log dei ToString() degli specifici eventi
        // IGNIS_CORE_TRACE("{}", e.ToString());

        // Gli eventi di SISTEMA li gestisce l'Application, prima dei layer.
        // WindowClose viene consumato di proposito: se un layer potesse trattenerlo,
        // un bug in quel layer renderebbe l'applicazione impossibile da chiudere.
        // WindowResize invece ritorna false e prosegue: interessa a tutti.
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& event) {
            return this->OnWindowClose(event);
        });
        dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& event) {
            return this->OnWindowResize(event);
        });

        // A RITROSO: dall'alto verso il basso. Il primo a ricevere l'evento è
        // l'ultimo ad aver disegnato, cioè quello che l'utente vede davanti.
        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            if (e.Handled)
                break;
            (*it)->OnEvent(e);
        }
    }

    bool Application::OnWindowClose(WindowCloseEvent&) {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e) {
        // Il viewport lo aggiorna l'Application, NON la Window: la Window è il
        // sistema di finestre, non il renderer — emette l'evento e basta, e infatti
        // non ha dovuto cambiare di una riga quando il RenderCommand è nato.
        RenderCommand::SetViewport(0, 0, e.GetWidth(), e.GetHeight());

        // false: l'evento NON è consumato. Il resize interessa anche a chi verrà
        // dopo — l'ImGuiLayer oggi, i layer di gioco appena esisteranno.
        return false;
    }

    void Application::Run() {
        m_LastFrameTime = std::chrono::steady_clock::now();

        while (m_Running) {
            const auto now = std::chrono::steady_clock::now();
            float deltaSeconds = std::chrono::duration<float>(now - m_LastFrameTime).count();
            m_LastFrameTime = now;

            if (deltaSeconds > s_MaxTimestepSeconds)
            {
                IGNIS_CORE_TRACE("Delta time troncato: {:.3f}s -> {:.3f}s (stallo o breakpoint?)",
                                 deltaSeconds, s_MaxTimestepSeconds);
                deltaSeconds = s_MaxTimestepSeconds;
            }

            const Timestep timestep(deltaSeconds);

            RenderCommand::Clear();

            // Avanti: dal basso verso l'alto. Il gioco si aggiorna prima della UI
            // che lo mostra.
            for (auto& layer : m_LayerStack)
                layer->OnUpdate(timestep);

            // Il dockspace NON è più qui: era l'engine che imponeva a ogni client una
            // superficie di docking a tutto schermo. È una scelta dell'editor, e vive
            // nell'EditorLayer. Con quella riga se n'è andato anche <imgui.h> da
            // questo file.
            if (m_ImGuiLayer)
            {
                m_ImGuiLayer->Begin();

                for (auto& layer : m_LayerStack)
                    layer->OnImGuiRender();

                m_ImGuiLayer->End();
            }

            m_Window->Update();
        }
    }

}
