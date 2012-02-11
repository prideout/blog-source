#include "Common.hpp"

GLuint CreateQuad()
{
    short positions[] = {
        -1, -1,
         1, -1,
        -1,  1,
         1,  1,
    };
    
    GLuint vbo;
    GLsizeiptr size = sizeof(positions);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, positions, GL_STATIC_DRAW);

    return vbo;
}

void RenderQuad(GLuint vbo)
{
    glEnableVertexAttribArray(PositionSlot);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(PositionSlot, 2, GL_SHORT, GL_FALSE, 2*sizeof(short), 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(PositionSlot);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
