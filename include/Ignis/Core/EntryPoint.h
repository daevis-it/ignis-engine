#pragma once
#include "Ignis/Core/Application.h"
#include "Ignis/Core/Logger.h"

extern Ignis::Application* Ignis::CreateApplication();

int main(int argc, char** argv) {
    // Inizializzazione globale del logging o sottosistemi
    Ignis::Logger::Info("Avvio di Ignis Engine...");

    auto app = Ignis::CreateApplication();
    app->Run();
    delete app;

    Ignis::Logger::Info("Chiusura di Ignis Engine.");
    return 0;
}