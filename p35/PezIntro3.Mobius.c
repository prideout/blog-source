#include <pez.h>
#include <glew.h>
#include <glsw.h>
#include <math.h>
#include <../c/vectormath_aos.h>

struct PackedVertex {
    float Position[3];
    float Normal[3];
};

struct Globals {
    GLuint ModelviewHandle;
    GLuint NormalMatrixHandle;
    VmathMatrix4 Modelview;
    float PackedNormalMatrix[9];
    float Theta;
};

Globals RenderContext;

enum { PositionSlot, NormalSlot };

#define Slices 128
#define Stacks 64
#define VertexCount (Slices * Stacks)
#define IndexCount (VertexCount * 6)

static VmathVector3 EvaluateMobius(float s, float t);
static GLuint CreateVertexBuffer();
static GLuint CreateIndexBuffer();
static GLuint LoadEffect();

void PezHandleMouse(int x, int y, int action) { }

void PezUpdate(unsigned int elapsedMilliseconds)
{
    Globals* rc = &RenderContext;
    int i, j;

    VmathMatrix4 rotation;
    VmathMatrix4 translation;

    VmathVector3 xlate;
    vmathV3MakeFromElems(&xlate, 0, 0, -20);

    vmathM4MakeRotationX(&rotation, rc->Theta * Pi / 180.0f);
    vmathM4MakeTranslation(&translation, &xlate);
    vmathM4Mul(&rc->Modelview, &translation, &rotation);
    //vmathM4Transpose(&rc->Modelview, &rc->Modelview);

    for (i = 0; i < 3; ++i)
        for (j = 0; j < 3; ++j)
            rc->PackedNormalMatrix[i + j * 3] = vmathM4GetElem(&rc->Modelview, i, j);

    rc->Theta += elapsedMilliseconds * 0.05f;
}

const char* PezInitialize(int width, int height)
{
    CreateVertexBuffer();
    CreateIndexBuffer();
    GLuint programHandle = LoadEffect();

    Globals* rc = &RenderContext;

    float left = -10, right = 10;
    float bottom = -10, top = 10;
    float zNear = 3, zFar = 100;
    VmathMatrix4 projection;
    vmathM4MakeFrustum(&projection, left, right, bottom, top, zNear, zFar);
    //vmathM4Transpose(&projection, &projection);

    GLuint projectionHandle = glGetUniformLocation(programHandle, "Projection");
    glUniformMatrix4fv(projectionHandle, 1, 0, &projection.col0.x);

    rc->ModelviewHandle = glGetUniformLocation(programHandle, "Modelview");
    rc->NormalMatrixHandle = glGetUniformLocation(programHandle, "NormalMatrix");
    rc->Theta = 0;

    // vec3 DiffuseMaterial
    // vec3 LightPosition;
    // vec3 AmbientMaterial;
    // vec3 SpecularMaterial;
    // float Shininess;

    return "Pez Intro";
}

void PezRender()
{
    Globals* rc = &RenderContext;
    glClearColor(0, 0.25f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUniformMatrix4fv(rc->ModelviewHandle, 1, 0, &rc->Modelview.col0.x);
    glUniformMatrix3fv(rc->NormalMatrixHandle, 1, 0, &rc->PackedNormalMatrix[0]);
    glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, 0);
}

static GLuint CreateIndexBuffer()
{
    GLuint inds[IndexCount];
    GLuint* pIndex = &inds[0];

    GLuint n = 0;
    for (GLuint i = 0; i < Slices; i++) {
        for (GLuint j = 0; j < Stacks; j++) {
            *pIndex++ = (n + j) % VertexCount;
            *pIndex++ = (n + (j + 1) % Stacks) % VertexCount;
            *pIndex++ = (n + j + Stacks) % VertexCount;
            *pIndex++ = (n + j + Stacks) % VertexCount;
            *pIndex++ = (n + (j + 1) % Stacks) % VertexCount;;
            *pIndex++ = (n + (j + 1) % Stacks + Stacks) % VertexCount;
        }
        n += Stacks;
    }

    PezCheckCondition(n == VertexCount, "Tessellation error.");
    PezCheckCondition(pIndex - &inds[0] == IndexCount, "Tessellation error.");

    GLuint handle;
    GLsizeiptr size = sizeof(inds);
    const GLvoid* data = &inds[0];
    GLenum usage = GL_STATIC_DRAW;

    glGenBuffers(1, &handle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);

    return handle;
}

