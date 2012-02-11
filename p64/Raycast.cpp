#include "Utility.h"

extern "C" {
#include "perlin.h"
}

using namespace vmath;
using std::string;

struct ProgramHandles {
    GLuint SinglePass;
    GLuint TwoPassRaycast;
    GLuint TwoPassIntervals;
};

static ProgramHandles Programs;
static GLuint CreatePyroclasticVolume(int n, float r);
static ITrackball* Trackball;
static GLuint CubeCenterVbo;
static Matrix4 ProjectionMatrix;
static Matrix4 ModelviewMatrix;
static Matrix4 ViewMatrix;
static Matrix4 ModelviewProjection;
static Point3 EyePosition;
static GLuint CloudTexture;
static SurfacePod IntervalsFbo[2];
static bool SinglePass = true;
static float FieldOfView = 0.7f;

PezConfig PezGetConfig()
{
    PezConfig config;
    config.Title = "Raycast";
    config.Width = 800;
    config.Height = 800;
    config.Multisampling = 0;
    config.VerticalSync = 0;
    return config;
}

void PezInitialize()
{
    PezConfig cfg = PezGetConfig();

    Trackball = CreateTrackball(cfg.Width * 1.0f, cfg.Height * 1.0f, cfg.Width * 0.5f);
    Programs.SinglePass = LoadProgram("SinglePass.VS", "SinglePass.GS", "SinglePass.FS");
    Programs.TwoPassIntervals = LoadProgram("TwoPass.VS", "TwoPass.Cube", "TwoPass.Intervals");
    Programs.TwoPassRaycast = LoadProgram("TwoPass.VS", "TwoPass.Fullscreen", "TwoPass.Raycast");
    CubeCenterVbo = CreatePointVbo(0, 0, 0);
    CloudTexture = CreatePyroclasticVolume(128, 0.025f);
    IntervalsFbo[0] = CreateSurface(cfg.Width, cfg.Height);
    IntervalsFbo[1] = CreateSurface(cfg.Width, cfg.Height);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void LoadUniforms()
{
    SetUniform("ModelviewProjection", ModelviewProjection);
    SetUniform("Modelview", ModelviewMatrix);
    SetUniform("ViewMatrix", ViewMatrix);
    SetUniform("ProjectionMatrix", ProjectionMatrix);
    SetUniform("RayStartPoints", 1);
    SetUniform("RayStopPoints", 2);
    SetUniform("EyePosition", EyePosition);

    Vector4 rayOrigin(transpose(ModelviewMatrix) * EyePosition);
    SetUniform("RayOrigin", rayOrigin.getXYZ());

    float focalLength = 1.0f / std::tan(FieldOfView / 2);
    SetUniform("FocalLength", focalLength);

    PezConfig cfg = PezGetConfig();
    SetUniform("WindowSize", float(cfg.Width), float(cfg.Height));
}

void PezRender()
{
    glBindBuffer(GL_ARRAY_BUFFER, CubeCenterVbo);
    glVertexAttribPointer(SlotPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(SlotPosition);
    glBindTexture(GL_TEXTURE_3D, CloudTexture);

    if (SinglePass)
    {
        glClearColor(0.2f, 0.2f, 0.2f, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(Programs.SinglePass);
        LoadUniforms();
        glDrawArrays(GL_POINTS, 0, 1);
    }
    else
    {
        glUseProgram(Programs.TwoPassIntervals);
        LoadUniforms();
        glClearColor(0, 0, 0, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, IntervalsFbo[0].FboHandle);
        glClear(GL_COLOR_BUFFER_BIT);
        glCullFace(GL_FRONT);
        glDrawArrays(GL_POINTS, 0, 1);
        glBindFramebuffer(GL_FRAMEBUFFER, IntervalsFbo[1].FboHandle);
        glClear(GL_COLOR_BUFFER_BIT);
        glCullFace(GL_BACK);
        glDrawArrays(GL_POINTS, 0, 1);

        glUseProgram(Programs.TwoPassRaycast);
        LoadUniforms();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, IntervalsFbo[0].ColorTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, IntervalsFbo[1].ColorTexture);
        glClearColor(0.2f, 0.2f, 0.2f, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void PezUpdate(unsigned int microseconds)
{
    float dt = microseconds * 0.000001f;
    
    Trackball->Update(microseconds);

    EyePosition = Point3(0, 0, 5 + Trackball->GetZoom());
    
    Vector3 up(0, 1, 0); Point3 target(0);
    ViewMatrix = Matrix4::lookAt(EyePosition, target, up);

    Matrix4 modelMatrix(transpose(Trackball->GetRotation()), Vector3(0));
    ModelviewMatrix = ViewMatrix * modelMatrix;

    float n = 1.0f;
    float f = 100.0f;

    ProjectionMatrix = Matrix4::perspective(FieldOfView, 1, n, f);
    ModelviewProjection = ProjectionMatrix * ModelviewMatrix;
}

void PezHandleMouse(int x, int y, int action)
{
    if (action & PEZ_DOWN)
        Trackball->MouseDown(x, y);
    else if (action & PEZ_UP)
        Trackball->MouseUp(x, y);
    else if (action & PEZ_MOVE)
        Trackball->MouseMove(x, y);
    else if (action & PEZ_DOUBLECLICK)
        Trackball->ReturnHome();
}

void PezHandleKey(char c)
{
    if (c == ' ') SinglePass = !SinglePass;
    if (c == '1') FieldOfView += 0.05f;
    if (c == '2') FieldOfView -= 0.05f;
}

static GLuint CreatePyroclasticVolume(int n, float r)
{
    GLuint handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_3D, handle);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unsigned char *data = new unsigned char[n*n*n];
    unsigned char *ptr = data;

    float frequency = 3.0f / n;
    float center = n / 2.0f + 0.5f;

    for(int x=0; x < n; ++x) {
        for (int y=0; y < n; ++y) {
            for (int z=0; z < n; ++z) {
                float dx = center-x;
                float dy = center-y;
                float dz = center-z;

                float off = fabsf((float) PerlinNoise3D(
                    x*frequency,
                    y*frequency,
                    z*frequency,
                    5,
                    6, 3));

                float d = sqrtf(dx*dx+dy*dy+dz*dz)/(n);
                bool isFilled = (d-off) < r;
                *ptr++ = isFilled ? 255 : 0;
            }
        }
        PezDebugString("Slice %d of %d\n", x, n);
    }

    glTexImage3D(GL_TEXTURE_3D, 0,
                 GL_LUMINANCE,
                 n, n, n, 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 data);

    delete[] data;
    return handle;
}
