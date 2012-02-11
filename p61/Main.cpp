#include <pez.h>
#include <string>
#include "Common.hpp"

using namespace vmath;
using std::string;

static enum Mode { Particle, Tube, Glow, Billboard } CurrentMode = Particle;
static ITrackball* ViewTrackball;
static ITrackball* ModelTrackball;
static GLuint TubeProgram;
static GLuint TronProgram;
static GLuint OverlayProgram;
static GLuint ParticleProgram;
static GLuint BillboardProgram;
static GLuint TextProgram;
static Texture TronGradient;
static Texture FireGradient;
static Matrix4 ProjectionMatrix;
static Matrix4 ViewMatrix;
static Matrix4 ModelviewProjection;
static Curve HilbertCurve;
static Surface ParticleSurface;
static GLuint Fullscreen;
static float Time = 0;

float fract(float f) { return f - floor(f); }
float sign(float f) { return f < 0.0f ? -1.0f : (f > 0.0f ? 1.0f : 0.0f); }

PezConfig PezGetConfig()
{
    PezConfig config;
    config.Title = "Tron";
    config.Width = 800;
    config.Height = 800;
    config.Multisampling = 1;
    config.VerticalSync = 0;
    return config;
}

void PezInitialize()
{
    string prefix (PezGetAssetsFolder());
    if (prefix[prefix.size() - 1] != '/')
        prefix = prefix + "/";

    TronGradient = LoadTexture(prefix + "Tron.dds");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    FireGradient = LoadTexture(prefix + "Fire.dds");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    PezConfig cfg = PezGetConfig();
    ParticleSurface = CreateSurface(cfg.Width, cfg.Height, 4);
    Fullscreen = CreateQuad();
    HilbertCurve = CreateHilbertCurve(16, true);

    ViewTrackball = CreateTrackball(cfg.Width * 1.0f, cfg.Height * 1.0f, cfg.Width * 0.5f);
    ModelTrackball = CreateTrackball(cfg.Width * 1.0f, cfg.Height * 1.0f, cfg.Width * 0.5f);

    TextProgram = LoadProgram("Common.Overlay.VS", 0, "Common.Text.FS", GL_TRIANGLE_STRIP);
    SetUniform("InverseSize", 1.0f / cfg.Width, 1.0f / cfg.Height);

    TubeProgram = LoadProgram("Common.LineStrip.VS", "Cylinder.GS", "Cylinder.FS", GL_TRIANGLE_STRIP);
    SetUniform("DiffuseMaterial", Vector3(1.0f, 0.5f, 0.125f));
    SetUniform("AmbientMaterial", Vector3(0.125f, 0.125f, 0.0f));
    SetUniform("SpecularMaterial", Vector3(0.5f, 0.5f, 0.5f));
    SetUniform("Shininess", 50.0f);

    OverlayProgram = LoadProgram("Common.Overlay.VS", 0, "Common.Overlay.FS", GL_TRIANGLE_STRIP);
    SetUniform("InverseSize", 1.0f / cfg.Width, 1.0f / cfg.Height);
    
    ParticleProgram = LoadProgram("Common.LineStrip.VS", "Particle.GS", "Particle.FS", GL_TRIANGLE_STRIP);
    BillboardProgram = LoadProgram("Billboard.VS", "Billboard.GS", "Billboard.FS", GL_LINES);

    TronProgram = LoadProgram("Common.LineStrip.VS", "Glow.GS", "Glow.FS", GL_TRIANGLE_STRIP);

    glClearColor(0, 0, 0, 0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

void PezRender()
{
    Matrix4 modelMatrix ( transpose(ModelTrackball->GetRotation()), Vector3(0, 0, 0) );
    Matrix4 modelView ( ViewMatrix * modelMatrix );
    Vector3 lightPosObjSpace ((0.25f, 0.25f, 1.0f));
    Vector3 lightPosEyeSpace = rowMul(lightPosObjSpace, ViewMatrix.getUpper3x3());
    Vector3 lightDir = -normalize(lightPosEyeSpace);

    if (CurrentMode == Tube) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glUseProgram(TubeProgram);
        SetUniform("LightDirection", lightDir);
        SetUniform("Projection", ProjectionMatrix);
        SetUniform("Modelview", modelView);
        SetUniform("ModelviewProjection", ProjectionMatrix * modelView);
        SetUniform("Radius", 0.05f);
        RenderCurve(HilbertCurve);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, ParticleSurface.FboHandle);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        switch (CurrentMode) {
            case Glow: glUseProgram(TronProgram); break;
            case Particle: glUseProgram(ParticleProgram); break;
            case Billboard: glUseProgram(BillboardProgram); break;
        }
        SetUniform("Time", Time);
        SetUniform("Projection", ProjectionMatrix);

        if (CurrentMode == Billboard)
            SetUniform("Modelview", modelView.getUpper3x3());
        else
            SetUniform("Modelview", modelView);
        
        SetUniform("ModelviewProjection", ProjectionMatrix * modelView);
        SetUniform("Radius", CurrentMode == Tube ? 0.05f : 0.1f);

        glBlendEquation(GL_MAX);
        glBlendFunc(GL_ONE, GL_ONE);
        
        if (CurrentMode == Billboard)
            RenderLines(HilbertCurve, Time);
        else
            RenderCurve(HilbertCurve);

        if (CurrentMode == Particle) {
            SetUniform("Time", Time + 0.25f);
            RenderCurve(HilbertCurve);
            SetUniform("Time", Time + 0.50f);
            RenderCurve(HilbertCurve);
            SetUniform("Time", Time + 0.75f);
            RenderCurve(HilbertCurve);
        }
    
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBlendEquation(GL_FUNC_ADD);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(OverlayProgram);
        SetUniform("GradientSampler", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, CurrentMode == Glow ? TronGradient.Handle : FireGradient.Handle);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ParticleSurface.TextureHandle);
        RenderQuad(Fullscreen);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Draw some text:
    if (true) {
        Texture message;
        switch (CurrentMode) {
            case Particle:  message = OverlayText("Particle Traces"); break;
            case Glow:      message = OverlayText("Glow"); break;
            case Tube:      message = OverlayText("Raytraced Tube"); break;
            case Billboard: message = OverlayText("Motion-Blurred Billboards"); break;
        }
        glUseProgram(TextProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, message.Handle);
        RenderQuad(Fullscreen);
    }
}

