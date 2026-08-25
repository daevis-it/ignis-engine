#include "Ignis/Core/Application.h"
#include "Ignis/Core/Input.h"
#include "Ignis/Core/Logger.h"
#include <imgui.h>

#include "Ignis/Core/Base.h"

namespace Ignis {

    Application* Application::s_Instance = nullptr;

    Application::Application() {
        s_Instance = this;
        if (!glfwInit()) {
            IGNIS_CORE_ERROR("Errore durante l'inizializzazione di GLFW!");
        }

        m_Window = std::make_unique<Window>(800, 600, "Ignis Engine");
        m_Window->SetEventCallback([this](Event& e) { this->OnEvent(e); });

        m_ImGuiLayer.Init();
    }

    Application::~Application() {
        m_ImGuiLayer.Shutdown();
        glfwTerminate();
    }

    void Application::OnEvent(Event& e) {
        IGNIS_CORE_TRACE("{}", e.ToString());

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& event) {
            return this->OnWindowClose(event);
        });
    }

    bool Application::OnWindowClose(WindowCloseEvent& e) {
        m_Running = false;
        return true;
    }

    void Application::Run() {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        while (m_Running) {
            if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
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
