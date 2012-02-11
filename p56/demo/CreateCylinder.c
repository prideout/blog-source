#include <pez.h>
#include <stdlib.h>
#include <math.h>
#include "Utility.h"

// Computes a cylinder whose spine aligns with the X axis
// Y and Z diameters are 1.0
// The entire cylinder is centered at (+0.5, +0.5, +0.5).
Mesh CreateCylinder(float height, float radius, int stacks, int slices)
{
    // Compute a 2D circle for the sweep shape:
    float* circle = (float*) malloc(sizeof(float) * slices * 2);
    {
        float theta = 0;
        const float dtheta = 2 * Pi / (float) slices;
        float* pDest = circle;
        for (int slice = 0; slice < slices; ++slice, theta += dtheta) {
            *pDest++ = cos(theta);
            *pDest++ = sin(theta);
        }
    }

    // Compute the cylinder vertex normals:
    int vertCount = slices * stacks;
    float* tubeNormals = (float*) malloc(sizeof(float) * vertCount * 3);
    {
        float* pDest = tubeNormals;
        for (int stack = 0; stack < stacks; ++stack) {
            const float* pSrc = circle;
            for (int slice = 0; slice < slices; ++slice) {
                *pDest++ = 0;
                *pDest++ = *pSrc++;
                *pDest++ = *pSrc++;
            }
        }
    }
    
    // Scale the circle:
    {
        float* pDest = circle;
        for (int slice = 0; slice < slices; ++slice) {
            float x = *pDest; *pDest = radius * x; ++pDest;
            float y = *pDest; *pDest = radius * y; ++pDest;
        }
    }

    // Compute the cylinder vertex positions:
    float* tubeVerts = (float*) malloc(sizeof(float) * vertCount * 3);
    {
        float x = 0.5 - height / 2;
        float dx = height / (float) (stacks - 1);
        float* pDest = tubeVerts;
        for (int stack = 0; stack < stacks; ++stack, x += dx) {
            const float* pSrc = circle;
            for (int slice = 0; slice < slices; ++slice) {
                *pDest++ = x;
                *pDest++ = *pSrc++;
                *pDest++ = *pSrc++;
            }
        }
    }

    free(circle);
    
    // Compute triangle connectivity:
    int faceCount = slices * (stacks - 1) * 2;
    int* tubeFaces = (int*) malloc(sizeof(int) * faceCount * 3);
    {
        int* pDest = tubeFaces;
        for (int stack = 0; stack < stacks - 1; ++stack) {
            int n = stack * slices;
            for (int slice = 0; slice < slices; ++slice) {
                int a = slice;
                int b = slice + slices;
                int c = ((slice + 1) % slices) + slices;
                int d = ((slice + 1) % slices);
                *pDest++ = n+c;
                *pDest++ = n+b;
                *pDest++ = n+a;
                *pDest++ = n+a;
                *pDest++ = n+d;
                *pDest++ = n+c;
            }
        }
    }
    
    // Create OpenGL VBOs:

    Mesh m;
    m.FaceCount = faceCount;
    m.VertexCount = vertCount;
    
    glGenBuffers(1, &m.Positions);
    glBindBuffer(GL_ARRAY_BUFFER, m.Positions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertCount * 3, tubeVerts, GL_STATIC_DRAW);

    glGenBuffers(1, &m.Normals);
    glBindBuffer(GL_ARRAY_BUFFER, m.Normals);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertCount * 3, tubeNormals, GL_STATIC_DRAW);

    glGenBuffers(1, &m.Faces);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.Faces);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * faceCount * 3, tubeFaces, GL_STATIC_DRAW);

    free(tubeVerts);
    free(tubeNormals);
    free(tubeFaces);

    return m;
}
