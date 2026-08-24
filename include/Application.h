#pragma once
#include "Window.h"
#include "ImGuiLayer.h"
#include <memory>

class Application {
public:
    Application();
    ~Application();
    void Run();

    // Permette di ottenere un riferimento alla finestra attuale
    Window& GetWindow() { return *m_Window; }

    // Recupera l'istanza globale dell'applicazione
    static Application& Get() { return *s_Instance; }

private:
    std::unique_ptr<Window> m_Window;
    ImGuiLayer m_ImGuiLayer;
    static Application* s_Instance; // Puntatore statico a se stessa
};