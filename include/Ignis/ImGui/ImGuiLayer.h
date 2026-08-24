#pragma once

namespace Ignis
{
    class ImGuiLayer {
    public:
        void Init();
        void Shutdown();
        void Begin();
        void End();
    };
}