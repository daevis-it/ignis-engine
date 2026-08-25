#include "Ignis/Ignis.h"
#include "Ignis/Core/EntryPoint.h"

#include "EditorLayer.h"

#include <memory>

class EditorApplication final : public Ignis::Application
{
public:
    EditorApplication()
    {
        // PushLayer e non PushOverlay: l'editor sta SOTTO l'ImGuiLayer, che deve
        // poter consumare gli eventi prima che arrivino qui.
        PushLayer(std::make_unique<EditorLayer>());
    }

    ~EditorApplication() override = default;
};

Ignis::Application* Ignis::CreateApplication()
{
    return new EditorApplication();
}
