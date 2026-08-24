#include "Application.h"
#include "Input.h"
#include "Logger.h"
#include <imgui/imgui.h>

// Inizializziamo la variabile statica a nullptr
Application* Application::s_Instance = nullptr;

Application::Application() {
    s_Instance = this;

    if (!glfwInit()) {
        Logger::Error("Impossibile inizializzare GLFW!");
    } else {
        Logger::Info("GLFW inizializzato correttamente.");
    }

    m_Window = std::make_unique<Window>(800, 600, "Ignis Engine");

    m_ImGuiLayer.Init();
}

Application::~Application() {
    m_ImGuiLayer.Shutdown();
    glfwTerminate();
}

void Application::Run() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    while (!m_Window->ShouldClose()) {
        if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
            glfwSetWindowShouldClose(m_Window->GetNativeWindow(), true);

        glClear(GL_COLOR_BUFFER_BIT);

        // --- INIZIO UI ---
        m_ImGuiLayer.Begin();

        // Trasforma l'intera finestra di Ignis Engine in un'area Dockable
        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

        // Mostra la finestra di debug piena di esempi
        ImGui::ShowDemoWindow();

        m_ImGuiLayer.End();
        // --- FINE UI ---

        m_Window->Update();
    }
}