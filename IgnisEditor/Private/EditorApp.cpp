#include "Ignis/Core/Application.h"
#include "Ignis/Core/EntryPoint.h"

class EditorApplication : public Ignis::Application {
public:
    EditorApplication() {
        // Inizializzazione specifica dell'Editor
    }

    ~EditorApplication() override {
        // Cleanup specifico dell'Editor
    }
};

// Implementazione del Factory Method
Ignis::Application* Ignis::CreateApplication() {
    return new EditorApplication();
}