#include "Ignis/ImGui/ImGuiLayer.h"
#include "Ignis/Core/Application.h"

// Includiamo i file di ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

namespace Ignis
{
    void ImGuiLayer::Init() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        // ABILITIAMO IL DOCKING E I VIEWPORT!
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        // Inizializziamo i backend usando la finestra di Ignis Engine
        GLFWwindow* window = Application::Get().GetWindow().GetNativeWindow();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        // Deve corrispondere al contesto richiesto in Window.cpp (4.5 Core -> GLSL 450).
        // Un mismatch qui non dà un errore chiaro: ImGui semplicemente non disegna,
        // oppure i suoi shader falliscono la compilazione in silenzio.
        ImGui_ImplOpenGL3_Init("#version 450");
    }

    void ImGuiLayer::Begin() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End() {
        ImGuiIO& io = ImGui::GetIO();

        // Genera la grafica dell'UI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Se i Viewport sono attivi, gestisce le finestre trascinate fuori dall'app principale
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void ImGuiLayer::Shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}