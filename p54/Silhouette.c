#include "Platform.h"
#include "Utility.h"
#include <vectormath.h>
#include <stdlib.h>

struct ProgramsRec
{
    GLuint Shading;
    GLuint ExtrudeLines;
    GLuint EarlyZ;
} Programs;

static Matrix4 ProjectionMatrix;
static Matrix4 ModelviewMatrix;
static Mesh DemoMesh;
static Mesh DemoQuad;
static GLuint GBuffer;
static GLuint NormalsTexture;
static GLuint DepthTexture;

static void LoadProgram(GLuint program)
{
    glUseProgram(program);

    GLint lineWidth = glGetUniformLocation(program, "HalfWidth");
    if (lineWidth > -1)
        glUniform1f(lineWidth, 0.005f);

    GLint overhangLength = glGetUniformLocation(program, "OverhangLength");
    if (overhangLength > -1)
        glUniform1f(overhangLength, 0.15f);

    GLint modelview = glGetUniformLocation(program, "Modelview");
    if (modelview > -1)
        glUniformMatrix4fv(modelview, 1, 0, &ModelviewMatrix.col0.x);

    GLint modelviewProjection = glGetUniformLocation(program, "ModelviewProjection");
    if (modelviewProjection > -1) {
        Matrix4 mvp = M4Mul(ProjectionMatrix, ModelviewMatrix);
        glUniformMatrix4fv(modelviewProjection, 1, 0, &mvp.col0.x);
    }

    GLint normalMatrix = glGetUniformLocation(program, "NormalMatrix");
    if (normalMatrix > -1) {
        Matrix3 nm = M3Transpose(M4GetUpper3x3(ModelviewMatrix));
        float packed[9] = {
            nm.col0.x, nm.col1.x, nm.col2.x,
            nm.col0.y, nm.col1.y, nm.col2.y,
            nm.col0.z, nm.col1.z, nm.col2.z };
        glUniformMatrix3fv(normalMatrix, 1, 0, &packed[0]);
    }

    GLint projection = glGetUniformLocation(program, "Projection");
    if (projection > -1)
        glUniformMatrix4fv(projection, 1, 0, &ProjectionMatrix.col0.x);

    GLint size = glGetUniformLocation(program, "Size");
    if (size > -1)
        glUniform2f(size, PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT);

    GLint diffuseMaterial = glGetUniformLocation(program, "DiffuseMaterial");
    if (diffuseMaterial > -1)
        glUniform3f(diffuseMaterial, 0, 0.75, 0.75);

    GLint specularMaterial = glGetUniformLocation(program, "SpecularMaterial");
    if (specularMaterial > -1)
        glUniform3f(specularMaterial, 0.5f, 0.5f, 0.5f);

    GLint shininess = glGetUniformLocation(program, "Shininess");
    if (shininess > -1)
        glUniform1f(shininess, 50);

    GLint ambientMaterial = glGetUniformLocation(program, "AmbientMaterial");
    if (ambientMaterial > -1)
        glUniform3f(ambientMaterial, 0.04f, 0.04f, 0.04f);

    GLint lightPosition = glGetUniformLocation(program, "LightPosition");
    if (lightPosition > -1)
        glUniform3f(lightPosition, 0.25, 0.25, 1);

    GLint normalMap = glGetUniformLocation(program, "NormalMap");
    if (normalMap > -1)
        glUniform1i(normalMap, 1);
}

