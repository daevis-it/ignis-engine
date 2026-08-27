#include "Ignis/Renderer/RenderCommand.h"

#include <glad/glad.h>

namespace Ignis
{
    void RenderCommand::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        // I cast sono qui e non nel chiamante: sono il punto in cui il vocabolario
        // dell'engine (uint32_t) diventa quello di OpenGL (GLint/GLsizei). Se un
        // giorno sotto ci fosse un'altra API, cambia solo questa riga.
        glViewport(static_cast<GLint>(x), static_cast<GLint>(y),
                   static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    }

    void RenderCommand::SetClearColor(const glm::vec4& colore)
    {
        glClearColor(colore.r, colore.g, colore.b, colore.a);
    }

    void RenderCommand::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }
}
