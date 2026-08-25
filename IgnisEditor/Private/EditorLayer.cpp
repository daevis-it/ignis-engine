#include "EditorLayer.h"

#include <imgui.h>

void EditorLayer::OnAttach()
{
    IGNIS_INFO("EditorLayer agganciato.");
}

void EditorLayer::OnUpdate()
{
    // Polling: questo NON passa dal sistema di eventi, quindi non è filtrato da
    // ImGui. È la differenza fra "cos'è successo" e "com'è adesso", e si vede bene
    // nel pannello qui sotto: scrivendo nel campo di testo, il contatore degli eventi
    // resta fermo mentre lo stato del tasto risulta comunque premuto.
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
    ImGui::Checkbox("Mostra la demo di ImGui", &m_MostraDemo);

    ImGui::End();

    if (m_MostraDemo)
        ImGui::ShowDemoWindow(&m_MostraDemo);
}