static void RenderMesh()
{
    GLuint programHandle;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &programHandle);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DemoMesh.Faces);

    glBindBuffer(GL_ARRAY_BUFFER, DemoMesh.Positions);
    GLint positionSlot = glGetAttribLocation(programHandle, "Position");
    glVertexAttribPointer(positionSlot, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    glEnableVertexAttribArray(positionSlot);

    GLint normalSlot = glGetAttribLocation(programHandle, "Normal");
    if (normalSlot > -1) {
        glBindBuffer(GL_ARRAY_BUFFER, DemoMesh.Normals);
        glVertexAttribPointer(normalSlot, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
        glEnableVertexAttribArray(normalSlot);
    }

    glDrawElements(GL_TRIANGLES_ADJACENCY, DemoMesh.FaceCount * 6, GL_UNSIGNED_SHORT, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(positionSlot);

    if (normalSlot > -1)
        glDisableVertexAttribArray(normalSlot);
}

static void RenderQuad()
{
    GLuint programHandle;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &programHandle);

    int positionSlot = glGetAttribLocation(programHandle, "Position");

    glBindBuffer(GL_ARRAY_BUFFER, DemoQuad.Positions);
    glVertexAttribPointer(positionSlot, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    glEnableVertexAttribArray(positionSlot);
    glDrawArrays(GL_TRIANGLES, 0, DemoQuad.VertexCount);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(positionSlot);
}

void PezRender(GLuint windowFbo)
{
    // Early Z pass:

    glEnable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, GBuffer);
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    LoadProgram(Programs.EarlyZ);
    RenderMesh();

    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_DEPTH_TEST);

    // Full-screen quad pass:

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDepthMask(GL_FALSE);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, NormalsTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, DepthTexture);

    LoadProgram(Programs.Shading);
    RenderQuad();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Render silhouette lines:

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    LoadProgram(Programs.ExtrudeLines);
    RenderMesh();
    glDisable(GL_BLEND);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

const char* PezInitialize(int width, int height)
{
    DemoMesh = CreateMesh("ChineseDragon.ctm", true);
    DemoQuad = CreateQuad();
    
    Programs.Shading = CreateProgram("Silhouette.Vertex.Quad", 0, "Silhouette.Fragment.Lighting");
    Programs.ExtrudeLines = CreateProgram("Silhouette.Vertex.Lines", "Silhouette.Geometry", "Silhouette.Fragment.Black");
    Programs.EarlyZ = CreateProgram("Silhouette.Vertex", 0, "Silhouette.Fragment.WriteNormals");

    // Create a depth texture:
    GLuint textureHandle;
    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 0);
    PezCheckCondition(GL_NO_ERROR == glGetError(), "Unable to create depth texture");
    DepthTexture = textureHandle;
    
    // Create a FBO and attach the depth texture:
    GLuint fboHandle;
    glGenFramebuffers(1, &fboHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureHandle, 0);
    PezCheckCondition(GL_NO_ERROR == glGetError(), "Unable to attach depth texture");

    // Create a normals texture:
    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    PezCheckCondition(GL_NO_ERROR == glGetError(), "Unable to create normals texture");
    NormalsTexture = textureHandle;

    // Attach a color buffer:
    GLuint colorbuffer;
    glGenRenderbuffers(1, &colorbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureHandle, 0);
    PezCheckCondition(GL_NO_ERROR == glGetError(), "Unable to attach color buffer");

    // Validate the FBO:
    PezCheckCondition(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER), "Unable to create FBO.");
    GBuffer = fboHandle;

    // Set up the projection matrix:
    const float HalfWidth = 0.1f;
    const float HalfHeight = HalfWidth * PEZ_VIEWPORT_HEIGHT / PEZ_VIEWPORT_WIDTH;
    ProjectionMatrix = M4MakeFrustum(-HalfWidth, +HalfWidth, -HalfHeight, +HalfHeight, 2, 70);

    // Initialize various GL state:
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glPolygonOffset(5, 5);
    glClearColor(0.5f, 0.3f, 0.1f, 1);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return "Silhouette Demo";
}

void PezUpdate(unsigned int elapsedMicroseconds)
{
    const float RadiansPerMicrosecond = 0.0000005f;
    static float Theta = 0;
    Theta += elapsedMicroseconds * RadiansPerMicrosecond;
    Vector3 offset = V3MakeFromElems(0, 0, 0);
    Matrix4 model = M4MakeRotationY(Theta);
    model = M4Mul(M4MakeTranslation(offset), model);
    model = M4Mul(model, M4MakeTranslation(V3Neg(offset)));
    Point3 eyePosition = P3MakeFromElems(0, 0, 2.25f);
    Vector3 upVector = V3MakeFromElems(0, 1, 0);
    Point3 targetPosition = P3MakeFromElems(0, 0, 0);
    Matrix4 view = M4MakeLookAt(eyePosition, targetPosition, upVector);
    ModelviewMatrix = M4Mul(view, model);
}

void PezHandleMouse(int x, int y, int action)
{
    if (action == PEZ_UP)
    {
        exit(0);
    }
}
