#pragma once
#include <utility> // Serve per std::pair

class Input {
public:
    // Tastiera
    static bool IsKeyPressed(int keycode);

    // Mouse
    static bool IsMouseButtonPressed(int button);
    static std::pair<float, float> GetMousePosition();
    static float GetMouseX();
    static float GetMouseY();
};