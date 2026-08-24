#pragma once

#include "Ignis/Events/Event.h"
#include <format>

namespace Ignis
{
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
}