#include "Platform.hpp"
#include "Matrix.hpp"
#include "glsw.h"

// PRIVATE DECLARATIONS:

static const int Slices = 512;
static const int Stacks = 256;
static const int VertexCount = Slices * Stacks;
static const int IndexCount = VertexCount * 6;

struct Vertex
{
    vec4 Position;
    vec3 Normal;
};

namespace VertexAttributes
{
    static GLuint Position = 0;
    static GLuint Normal = 1;
    static const GLvoid* NormalOffset = (GLvoid*) sizeof(vec4);
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
    mat4 Projection;
    mat4 Modelview;
    mat3 NormalMatrix;
};

static RenderContext GlobalRenderContext;

static void UpdateVertexBuffer();
static vec3 EvaluateMobius(float s, float t);
static GLuint CreateVertexBuffer();
static GLuint CreateIndexBuffer();
static GLuint BuildShader(const char* source, GLenum shaderType);
static GLuint BuildProgram(const char* vsKey, const char* fsKey);

// PUBLIC FUNCTIONS:

const char* GetWindowName()
{
    return "Mobius";
}

void Initialize()
{
    RenderContext& rc = GlobalRenderContext;

    glswInit();
    glswSetPath("../", ".glsl");
    glswAddDirectiveToken("GL3", "#version 130");

    rc.VertexBuffer = CreateVertexBuffer();
    rc.IndexBuffer = CreateIndexBuffer();
    
#if defined(__APPLE__)
    rc.ToonHandle = BuildProgram("Toon.Vertex.GL2", "Toon.Fragment.GL2");
#else
    rc.ToonHandle = BuildProgram("Toon.Vertex.GL3", "Toon.Fragment.GL3");
#endif
    
    rc.ToonUniforms.Projection = glGetUniformLocation(rc.ToonHandle, "Projection");
    rc.ToonUniforms.Modelview = glGetUniformLocation(rc.ToonHandle, "Modelview");
    rc.ToonUniforms.NormalMatrix = glGetUniformLocation(rc.ToonHandle, "NormalMatrix");
    rc.ToonUniforms.LightPosition = glGetUniformLocation(rc.ToonHandle, "LightPosition");
    rc.ToonUniforms.AmbientMaterial = glGetUniformLocation(rc.ToonHandle, "AmbientMaterial");
    rc.ToonUniforms.DiffuseMaterial = glGetUniformLocation(rc.ToonHandle, "DiffuseMaterial");
    rc.ToonUniforms.SpecularMaterial = glGetUniformLocation(rc.ToonHandle, "SpecularMaterial");
    rc.ToonUniforms.Shininess = glGetUniformLocation(rc.ToonHandle, "Shininess");

    glEnable(GL_DEPTH_TEST);

    const float S = 0.46f;
    const float H = S * ViewportHeight / ViewportWidth;
    rc.Projection = mat4::Frustum(-S, S, -H, H, 4, 10);
}

void Render()
{
    RenderContext& rc = GlobalRenderContext;

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(rc.ToonHandle);

    glUniform3f(rc.ToonUniforms.DiffuseMaterial, 0, 0.75, 0.75);
    glUniform3f(rc.ToonUniforms.AmbientMaterial, 0.04f, 0.04f, 0.04f);
    glUniform3f(rc.ToonUniforms.SpecularMaterial, 0.5, 0.5, 0.5);
    glUniform1f(rc.ToonUniforms.Shininess, 50);
    
    vec4 lightPosition(0.25, 0.25, 1, 0);
    glUniform3fv(rc.ToonUniforms.LightPosition, 1, lightPosition.Pointer());
    
    glUniformMatrix4fv(rc.ToonUniforms.Projection, 1, 0, rc.Projection.Pointer());
    glUniformMatrix4fv(rc.ToonUniforms.Modelview, 1, 0, rc.Modelview.Pointer());
    glUniformMatrix3fv(rc.ToonUniforms.NormalMatrix, 1, 0, rc.NormalMatrix.Pointer());
    
    glBindBuffer(GL_ARRAY_BUFFER, rc.VertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rc.IndexBuffer);
    
    glEnableVertexAttribArray(VertexAttributes::Position);
    glEnableVertexAttribArray(VertexAttributes::Normal);
    
    glVertexAttribPointer(VertexAttributes::Position, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(VertexAttributes::Normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), VertexAttributes::NormalOffset);

    glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, 0);
}

