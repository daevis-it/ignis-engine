#include "Ignis/Ignis.h"
#include "Ignis/Core/EntryPoint.h"

#include "EditorLayer.h"

#include <memory>

class EditorApplication final : public Ignis::Application
{
public:
    explicit EditorApplication(const Ignis::ApplicationSpecification& spec)
        : Application(spec)
    {
        // PushLayer e non PushOverlay: l'editor sta SOTTO l'ImGuiLayer, che deve
        // poter consumare gli eventi prima che arrivino qui.
        PushLayer(std::make_unique<EditorLayer>());

        // Il percorso di progetto che il Launcher passerà un giorno. Oggi lo
        // logghiamo e basta: è impalcatura, non arredo.
        const auto& args = GetSpecification().CommandLineArgs;
        if (args.Count > 1)
            IGNIS_INFO("Argomento ricevuto: \"{}\" (non ancora usato)", args[1]);
        else
            IGNIS_INFO("Nessun progetto passato da riga di comando.");
    }

    ~EditorApplication() override = default;
};

Ignis::Application* Ignis::CreateApplication(Ignis::ApplicationCommandLineArgs args)
{
    Ignis::ApplicationSpecification spec;
    spec.Name            = "Ignis Editor";
    spec.Window.Title    = "Ignis Editor";
    spec.Window.Width    = 1280;
    spec.Window.Height   = 720;
    spec.Window.VSync    = true;
    spec.EnableImGui     = true;      // l'editor È interfaccia
    spec.CommandLineArgs = args;

    return new EditorApplication(spec);
}
