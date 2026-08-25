#pragma once

#include "Ignis/Core/Window.h"
#include "Ignis/ImGui/ImGuiLayer.h"
#include "Ignis/Events/ApplicationEvent.h"
#include <memory>

namespace Ignis {

    // Definita in Private/: che sotto ci sia GLFW è un dettaglio interno
    // dell'engine, non qualcosa che un client debba sapere.
    class GLFWContext;

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
        bool OnWindowResize(WindowResizeEvent& e);

        // ══ L'ORDINE DI QUESTI DUE MEMBRI È LA CORREZIONE DEL TASK 04 ══
        // I membri si distruggono in ordine INVERSO di dichiarazione. Il contesto
        // GLFW, dichiarato per primo, è quindi l'ULTIMO a morire — dopo la finestra.
        // Prima glfwTerminate() stava nel corpo di ~Application e girava PRIMA di
        // ~Window, cioè si distruggeva una finestra già liberata.
        // NON riordinare: il compilatore non protesterebbe, il bug tornerebbe.
        std::unique_ptr<GLFWContext> m_GLFWContext;
        std::unique_ptr<Window>      m_Window;

        ImGuiLayer m_ImGuiLayer;
        bool m_Running = true;

        static Application* s_Instance;
    };

    // Funzione da definire nel CLIENT (Editor o Gioco Standalone)
    Application* CreateApplication();

}