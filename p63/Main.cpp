#include <pez.h>
#include <algorithm>
#include "Common.hpp"

using namespace vmath;
using std::sort;

static ITrackball* Trackball = 0;
static TexturePod Background;
static TexturePod Sprite;
static TexturePod Visualization;
static MeshPod ScreenQuad;
static MeshPod ObstacleMesh;
static GLuint BlitProgram;
static GLuint LitProgram;
static GLuint ParticleProgram;
static GLuint CompositeProgram;
static Matrix4 ProjectionMatrix;
static Matrix4 ViewMatrix;
static Matrix4 ModelviewProjection;
bool ShowStreamlines = false;
bool ShowPotential = false;
static const float TimeStep = ShowStreamlines ? 1.0f : 5.0f;
static float Time = 0;
static const int MAX_PARTICLES = 1024;
static Particle Particles[MAX_PARTICLES] = {0};
static SurfacePod BackgroundSurface;
static SurfacePod ParticleSurface;

static const Vector4 Yellow(1, 1, 0, 1);
static const Vector4 Black(0, 0, 0, 1);

PezConfig PezGetConfig()
{
    PezConfig config;
    config.Title = "Turbulent Particles";
    config.Width = 256;
    config.Height = 512;
    config.Multisampling = 0;
    config.VerticalSync = 0;
    return config;
}

void PezInitialize()
{
    PezConfig cfg = PezGetConfig();
    if (ShowPotential)
        Visualization = VisualizePotential(cfg.Width, cfg.Height);
    ParticleSurface = CreateSurface(cfg.Width, cfg.Height);
    BackgroundSurface = CreateSurface(cfg.Width, cfg.Height);
    Trackball = CreateTrackball(cfg.Width * 1.0f, cfg.Height * 1.0f, cfg.Width * 3.0f / 4.0f);
    ScreenQuad = CreateQuad();
    Background = LoadTexture("Scroll.png");
    Sprite = LoadTexture("Sprite.png");
    ObstacleMesh = LoadMesh("Sphere.ctm");
    BlitProgram = LoadProgram("Blit.VS", 0, "Blit.FS");
    LitProgram = LoadProgram("Lit.VS", 0, "Lit.FS");
    CompositeProgram = LoadProgram("Composite.VS", 0, "Composite.FS");
    ParticleProgram = LoadProgram("Particle.VS", "Particle.GS", "Particle.FS");
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // http://http.developer.nvidia.com/GPUGems3/gpugems3_ch23.html
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
}

void PezRender()
{
    PezConfig cfg = PezGetConfig();
    glBindFramebuffer(GL_FRAMEBUFFER, BackgroundSurface.Fbo);

    // Render the background:
    glUseProgram(BlitProgram);
    SetUniform("Depth", 1.0f);
    SetUniform("ScrollOffset", Time);
    glDepthFunc(GL_ALWAYS);
    glBindTexture(GL_TEXTURE_2D, Background.Handle);
    if (ShowStreamlines)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    else
        RenderMesh(ScreenQuad);
    glDepthFunc(GL_LESS);

    // Render the sphere:
    glUseProgram(LitProgram);
    Matrix4 model( Matrix4::identity() );
    Matrix4 modelview = ViewMatrix * model;
    SetUniform("ModelviewProjection", ProjectionMatrix * modelview);
    SetUniform("ViewMatrix", ViewMatrix.getUpper3x3());
    SetUniform("NormalMatrix", modelview.getUpper3x3());
    SetUniform("LightPosition", Vector3(0.25f, 0.25f, 1));
    SetUniform("DiffuseMaterial", Vector3(1.0f, 0.5f, 0.125f));
    SetUniform("AmbientMaterial", Vector3(0.125f, 0.125f, 0.0f));
    SetUniform("SpecularMaterial", Vector3(0.5f, 0.5f, 0.5f));
    SetUniform("Shininess", 50.0f);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (!ShowStreamlines) RenderMesh(ObstacleMesh);

    // Render the particles:
    if (true) {
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
        SetUniform("ModelviewProjection", ModelviewProjection);
        SetUniform("Color", ShowStreamlines ? Yellow : Black);
        SetUniform("FadeRate", TimeStep * 1.0f);
        SetUniform("DepthSampler", 0);
        SetUniform("SpriteSampler", 1);
        SetUniform("Time", Time);
        SetUniform("PointSize", ShowStreamlines ? 0.01f : 0.3f);
        SetUniform("Modelview", modelview.getUpper3x3());
        SetUniform("InverseSize", 1.0f / cfg.Width, 1.0f / cfg.Height);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glEnableVertexAttribArray(SlotPosition);
        glEnableVertexAttribArray(SlotBirthTime);
        glEnableVertexAttribArray(SlotVelocity);
        unsigned char* pData = (unsigned char*) &Particles[0].Px;
        glVertexAttribPointer(SlotPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), pData);
        glVertexAttribPointer(SlotBirthTime, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), 12 + pData);
        glVertexAttribPointer(SlotVelocity, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), 16 + pData);
        if (!ShowPotential)
            glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);
        glDisableVertexAttribArray(SlotPosition);
        glDisableVertexAttribArray(SlotBirthTime);
        glDisableVertexAttribArray(SlotVelocity);
        glDisable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Compose the particle layer with the obstacles layer:
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ShowPotential ? Visualization.Handle : BackgroundSurface.ColorTexture);
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
    glEnable(GL_DEPTH_TEST);
}

void PezUpdate(unsigned int microseconds)
{
    float dt = microseconds * 0.0000001f;
    Time += dt;
    Trackball->Update(microseconds);

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

    AdvanceTime(Particles, MAX_PARTICLES, dt, dt * TimeStep);
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