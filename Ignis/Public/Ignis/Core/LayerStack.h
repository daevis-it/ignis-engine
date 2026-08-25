#pragma once

#include "Ignis/Core/Layer.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace Ignis
{
    // ══════════════════════════════════════════════════════════════════════════
    //  Lo stack dei livelli, e le due direzioni in cui si percorre.
    //
    //    update e render  ->  dal BASSO verso l'ALTO   (begin  .. end)
    //    eventi           ->  dall'ALTO verso il BASSO (rbegin .. rend)
    //
    //  L'ordine degli eventi è l'INVERSO dell'ordine visivo, ed è l'unica regola che
    //  rende sensata un'interfaccia: se clicchi su un pannello che copre il gioco,
    //  deve rispondere il pannello — cioè la cosa che VEDI, che è quella disegnata
    //  per ultima. Propagando a ritroso, il primo a ricevere è l'ultimo ad aver
    //  disegnato.
    //
    //  Gli OVERLAY stanno sempre in cima, anche se inseriti prima: ImGui, un HUD di
    //  debug, una console. Così un layer di gioco aggiunto dopo non finisce mai
    //  sopra l'interfaccia.
    //
    //      [ 0 ][ 1 ][ 2 ]  [ overlay ][ overlay ]
    //                    ^
    //             m_LayerInsertIndex
    //      PushLayer inserisce qui, PushOverlay in fondo.
    // ══════════════════════════════════════════════════════════════════════════
    class LayerStack
    {
    public:
        LayerStack() = default;
        ~LayerStack();

        LayerStack(const LayerStack&)            = delete;
        LayerStack& operator=(const LayerStack&) = delete;
        LayerStack(LayerStack&&)                 = delete;
        LayerStack& operator=(LayerStack&&)      = delete;

        // Lo stack POSSIEDE il layer; il puntatore restituito è solo un riferimento
        // osservatore, comodo per chiamare metodi specifici senza cercarlo.
        // Chiamano OnAttach.
        Layer* PushLayer(std::unique_ptr<Layer> layer);
        Layer* PushOverlay(std::unique_ptr<Layer> overlay);

        // Chiamano OnDetach e distruggono il layer.
        void PopLayer(Layer* layer);
        void PopOverlay(Layer* overlay);

        auto begin()        { return m_Layers.begin();  }
        auto end()          { return m_Layers.end();    }
        auto rbegin()       { return m_Layers.rbegin(); }
        auto rend()         { return m_Layers.rend();   }
        auto begin()  const { return m_Layers.begin();  }
        auto end()    const { return m_Layers.end();    }

        std::size_t Size() const { return m_Layers.size(); }

    private:
        std::vector<std::unique_ptr<Layer>> m_Layers;

        // Confine fra la zona dei layer e quella degli overlay.
        std::size_t m_LayerInsertIndex = 0;
    };
}
