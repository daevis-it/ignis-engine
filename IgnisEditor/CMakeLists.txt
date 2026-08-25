# ══ IgnisEditor — un client. Linka l'engine come farebbe chiunque altro. ══

file(GLOB_RECURSE EDITOR_FILES CONFIGURE_DEPENDS "Private/*.cpp" "Private/*.h")

add_executable(IgnisEditor ${EDITOR_FILES})
target_include_directories(IgnisEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Private)
# Ignis nasconde ImGui ai suoi client (task 05c), quindi l'editor lo linka per conto
# proprio: NON lo eredita dall'engine, lo SCEGLIE. La distinzione conta — l'editor ha
# bisogno di ImGui per disegnare i suoi pannelli, un gioco no. Il Sandbox infatti non
# lo linka, ed è la prova che un gioco costruito su Ignis non se lo porta dietro.
target_link_libraries(IgnisEditor PRIVATE Ignis imgui)

if(MSVC)
    target_compile_options(IgnisEditor PRIVATE /W4)
else()
    target_compile_options(IgnisEditor PRIVATE -Wall -Wextra)
endif()
