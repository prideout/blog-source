#include "Splat.h"

GLuint CreateCube()
{
    #define O -1.0f,
    #define X +1.0f,
    float positions[] = { O O O O O X O X O O X X X O O X O X X X O X X X };
    #undef O
    #undef X
    
    short indices[] = {
        7, 3, 1, 1, 5, 7, // Z+
        0, 2, 6, 6, 4, 0, // Z-
        6, 2, 3, 3, 7, 6, // Y+
        1, 0, 4, 4, 5, 1, // Y-
        3, 2, 0, 0, 1, 3, // X-
        4, 6, 7, 7, 5, 4, // X+
    };

    // Create the VAO:
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create the VBO for positions:
    {
        GLuint vbo;
        GLsizeiptr size = sizeof(positions);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, size, positions, GL_STATIC_DRAW);
    }

    // Create the VBO for indices:
    {
        GLuint vbo;
        GLsizeiptr size = sizeof(indices);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
    }

    // Set up the vertex layout:
    GLsizeiptr stride = 3 * sizeof(positions[0]);
    glEnableVertexAttribArray(PositionSlot);
    glVertexAttribPointer(PositionSlot, 3, GL_FLOAT, GL_FALSE, stride, 0);

    return vao;
}
