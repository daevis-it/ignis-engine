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
    void OnUpdate(Ignis::Timestep ts) override;
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

    // --- misura del frame ---
    float m_FrameTimeMs   = 0.0f;   // ultimo frame
    float m_MediaMs       = 0.0f;   // media dell'ultimo secondo
    float m_AccumuloSec   = 0.0f;
    int   m_FrameNelSec   = 0;
    float m_SommaMsNelSec = 0.0f;
};
