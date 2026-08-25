#include "EditorLayer.h"

#include <imgui.h>

#include <chrono>
#include <thread>

void EditorLayer::OnAttach()
{
    IGNIS_INFO("EditorLayer agganciato.");
}

void EditorLayer::OnUpdate(Ignis::Timestep ts)
{
    m_FrameTimeMs = ts.GetMilliseconds();

    // Media sull'ultimo secondo: il valore istantaneo balla troppo per essere letto.
    m_SommaMsNelSec += m_FrameTimeMs;
    ++m_FrameNelSec;
    m_AccumuloSec += ts.GetSeconds();

    if (m_AccumuloSec >= 1.0f)
    {
        m_MediaMs = m_SommaMsNelSec / static_cast<float>(m_FrameNelSec);

        // Temporaneo, per verificare il task 10 anche senza guardare il pannello:
        // va tolto quando il Timestep sarà dato per assodato.
        IGNIS_INFO("frame medio: {:.2f} ms  ({:.0f} FPS su {} frame)",
                   m_MediaMs, 1000.0f / m_MediaMs, m_FrameNelSec);

        m_AccumuloSec   = 0.0f;
        m_FrameNelSec   = 0;
        m_SommaMsNelSec = 0.0f;
    }

    // Stallo simulato: serve a vedere il tetto al delta time entrare in funzione.
    // Il frame DOPO questo deve arrivare troncato a 100 ms, non a ~1000.
    if (m_StalloRichiesto)
    {
        m_StalloRichiesto = false;
        IGNIS_WARN("Stallo simulato di 1 secondo: il prossimo delta dovrà essere troncato.");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void EditorLayer::OnEvent(Ignis::Event& event)
{
    using namespace Ignis;

    EventDispatcher dispatcher(event);

    dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) {
        ++m_TastiRicevuti;
        m_UltimoKeyCode = e.GetKeyCode();
        return false;   // non lo consumiamo: vogliamo solo osservarlo
    });

    dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent&) {
        ++m_ClicRicevuti;
        return false;
    });
}

void EditorLayer::OnImGuiRender()
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui::Begin("Ignis — prova della cattura input");

    ImGui::TextWrapped(
        "Scrivi nel campo qui sotto: i contatori NON devono aumentare, perche' "
        "l'ImGuiLayer (overlay, quindi sopra) consuma gli eventi quando ImGui vuole "
        "l'input. Cliccando invece sullo sfondo della finestra, i clic arrivano.");

    ImGui::Separator();

    ImGui::InputText("campo di testo", m_Testo, sizeof(m_Testo));

    ImGui::Separator();

    ImGui::Text("ImGui vuole la tastiera : %s", io.WantCaptureKeyboard ? "SI" : "no");
    ImGui::Text("ImGui vuole il mouse    : %s", io.WantCaptureMouse    ? "SI" : "no");

    ImGui::Separator();

    ImGui::Text("Tasti arrivati all'EditorLayer : %d", m_TastiRicevuti);
    ImGui::Text("Ultimo keycode                 : %d", m_UltimoKeyCode);
    ImGui::Text("Clic arrivati all'EditorLayer  : %d", m_ClicRicevuti);

    if (ImGui::Button("Azzera contatori"))
    {
        m_TastiRicevuti = 0;
        m_ClicRicevuti  = 0;
        m_UltimoKeyCode = 0;
    }

    ImGui::Separator();
    ImGui::Text("Frame corrente : %.2f ms", m_FrameTimeMs);
    ImGui::Text("Media 1 secondo: %.2f ms  (%.0f FPS)", m_MediaMs,
                m_MediaMs > 0.0f ? 1000.0f / m_MediaMs : 0.0f);
    ImGui::TextWrapped("Con il VSync attivo la media deve stare intorno a 16.6 ms su uno "
                       "schermo a 60 Hz. Senza, molto meno.");

    if (ImGui::Button("Simula uno stallo di 1 secondo"))
        m_StalloRichiesto = true;
    ImGui::SameLine();
    ImGui::TextDisabled("(il frame dopo va troncato a 100 ms)");

    ImGui::Separator();
    ImGui::Checkbox("Mostra la demo di ImGui", &m_MostraDemo);

    ImGui::End();

    if (m_MostraDemo)
        ImGui::ShowDemoWindow(&m_MostraDemo);
}