static GLuint CreateVertexBuffer()
{
    PackedVertex verts[VertexCount];
    PackedVertex* pVert = &verts[0];

    float ds = 1.0f / (Slices - 1);
    float dt = 1.0f / Stacks;

    for (float s = 0; s < 1 + ds / 2; s += ds) {
        for (float t = 0; t < 1 - dt / 2; t += dt) {
            const float E = 0.01f;
            
            // Find three points on the surface:
            VmathVector3 p = EvaluateMobius(s, t);
            VmathVector3 u = EvaluateMobius(s + E, t);
            VmathVector3 v = EvaluateMobius(s, t + E);

            // Find tangent vectors:
            vmathV3Sub(&u, &p, &u);
            vmathV3Sub(&v, &p, &v);

            // Compute normal vectors:
            VmathVector3 n;
            vmathV3Cross(&n, &u, &v);
            vmathV3Normalize(&n, &n);

            // Push the vertex:
            PackedVertex pod = {p.x, p.y, p.z, n.x, n.y, n.z};
            *pVert++ = pod;
        }
    }

    PezCheckCondition(pVert - &verts[0] == VertexCount, "Tessellation error.");

    GLuint handle;
    const GLvoid* data = &verts[0];
    GLsizeiptr size = sizeof(verts);
    GLenum usage = GL_STATIC_DRAW;

    glGenBuffers(1, &handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);

    return handle;
}

static VmathVector3 EvaluateMobius(float s, float t)
{
    s *= TwoPi; t *= TwoPi;
    const float major = 1.25f, a = 0.125f, b = 0.5f, scale = 0.5f;
    float u = a * cos(t) * cos(s/2) - b * sin(t) * sin(s/2);
    float v = a * cos(t) * sin(s/2) + b * sin(t) * cos(s/2);
    float x = (major + u) * cos(s) * scale;
    float y = (major + u) * sin(s) * scale;
    float z = v * scale;
    VmathVector3 result;
    vmathV3MakeFromElems(&result, x, y, x);
    return result;
}

static GLuint LoadEffect()
{
    glswInit();
    glswSetPath("../../", ".glsl");
    glswAddDirectiveToken("GL3", "#version 130");

    const char* vsSource = glswGetShader("PixelLighting.Vertex." PEZ_GL_VERSION_TOKEN);
    const char* fsSource = glswGetShader("PixelLighting.Fragment." PEZ_GL_VERSION_TOKEN);
    GLuint vsHandle = glCreateShader(GL_VERTEX_SHADER);
    GLuint fsHandle = glCreateShader(GL_FRAGMENT_SHADER);
    
    GLint compileSuccess;
    GLchar compilerSpew[256];

    glShaderSource(vsHandle, 1, &vsSource, 0);
    glCompileShader(vsHandle);
    glGetShaderiv(vsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(vsHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, compilerSpew);

    glShaderSource(fsHandle, 1, &fsSource, 0);
    glCompileShader(fsHandle);
    glGetShaderiv(fsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(fsHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, compilerSpew);

    GLint linkSuccess;
    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vsHandle);
    glAttachShader(programHandle, fsHandle);
    glBindAttribLocation(programHandle, PositionSlot, "Position");
    glBindAttribLocation(programHandle, NormalSlot, "Normal");
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    glGetProgramInfoLog(programHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(linkSuccess, compilerSpew);

    glUseProgram(programHandle);
    return programHandle;
}
