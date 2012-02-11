#include <pez.h>
#include "Common.hpp"

using namespace vmath;

const bool ShowStreamlines = false;
const bool CpuAdvection = false;
const bool RenderParticles = true;
const bool ShowFrameRate = true;
const float PointSize = ShowStreamlines ? 0.0075f : 0.03f;
static const int ParticleCount = ShowStreamlines ? 1024 : 100000;

static ITrackball* Trackball = 0;
static TexturePod TileFloor;
static TexturePod Sprite;
static MeshPod ScreenQuad;
static MeshPod FloorQuad;
static MeshPod ObstacleMesh;
static GLuint BlitProgram;
static GLuint FloorProgram;
static GLuint ParticleProgram;
static GLuint AdvectProgram;
static GLuint CompositeProgram;
static GLuint ParticleBufferB;
static GLuint ParticleBufferA;
static Matrix4 ProjectionMatrix;
static Matrix4 ViewMatrix;
static Matrix4 ModelviewProjection;
static const float TimeStep = ShowStreamlines ? 1.0f : 5.0f;
static float Time = 0;
static Particle Particles[ParticleCount] = {0};
static SurfacePod BackgroundSurface;
static SurfacePod ParticleSurface;
static TexturePod VelocityTexture;
static float Fips = -4;

static const Vector4 Yellow(1, 1, 0, 0.75f);
static const Vector4 Black(0, 0.05f, 0.1f, 0.85f);

PezConfig PezGetConfig()
{
    PezConfig config;
    config.Title = "Turbulent Particles";
    config.Width = 512;
    config.Height = 1024;
    config.Multisampling = 0;
    config.VerticalSync = 0;
    return config;
}

void Progress(int slice)
{
    PezConfig cfg = PezGetConfig();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    char msg[256];
    sprintf(msg,
        "Generating velocity texture.\n"
        "This only needs to be done once.\n"
        "%d slices remaining.", slice);
    TexturePod message = OverlayText(std::string(msg));
    glUseProgram(BlitProgram);
    SetUniform("Depth", 1.0f);
    SetUniform("ScrollOffset", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, message.Handle);
    RenderMesh(ScreenQuad);
    PezSwapBuffers();
}

void PezInitialize()
{
    PezConfig cfg = PezGetConfig();
    ParticleSurface = CreateSurface(cfg.Width, cfg.Height);
    BackgroundSurface = CreateSurface(cfg.Width, cfg.Height);
    Trackball = CreateTrackball(cfg.Width * 1.0f, cfg.Height * 1.0f, cfg.Width * 3.0f / 4.0f);
    ScreenQuad = CreateQuad();
    FloorQuad = CreateQuad(-10, +10, +10, -10);
    TileFloor = LoadTexture("TileFloor.png");
    Sprite = LoadTexture("Sprite.png");
    ObstacleMesh = LoadMesh("Sphere.ctm");
    AdvectProgram = LoadProgram("Advect.VS", 0, 0);
    BlitProgram = LoadProgram("Blit.VS", 0, "Blit.FS");
    CompositeProgram = LoadProgram("Composite.VS", 0, "Composite.FS");
    FloorProgram = LoadProgram("Floor.VS", 0, "Floor.FS");

    ParticleProgram = LoadProgram("Particle.VS", "Particle.GS", "Particle.FS");
    SetUniform("Color", ShowStreamlines ? Yellow : Black);
    SetUniform("FadeRate", TimeStep * 0.5f);
    SetUniform("DepthSampler", 0);
    SetUniform("SpriteSampler", 1);
    SetUniform("PointSize", PointSize);
    SetUniform("InverseSize", 1.0f / cfg.Width, 1.0f / cfg.Height);

    AdvectProgram = LoadProgram("Advect.VS", 0, 0);
    SetUniform("Size", Vector3(float(VelocityTexture.Width),float(VelocityTexture.Height),float(VelocityTexture.Depth)));
    SetUniform("Extent", Vector3(2,4,2));

    glEnable(GL_DEPTH_TEST);

    // http://http.developer.nvidia.com/GPUGems3/gpugems3_ch23.html
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

    // Create VBO for input on even-numbered frames and output on odd-numbered frames:
    glGenBuffers(1, &ParticleBufferA);
    glBindBuffer(GL_ARRAY_BUFFER, ParticleBufferA);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Particles), &Particles[0].Px, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create VBO for output on even-numbered frames and input on odd-numbered frames:
    glGenBuffers(1, &ParticleBufferB);
    glBindBuffer(GL_ARRAY_BUFFER, ParticleBufferB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Particles), 0, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    VelocityTexture = CreateVelocityTexture(128, 256, 128, Progress);
}

