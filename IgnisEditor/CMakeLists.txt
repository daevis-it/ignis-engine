# ══ IgnisEditor — un client. Linka l'engine come farebbe chiunque altro. ══

file(GLOB_RECURSE EDITOR_FILES CONFIGURE_DEPENDS "Private/*.cpp" "Private/*.h")

add_executable(IgnisEditor ${EDITOR_FILES})
target_include_directories(IgnisEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Private)
target_link_libraries(IgnisEditor PRIVATE Ignis)

if(MSVC)
    target_compile_options(IgnisEditor PRIVATE /W4)
else()
    target_compile_options(IgnisEditor PRIVATE -Wall -Wextra)
endif()
