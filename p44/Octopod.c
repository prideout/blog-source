#include <pez.h>
#include <glew.h>
#include <glsw.h>
#include <openctm.h>
#include <vectormath_aos.h>

static void LoadMesh();
static void LoadEffect();

static GLsizei IndexCount;
static const GLuint PositionSlot = 0;
static GLuint ProgramHandle;
static float Theta = 0;

void PezRender(GLuint fbo)
{
    GLint projectionLocation, modelviewLocation, colorLocation;
    VmathMatrix4 projectionMatrix, modelviewMatrix;
    VmathPoint3 eyePosition, targetPosition;
    VmathVector3 upVector;
    VmathTransform3 rotation;

    const float HalfWidth = 2;
    const float HalfHeight = HalfWidth * PEZ_VIEWPORT_HEIGHT / PEZ_VIEWPORT_WIDTH;

    // Set up the projection matrix:
    vmathM4MakeFrustum(&projectionMatrix, -HalfWidth, +HalfWidth, -HalfHeight, +HalfHeight, 5, 150);
    projectionLocation = glGetUniformLocation(ProgramHandle, "Projection");
    glUniformMatrix4fv(projectionLocation, 1, 0, &projectionMatrix.col0.x);

    // Set up the model-view matrix:

    vmathT3MakeRotationY(&rotation, Theta);
    vmathP3MakeFromElems(&eyePosition, 0, 0, 25);
    vmathT3MulP3(&eyePosition, &rotation, &eyePosition);

    vmathP3MakeFromElems(&targetPosition, 0, 0, 0);
    vmathV3MakeFromElems(&upVector, 0, 1, 0);
    vmathM4MakeLookAt(&modelviewMatrix, &eyePosition, &targetPosition, &upVector);
    modelviewLocation = glGetUniformLocation(ProgramHandle, "Modelview");
    glUniformMatrix4fv(modelviewLocation, 1, 0, &modelviewMatrix.col0.x);

    colorLocation = glGetUniformLocation(ProgramHandle, "FillColor");

    glClearColor(0.5f, 0.6f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glPolygonOffset(4, 8);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw the white triangles:
    if (0) {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUniform4f(colorLocation, 0.9f, 0.9f, 0.75f, 1);
        glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, 0);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    // Draw the black lines:
    glUniform4f(colorLocation, 0, 0, 0, 0.25f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, 0);
}

const char* PezInitialize(int width, int height)
{
    LoadMesh();
    LoadEffect();
    return "Wireframe";
}

static void LoadMesh()
{
    CTMcontext ctmContext;
    CTMuint vertexCount;
    const CTMfloat* positions;
    const CTMuint* indices;

    // Open the CTM file:
    ctmContext = ctmNewContext(CTM_IMPORT);
    ctmLoad(ctmContext, "../octopod.ctm");
    PezCheckCondition(ctmGetError(ctmContext) == CTM_NONE, "OpenCTM Issue");
    vertexCount = ctmGetInteger(ctmContext, CTM_VERTEX_COUNT);
    IndexCount = 3 * ctmGetInteger(ctmContext, CTM_TRIANGLE_COUNT);

    // Create the VBO for positions:
    positions = ctmGetFloatArray(ctmContext, CTM_VERTICES);
    if (positions) {
        GLuint handle;
        GLsizeiptr size = vertexCount * sizeof(float) * 3;
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ARRAY_BUFFER, handle);
        glBufferData(GL_ARRAY_BUFFER, size, positions, GL_STATIC_DRAW);
        glEnableVertexAttribArray(PositionSlot);
        glVertexAttribPointer(PositionSlot, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    }

    // Create the VBO for indices:
    indices = ctmGetIntegerArray(ctmContext, CTM_INDICES);
    if (indices) {
        GLuint handle;
        GLsizeiptr size = IndexCount * sizeof(CTMuint);
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
    }

    ctmFreeContext(ctmContext);
}

static void LoadEffect()
{
    const char* vsSource, * fsSource;
    GLuint vsHandle, fsHandle;
    GLint compileSuccess, linkSuccess;
    GLchar compilerSpew[256];

    glswInit();
    glswSetPath("../", ".glsl");
    glswAddDirectiveToken("GL3", "#version 130");

    vsSource = glswGetShader("Wireframe.Vertex." PEZ_GL_VERSION_TOKEN);
    fsSource = glswGetShader("Wireframe.Fragment." PEZ_GL_VERSION_TOKEN);
    PezCheckCondition(vsSource != 0, "Can't find vertex shader.\n");
    PezCheckCondition(fsSource != 0, "Can't find fragment shader.\n");

    vsHandle = glCreateShader(GL_VERTEX_SHADER);
    fsHandle = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vsHandle, 1, &vsSource, 0);
    glCompileShader(vsHandle);
    glGetShaderiv(vsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(vsHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, "Vertex Shader Errors:\n%s", compilerSpew);

    glShaderSource(fsHandle, 1, &fsSource, 0);
    glCompileShader(fsHandle);
    glGetShaderiv(fsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(fsHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, "Fragment Shader Errors:\n%s", compilerSpew);

    ProgramHandle = glCreateProgram();
    glAttachShader(ProgramHandle, vsHandle);
    glAttachShader(ProgramHandle, fsHandle);
    glBindAttribLocation(ProgramHandle, PositionSlot, "Position");
    glLinkProgram(ProgramHandle);
    glGetProgramiv(ProgramHandle, GL_LINK_STATUS, &linkSuccess);
    glGetProgramInfoLog(ProgramHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(linkSuccess, "Shader Link Errors:\n%s", compilerSpew);

    glUseProgram(ProgramHandle);
}

void PezHandleMouse(int x, int y, int action) { }

void PezUpdate(unsigned int elapsedMilliseconds)
{
    Theta += (float) elapsedMilliseconds / 10000.0f;
}