void PezUpdate(unsigned int microseconds)
{
    float dt = microseconds * 0.0000001f;
    Time += dt;
    
    // Each display mode is shown for three seconds:
    CurrentMode = (Mode) ((int) (Time * 10.0 / 3.0f) % 4);
    
    ViewTrackball->Update(microseconds);
    ModelTrackball->Update(microseconds);

    PezConfig cfg = PezGetConfig();
    const float W = 0.2f;
    const float H = W * cfg.Height / cfg.Width;
    ProjectionMatrix = Matrix4::frustum(-W, W, -H, H, 2, 500);

    Point3 eye(0, 0, 15);
    Vector3 up(0, 1, 0);
    Point3 target(0, 0, 0);
    Matrix3 spin = ViewTrackball->GetRotation();
    eye = Transform3(spin, Vector3(0,0,0)) * eye;
    up = spin * up;
    ViewMatrix = Matrix4::lookAt(eye, target, up);

    ModelviewProjection = ProjectionMatrix * ViewMatrix;
}

void PezHandleMouse(int x, int y, int action)
{
    ITrackball* pTrackball = (action & PEZ_LEFT) ? ModelTrackball : ViewTrackball;
    if (action & PEZ_DOWN)
        pTrackball->MouseDown(x, y);
    else if (action & PEZ_UP)
        pTrackball->MouseUp(x, y);
    else if (action & PEZ_MOVE)
        pTrackball->MouseMove(x, y);
    else if (action & PEZ_DOUBLECLICK)
        pTrackball->ReturnHome();
}
