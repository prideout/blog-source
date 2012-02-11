#include "Utility.h"
#include <openctm.h>
#include <string.h>
#include <stdlib.h>
#include <vectormath.h>

Mesh CreateMesh(const char* ctmFile, float totalScale, float lengthScale)
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
    
        // Find bounding box
        float m = 99999.0f;
        Point3 minCorner = P3MakeFromElems(m, m, m);
        Point3 maxCorner = P3MakeFromElems(-m, -m, -m);
        const CTMfloat* pSrc = positions;
        CTMuint remainingVerts = vertexCount;
        while (remainingVerts--)
        {
            float x = *pSrc++;
            float y = *pSrc++;
            float z = *pSrc++;
            Point3 p = P3MakeFromElems(x, y, z);
            minCorner = P3MinPerElem(p, minCorner);
            maxCorner = P3MaxPerElem(p, maxCorner);
        }
		
        // Scale such that the Z extent is 'scale'
        // The X and Y scales are computed according to the aspect ratio.
        // The model is centered at (+0.5, +0.5, +0.5).
        float xratio = (maxCorner.x - minCorner.x) / (maxCorner.z - minCorner.z);
        float yratio = (maxCorner.y - minCorner.y) / (maxCorner.z - minCorner.z);

        float sx = lengthScale * totalScale * xratio / (maxCorner.x - minCorner.x);
        float sy = totalScale * yratio / (maxCorner.y - minCorner.y);
        float sz = totalScale / (maxCorner.z - minCorner.z);
        pSrc = positions;
        remainingVerts = vertexCount;
        CTMfloat* pDest = (CTMfloat*) positions;
        while (remainingVerts--)
        {
            float x = *pSrc++;
            float y = *pSrc++;
            float z = *pSrc++;
            *pDest++ = (x - minCorner.x) * sx - totalScale * xratio / 2;
            *pDest++ = (y - minCorner.y) * sy - totalScale * yratio / 2;
            *pDest++ = (z - minCorner.z) * sz - totalScale / 2;
        }

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
        
        GLsizeiptr bufferSize = faceCount * 3 * sizeof(GLuint);
        
        GLuint handle;
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, indices, GL_STATIC_DRAW);
        mesh.Faces = handle;
    }
    
    ctmFreeContext(ctmContext);

    mesh.FaceCount = faceCount;
    mesh.VertexCount = vertexCount;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return mesh;
}