void Update(unsigned int elapsedMilliseconds)
{
    RenderContext& rc = GlobalRenderContext;

    static float theta = 0;
    static float time = 0;
    const float InitialPause = 0;
    const bool LoopForever = true;
    time += elapsedMilliseconds;
    if (time > InitialPause && (LoopForever || theta < 360))
    {
        theta += elapsedMilliseconds * 0.1f;
    }

    mat4 rotation = mat4::Rotate(theta, vec3(0, 1, 0));
    mat4 translation = mat4::Translate(0, 0, -7);

    rc.Modelview = rotation * translation;
    rc.NormalMatrix = rc.Modelview.ToMat3();
}

// PRIVATE FUNCTIONS:

static GLuint CreateIndexBuffer()
{
    GLuint inds[IndexCount];
    GLuint* pIndex = &inds[0];

    GLuint n = 0;
    for (GLuint i = 0; i < Slices; i++)
    {
        for (GLuint j = 0; j < Stacks; j++)
        {
            *pIndex++ = n + j;
            *pIndex++ = n + (j + 1) % Stacks;
            *pIndex++ = n + j + Stacks;

            *pIndex++ = n + j + Stacks;
            *pIndex++ = n + (j + 1) % Stacks;
            *pIndex++ = n + (j + 1) % Stacks + Stacks;
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
    Vertex verts[VertexCount];
    Vertex* pVert = &verts[0];

    float ds = 1.0f / (Slices - 1);
    float dt = 1.0f / Stacks;

    // The upper bounds in these loops are tweaked to reduce the
    // chance of precision error causing an incorrect # of iterations.

    for (float s = 0; s < 1 + ds / 2; s += ds)
    {
        for (float t = 0; t < 1 - dt / 2; t += dt)
        {
            const float E = 0.01f;
            vec3 p = EvaluateMobius(s, t);
            vec3 u = EvaluateMobius(s + E, t) - p;
            vec3 v = EvaluateMobius(s, t + E) - p;
            vec3 n = u.Cross(v).Normalized();

            pVert->Position = vec4(p, 1.0f);
            pVert->Normal = n;
            ++pVert;
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

static vec3 EvaluateMobius(float s, float t)
{
    float u = s * TwoPi, v = t * TwoPi;
    float major = 1.25f, a = 0.125f, b = 0.5f;
    float phi = u / 2;
    float scale = 0.5f;
    float x = a * cos(v) * cos(phi) - b * sin(v) * sin(phi);
    float y = a * cos(v) * sin(phi) + b * sin(v) * cos(phi);

    vec3 range;
    range.x = (major + x) * cos(u);
    range.y = (major + x) * sin(u);
    range.z = y;
    return range * scale;
}

static GLuint BuildShader(const char* source, GLenum shaderType)
{
    GLuint shaderHandle = glCreateShader(shaderType);
    glShaderSource(shaderHandle, 1, &source, 0);
    glCompileShader(shaderHandle);

    GLint compileSuccess;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);

    if (compileSuccess == GL_FALSE)
    {
        GLchar messages[256];
        glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
        FatalError(messages);
    }

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
    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vsHandle);
    glAttachShader(programHandle, fsHandle);
    glBindAttribLocation(programHandle, VertexAttributes::Position, "Position");
    glBindAttribLocation(programHandle, VertexAttributes::Normal, "Normal");
    glLinkProgram(programHandle);

    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE)
    {
        GLchar messages[256];
        glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        FatalError(messages);
    }
    else
    {
        GLchar messages[256];
        glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        DebugString(messages);
    }

    return programHandle;
}
