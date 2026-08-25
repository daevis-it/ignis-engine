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

    // NON è un duplicato di KeyPressedEvent. KeyPressed dice quale TASTO è sceso;
    // KeyTyped dice quale CARATTERE ne è uscito, con layout di tastiera, modificatori
    // e dead key già applicati dal sistema operativo. Premere il tasto della E con
    // Shift produce un KeyPressed(E) e un KeyTyped('E').
    // Serve all'input di testo — ImGui lo usa per i campi di scrittura.
    class KeyTypedEvent : public KeyEvent {
    public:
        explicit KeyTypedEvent(unsigned int codepoint)
            : KeyEvent(static_cast<int>(codepoint)) {}

        // Il codepoint Unicode, non un keycode GLFW.
        inline unsigned int GetCodepoint() const { return static_cast<unsigned int>(m_KeyCode); }

        std::string ToString() const override {
            return std::format("KeyTypedEvent: U+{:04X}", static_cast<unsigned int>(m_KeyCode));
        }

        static EventType GetStaticType() { return EventType::KeyTyped; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "KeyTyped"; }
    };
}