#include "Splat.h"

using namespace std;
using namespace VectorMath;

GLuint CreatePoints()
{
    const int size = GridDensity;
    vector<float> pointGrid(size * size * size * 3);
    vector<float>::iterator pPoint = pointGrid.begin();
    for (int z = 0; z < size; ++z) {
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                *pPoint++ = -1 + 2 * float(x) / (size - 1);
                *pPoint++ = -1 + 2 * float(y) / (size - 1);
                *pPoint++ = -1 + 2 * float(z) / (size - 1);
            }
        }
    }
   
    // Create the VAO:
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create the VBO:
    GLuint vbo;
    GLsizeiptr byteCount = pointGrid.size() * sizeof(float);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, byteCount, &pointGrid[0], GL_STATIC_DRAW);

    // Set up the vertex layout:
    GLsizeiptr stride = 3 * sizeof(float);
    glEnableVertexAttribArray(PositionSlot);
    glVertexAttribPointer(PositionSlot, 3, GL_FLOAT, GL_FALSE, stride, 0);

    return vao;
}
