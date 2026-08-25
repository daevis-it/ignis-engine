#include "Ignis/Core/LayerStack.h"
#include "Ignis/Core/Logger.h"

#include <algorithm>

namespace Ignis
{
    LayerStack::~LayerStack()
    {
        // A ritroso, simmetrico all'ordine di attach: l'ultimo entrato è il primo a
        // uscire. Un layer che ha registrato qualcosa in OnAttach lo disfa nell'ordine
        // opposto, che è l'unico che non lascia riferimenti pendenti.
        for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it)
            (*it)->OnDetach();

        m_Layers.clear();
    }

    Layer* LayerStack::PushLayer(std::unique_ptr<Layer> layer)
    {
        if (!layer)
        {
            IGNIS_CORE_WARN("PushLayer chiamata con un puntatore nullo: ignorata.");
            return nullptr;
        }

        Layer* observer = layer.get();
        m_Layers.insert(m_Layers.begin() + static_cast<std::ptrdiff_t>(m_LayerInsertIndex),
                        std::move(layer));
        ++m_LayerInsertIndex;

        observer->OnAttach();
        IGNIS_CORE_TRACE("Layer aggiunto: \"{}\" (posizione {}, layer)", observer->GetName(), m_LayerInsertIndex - 1);
        return observer;
    }

    Layer* LayerStack::PushOverlay(std::unique_ptr<Layer> overlay)
    {
        if (!overlay)
        {
            IGNIS_CORE_WARN("PushOverlay chiamata con un puntatore nullo: ignorata.");
            return nullptr;
        }

        Layer* observer = overlay.get();
        m_Layers.push_back(std::move(overlay));

        observer->OnAttach();
        IGNIS_CORE_TRACE("Overlay aggiunto: \"{}\" (posizione {})", observer->GetName(), m_Layers.size() - 1);
        return observer;
    }

    void LayerStack::PopLayer(Layer* layer)
    {
        // Cerca SOLO nella zona dei layer: un overlay passato qui non va trovato,
        // altrimenti m_LayerInsertIndex verrebbe decrementato per sbaglio e ogni
        // inserimento successivo finirebbe nel posto sbagliato.
        const auto endOfLayers = m_Layers.begin() + static_cast<std::ptrdiff_t>(m_LayerInsertIndex);
        const auto it = std::find_if(m_Layers.begin(), endOfLayers,
                                     [layer](const auto& owned) { return owned.get() == layer; });

        if (it == endOfLayers)
        {
            // Rumoroso di proposito: chiedere di rimuovere un layer che non c'è è un
            // bug del chiamante, e in silenzio diventerebbe un layer che resta vivo
            // mentre chi l'ha chiesto crede di averlo tolto.
            IGNIS_CORE_WARN("PopLayer: il layer richiesto non è nello stack.");
            return;
        }

        (*it)->OnDetach();
        m_Layers.erase(it);
        --m_LayerInsertIndex;
    }

    void LayerStack::PopOverlay(Layer* overlay)
    {
        const auto startOfOverlays = m_Layers.begin() + static_cast<std::ptrdiff_t>(m_LayerInsertIndex);
        const auto it = std::find_if(startOfOverlays, m_Layers.end(),
                                     [overlay](const auto& owned) { return owned.get() == overlay; });

        if (it == m_Layers.end())
        {
            IGNIS_CORE_WARN("PopOverlay: l'overlay richiesto non è nello stack.");
            return;
        }

        (*it)->OnDetach();
        m_Layers.erase(it);
        // m_LayerInsertIndex NON cambia: gli overlay stanno oltre il confine.
    }
}
