#include "Platform.h"
#ifndef __cplusplus
#include <stdbool.h>
#endif

typedef struct MeshRec
{
    GLuint Positions;
    GLuint Normals;
    GLuint Faces;
    GLsizei FaceCount;
    GLsizei VertexCount;
} Mesh;

Mesh CreateMesh(const char* ctmFile, bool computeAdjacency);
Mesh CreateQuad();
GLuint CreateProgram(const char* vsKey, const char* gsKey, const char* fsKey);
void ComputeAdjacency(unsigned short* dest, const unsigned short* source, int faceCount, int vertCount);
