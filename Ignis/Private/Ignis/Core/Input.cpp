#include "Ignis/Core/Input.h"
#include "Ignis/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Ignis
{
    bool Input::IsKeyPressed(int keycode) {
        // Recuperiamo la finestra attiva dall'Application
        auto window = Application::Get().GetWindow().GetNativeWindow();
        auto state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonPressed(int button) {
        auto window = Application::Get().GetWindow().GetNativeWindow();
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    std::pair<float, float> Input::GetMousePosition() {
        auto window = Application::Get().GetWindow().GetNativeWindow();
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return { (float)xpos, (float)ypos };
    }

    float Input::GetMouseX() {
        return GetMousePosition().first;
    }

    float Input::GetMouseY() {
        return GetMousePosition().second;
    }
}