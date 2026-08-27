#include "Ignis/Ignis.h"
#include "Ignis/Core/EntryPoint.h"

// Il Sandbox è un gioco finto: serve a dimostrare che l'engine si linka da fuori
// usando SOLO gli header pubblici. Se un giorno questo file non compila più mentre
// l'editor sì, vuol dire che abbiamo fatto passare un dettaglio interno nell'API.

class SandboxApplication final : public Ignis::Application
{
public:
    explicit SandboxApplication(const Ignis::ApplicationSpecification& spec)
        : Application(spec)
    {
        IGNIS_INFO("Sandbox avviato: sto usando Ignis come farebbe un gioco vero.");

        // Il colore di sfondo è una decisione DEL GIOCO, non del motore. Funziona
        // perché il costruttore di Ignis::Application — che imposta il default e ha
        // già creato finestra e contesto GL — è finito prima che questo corpo
        // cominci. Magenta e non un altro verde-azzurro: se il task 13 fallisse, la
        // differenza deve saltare all'occhio invece di somigliare al default.
        Ignis::RenderCommand::SetClearColor({ 0.9f, 0.1f, 0.6f, 1.0f });
    }

    ~SandboxApplication() override = default;
};

Ignis::Application* Ignis::CreateApplication(Ignis::ApplicationCommandLineArgs args)
{
    Ignis::ApplicationSpecification spec;
    spec.Name          = "Sandbox";
    spec.Window.Title  = "Sandbox — un gioco su Ignis";
    spec.Window.Width  = 1280;
    spec.Window.Height = 720;
    spec.Window.VSync  = true;

    // IL PUNTO DI QUESTO FILE: un gioco non ha bisogno di ImGui, e non deve pagarlo.
    // Con questa riga il Sandbox non crea il contesto, non carica l'atlas dei font e
    // non fa un NewFrame/Render a vuoto a ogni frame.
    spec.EnableImGui = false;

    spec.CommandLineArgs = args;

    return new SandboxApplication(spec);
}
