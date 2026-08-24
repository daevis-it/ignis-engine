#include "Ignis/Ignis.h"
#include "Ignis/Core/EntryPoint.h"

// Il Sandbox è un gioco finto: serve a dimostrare che l'engine si linka da fuori
// usando SOLO gli header pubblici. Se un giorno questo file non compila più mentre
// l'editor sì, vuol dire che abbiamo fatto passare un dettaglio interno nell'API.

class SandboxApplication : public Ignis::Application
{
public:
    SandboxApplication()
    {
        Ignis::Logger::Info("Sandbox avviato: sto usando Ignis come farebbe un gioco vero.");
    }

    ~SandboxApplication() override = default;
};

Ignis::Application* Ignis::CreateApplication()
{
    return new SandboxApplication();
}