void PezRender()
{
    PezConfig cfg = PezGetConfig();
    glBindFramebuffer(GL_FRAMEBUFFER, BackgroundSurface.Fbo);
    if (ShowStreamlines)
        glClearColor(0,0,0,0);
    else
        glClearColor(0.7f,0.8f,0.8f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the floor:
    glUseProgram(FloorProgram);
    Matrix4 model( Matrix3::rotationX(Pi/2), Vector3(0,-2,0));
    Matrix4 modelview = ViewMatrix * model;
    SetUniform("ModelviewProjection", ProjectionMatrix * modelview);
    SetUniform("ViewMatrix", ViewMatrix.getUpper3x3());
    SetUniform("NormalMatrix", modelview.getUpper3x3());
    if (!ShowStreamlines) {
        glDisable(GL_CULL_FACE);
        glBindTexture(GL_TEXTURE_2D, TileFloor.Handle);
        RenderMesh(FloorQuad);
        glEnable(GL_CULL_FACE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    model = Matrix4::identity();
    modelview = ViewMatrix * model;

    // Move the particles on the GPU:
    if (!CpuAdvection) {

        // Set up the advection shader:
        glUseProgram(AdvectProgram);
        SetUniform("Time", Time);

        // Specify the source buffer:
        glEnable(GL_RASTERIZER_DISCARD);
        glBindBuffer(GL_ARRAY_BUFFER, ParticleBufferA);
        glEnableVertexAttribArray(SlotPosition);
        glEnableVertexAttribArray(SlotBirthTime);
        glEnableVertexAttribArray(SlotVelocity);
        unsigned char* pData = 0;
        glVertexAttribPointer(SlotPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), pData);
        glVertexAttribPointer(SlotBirthTime, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), 12 + pData);
        glVertexAttribPointer(SlotVelocity, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), 16 + pData);

        // Specify the target buffer:
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, ParticleBufferB);

        // Draw it:
        glBeginTransformFeedback(GL_POINTS);
        glBindTexture(GL_TEXTURE_3D, VelocityTexture.Handle);
        glDrawArrays(GL_POINTS, 0, ParticleCount);

        // Restore:
        glEndTransformFeedback();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisableVertexAttribArray(SlotPosition);
        glDisableVertexAttribArray(SlotBirthTime);
        glDisableVertexAttribArray(SlotVelocity);
        glDisable(GL_RASTERIZER_DISCARD);
        std::swap(ParticleBufferA, ParticleBufferB);
    }

    // Render the particles:
    glBindFramebuffer(GL_FRAMEBUFFER, ParticleSurface.Fbo);
    glClearColor(0, 0, 0, 1);

    static bool first = true;
    if (!ShowStreamlines || first) {
        glClear(GL_COLOR_BUFFER_BIT);
        first = false;
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, BackgroundSurface.DepthTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, Sprite.Handle);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glUseProgram(ParticleProgram);
    SetUniform("Time", Time);
    SetUniform("ModelviewProjection", ModelviewProjection);
    SetUniform("Modelview", modelview.getUpper3x3());
    glBindBuffer(GL_ARRAY_BUFFER, ParticleBufferA);
    glEnableVertexAttribArray(SlotPosition);
    glEnableVertexAttribArray(SlotBirthTime);
    glEnableVertexAttribArray(SlotVelocity);
    char* pData = 0;
    glVertexAttribPointer(SlotPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), pData);
    glVertexAttribPointer(SlotBirthTime, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), 12 + pData);
    glVertexAttribPointer(SlotVelocity, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), 16 + pData);
    if (RenderParticles) {
        glDrawArrays(GL_POINTS, 0, ParticleCount);
    }
    glDisableVertexAttribArray(SlotPosition);
    glDisableVertexAttribArray(SlotBirthTime);
    glDisableVertexAttribArray(SlotVelocity);
    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Compose the particle layer with the obstacles layer:
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, BackgroundSurface.ColorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ParticleSurface.ColorTexture);
    glUseProgram(CompositeProgram);
    SetUniform("BackgroundSampler", 0);
    SetUniform("ParticlesSampler", 1);
    RenderMesh(ScreenQuad);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Draw the HUD:
    if (ShowFrameRate) {
        glEnable(GL_BLEND);
        char msg[64]; sprintf(msg, "%03.1f fps\n%d particles\n%s", Fips, ParticleCount, glGetString(GL_RENDERER));
        TexturePod message = OverlayText(std::string(msg));
        glUseProgram(BlitProgram);
        SetUniform("Depth", 1.0f);
        SetUniform("ScrollOffset", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, message.Handle);
        RenderMesh(ScreenQuad);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_BLEND);
    }

    glEnable(GL_DEPTH_TEST);
}

void PezUpdate(unsigned int microseconds)
{
    float dt = microseconds * 0.0000001f;
    Time += dt;
    Trackball->Update(microseconds);

    float fips = 1.0f / (dt*10);
    float alpha = 0.05f;
    if (Fips < 0)
        Fips++;
    else if (Fips == 0)
        Fips = fips;
    else
        Fips = fips * alpha + Fips * (1.0f - alpha);

    PezConfig cfg = PezGetConfig();
    const float W = 0.4f;
    const float H = W * cfg.Height / cfg.Width;
    ProjectionMatrix = Matrix4::frustum(-W, W, -H, H, 2, 500);

    Point3 eye(0, 0, 7);
    Vector3 up(0, 1, 0);
    Point3 target(0, 0, 0);
    Matrix3 spin = Trackball->GetRotation();
    eye = Transform3(spin, Vector3(0,0,0)) * eye;
    up = spin * up;
    ViewMatrix = Matrix4::lookAt(eye, target, up);

    ModelviewProjection = ProjectionMatrix * ViewMatrix;

    glUseProgram(AdvectProgram);
    SetUniform("TimeStep", dt * TimeStep);

    if (CpuAdvection) {
        AdvanceTime(Particles, ParticleCount, dt, dt * TimeStep);
        glBindBuffer(GL_ARRAY_BUFFER, ParticleBufferA);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Particles), &Particles[0].Px, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void PezHandleMouse(int x, int y, int action)
{
    switch (action) {
        case PEZ_DOWN:
            Trackball->MouseDown(x, y);
            break;
        case PEZ_UP:
            Trackball->MouseUp(x, y);
            break;
        case PEZ_MOVE:
            Trackball->MouseMove(x, y);
            break;
        case PEZ_DOUBLECLICK:
            Trackball->ReturnHome();
            break;
    }
}