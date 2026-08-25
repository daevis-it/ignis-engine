#include "EditorLayer.h"

#include <imgui.h>

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
        m_AccumuloSec   = 0.0f;
        m_FrameNelSec   = 0;
        m_SommaMsNelSec = 0.0f;
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

    // ESC chiude l'editor. È una decisione DEL CLIENT, non del motore: prima stava
    // cablata nel game loop di Application, e nessun gioco vero si chiude con ESC.
    //
    // Ed è un EVENTO, non più polling: perciò passa dall'ImGuiLayer, che lo consuma
    // quando ImGui vuole la tastiera. Risultato: scrivendo in un campo di testo,
    // ESC non chiude più l'applicazione. Prima lo faceva, ed era sbagliato.
    dispatcher.Dispatch<KeyPressedEvent>([](KeyPressedEvent& e) {
        if (e.GetKeyCode() == static_cast<int>(KeyCode::Escape))
        {
            IGNIS_INFO("ESC premuto: chiusura richiesta dall'editor.");
            Application::Get().Close();
            return true;   // consumato
        }
        return false;
    });
}

void EditorLayer::OnImGuiRender()
{
    // Il dockspace vive QUI e non nell'engine: una superficie di docking a tutto
    // schermo è una scelta dell'editor. Un gioco con due pannelli di debug non la
    // vuole, e il motore non deve imporgliela.
    // Va creato PRIMA degli altri pannelli, così possono agganciarcisi.
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

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

    ImGui::Separator();
    ImGui::Checkbox("Mostra la demo di ImGui", &m_MostraDemo);

    ImGui::End();

    if (m_MostraDemo)
        ImGui::ShowDemoWindow(&m_MostraDemo);
}
