#include "Splat.h"
#include "Trackball.h"
#include <limits>

using namespace std;
using namespace VectorMath;

static GLuint QuadVao;
static GLuint CubeVao;
static GLuint GridVao;
static GLuint RayEndpointsProgram;
static GLuint RaycastProgram;
static GLuint StreamlineProgram;
static GLuint WireframeProgram;
static Surface RayEndPointsSurface;
static GLuint SplatTexture;
static GLuint NoiseTexture;
static Matrix4 ProjectionMatrix;
static Matrix4 ModelviewMatrix;
static Trackball Trackball(PEZ_VIEWPORT_HEIGHT / 2);

static void BindProgram(GLuint program);
static GLenum* EnumArray(GLenum a, GLenum b);

const char* PezInitialize(int width, int height)
{
    GridVao = CreatePoints();
    QuadVao = CreateQuad();
    CubeVao = CreateCube();
    RayEndpointsProgram = CreateProgram("Volume.Vertex", 0, "Volume.Endpoints");
    RaycastProgram = CreateProgram("Volume.Quad", 0, "Volume.Lighting");
    //RaycastProgram = CreateProgram("Volume.Quad", 0, "Volume.Semitransparent");
    WireframeProgram = CreateProgram("Wireframe.VS", "Wireframe.GS", "Wireframe.FS");
    StreamlineProgram = CreateProgram("Streamline.VS", "Streamline.GS", "Streamline.FS");
    RayEndPointsSurface = CreateSurface(width, height, 3, 2);

    PointList positions = CreatePathline();
    SplatTexture = CreateSplat(QuadVao, positions);

    NoiseTexture = CreateNoise();

    glBlendFunc(GL_ONE, GL_ONE);
    glDisable(GL_DEPTH_TEST);
    return "Splat Demo";
}

void PezRender(GLuint windowFbo)
{
    // Clear the texture bindings:
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_3D, 0);

    // Find screen-space AABB for scissoring:
    const float M = std::numeric_limits<float>::max();
    Point3 minCorner(M, M, 0);
    Point3 maxCorner(-M, -M, 0);
    for (int cornerIndex = 0; cornerIndex < 8; cornerIndex++) {
        Point3 corner(
            (cornerIndex & 0x1) ? -1.0f : +1.0f,
            (cornerIndex & 0x2) ? -1.0f : +1.0f,
            (cornerIndex & 0x4) ? -1.0f : +1.0f);

        Vector4 v = ProjectionMatrix * ModelviewMatrix * corner;
        Point3 p = perspective(v);
        minCorner = minPerElem(p, minCorner);
        maxCorner = maxPerElem(p, maxCorner);
    }
    minCorner += Vector3(1.0f, 1.0f, 0);
    maxCorner += Vector3(1.0f, 1.0f, 0);
    Vector3 viewport(PEZ_VIEWPORT_WIDTH / 2, PEZ_VIEWPORT_HEIGHT / 2, 1.0f);
    minCorner = scale(minCorner, viewport);
    maxCorner = scale(maxCorner, viewport);
    Vector3 extent = maxCorner - minCorner;
    glScissor(
        (GLint) minCorner.getX(), (GLint) minCorner.getY(),
        (GLsizei) extent.getX(), (GLsizei) extent.getY());

    // Update the ray start & stop surfaces:
    BindProgram(RayEndpointsProgram);
    glBindFramebuffer(GL_FRAMEBUFFER, RayEndPointsSurface.FboHandle);
    glEnable(GL_BLEND);
    glDrawBuffers(2, EnumArray(GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1));
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(CubeVao);
    glEnable(GL_SCISSOR_TEST);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);

    // Perform the raycast:
    BindProgram(RaycastProgram);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffers(2, EnumArray(GL_BACK_LEFT, GL_NONE));
    glClearColor(0, 0.25f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, RayEndPointsSurface.TextureHandle[0]);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, RayEndPointsSurface.TextureHandle[1]);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_3D, SplatTexture);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, NoiseTexture);
    glBindVertexArray(QuadVao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Draw the point grid:
    glEnable(GL_BLEND);
    BindProgram(StreamlineProgram);
    glBindVertexArray(GridVao);
    glDrawArrays(GL_POINTS, 0, GridDensity * GridDensity * GridDensity);
    glDisable(GL_BLEND);

    // Draw the cube:
    if (true) {
        BindProgram(WireframeProgram);
        glBindVertexArray(CubeVao);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
    }

    glDisable(GL_SCISSOR_TEST);
}

