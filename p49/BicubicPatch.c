#include <pez.h>
#include <glew.h>
#include <glsw.h>
#include <vectormath.h>
#include "gumbo.h"

static void CreateGumby();
static void LoadEffect();

typedef struct {
    GLuint Projection;
    GLuint Modelview;
    GLuint NormalMatrix;
    GLuint LightPosition;
    GLuint AmbientMaterial;
    GLuint DiffuseMaterial;
    GLuint SpecularMaterial;
    GLuint Shininess;
    GLuint PatchMatrix;
    GLuint TransposedPatchMatrix;
    GLuint TessLevelInner;
    GLuint TessLevelOuter;
} ShaderUniforms;

static GLsizei VertCount;
static const GLuint PositionSlot = 0;
static GLuint ProgramHandle;
static Matrix4 ProjectionMatrix;
static Matrix4 ModelviewMatrix;
static Matrix3 NormalMatrix;
static Point3 CenterPoint;
static ShaderUniforms Uniforms;
static float TessLevelInner;
static float TessLevelOuter;

void PezRender(GLuint fbo)
{
    // Update dynamic uniforms:
    glUniform1f(Uniforms.TessLevelInner, TessLevelInner);
    glUniform1f(Uniforms.TessLevelOuter, TessLevelOuter);
    glUniformMatrix4fv(Uniforms.Modelview, 1, 0, &ModelviewMatrix.col0.x);

    Matrix3 nm = M3Transpose(NormalMatrix);
    float packed[9] = { nm.col0.x, nm.col1.x, nm.col2.x,
                        nm.col0.y, nm.col1.y, nm.col2.y,
                        nm.col0.z, nm.col1.z, nm.col2.z };
    glUniformMatrix3fv(Uniforms.NormalMatrix, 1, 0, &packed[0]);

    // Render the scene:
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPatchParameteri(GL_PATCH_VERTICES, 16);
    glDrawArrays(GL_PATCHES, 0, VertCount);
}

const char* PezInitialize(int width, int height)
{
    TessLevelInner = 6;
    TessLevelOuter = 6;

    CreateGumby();
    LoadEffect();

    Uniforms.Projection = glGetUniformLocation(ProgramHandle, "Projection");
    Uniforms.Modelview = glGetUniformLocation(ProgramHandle, "Modelview");
    Uniforms.NormalMatrix = glGetUniformLocation(ProgramHandle, "NormalMatrix");
    Uniforms.LightPosition = glGetUniformLocation(ProgramHandle, "LightPosition");
    Uniforms.AmbientMaterial = glGetUniformLocation(ProgramHandle, "AmbientMaterial");
    Uniforms.DiffuseMaterial = glGetUniformLocation(ProgramHandle, "DiffuseMaterial");
    Uniforms.SpecularMaterial = glGetUniformLocation(ProgramHandle, "SpecularMaterial");
    Uniforms.Shininess = glGetUniformLocation(ProgramHandle, "Shininess");
    Uniforms.TessLevelInner = glGetUniformLocation(ProgramHandle, "TessLevelInner");
    Uniforms.TessLevelOuter = glGetUniformLocation(ProgramHandle, "TessLevelOuter");
    Uniforms.PatchMatrix = glGetUniformLocation(ProgramHandle, "B");
    Uniforms.TransposedPatchMatrix = glGetUniformLocation(ProgramHandle, "BT");

    // Set up the projection matrix:
    const float HalfWidth = 1.5f;
    const float HalfHeight = HalfWidth * PEZ_VIEWPORT_HEIGHT / PEZ_VIEWPORT_WIDTH;
    ProjectionMatrix = M4MakeFrustum(-HalfWidth, +HalfWidth, -HalfHeight, +HalfHeight, 5, 150);

    // Initialize various uniforms:
    glUniform3f(Uniforms.DiffuseMaterial, 0, 0.75, 0.75);
    glUniform3f(Uniforms.AmbientMaterial, 0.04f, 0.04f, 0.04f);
    glUniform3f(Uniforms.SpecularMaterial, 0.5f, 0.5f, 0.5f);
    glUniform1f(Uniforms.Shininess, 50);
    glUniformMatrix4fv(Uniforms.Projection, 1, 0, &ProjectionMatrix.col0.x);

    Vector4 lightPosition = V4MakeFromElems(0.25, 0.25, 1, 0);
    glUniform3fv(Uniforms.LightPosition, 1, &lightPosition.x);

    Matrix4 bezier = M4MakeFromCols(
        V4MakeFromElems(-1, 3, -3, 1),
        V4MakeFromElems(3, -6, 3, 0),
        V4MakeFromElems(-3, 3, 0, 0),
        V4MakeFromElems(1, 0, 0, 0) );
    glUniformMatrix4fv(Uniforms.PatchMatrix, 1, GL_FALSE, &bezier.col0.x);
    glUniformMatrix4fv(Uniforms.TransposedPatchMatrix, 1, GL_TRUE, &bezier.col0.x);

    // Initialize various state:
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.7f, 0.6f, 0.5f, 1.0f);

    return "Tessellation Demo";
}

