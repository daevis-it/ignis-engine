cmake_minimum_required(VERSION 3.10)
project(IgnisEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Directory di inclusione
include_directories(include)
include_directories(include/vendor) # Isola le librerie esterne

# 2. Raccogliamo i sorgenti dell'Engine in modo modulare
# Nota: L'uso di GLOB_RECURSE è comodo, ma ricordati di ricaricare CMake se aggiungi nuovi file.
file(GLOB IGNIS_APP_SOURCES "src/Ignis/*.cpp")
file(GLOB_RECURSE IGNIS_CORE_SOURCES "src/Ignis/Core/*.cpp")
file(GLOB_RECURSE IGNIS_IMGUI_SOURCES "src/Ignis/ImGui/*.cpp")

# 3. Raccogliamo i sorgenti del Vendor
set(VENDOR_SOURCES src/vendor/glad.c)
file(GLOB VENDOR_IMGUI_SOURCES "src/vendor/imgui/*.cpp")

# 4. Creazione dell'eseguibile unendo i moduli
add_executable(ignis-engine
        ${IGNIS_APP_SOURCES}
        ${IGNIS_CORE_SOURCES}
        ${IGNIS_IMGUI_SOURCES}
        ${VENDOR_SOURCES}
        ${VENDOR_IMGUI_SOURCES}
)

find_package(glfw3 REQUIRED)
target_link_libraries(ignis-engine glfw)