#include <pez.h>

typedef struct MeshRec
{
    GLuint Positions;
    GLuint Normals;
    GLuint Faces;
    GLsizei FaceCount;
    GLsizei VertexCount;
} Mesh;

typedef struct TextureRec
{
    GLuint Handle;
    GLsizei Width;
    GLsizei Height;
} Texture;

Mesh CreateMesh(const char* ctmFile, float totalScale, float lengthScale);
Mesh CreateCylinder(float height, float radius, int stacks, int slices);
GLuint CreateProgram(const char* vsKey, const char* gsKey, const char* fsKey);
Texture CreatePathTexture();
