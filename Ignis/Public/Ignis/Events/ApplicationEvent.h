#pragma once

#include "Ignis/Events/Event.h"

#include <format>

namespace Ignis
{
    // ATTENZIONE: larghezza e altezza sono PIXEL del framebuffer, non screen
    // coordinates. È la misura che serve a glViewport, e su schermi HiDPI le due
    // divergono. L'evento nasce da glfwSetFramebufferSizeCallback.
    class WindowResizeEvent : public Event
    {
    public:
        WindowResizeEvent(unsigned int width, unsigned int height)
            : m_Width(width), m_Height(height) {}

        inline unsigned int GetWidth()  const { return m_Width;  }
        inline unsigned int GetHeight() const { return m_Height; }

        std::string ToString() const override {
            return std::format("WindowResizeEvent: {}x{}", m_Width, m_Height);
        }

        IGNIS_EVENT_CLASS_TYPE(WindowResize)
        IGNIS_EVENT_CLASS_CATEGORY(EventCategory::Application)

    private:
        unsigned int m_Width, m_Height;
    };

    class WindowCloseEvent : public Event
    {
    public:
        WindowCloseEvent() = default;

        IGNIS_EVENT_CLASS_TYPE(WindowClose)
        IGNIS_EVENT_CLASS_CATEGORY(EventCategory::Application)
    };

    class WindowFocusEvent : public Event
    {
    public:
        WindowFocusEvent() = default;

        IGNIS_EVENT_CLASS_TYPE(WindowFocus)
        IGNIS_EVENT_CLASS_CATEGORY(EventCategory::Application)
    };

    class WindowLostFocusEvent : public Event
    {
    public:
        WindowLostFocusEvent() = default;

        IGNIS_EVENT_CLASS_TYPE(WindowLostFocus)
        IGNIS_EVENT_CLASS_CATEGORY(EventCategory::Application)
    };

    class WindowMovedEvent : public Event
    {
    public:
        WindowMovedEvent(int x, int y) : m_X(x), m_Y(y) {}

        inline int GetX() const { return m_X; }
        inline int GetY() const { return m_Y; }

        std::string ToString() const override {
            return std::format("WindowMovedEvent: {}, {}", m_X, m_Y);
        }

        IGNIS_EVENT_CLASS_TYPE(WindowMoved)
        IGNIS_EVENT_CLASS_CATEGORY(EventCategory::Application)

    private:
        int m_X, m_Y;
    };
}
