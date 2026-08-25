#pragma once

#include "Ignis/Events/Event.h"

#include <string>
#include <utility>

namespace Ignis
{
    // Un livello dell'applicazione: un pezzo che vuole aggiornarsi, disegnare
    // interfaccia e ricevere eventi. L'editor sarà un layer, un gioco sarà un layer,
    // un HUD di debug sarà un overlay.
    class Layer
    {
    public:
        explicit Layer(std::string name = "Layer") : m_Name(std::move(name)) {}
        virtual ~Layer() = default;

        // I layer vivono dentro il LayerStack e si maneggiano per puntatore: copiarli
        // o spostarli non ha significato, e un layer copiato riceverebbe OnAttach una
        // volta sola pur esistendo in due esemplari.
        Layer(const Layer&)            = delete;
        Layer& operator=(const Layer&) = delete;
        Layer(Layer&&)                 = delete;
        Layer& operator=(Layer&&)      = delete;

        // Chiamati da LayerStack quando il layer entra e quando esce. Non esiste un
        // percorso in cui un layer viene distrutto senza essere prima staccato.
        virtual void OnAttach() {}
        virtual void OnDetach() {}

        // Aggiornamento logico. Il Timestep arriva al task 10.
        virtual void OnUpdate() {}

        // Disegno dell'interfaccia. SEPARATO da OnUpdate e non è ridondanza: il frame
        // ImGui è delimitato da Begin()/End(), e tutto ciò che disegna UI deve stare
        // dentro quelle due chiamate. Un layer che facesse ImGui dentro OnUpdate
        // funzionerebbe per caso e si romperebbe al primo cambio d'ordine.
        virtual void OnImGuiRender() {}

        // Riceve gli eventi. Mettere event.Handled = true ne ferma la propagazione
        // verso i layer sottostanti.
        virtual void OnEvent(Event& /*event*/) {}

        const std::string& GetName() const { return m_Name; }

    protected:
        std::string m_Name;
    };
}
