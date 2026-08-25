#include "Ignis/ImGui/ImGuiLayer.h"
#include "Ignis/Core/Application.h"
#include "Ignis/Core/Logger.h"
#include "Ignis/Core/Window.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

namespace Ignis
{
    void ImGuiLayer::OnAttach()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // ══════════════════════════════════════════════════════════════════════
        //  ImGuiConfigFlags_ViewportsEnable è DISATTIVATO di proposito (2026-08-25).
        //
        //  I viewport permettono di trascinare un pannello fuori dalla finestra,
        //  trasformandolo in una finestra del sistema operativo. Su Linux questo si
        //  rompe in modo dipendente dal window manager: le finestre senza decorazioni
        //  che ImGui crea non ricevono il focus della TASTIERA (il mouse sì, perché
        //  ImGui traccia la posizione globalmente). Sintomo osservato su Mint:
        //  un campo di testo trascinato fuori si attiva col clic e poi non scrive.
        //  È un problema noto e aperto a monte — vedi ocornut/imgui issue #2117,
        //  il thread dedicato ai window manager Linux.
        //
        //  In più espone un limite NOSTRO: Input::IsKeyPressed interroga sempre la
        //  finestra principale, quindi con più finestre il polling non vede i tasti
        //  di quella che ha davvero il focus. Va risolto PRIMA di riattivare la riga
        //  qui sotto, non dopo.
        //
        //  Il docking — la parte che serve a comporre il layout dell'editor — resta
        //  attivo e non è toccato da niente di tutto questo.
        //
        //  Da rivalutare alla Fase 4, quando ci saranno pannelli che vale la pena
        //  mettere su un secondo monitor.
        //
        //  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        // ══════════════════════════════════════════════════════════════════════

        ImGui::StyleColorsDark();

        // ORDINE CRITICO, e nessun errore te lo direbbe: la Window installa le proprie
        // callback GLFW nel costruttore; qui il 'true' fa sì che ImGui installi le sue
        // SALVANDO le precedenti e richiamandole a catena. Se l'ordine fosse invertito,
        // gli eventi dell'engine sparirebbero in silenzio.
        GLFWwindow* window = Application::Get().GetWindow().GetNativeWindow();
        ImGui_ImplGlfw_InitForOpenGL(window, true);

        // Deve corrispondere al contesto richiesto in Window.cpp (4.5 Core -> GLSL 450).
        // Un mismatch non dà un errore chiaro: ImGui semplicemente non disegna.
        ImGui_ImplOpenGL3_Init("#version 450");

        IGNIS_CORE_INFO("ImGui inizializzato (docking attivo, viewport disattivati, GLSL 450)");
    }

    void ImGuiLayer::OnDetach()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::OnEvent(Event& event)
    {
        if (!m_BlockEvents)
            return;

        const ImGuiIO& io = ImGui::GetIO();

        // ImGui riceve l'input dalle proprie callback GLFW, non dal nostro sistema di
        // eventi: qui non gli STIAMO PASSANDO niente, gli stiamo CHIEDENDO se quello
        // che è appena successo riguardava lui. In caso affermativo consumiamo
        // l'evento, così i layer sottostanti non lo vedono.
        //
        // NOTA: WantCaptureMouse/Keyboard sono calcolati durante ImGui::NewFrame(),
        // quindi il valore letto qui è quello del frame PRECEDENTE. È il comportamento
        // normale e la latenza di un frame è impercettibile — ma se un giorno un clic
        // sembrasse "passare attraverso" nel primo frame in cui apri un pannello, la
        // causa è questa.
        if (event.IsInCategory(EventCategory::Mouse) && io.WantCaptureMouse)
            event.Handled = true;

        if (event.IsInCategory(EventCategory::Keyboard) && io.WantCaptureKeyboard)
            event.Handled = true;
    }

    void ImGuiLayer::Begin()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End()
    {
        const ImGuiIO& io = ImGui::GetIO();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Con i viewport attivi, le finestre trascinate fuori dall'applicazione hanno
        // un proprio contesto GL: va ripristinato quello nostro prima di proseguire.
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backupContext = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupContext);
        }
    }
}
