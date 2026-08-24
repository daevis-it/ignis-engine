#pragma once

#include "Ignis/Core/Application.h"
#include "Ignis/Core/Logger.h"

extern Ignis::Application* Ignis::CreateApplication();

int main(int argc, char** argv)
{
    // Primo di tutto: senza questa, i colori non sono attivi e su Windows
    // uscirebbero i codici ANSI grezzi. Sta qui e non nel client perché il main
    // appartiene all'engine — così non si può dimenticare.
    Ignis::Logger::Init();

    IGNIS_CORE_INFO("Avvio di Ignis Engine...");

    auto app = Ignis::CreateApplication();
    app->Run();
    delete app;

    IGNIS_CORE_INFO("Chiusura di Ignis Engine.");
    return 0;
}
