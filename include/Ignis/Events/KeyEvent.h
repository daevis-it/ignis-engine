#pragma once
#include "Ignis/Events/Event.h"
#include <format>

namespace Ignis
{
    // Classe base astratta per tutti gli eventi della tastiera
    class KeyEvent : public Event {
    public:
        inline int GetKeyCode() const { return m_KeyCode; }

        // NOTA IL BITMASKING: Questo evento appartiene a DUE categorie!
        int GetCategoryFlags() const override { return EventCategoryKeyboard | EventCategoryInput; }

    protected:
        // Essendo protected, non possiamo creare un "KeyEvent" generico, ma solo le sue sottoclassi
        KeyEvent(int keycode) : m_KeyCode(keycode) {}
        int m_KeyCode;
    };

    class KeyPressedEvent : public KeyEvent {
    public:
        KeyPressedEvent(int keycode, int repeatCount)
            : KeyEvent(keycode), m_RepeatCount(repeatCount) {}

        inline int GetRepeatCount() const { return m_RepeatCount; }

        std::string ToString() const override {
            return std::format("KeyPressedEvent: {} (Ripetizioni: {})", m_KeyCode, m_RepeatCount);
        }

        static EventType GetStaticType() { return EventType::KeyPressed; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "KeyPressed"; }

    private:
        int m_RepeatCount;
    };

    class KeyReleasedEvent : public KeyEvent {
    public:
        KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

        std::string ToString() const override {
            return std::format("KeyReleasedEvent: {}", m_KeyCode);
        }

        static EventType GetStaticType() { return EventType::KeyReleased; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "KeyReleased"; }
    };
}