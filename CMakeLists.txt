cmake_minimum_required(VERSION 3.10)
project(IgnisEngine)

# AGGIORNATO AL C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Raccoglie tutti i file .cpp di ImGui nella variabile IMGUI_SOURCES
file(GLOB IMGUI_SOURCES "include/imgui/*.cpp")

# Compila l'eseguibile aggiungendo anche i sorgenti di ImGui
add_executable(ignis-engine
        src/main.cpp
        src/glad.c
        src/Window.cpp
        src/Application.cpp
        src/Input.cpp
        src/ImGuiLayer.cpp
        ${IMGUI_SOURCES}
)

find_package(glfw3 REQUIRED)
target_link_libraries(ignis-engine glfw)