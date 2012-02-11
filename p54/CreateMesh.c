#include "Platform.h"
#include "Utility.h"
#include <openctm.h>
#include <string.h>
#include <malloc.h>

Mesh CreateQuad()
{
    Mesh mesh = {0};
    mesh.VertexCount = 6;

    float positions[] = {
        -1, -1, 0,
         1, -1, 0,
         1,  1, 0,
         1,  1, 0,
        -1,  1, 0,
        -1, -1, 0,
    };
    
    GLuint handle;
    GLsizeiptr size = mesh.VertexCount * sizeof(float) * 3;
    glGenBuffers(1, &handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferData(GL_ARRAY_BUFFER, size, positions, GL_STATIC_DRAW);
    mesh.Positions = handle;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return mesh;
}

Mesh CreateMesh(const char* ctmFile, bool computeAdjacency)
{
    Mesh mesh = {0};
    
    char qualifiedPath[256] = {0};
    strcpy(qualifiedPath, PezResourcePath());
    strcat(qualifiedPath, "/\0");
    strcat(qualifiedPath, ctmFile);
    
    // Open the CTM file:
    CTMcontext ctmContext = ctmNewContext(CTM_IMPORT);
    ctmLoad(ctmContext, qualifiedPath);
    PezCheckCondition(ctmGetError(ctmContext) == CTM_NONE, "OpenCTM issue with loading %s", qualifiedPath);
    CTMuint vertexCount = ctmGetInteger(ctmContext, CTM_VERTEX_COUNT);
    CTMuint faceCount = ctmGetInteger(ctmContext, CTM_TRIANGLE_COUNT);

    // Create the VBO for positions:
    const CTMfloat* positions = ctmGetFloatArray(ctmContext, CTM_VERTICES);
    if (positions) {
        GLuint handle;
        GLsizeiptr size = vertexCount * sizeof(float) * 3;
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ARRAY_BUFFER, handle);
        glBufferData(GL_ARRAY_BUFFER, size, positions, GL_STATIC_DRAW);
        mesh.Positions = handle;
    }
    
    // Create the VBO for normals:
    const CTMfloat* normals = ctmGetFloatArray(ctmContext, CTM_NORMALS);
    if (normals) {
        GLuint handle;
        GLsizeiptr size = vertexCount * sizeof(float) * 3;
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ARRAY_BUFFER, handle);
        glBufferData(GL_ARRAY_BUFFER, size, normals, GL_STATIC_DRAW);
        mesh.Normals = handle;
    }
    
    // Create the VBO for indices:
    const CTMuint* indices = ctmGetIntegerArray(ctmContext, CTM_INDICES);
    if (indices) {
        
        GLsizeiptr bufferSize = faceCount * 3 * sizeof(unsigned short);
        
        // Convert indices from 32-bit to 16-bit:
        PezCheckCondition(vertexCount < (1 << 16), "Too many indices to fit in 16 bits");
        unsigned short* faceBuffer = (unsigned short*) malloc(bufferSize);
        unsigned short* pDest = faceBuffer;
        const CTMuint* pSrc = indices;
        unsigned int remainingFaces = faceCount;
        while (remainingFaces--)
        {
            *pDest++ = (unsigned short) *pSrc++;
            *pDest++ = (unsigned short) *pSrc++;
            *pDest++ = (unsigned short) *pSrc++;
        }

        // Compute adjacency if desired:
        if (computeAdjacency)
        {
            bufferSize = faceCount * 6 * sizeof(unsigned short);
            unsigned short* destBuffer = (unsigned short*) malloc(bufferSize);
            ComputeAdjacency(destBuffer, faceBuffer, faceCount, vertexCount);
            free(faceBuffer);
            faceBuffer = destBuffer;
        }
        
        GLuint handle;
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, faceBuffer, GL_STATIC_DRAW);
        mesh.Faces = handle;
        
        free(faceBuffer);
    }
    
    ctmFreeContext(ctmContext);

    mesh.FaceCount = faceCount;
    mesh.VertexCount = vertexCount;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return mesh;
}