static void CreateGumby()
{
    VertCount = sizeof(PatchData) / (sizeof(float) * 3);

    Point3 minBound = P3MakeFromElems(100, 100, 100);
    Point3 maxBound = P3MakeFromElems(-100, -100, -100);
    int vert;
    for (vert = 0; vert < VertCount; vert++) {
        float x = PatchData[vert][0];
        float y = PatchData[vert][1];
        float z = PatchData[vert][2];
        Point3 p = P3MakeFromElems(x, y, z);
        minBound = P3MinPerElem(minBound, p);
        maxBound = P3MaxPerElem(maxBound, p);
    }
    CenterPoint = P3Scale(P3AddV3(maxBound, V3MakeFromP3(minBound)), 0.5f);

    // Create the VAO:
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create the VBO for positions:
    GLuint positions;
    GLsizei stride = 3 * sizeof(float);
    GLsizei totalSize = stride * VertCount;
    glGenBuffers(1, &positions);
    glBindBuffer(GL_ARRAY_BUFFER, positions);
    glBufferData(GL_ARRAY_BUFFER, totalSize, PatchData, GL_STATIC_DRAW);
    glEnableVertexAttribArray(PositionSlot);
    glVertexAttribPointer(PositionSlot, 3, GL_FLOAT, GL_FALSE, stride, 0);
}

static void LoadEffect()
{
    GLint compileSuccess, linkSuccess;
    GLchar compilerSpew[256];

    glswInit();
    glswSetPath("../", ".glsl");
    glswAddDirectiveToken("*", "#version 400");

    const char* vsSource = glswGetShader("BicubicPatch.Vertex");
    const char* tcsSource = glswGetShader("BicubicPatch.TessControl");
    const char* tesSource = glswGetShader("BicubicPatch.TessEval");
    const char* gsSource = glswGetShader("BicubicPatch.Geometry");
    const char* fsSource = glswGetShader("BicubicPatch.Fragment");
    const char* msg = "Can't find %s shader.  Does '../BicubicPath.glsl' exist?\n";
    PezCheckCondition(vsSource != 0, msg, "vertex");
    PezCheckCondition(tcsSource != 0, msg, "tess control");
    PezCheckCondition(tesSource != 0, msg, "tess eval");
    PezCheckCondition(gsSource != 0, msg, "geometry");
    PezCheckCondition(fsSource != 0, msg, "fragment");

    GLuint vsHandle = glCreateShader(GL_VERTEX_SHADER);
    GLuint tcsHandle = glCreateShader(GL_TESS_CONTROL_SHADER);
    GLuint tesHandle = glCreateShader(GL_TESS_EVALUATION_SHADER);
    GLuint gsHandle = glCreateShader(GL_GEOMETRY_SHADER);
    GLuint fsHandle = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vsHandle, 1, &vsSource, 0);
    glCompileShader(vsHandle);
    glGetShaderiv(vsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(vsHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, "Vertex Shader Errors:\n%s", compilerSpew);

    glShaderSource(tcsHandle, 1, &tcsSource, 0);
    glCompileShader(tcsHandle);
    glGetShaderiv(tcsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(tcsHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, "Tess Control Shader Errors:\n%s", compilerSpew);

    glShaderSource(tesHandle, 1, &tesSource, 0);
    glCompileShader(tesHandle);
    glGetShaderiv(tesHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(tesHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, "Tess Eval Shader Errors:\n%s", compilerSpew);

    glShaderSource(gsHandle, 1, &gsSource, 0);
    glCompileShader(gsHandle);
    glGetShaderiv(gsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(gsHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, "Geometry Shader Errors:\n%s", compilerSpew);

    glShaderSource(fsHandle, 1, &fsSource, 0);
    glCompileShader(fsHandle);
    glGetShaderiv(fsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(fsHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, "Fragment Shader Errors:\n%s", compilerSpew);

    ProgramHandle = glCreateProgram();
    glAttachShader(ProgramHandle, vsHandle);
    glAttachShader(ProgramHandle, tcsHandle);
    glAttachShader(ProgramHandle, tesHandle);
    glAttachShader(ProgramHandle, gsHandle);
    glAttachShader(ProgramHandle, fsHandle);
    glBindAttribLocation(ProgramHandle, PositionSlot, "Position");
    glLinkProgram(ProgramHandle);
    glGetProgramiv(ProgramHandle, GL_LINK_STATUS, &linkSuccess);
    glGetProgramInfoLog(ProgramHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(linkSuccess, "Shader Link Errors:\n%s", compilerSpew);

    glUseProgram(ProgramHandle);
}

void PezUpdate(unsigned int elapsedMicroseconds)
{
    const float RadiansPerMicrosecond = 0.0000005f;
    static float Theta = 0;
    Theta += elapsedMicroseconds * RadiansPerMicrosecond;
    
    Vector3 offset = V3MakeFromElems(CenterPoint.x, CenterPoint.z, 0);
    Matrix4 model = M4MakeRotationZ(Theta);
    model = M4Mul(M4MakeTranslation(offset), model);
    model = M4Mul(model, M4MakeTranslation(V3Neg(offset)));

    Point3 eyePosition = P3MakeFromElems(CenterPoint.x, -50, CenterPoint.z);
    Point3 targetPosition = P3MakeFromElems(CenterPoint.x, 0, CenterPoint.z);
    Vector3 upVector = V3MakeFromElems(0, 0, 1);
    Matrix4 view = M4MakeLookAt(eyePosition, targetPosition, upVector);
    
    ModelviewMatrix = M4Mul(view, model);
    NormalMatrix = M4GetUpper3x3(ModelviewMatrix);

    const int VK_LEFT = 0x25;
    const int VK_UP = 0x26;
    const int VK_RIGHT = 0x27;
    const int VK_DOWN = 0x28;

    if (PezIsPressing(VK_RIGHT)) TessLevelInner++;
    if (PezIsPressing(VK_LEFT))  TessLevelInner = TessLevelInner > 1 ? TessLevelInner - 1 : 1;
    if (PezIsPressing(VK_UP))    TessLevelOuter++;
    if (PezIsPressing(VK_DOWN))  TessLevelOuter = TessLevelOuter > 1 ? TessLevelOuter - 1 : 1;
}

void PezHandleMouse(int x, int y, int action)
{
}
