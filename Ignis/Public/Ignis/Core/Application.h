#pragma once

#include "Ignis/Core/Window.h"
#include "Ignis/ImGui/ImGuiLayer.h"
#include "Ignis/Events/ApplicationEvent.h"
#include <memory>

namespace Ignis {

    class Application {
    public:
        Application();
        virtual ~Application();

        void Run();
        void OnEvent(Event& e);

        Window& GetWindow() { return *m_Window; }
        static Application& Get() { return *s_Instance; }

    private:
        bool OnWindowClose(WindowCloseEvent& e);

        std::unique_ptr<Window> m_Window;
        ImGuiLayer m_ImGuiLayer;
        bool m_Running = true;

        static Application* s_Instance;
    };

    // Funzione da definire nel CLIENT (Editor o Gioco Standalone)
    Application* CreateApplication();

}