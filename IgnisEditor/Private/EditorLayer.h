#pragma once

#include "Ignis/Ignis.h"

// Il primo layer vero dell'editor. Oggi contiene poco: la demo di ImGui (che prima
// stava dentro l'engine, dove non doveva stare) e un pannello che serve a verificare
// la cattura dell'input.
class EditorLayer final : public Ignis::Layer
{
public:
    EditorLayer() : Layer("EditorLayer") {}

    void OnAttach() override;
    void OnUpdate() override;
    void OnImGuiRender() override;
    void OnEvent(Ignis::Event& event) override;

private:
    // Contano gli eventi che ARRIVANO FIN QUI, cioè quelli che l'ImGuiLayer sopra di
    // noi non ha consumato.
    int  m_TastiRicevuti  = 0;
    int  m_ClicRicevuti   = 0;
    int  m_UltimoKeyCode  = 0;

    char m_Testo[128] = "";
    bool m_MostraDemo  = true;
};
