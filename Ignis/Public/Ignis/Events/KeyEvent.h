#pragma once

#include "Ignis/Events/Event.h"

#include <format>

namespace Ignis
{
    // Base astratta: protected apposta, così non si può creare un KeyEvent generico.
    class KeyEvent : public Event
    {
    public:
        inline int GetKeyCode() const { return m_KeyCode; }

        // BITMASKING: un evento di tastiera appartiene a DUE categorie insieme.
        // Con un campo singolo dovresti scegliere quale delle due verità scrivere.
        IGNIS_EVENT_CLASS_CATEGORY(EventCategory::Keyboard | EventCategory::Input)

    protected:
        explicit KeyEvent(int keycode) : m_KeyCode(keycode) {}
        int m_KeyCode;
    };

    class KeyPressedEvent : public KeyEvent
    {
    public:
        KeyPressedEvent(int keycode, int repeatCount)
            : KeyEvent(keycode), m_RepeatCount(repeatCount) {}

        inline int GetRepeatCount() const { return m_RepeatCount; }

        std::string ToString() const override {
            return std::format("KeyPressedEvent: {} (Ripetizioni: {})", m_KeyCode, m_RepeatCount);
        }

        IGNIS_EVENT_CLASS_TYPE(KeyPressed)

    private:
        int m_RepeatCount;
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:
        explicit KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

        std::string ToString() const override {
            return std::format("KeyReleasedEvent: {}", m_KeyCode);
        }

        IGNIS_EVENT_CLASS_TYPE(KeyReleased)
    };

    // NON è un duplicato di KeyPressedEvent. KeyPressed dice quale TASTO è sceso;
    // KeyTyped dice quale CARATTERE ne è uscito, con layout di tastiera, modificatori
    // e dead key già applicati dal sistema operativo. Premere il tasto della E con
    // Shift produce un KeyPressed(E) e un KeyTyped('E').
    // Serve all'input di testo — ImGui lo usa per i campi di scrittura.
    class KeyTypedEvent : public KeyEvent
    {
    public:
        explicit KeyTypedEvent(unsigned int codepoint)
            : KeyEvent(static_cast<int>(codepoint)) {}

        // Il codepoint Unicode, non un keycode.
        inline unsigned int GetCodepoint() const { return static_cast<unsigned int>(m_KeyCode); }

        std::string ToString() const override {
            return std::format("KeyTypedEvent: U+{:04X}", static_cast<unsigned int>(m_KeyCode));
        }

        IGNIS_EVENT_CLASS_TYPE(KeyTyped)
    };
}
