# ══ Sandbox — il secondo client. ══
#
# Non è un vezzo: è il test che l'engine sia davvero linkabile dall'esterno.
# Con un target solo, un accoppiamento sbagliato resta invisibile perché tanto
# compila lo stesso. Con due client sulla stessa libreria, esce subito.

file(GLOB_RECURSE SANDBOX_FILES CONFIGURE_DEPENDS "Private/*.cpp" "Private/*.h")

add_executable(Sandbox ${SANDBOX_FILES})
target_include_directories(Sandbox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Private)
target_link_libraries(Sandbox PRIVATE Ignis)

if(MSVC)
    target_compile_options(Sandbox PRIVATE /W4)
else()
    target_compile_options(Sandbox PRIVATE -Wall -Wextra)
endif()
