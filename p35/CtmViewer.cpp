// CtmViewer.cpp by Philip Rideout in May 2010.
// Covered by the MIT License.

#include <pez.h>
#include <vectormath_aos.h>
#include <glsw.h>
#include <glew.h>

#if defined(__APPLE__)
#define VERSION_TOKEN "GL2"
#else
#define VERSION_TOKEN "GL3"
#endif

using namespace Vectormath::Aos;

static const int Slices = 128;
static const int Stacks = 64;
static const int VertexCount = Slices * Stacks;
static const int IndexCount = VertexCount * 6;

struct PackedVertex
{
    float Position[3];
    float Normal[3];
};

namespace VertexAttributes
{
    static GLuint Position = 0;
    static GLuint Normal = 1;
    static const GLvoid* NormalOffset = (GLvoid*) (3 * sizeof(float));
};

struct ShaderUniforms
{
    GLuint Projection;
    GLuint Modelview;
    GLuint NormalMatrix;
    GLuint LightPosition;
    GLuint AmbientMaterial;
    GLuint DiffuseMaterial;
    GLuint SpecularMaterial;
    GLuint Shininess;
};

struct RenderContext
{
    GLuint VertexBuffer;
    GLuint IndexBuffer;
    GLuint ToonHandle;
    ShaderUniforms ToonUniforms;
    Matrix4 Projection;
    Matrix4 Modelview;
    Matrix3 NormalMatrix;
    float PackedNormalMatrix[9];
    float Theta;
};

static RenderContext GlobalRenderContext;
static Point3 EvaluateMobius(float s, float t);
static GLuint CreateVertexBuffer();
static GLuint CreateIndexBuffer();
static GLuint BuildShader(const char* source, GLenum shaderType);
static GLuint BuildProgram(const char* vsKey, const char* fsKey);

//   _____     _____    ______   
//  (  __ \   / ___/   (____  )  
//   ) )_) ) ( (__         / /   
//  (  ___/   ) __)    ___/ /_   
//   ) )     ( (      /__  ___)  
//  ( (       \ \___    / /____  
//  /__\       \____\  (_______) 
//

const char* pez_initialize(int width, int height)
{
    // Shader keys:
    const char* vsKey = "PixelLighting.Vertex." VERSION_TOKEN;
    const char* fsKey = "PixelLighting.Fragment." VERSION_TOKEN;

    // Frustum parameters:
    const float x = 0.25f;
    const float y = x * height / width;
    const float left = -x;
    const float right = x;
    const float bottom = -y;
    const float top = y;
    const float zNear = 4;
    const float zFar = 100;

    // Set up the shader wrangler:
    glswInit();
    glswSetPath("../../", ".glsl");
    glswAddDirectiveToken("GL3", "#version 130");

    // Create various OpenGL objects:
    RenderContext& rc = GlobalRenderContext;
    rc.VertexBuffer = CreateVertexBuffer();
    rc.IndexBuffer = CreateIndexBuffer();
    rc.ToonHandle = BuildProgram(vsKey, fsKey);
    rc.ToonUniforms.Projection = glGetUniformLocation(rc.ToonHandle, "Projection");
    rc.ToonUniforms.Modelview = glGetUniformLocation(rc.ToonHandle, "Modelview");
    rc.ToonUniforms.NormalMatrix = glGetUniformLocation(rc.ToonHandle, "NormalMatrix");
    rc.ToonUniforms.LightPosition = glGetUniformLocation(rc.ToonHandle, "LightPosition");
    rc.ToonUniforms.AmbientMaterial = glGetUniformLocation(rc.ToonHandle, "AmbientMaterial");
    rc.ToonUniforms.DiffuseMaterial = glGetUniformLocation(rc.ToonHandle, "DiffuseMaterial");
    rc.ToonUniforms.SpecularMaterial = glGetUniformLocation(rc.ToonHandle, "SpecularMaterial");
    rc.ToonUniforms.Shininess = glGetUniformLocation(rc.ToonHandle, "Shininess");
    rc.Projection = Matrix4::frustum(left, right, bottom, top, zNear, zFar);
    rc.Theta = 0;

    glEnable(GL_DEPTH_TEST);

    return "OpenCTM Viewer";
}

void pez_render()
{
    RenderContext& rc = GlobalRenderContext;
    Vector4 lightPosition(0.25, 0.25, 1, 0);

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(rc.ToonHandle);
    glUniform3f(rc.ToonUniforms.DiffuseMaterial, 0, 0.75, 0.75);
    glUniform3f(rc.ToonUniforms.AmbientMaterial, 0.04f, 0.04f, 0.04f);
    glUniform3f(rc.ToonUniforms.SpecularMaterial, 0.5, 0.5, 0.5);
    glUniform1f(rc.ToonUniforms.Shininess, 50);
    glUniform3fv(rc.ToonUniforms.LightPosition, 1, &lightPosition[0]);
    glUniformMatrix4fv(rc.ToonUniforms.Projection, 1, 0, &rc.Projection[0][0]);
    glUniformMatrix4fv(rc.ToonUniforms.Modelview, 1, 0, &rc.Modelview[0][0]);
    glUniformMatrix3fv(rc.ToonUniforms.NormalMatrix, 1, 0, &rc.PackedNormalMatrix[0]);
    glBindBuffer(GL_ARRAY_BUFFER, rc.VertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rc.IndexBuffer);
    glEnableVertexAttribArray(VertexAttributes::Position);
    glEnableVertexAttribArray(VertexAttributes::Normal);
    glVertexAttribPointer(VertexAttributes::Position, 3, GL_FLOAT, GL_FALSE, sizeof(PackedVertex), 0);
    glVertexAttribPointer(VertexAttributes::Normal, 3, GL_FLOAT, GL_FALSE, sizeof(PackedVertex), VertexAttributes::NormalOffset);
    glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, 0);
}

