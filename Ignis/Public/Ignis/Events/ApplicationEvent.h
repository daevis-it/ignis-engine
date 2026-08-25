#pragma once

#include "Ignis/Events/Event.h"
#include <format>

namespace Ignis
{
    // ATTENZIONE: larghezza e altezza sono PIXEL del framebuffer, non screen
    // coordinates. È la misura che serve a glViewport, e su schermi HiDPI le due
    // divergono. L'evento nasce da glfwSetFramebufferSizeCallback.
    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(unsigned int width, unsigned int height)
            : m_Width(width), m_Height(height) {}

        inline unsigned int GetWidth() const { return m_Width; }
        inline unsigned int GetHeight() const { return m_Height; }

        // Override per la stampa formattata
        std::string ToString() const override {
            return std::format("WindowResizeEvent: {}x{}", m_Width, m_Height);
        }

        // Metodi necessari per il nostro EventDispatcher
        static EventType GetStaticType() { return EventType::WindowResize; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "WindowResize"; }

        // Appartiene solo alla categoria dell'applicazione
        int GetCategoryFlags() const override { return EventCategoryApplication; }

    private:
        unsigned int m_Width, m_Height;
    };

    class WindowCloseEvent : public Event {
    public:
        WindowCloseEvent() = default;

        static EventType GetStaticType() { return EventType::WindowClose; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "WindowClose"; }
        int GetCategoryFlags() const override { return EventCategoryApplication; }
    };

    class WindowFocusEvent : public Event {
    public:
        WindowFocusEvent() = default;

        static EventType GetStaticType() { return EventType::WindowFocus; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "WindowFocus"; }
        int GetCategoryFlags() const override { return EventCategoryApplication; }
    };

    class WindowLostFocusEvent : public Event {
    public:
        WindowLostFocusEvent() = default;

        static EventType GetStaticType() { return EventType::WindowLostFocus; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "WindowLostFocus"; }
        int GetCategoryFlags() const override { return EventCategoryApplication; }
    };

    class WindowMovedEvent : public Event {
    public:
        WindowMovedEvent(int x, int y) : m_X(x), m_Y(y) {}

        inline int GetX() const { return m_X; }
        inline int GetY() const { return m_Y; }

        std::string ToString() const override {
            return std::format("WindowMovedEvent: {}, {}", m_X, m_Y);
        }

        static EventType GetStaticType() { return EventType::WindowMoved; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "WindowMoved"; }
        int GetCategoryFlags() const override { return EventCategoryApplication; }

    private:
        int m_X, m_Y;
    };
}