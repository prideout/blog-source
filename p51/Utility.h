#include "Platform.h"

typedef struct MeshRec
{
    GLuint Positions;
    GLuint Normals;
    GLuint Faces;
    GLsizei FaceCount;
} Mesh;

Mesh CreateMesh(const char* ctmFile);
GLuint CreateQuad(float left, float bottom, float right, float top);
GLuint CreateProgram(const char* vsKey, const char* fsKey);
