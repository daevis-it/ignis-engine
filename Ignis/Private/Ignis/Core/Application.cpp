#include "Ignis/Core/Application.h"
#include "Ignis/Core/Base.h"
#include "Ignis/Core/GLFWContext.h"
#include "Ignis/Core/Input.h"
#include "Ignis/Core/Logger.h"

// Window.h non li espone più: chi usa OpenGL se lo include da sé. Queste due
// chiamate GL sono provvisorie e migreranno nel Renderer alla Fase 2.
#include <glad/glad.h>

#include <imgui.h>

namespace Ignis {

    Application* Application::s_Instance = nullptr;

    Application::Application() {
        s_Instance = this;

        // Se GLFW non parte, il costruttore di GLFWContext lancia e l'avvio si
        // ferma qui: non c'è nessun percorso in cui si prosegue con una GLFW morta.
        m_GLFWContext = std::make_unique<GLFWContext>();

        m_Window = std::make_unique<Window>(WindowProps{
            .Title  = "Ignis Engine",
            .Width  = 1280,
            .Height = 720,
            .VSync  = true
        });
        m_Window->SetEventCallback([this](Event& e) { this->OnEvent(e); });

        // L'ImGuiLayer entra come overlay: essendo in cima, riceverà gli eventi per
        // primo e potrà consumarli quando riguardano l'interfaccia.
        auto imguiLayer = std::make_unique<ImGuiLayer>();
        m_ImGuiLayer = imguiLayer.get();
        PushOverlay(std::move(imguiLayer));
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
        IGNIS_CORE_TRACE("{}", e.ToString());

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
        // glViewport lo chiama l'Application, NON la Window: la Window è il sistema
        // di finestre, non il renderer. Quando il Renderer esisterà, si prenderà
        // questo compito senza che la Window debba cambiare di una riga.
        glViewport(0, 0, static_cast<GLsizei>(e.GetWidth()), static_cast<GLsizei>(e.GetHeight()));

        // false: l'evento NON è consumato. Il resize interessa anche a chi verrà
        // dopo — l'ImGuiLayer oggi, i layer di gioco appena esisteranno.
        return false;
    }

    void Application::Run() {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        while (m_Running) {
            if (Input::IsKeyPressed(KeyCode::Escape))
                m_Running = false;

            glClear(GL_COLOR_BUFFER_BIT);

            // Avanti: dal basso verso l'alto. Il gioco si aggiorna prima della UI
            // che lo mostra.
            for (auto& layer : m_LayerStack)
                layer->OnUpdate();

            m_ImGuiLayer->Begin();

            // Il dockspace PRIMA dei pannelli dei layer, così possono agganciarcisi.
            ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

            for (auto& layer : m_LayerStack)
                layer->OnImGuiRender();

            m_ImGuiLayer->End();

            m_Window->Update();
        }
    }

}
