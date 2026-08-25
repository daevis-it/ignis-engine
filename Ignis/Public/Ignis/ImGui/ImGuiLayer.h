#pragma once

#include "Ignis/Core/Layer.h"

namespace Ignis
{
    // L'interfaccia Dear ImGui, come OVERLAY dell'applicazione.
    //
    // Essendo un overlay sta in cima allo stack, quindi riceve gli eventi PER PRIMO —
    // ed è esattamente ciò che serve: se il puntatore è sopra un pannello, il clic
    // deve fermarsi lì e non arrivare al gioco sotto.
    class ImGuiLayer final : public Layer
    {
    public:
        ImGuiLayer() : Layer("ImGuiLayer") {}
        ~ImGuiLayer() override = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnEvent(Event& event) override;

        // Delimitano il frame ImGui. Le chiama Application::Run, non lo stack:
        // tutto ciò che disegna UI deve stare in mezzo a queste due.
        void Begin();
        void End();

        // Quando l'editor avrà un viewport di gioco a schermo intero, vorrà che
        // l'input arrivi al gioco anche col puntatore sopra l'area di ImGui.
        // Impalcatura per la Fase 4: il meccanismo c'è, il vocabolario è minimo.
        void SetBlockEvents(bool block) { m_BlockEvents = block; }
        bool GetBlockEvents() const { return m_BlockEvents; }

    private:
        bool m_BlockEvents = true;
    };
}
