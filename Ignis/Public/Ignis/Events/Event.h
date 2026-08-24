#pragma once
#include <string>
#include <functional>

namespace Ignis
{
    // 1. Definiamo tutti i possibili tipi di eventi che l'engine può gestire
    enum class EventType {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        AppTick, AppUpdate, AppRender,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

    // 2. Definiamo le Categorie usando lo "Shift dei bit" (Bitmasking).
    // Questo ci permette di assegnare a un evento PIÙ categorie contemporaneamente.
    enum EventCategory {
        None = 0,
        EventCategoryApplication    = 1 << 0, // In binario: 00001
        EventCategoryInput          = 1 << 1, // In binario: 00010
        EventCategoryKeyboard       = 1 << 2, // In binario: 00100
        EventCategoryMouse          = 1 << 3, // In binario: 01000
        EventCategoryMouseButton    = 1 << 4  // In binario: 10000
    };

    class Event {
    public:
        virtual ~Event() = default;

        // Metodi virtuali puri che le sottoclassi dovranno implementare
        virtual EventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
        virtual int GetCategoryFlags() const = 0;

        // Per stampare l'evento nel nostro Logger in modo leggibile
        virtual std::string ToString() const { return GetName(); }

        // Funzione comodissima per capire se un evento fa parte di una categoria
        // Usa l'operatore AND bit a bit (&)
        inline bool IsInCategory(EventCategory category) {
            return GetCategoryFlags() & category;
        }

        // Se "Handled" diventa true, l'evento smette di propagarsi ad altri livelli
        bool Handled = false;
    };

    class EventDispatcher {
    public:
        // Quando creiamo un dispatcher, gli passiamo l'evento appena avvenuto
        EventDispatcher(Event& event) : m_Event(event) {}

        // Usiamo i template del C++ per gestire qualsiasi tipo di evento in modo generico
        template<typename T, typename F>
        bool Dispatch(const F& func) {
            // Se il tipo di evento in transito corrisponde al tipo che vogliamo gestire...
            if (m_Event.GetEventType() == T::GetStaticType()) {

                // ...eseguiamo la funzione e aggiorniamo lo stato "Handled" dell'evento
                m_Event.Handled |= func(static_cast<T&>(m_Event));
                return true;
            }
            return false;
        }
    private:
        Event& m_Event;
    };

    // Un piccolo extra per facilitare la stampa degli eventi col nostro Logger std::format
    inline std::ostream& operator<<(std::ostream& os, const Event& e) {
        return os << e.ToString();
    }
}