void PezUpdate(unsigned int elapsedMicroseconds)
{
    Trackball.Update(elapsedMicroseconds);

    const float HalfWidth = 0.5f;
    const float HalfHeight = HalfWidth * PEZ_VIEWPORT_HEIGHT / PEZ_VIEWPORT_WIDTH;
    ProjectionMatrix = Matrix4::frustum(-HalfWidth, +HalfWidth, -HalfHeight, +HalfHeight, 2, 70);
    Point3 eyePosition = Point3(0, 0, 10);
    Vector3 upVector = Vector3(0, -1, 0);
    Point3 targetPosition = Point3(0, 0, 0);
    Matrix4 view = Matrix4::lookAt(eyePosition, targetPosition, upVector);

    Matrix4 model = Matrix4( Trackball.GetRotation(), Vector3(0, 0, 0) );
    
    const bool Autorotate = false;
    if (Autorotate) {
        static float radians = 0.2f;
        radians += 0.0000005f * elapsedMicroseconds;
        model *= Matrix4::rotation(radians, Vector3(0, 1, 0));
    }
    
    ModelviewMatrix = view * model;
}

void PezHandleMouse(int x, int y, int action)
{
    if (action == (PEZ_DOWN | PEZ_LEFT))
        Trackball.MouseDown(x, y);
    else if (action == (PEZ_UP | PEZ_LEFT))
        Trackball.MouseUp(x, y);
    else if (action == (PEZ_MOVE | PEZ_DOWN | PEZ_LEFT))
        Trackball.MouseMove(x, y);
    else if (action == (PEZ_DOUBLECLICK | PEZ_LEFT))
        Trackball.ReturnHome();
}

static void BindProgram(GLuint program)
{
    glUseProgram(program);

    GLint modelview = glGetUniformLocation(program, "Modelview");
    if (modelview > -1) {
        glUniformMatrix4fv(modelview, 1, 0, (float*) &ModelviewMatrix);
    }

    GLint modelviewProjection = glGetUniformLocation(program, "ModelviewProjection");
    if (modelviewProjection > -1) {
        Matrix4 mvp = ProjectionMatrix * ModelviewMatrix;
        glUniformMatrix4fv(modelviewProjection, 1, 0, (float*) &mvp);
    }

    GLint normalMatrix = glGetUniformLocation(program, "NormalMatrix");
    if (normalMatrix > -1) {
        Matrix3 nm = transpose(ModelviewMatrix.getUpper3x3());
        float packed[9] = {
            nm.getCol0().getX(), nm.getCol1().getX(), nm.getCol2().getX(),
            nm.getCol0().getY(), nm.getCol1().getY(), nm.getCol2().getY(),
            nm.getCol0().getZ(), nm.getCol1().getZ(), nm.getCol2().getZ() };
        glUniformMatrix3fv(normalMatrix, 1, 0, (float*) &packed);
    }
    
    GLint projection = glGetUniformLocation(program, "Projection");
    if (projection > -1)
        glUniformMatrix4fv(projection, 1, 0, (float*) &ProjectionMatrix);

    GLint size = glGetUniformLocation(program, "InverseSize");
    if (size > -1)
        glUniform2f(size, 1.0f / PEZ_VIEWPORT_WIDTH, 1.0f / PEZ_VIEWPORT_HEIGHT);

    GLint diffuseMaterial = glGetUniformLocation(program, "DiffuseMaterial");
    if (diffuseMaterial > -1)
        glUniform3f(diffuseMaterial, 1, 1, 0.5f);

    GLint lightPosition = glGetUniformLocation(program, "LightPosition");
    if (lightPosition > -1) {
        Vector3 v = normalize(Vector3(0.25f, 0.25f, 1));
        glUniform3fv(lightPosition, 1, (float*) &v);
    }

    GLint rayStart = glGetUniformLocation(program, "RayStart");
    if (rayStart > -1)
        glUniform1i(rayStart, 0);

    GLint rayStop = glGetUniformLocation(program, "RayStop");
    if (rayStop > -1)
        glUniform1i(rayStop, 1);

    GLint volume = glGetUniformLocation(program, "Volume");
    if (volume > -1)
        glUniform1i(volume, 2);

    GLint noise = glGetUniformLocation(program, "Noise");
    if (noise > -1)
        glUniform1i(noise, 3);
}

static GLenum* EnumArray(GLenum a, GLenum b)
{
    static GLenum enums[2];
    enums[0] = a;
    enums[1] = b;
    return &enums[0];
}
