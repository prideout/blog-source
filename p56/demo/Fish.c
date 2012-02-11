#include <pez.h>
#include <vectormath.h>
#include "Utility.h"

static GLuint BentProgram;
static Matrix4 ProjectionMatrix;
static Matrix4 ModelviewMatrix;
static Mesh SquidMesh;
static Mesh TunaMesh;
static Mesh DolphinMesh;
static Mesh CylinderMesh;
static Texture PathTexture;
static int ShowPaths = 0;
static float PathOffset;
static float PathScale;

void PezHandleMouse(int x, int y, int action)
{
    if (action == PEZ_UP) {
        ShowPaths = 1 - ShowPaths;
    }
}

static void LoadProgram(GLuint program)
{
    glUseProgram(program);

    GLint pathScale = glGetUniformLocation(program, "PathScale");
    glUniform1f(pathScale, PathScale);
    
    GLint pathOffset = glGetUniformLocation(program, "PathOffset");
    glUniform1f(pathOffset, PathOffset);

    GLint inverseWidth = glGetUniformLocation(program, "InverseWidth");
    glUniform1f(inverseWidth, 1.0f / PathTexture.Width);

    GLint inverseHeight = glGetUniformLocation(program, "InverseHeight");
    glUniform1f(inverseHeight, 1.0f / PathTexture.Height);

    GLint modelviewProjection = glGetUniformLocation(program, "ModelviewProjection");
    Matrix4 mvp = M4Mul(ProjectionMatrix, ModelviewMatrix);
    glUniformMatrix4fv(modelviewProjection, 1, 0, &mvp.col0.x);

    GLint normalMatrix = glGetUniformLocation(program, "NormalMatrix");
    Matrix3 nm = M3Transpose(M4GetUpper3x3(ModelviewMatrix));
    float packed[9] = {
        nm.col0.x, nm.col1.x, nm.col2.x,
        nm.col0.y, nm.col1.y, nm.col2.y,
        nm.col0.z, nm.col1.z, nm.col2.z };
    glUniformMatrix3fv(normalMatrix, 1, 0, &packed[0]);

    GLint projection = glGetUniformLocation(program, "Projection");
    glUniformMatrix4fv(projection, 1, 0, &ProjectionMatrix.col0.x);

    GLint size = glGetUniformLocation(program, "Size");
    glUniform2f(size, PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT);

    GLint specularMaterial = glGetUniformLocation(program, "SpecularMaterial");
    glUniform3f(specularMaterial, 0.5f, 0.5f, 0.5f);

    GLint shininess = glGetUniformLocation(program, "Shininess");
    glUniform1f(shininess, 50);

    GLint ambientMaterial = glGetUniformLocation(program, "AmbientMaterial");
    glUniform3f(ambientMaterial, 0.04f, 0.04f, 0.04f);

    GLint lightPosition = glGetUniformLocation(program, "LightPosition");
    glUniform3f(lightPosition, 0.25, 0.25, 1);
}

static void BindMesh(Mesh m)
{
    GLuint programHandle;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &programHandle);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.Faces);

    glBindBuffer(GL_ARRAY_BUFFER, m.Positions);
    GLint positionSlot = glGetAttribLocation(programHandle, "Position");
    glVertexAttribPointer(positionSlot, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    glEnableVertexAttribArray(positionSlot);

    GLint normalSlot = glGetAttribLocation(programHandle, "Normal");
    if (normalSlot > -1) {
        glBindBuffer(GL_ARRAY_BUFFER, m.Normals);
        glVertexAttribPointer(normalSlot, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
        glEnableVertexAttribArray(normalSlot);
    }
}

void PezRender(GLuint windowFbo)
{
    LoadProgram(BentProgram);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, PathTexture.Handle);

    GLint instanceOffset = glGetUniformLocation(BentProgram, "InstanceOffset");
    GLint diffuseMaterial = glGetUniformLocation(BentProgram, "DiffuseMaterial");

    if (ShowPaths) {
        glUniform3f(diffuseMaterial, 255/255.0f, 167/255.0f, 178/255.0f);
        BindMesh(CylinderMesh);
        
        glUniform1i(instanceOffset, 0);
        glDrawElements(GL_TRIANGLES, CylinderMesh.FaceCount * 3, GL_UNSIGNED_INT, 0);

        glUniform1i(instanceOffset, 1);
        glDrawElements(GL_TRIANGLES, CylinderMesh.FaceCount * 3, GL_UNSIGNED_INT, 0);
    }

    glClearColor(143/255.0f, 188/255.0f, 204/255.0f, 1);

    glUniform3f(diffuseMaterial, 0.5f, 0.5f, 0.5f);
    glUniform1i(instanceOffset, 0);
    BindMesh(DolphinMesh);
    glDrawElementsInstanced(GL_TRIANGLES, DolphinMesh.FaceCount * 3, GL_UNSIGNED_INT, 0, 1);

    glUniform3f(diffuseMaterial, 255/255.0f, 250/255.0f, 179/255.0f);
    glUniform1i(instanceOffset, 1);
    BindMesh(SquidMesh);
    glDrawElementsInstanced(GL_TRIANGLES, SquidMesh.FaceCount * 3, GL_UNSIGNED_INT, 0, 1);

    glUniform3f(diffuseMaterial, 0.9, 0.9, 1.0);
    glUniform1i(instanceOffset, 2);
    BindMesh(TunaMesh);
    glDrawElementsInstanced(GL_TRIANGLES, TunaMesh.FaceCount * 3, GL_UNSIGNED_INT, 0, 126);
}

const char* PezInitialize(int width, int height)
{
    int slices = 8;
    int stacks = 128;
    float radius = 0.05f;
    CylinderMesh = CreateCylinder(20.0f, radius, stacks, slices);
    DolphinMesh = CreateMesh("Dolphin.ctm", 1, 1.25f);
    SquidMesh = CreateMesh("Squid.ctm", 1, 1);
    TunaMesh = CreateMesh("Tuna.ctm", 0.25, 0.25);
    PathTexture = CreatePathTexture();
    BentProgram = CreateProgram("Fish.Vertex.Bent", 0, "Fish.Fragment");

    // Set up the projection matrix:
    const float HalfWidth = 0.75f;
    const float HalfHeight = HalfWidth * PEZ_VIEWPORT_HEIGHT / PEZ_VIEWPORT_WIDTH;
    ProjectionMatrix = M4MakeFrustum(-HalfWidth, +HalfWidth, -HalfHeight, +HalfHeight, 2, 15);

    // Initialize various GL state:
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    return "Fish Demo";
}

void PezUpdate(unsigned int elapsedMicroseconds)
{
    const float RadiansPerMicrosecond = 0.0000005f;
    static float Theta = 0.0f;
    //Theta += elapsedMicroseconds * RadiansPerMicrosecond;
    PathOffset += elapsedMicroseconds * 0.0000002f;
    PathScale = 0.05f;

    Vector3 offset = V3MakeFromElems(0, 0, 0);
    Matrix4 model = M4MakeRotationX(Theta);
    model = M4Mul(M4MakeTranslation(offset), model);
    model = M4Mul(model, M4MakeTranslation(V3Neg(offset)));
    Point3 eyePosition = P3MakeFromElems(0, 10, 0.1f);
    Point3 targetPosition = P3MakeFromElems(0, 0, 0.1f);
    Vector3 upVector = V3MakeFromElems(0, 0, 1);
    Matrix4 view = M4MakeLookAt(eyePosition, targetPosition, upVector);
    ModelviewMatrix = M4Mul(view, model);
}
