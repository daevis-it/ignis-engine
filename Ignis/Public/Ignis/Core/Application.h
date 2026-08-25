#pragma once

#include "Ignis/Core/LayerStack.h"
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

        // Lo stack possiede il layer; il puntatore restituito è un osservatore.
        Layer* PushLayer(std::unique_ptr<Layer> layer);
        Layer* PushOverlay(std::unique_ptr<Layer> overlay);

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
        bool m_Running = true;

        static Application* s_Instance;
    };

    // Funzione da definire nel CLIENT (Editor o Gioco Standalone)
    Application* CreateApplication();

}