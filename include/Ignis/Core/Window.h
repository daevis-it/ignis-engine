#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <functional>
#include "Ignis/Events/Event.h"

namespace Ignis
{
    class Window {
    public:
        // Definiamo il tipo di funzione che la Window chiamerà quando accade qualcosa
        using EventCallbackFn = std::function<void(Event&)>;

        Window(int width, int height, const std::string& title);
        ~Window();

        void Update();
        GLFWwindow* GetNativeWindow() const { return m_Window; }

        // Metodo per collegare l'Application alla Window
        void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }

    private:
        GLFWwindow* m_Window;

        // Struttura interna che passeremo a GLFW
        struct WindowData {
            std::string Title;
            unsigned int Width, Height;
            EventCallbackFn EventCallback;
        };

        WindowData m_Data;
    };
}