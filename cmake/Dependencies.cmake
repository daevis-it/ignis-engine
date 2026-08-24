# Dipendenze esterne — decisione D2: FetchContent, niente installazioni a sistema.
# Il primo configure scarica e compila (qualche minuto); i successivi usano la cache.

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# --------------------------------------------------------------------- GLFW
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

# Su Linux GLFW 3.4 compila per default SIA X11 SIA Wayland, e il backend Wayland
# pretende wayland-scanner, libwayland-dev e libxkbcommon-dev già in fase di configure:
# se mancano, la configure muore con "Failed to find wayland-scanner".
# Mint 22.3 / Cinnamon gira su X11, quindi teniamo Wayland spento finché non serve.
if(UNIX AND NOT APPLE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11     ON  CACHE BOOL "" FORCE)
endif()
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)

# --------------------------------------------------------------------- GLM
# Non serve ancora a nessuno: servirà dal Renderer 2D in poi. La dichiariamo ora
# perché aggiungere una dipendenza a build funzionante è un cambio isolato.
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)

# --------------------------------------------------------------------- Dear ImGui
# 'docking' è un BRANCH, non un tag: si muove sotto i piedi. Pinniamo il commit
# esatto su cui questa build è stata verificata, così fra sei mesi il repo compila
# ancora con lo stesso ImGui invece che con quello del giorno.
#
# Per aggiornare in futuro, deliberatamente:
#   1. rimetti GIT_TAG docking, riconfigura
#   2. git -C build/<preset>/_deps/imgui-src rev-parse HEAD
#   3. rimetti lo SHA qui sotto e verifica che compili ancora
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        fd13a1e8923a0a7077b404fc36fd063b25a0c0b5  # branch docking @ 2026-08-19
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(glfw glm imgui)

# ImGui non fornisce un CMakeLists: il target lo costruiamo noi, scegliendo
# esplicitamente i due backend che ci servono (GLFW + OpenGL3).
if(NOT TARGET imgui)
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui PUBLIC glfw)
endif()
