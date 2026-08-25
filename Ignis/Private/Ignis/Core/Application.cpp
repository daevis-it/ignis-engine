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

        m_ImGuiLayer.Init();
    }

    Application::~Application() {
        m_ImGuiLayer.Shutdown();
        // glfwTerminate() NON va più qui: la chiama ~GLFWContext, che per ordine di
        // dichiarazione dei membri gira DOPO ~Window. Vedi Application.h.
    }

    void Application::OnEvent(Event& e) {
        IGNIS_CORE_TRACE("{}", e.ToString());

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& event) {
            return this->OnWindowClose(event);
        });
        dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& event) {
            return this->OnWindowResize(event);
        });
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

            m_ImGuiLayer.Begin();
            ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
            ImGui::ShowDemoWindow();
            m_ImGuiLayer.End();

            m_Window->Update();
        }
    }

}