void pez_update(unsigned int elapsedMilliseconds)
{
    RenderContext& rc = GlobalRenderContext;
    Matrix4 rotation, translation;

    rc.Theta += elapsedMilliseconds * 0.05f;
    rotation = Matrix4::rotationX(rc.Theta * Pi / 180.0f);
    translation = Matrix4::translation(Vector3(0, 0, -20));
    rc.Modelview = translation * rotation;
    rc.NormalMatrix = rc.Modelview.getUpper3x3();

    for (int i = 0; i < 9; ++i)
    {
        rc.PackedNormalMatrix[i] = rc.NormalMatrix[i / 3][i % 3];
    }
}

void pez_handle_mouse(int x, int y, int action)
{
}

//   _____    ______      _____   __    __     ____     ________    _____  
//  (  __ \  (   __ \    (_   _)  ) )  ( (    (    )   (___  ___)  / ___/  
//   ) )_) )  ) (__) )     | |   ( (    ) )   / /\ \       ) )    ( (__    
//  (  ___/  (    __/      | |    \ \  / /   ( (__) )     ( (      ) __)   
//   ) )      ) \ \  _     | |     \ \/ /     )    (       ) )    ( (      
//  ( (      ( ( \ \_))   _| |__    \  /     /  /\  \     ( (      \ \___  
//  /__\      )_) \__/   /_____(     \/     /__(  )__\    /__\      \____\ 
//

static GLuint CreateIndexBuffer()
{
    GLuint inds[IndexCount];
    GLuint* pIndex = &inds[0];

    GLuint n = 0;
    for (GLuint i = 0; i < Slices; i++)
    {
        for (GLuint j = 0; j < Stacks; j++)
        {
            *pIndex++ = (n + j) % VertexCount;
            *pIndex++ = (n + (j + 1) % Stacks) % VertexCount;
            *pIndex++ = (n + j + Stacks) % VertexCount;
            *pIndex++ = (n + j + Stacks) % VertexCount;
            *pIndex++ = (n + (j + 1) % Stacks) % VertexCount;;
            *pIndex++ = (n + (j + 1) % Stacks + Stacks) % VertexCount;
        }
        n += Stacks;
    }

    CheckCondition(n == VertexCount, "Tessellation error.");
    CheckCondition(pIndex - &inds[0] == IndexCount, "Tessellation error.");

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
    RenderContext& rc = GlobalRenderContext;
    PackedVertex verts[VertexCount];
    PackedVertex* pVert = &verts[0];

    float ds = 1.0f / (Slices - 1);
    float dt = 1.0f / Stacks;

    // The upper bounds in these loops are tweaked to reduce the
    // chance of precision error causing an incorrect # of iterations.

    for (float s = 0; s < 1 + ds / 2; s += ds)
    {
        for (float t = 0; t < 1 - dt / 2; t += dt)
        {
            const float E = 0.01f;
            Point3 p = EvaluateMobius(s, t);
            Vector3 u = EvaluateMobius(s + E, t) - p;
            Vector3 v = EvaluateMobius(s, t + E) - p;
            Vector3 n = normalize(cross(u, v));
            PackedVertex pod = {p[0], p[1], p[2], n[0], n[1], n[2]};
            *pVert++ = pod;
        }
    }

    CheckCondition(pVert - &verts[0] == VertexCount, "Tessellation error.");

    GLuint handle;
    const GLvoid* data = &verts[0];
    GLsizeiptr size = sizeof(verts);
    GLenum usage = GL_DYNAMIC_DRAW;

    glGenBuffers(1, &handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);

    return handle;
}

static Point3 EvaluateMobius(float s, float t)
{
    s *= TwoPi; t *= TwoPi;
    const float major = 1.25f, a = 0.125f, b = 0.5f, scale = 0.5f;
    float u = a * cos(t) * cos(s/2) - b * sin(t) * sin(s/2);
    float v = a * cos(t) * sin(s/2) + b * sin(t) * cos(s/2);
    float x = (major + u) * cos(s) * scale;
    float y = (major + u) * sin(s) * scale;
    float z = v * scale;
    return Point3(x, y, z);
}

static GLuint BuildShader(const char* source, GLenum shaderType)
{
    GLint compileSuccess;
    GLuint shaderHandle;
    GLchar messages[256];

    shaderHandle = glCreateShader(shaderType);
    glShaderSource(shaderHandle, 1, &source, 0);
    glCompileShader(shaderHandle);
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);

    CheckCondition(compileSuccess, messages);
    return shaderHandle;
}

static GLuint BuildProgram(const char* vsKey, const char* fsKey)
{
    DebugString("Compiling '%s'...\n", vsKey);
    const char* vsString = glswGetShader(vsKey);
    if (!vsString)
    {
        FatalError("%s\n", glswGetError());
    }
    GLuint vsHandle = BuildShader(vsString, GL_VERTEX_SHADER);

    DebugString("Compiling '%s'...\n", fsKey);
    const char* fsString = glswGetShader(fsKey);
    if (!fsString)
    {
        FatalError("%s\n", glswGetError());
    }
    GLuint fsHandle = BuildShader(fsString, GL_FRAGMENT_SHADER);

    DebugString("Linking...\n");

    GLint linkSuccess;
    GLchar messages[256];
    GLuint programHandle;

    programHandle = glCreateProgram();
    glAttachShader(programHandle, vsHandle);
    glAttachShader(programHandle, fsHandle);
    glBindAttribLocation(programHandle, VertexAttributes::Position, "Position");
    glBindAttribLocation(programHandle, VertexAttributes::Normal, "Normal");
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);

    CheckCondition(linkSuccess, messages);
    return programHandle;
}
