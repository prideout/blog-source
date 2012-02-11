#include <glew.h>
#include <pez.h>
#include <glsw.h>
#include <openctm.h>
#include <vectormath_aos.h>

static void LoadMesh();
static GLuint LoadProgram(const char* vsKey, const char* gsKey, const char* fsKey);


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
    int index, color, slices, stacks, indsPerPatch;

    const float HalfWidth = 1.15f;
    const float HalfHeight = HalfWidth * PEZ_VIEWPORT_HEIGHT / PEZ_VIEWPORT_WIDTH;

    // Set up the projection matrix:
    vmathM4MakeFrustum(&projectionMatrix, -HalfWidth, +HalfWidth, -HalfHeight, +HalfHeight, 5, 1500);
    projectionLocation = glGetUniformLocation(ProgramHandle, "Projection");
    glUniformMatrix4fv(projectionLocation, 1, 0, &projectionMatrix.col0.x);

    // Set up the model-view matrix:

    vmathT3MakeRotationZ(&rotation, Theta);
    vmathP3MakeFromElems(&eyePosition, 15, 100, 10);
    vmathT3MulP3(&eyePosition, &rotation, &eyePosition);

    vmathP3MakeFromElems(&targetPosition, 15, 0, 10);
    vmathV3MakeFromElems(&upVector, 0, 0, 1);
    vmathM4MakeLookAt(&modelviewMatrix, &eyePosition, &targetPosition, &upVector);
    modelviewLocation = glGetUniformLocation(ProgramHandle, "Modelview");
    glUniformMatrix4fv(modelviewLocation, 1, 0, &modelviewMatrix.col0.x);

    colorLocation = glGetUniformLocation(ProgramHandle, "FillColor");

    glClearColor(0.7f, 0.6f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glPolygonOffset(4, 8);

    // Draw the white triangles:
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    color = 0;
    slices = stacks = 8;
    indsPerPatch = (slices - 1) * (stacks - 1) * 6;
    for (index = 0; index < IndexCount; index += indsPerPatch, color++)
    {
        int offset = index * sizeof(int);
        glUniform4f(colorLocation, color % 2 ? 0.9f : 0.4f, 0.9f, color % 3 ? 0.4f : 0.75f, 1);
        glDrawElements(GL_TRIANGLES, indsPerPatch, GL_UNSIGNED_INT, (void*) offset);
    }

    glDisable(GL_POLYGON_OFFSET_FILL);

    // Draw the black lines:
    glUniform4f(colorLocation, 0.25, 0.25, 0.25, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//    glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, 0);
}

const char* PezInitialize(int width, int height)
{
    LoadMesh();
    ProgramHandle = LoadProgram("Wireframe.vertex", "Wireframe.geometry", "Wireframe.fragment");
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
    ctmLoad(ctmContext, "../gumbo.ctm");
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

GLuint LoadProgram(const char* vsKey, const char* gsKey, const char* fsKey)
{
    // Read shader strings from the effect file
    const char* vsSource;
    const char* fsSource;
    const char* gsSource;
    GLuint vsHandle = glCreateShader(GL_VERTEX_SHADER);
    GLuint gsHandle = glCreateShader(GL_GEOMETRY_SHADER_EXT);
    GLuint fsHandle = glCreateShader(GL_FRAGMENT_SHADER);
    GLchar spew[256];
    GLint compileSuccess;
    GLuint programHandle = glCreateProgram();
    GLint linkSuccess;

    glswInit();
    glswSetPath("../", ".glsl");
    glswAddDirectiveToken("GL3", "#version 130");

    vsSource = glswGetShader(vsKey);
    fsSource = glswGetShader(fsKey);
    gsSource = glswGetShader(gsKey);

    PezCheckCondition(vsSource != 0, "Can't find vshader: %s\n", vsKey);
    PezCheckCondition(gsSource != 0, "Can't find gshader: %s\n", gsKey);
    PezCheckCondition(fsSource != 0, "Can't find fshader: %s\n", fsKey);

    glShaderSource(vsHandle, 1, &vsSource, 0);
    glShaderSource(gsHandle, 1, &gsSource, 0);
    glShaderSource(fsHandle, 1, &fsSource, 0);

    // Declare space for error messages from the compiler and linker
    glCompileShader(vsHandle);
    glGetShaderiv(vsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(vsHandle, sizeof(spew), 0, spew);
    PezCheckCondition(compileSuccess, "Can't compile vshader:\n%s", spew);
    
    glCompileShader(gsHandle);
    glGetShaderiv(gsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(gsHandle, sizeof(spew), 0, spew);
    PezCheckCondition(compileSuccess, "Can't compile gshader:\n%s", spew);

    glCompileShader(fsHandle);
    glGetShaderiv(fsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(fsHandle, sizeof(spew), 0, spew);
    PezCheckCondition(compileSuccess, "Can't compile fshader:\n%s", spew);

    // Create GLSL program object
 
    glAttachShader(programHandle, vsHandle);
    glAttachShader(programHandle, gsHandle);
    glAttachShader(programHandle, fsHandle);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    glGetProgramInfoLog(programHandle, sizeof(spew), 0, spew);
    PezCheckCondition(linkSuccess, "Can't link shaders:\n%s", spew);

    glUseProgram(programHandle);
    return programHandle;
}

void PezHandleMouse(int x, int y, int action) { }

void PezUpdate(unsigned int elapsedMilliseconds)
{
    Theta += (float) elapsedMilliseconds / 10000.0f;
}
