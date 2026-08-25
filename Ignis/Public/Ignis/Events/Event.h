#pragma once

#include <format>
#include <functional>
#include <ostream>     // serve a operator<<: prima arrivava per inclusione transitiva
#include <string>

namespace Ignis
{
    // ══════════════════════════════════════════════════════════════════════════
    //  Due assi indipendenti descrivono un evento:
    //    - il TIPO,       uno solo per evento  -> EventType
    //    - le CATEGORIE,  anche più d'una      -> EventCategory (bitmask)
    // ══════════════════════════════════════════════════════════════════════════

    enum class EventType
    {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

    // Rimossi al task 06: AppTick, AppUpdate, AppRender. Non li emetteva nessuno, e
    // non li emetterà: l'aggiornamento dei layer sarà una chiamata diretta
    // (layer->OnUpdate(ts)), non un evento che attraversa il dispatcher — passare da
    // un evento per dire "aggiornati" è indiretto e costoso senza alcun vantaggio.
    // Un enum che promette cose inesistenti è documentazione falsa.

    // enum CLASS, dal task 06: prima era un enum semplice, quindi 'None',
    // 'EventCategoryInput' e compagnia erano simboli nudi dentro namespace Ignis —
    // con un 'Ignis::None' che convive male con EventType::None.
    enum class EventCategory : int
    {
        None        = 0,
        Application = 1 << 0,   // 00001
        Input       = 1 << 1,   // 00010
        Keyboard    = 1 << 2,   // 00100
        Mouse       = 1 << 3,   // 01000
        MouseButton = 1 << 4    // 10000
    };

    // Gli enum class non hanno operatori bitwise: vanno dati noi. È il prezzo della
    // sicurezza sui tipi, ed è dieci righe una volta sola.
    constexpr EventCategory operator|(EventCategory a, EventCategory b) noexcept
    {
        return static_cast<EventCategory>(static_cast<int>(a) | static_cast<int>(b));
    }

    constexpr EventCategory operator&(EventCategory a, EventCategory b) noexcept
    {
        return static_cast<EventCategory>(static_cast<int>(a) & static_cast<int>(b));
    }

    class Event
    {
    public:
        virtual ~Event() = default;

        virtual EventType     GetEventType()     const = 0;
        virtual const char*   GetName()          const = 0;
        virtual EventCategory GetCategoryFlags() const = 0;

        virtual std::string ToString() const { return GetName(); }

        // const, dal task 06: chiedere a che categoria appartiene un evento non lo
        // modifica, e senza const non si può interrogare un 'const Event&'.
        bool IsInCategory(EventCategory category) const
        {
            return (GetCategoryFlags() & category) != EventCategory::None;
        }

        // Quando diventa true, l'evento smette di propagarsi ai livelli successivi.
        bool Handled = false;
    };

    class EventDispatcher
    {
    public:
        explicit EventDispatcher(Event& event) : m_Event(event) {}

        // Se il tipo in transito corrisponde a T, chiama func e aggiorna Handled.
        template<typename T, typename F>
        bool Dispatch(const F& func)
        {
            if (m_Event.GetEventType() == T::GetStaticType())
            {
                m_Event.Handled |= func(static_cast<T&>(m_Event));
                return true;
            }
            return false;
        }

    private:
        Event& m_Event;
    };

    inline std::ostream& operator<<(std::ostream& os, const Event& e)
    {
        return os << e.ToString();
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  Boilerplate degli eventi
//
//  Ogni classe evento ripeteva gli stessi tre metodi, dodici volte. Espansione di
//  IGNIS_EVENT_CLASS_TYPE(KeyPressed):
//
//      static EventType GetStaticType() { return EventType::KeyPressed; }
//      EventType GetEventType() const override { return GetStaticType(); }
//      const char* GetName() const override { return "KeyPressed"; }
//
//  Il #type stringifica il nome, quindi nome del tipo e stringa non possono più
//  divergere: erano tre punti in cui sbagliare a ogni evento nuovo.
// ══════════════════════════════════════════════════════════════════════════════

#define IGNIS_EVENT_CLASS_TYPE(type)                                     \
    static EventType GetStaticType() { return EventType::type; }         \
    EventType GetEventType() const override { return GetStaticType(); }  \
    const char* GetName() const override { return #type; }

#define IGNIS_EVENT_CLASS_CATEGORY(categories)                           \
    EventCategory GetCategoryFlags() const override { return categories; }
