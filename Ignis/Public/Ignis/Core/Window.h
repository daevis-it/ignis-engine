#pragma once

#include "Ignis/Events/Event.h"

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

// GLFWwindow è un tipo OPACO: GLFW non ne espone mai la definizione, quindi una
// dichiarazione in avanti basta e avanza per tenerne un puntatore. È tutto ciò che
// serviva per togliere ~920 KB di glad.h + GLFW dall'API pubblica dell'engine.
struct GLFWwindow;

namespace Ignis
{
    // Un aggregato invece di tre parametri sciolti: al task 11
    // l'ApplicationSpecification dovrà passarli, e allargare una struct non tocca
    // i punti di costruzione, mentre cambiare una firma li tocca tutti.
    struct WindowProps
    {
        std::string Title  = "Ignis Engine";
        uint32_t    Width  = 1280;
        uint32_t    Height = 720;
        bool        VSync  = true;
    };

    class Window
    {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        explicit Window(const WindowProps& props = WindowProps{});
        ~Window();

        // Regola ferrea: chi possiede una risorsa dichiara come si copia e come si
        // sposta. Window possiede una GLFWwindow*; copiarla darebbe due oggetti con
        // lo stesso handle e una doppia glfwDestroyWindow. Il move è possibile in
        // teoria, ma richiederebbe di ri-puntare glfwSetWindowUserPointer al nuovo
        // indirizzo di m_Data: finché nessuno ne ha bisogno, = delete è più onesto
        // di un move sottilmente rotto.
        Window(const Window&)            = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&)                 = delete;
        Window& operator=(Window&&)      = delete;

        void Update();

        // ATTENZIONE: sono i pixel del FRAMEBUFFER, non le screen coordinates.
        // Su schermi HiDPI i due valori divergono, e per glViewport servono i pixel.
        uint32_t GetWidth()  const { return m_Data.Width;  }
        uint32_t GetHeight() const { return m_Data.Height; }

        void SetVSync(bool enabled);
        bool IsVSync() const { return m_Data.VSync; }

        GLFWwindow* GetNativeWindow() const { return m_Window; }

        void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }

    private:
        GLFWwindow* m_Window = nullptr;

        // Questa struttura viene passata a GLFW con glfwSetWindowUserPointer ed è
        // il ponte fra le callback (funzioni C, che non possono catturare stato) e
        // l'oggetto C++ che ha generato l'evento.
        struct WindowData
        {
            std::string     Title;
            uint32_t        Width  = 0;   // pixel del framebuffer
            uint32_t        Height = 0;   // pixel del framebuffer
            bool            VSync  = true;
            EventCallbackFn EventCallback;

            // GLFW segnala CHE un tasto si ripete, non QUANTE volte: il conteggio
            // ce lo teniamo noi. Azzerato al rilascio.
            std::unordered_map<int, int> KeyRepeatCount;
        };

        WindowData m_Data;
    };
}
