#pragma once

#include "Ignis/Core/Application.h"
#include "Ignis/Core/Logger.h"

#include <cstdlib>
#include <exception>
#include <memory>

extern Ignis::Application* Ignis::CreateApplication(Ignis::ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
    // Primo di tutto: senza questa, i colori non sono attivi e su Windows
    // uscirebbero i codici ANSI grezzi. Sta qui e non nel client perché il main
    // appartiene all'engine — così non si può dimenticare.
    Ignis::Logger::Init();

    IGNIS_CORE_INFO("Avvio di Ignis Engine...");

    // Le eccezioni servono SOLO per gli errori fatali di avvio: succedono una volta
    // o mai, quindi non costano niente, e lasciano un punto unico dove decidere cosa
    // farne. Non sono flusso di controllo, e non entrano nel game loop.
    try
    {
        // unique_ptr e non 'delete app': se Run() lancia, il delete non verrebbe
        // mai eseguito e l'Application resterebbe in piedi fino a fine processo.
        // argc/argv arrivano finalmente da qualche parte: erano i due warning
        // "unused parameter" che tenevamo accesi apposta dal task 01.
        std::unique_ptr<Ignis::Application> app(
            Ignis::CreateApplication(Ignis::ApplicationCommandLineArgs{ argc, argv }));
        app->Run();
    }
    catch (const std::exception& e)
    {
        IGNIS_CORE_ERROR("Avvio fallito: {}", e.what());
        return EXIT_FAILURE;
    }
    catch (...)
    {
        IGNIS_CORE_ERROR("Avvio fallito per un'eccezione di tipo sconosciuto.");
        return EXIT_FAILURE;
    }

    IGNIS_CORE_INFO("Chiusura di Ignis Engine.");
    return EXIT_SUCCESS;
}
