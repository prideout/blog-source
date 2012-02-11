#pragma once
#include <vmath.hpp>
#include <pez.h>
#include <glew.h>

struct Particle {
    float Px;  // Position X
    float Py;  // Position Y
    float Pz;  // Position Z
    float ToB; // Time of Birth
    float Vx;  // Velocity X
    float Vy;  // Velocity Y
    float Vz;  // Velocity Z
};

enum AttributeSlot {
    SlotPosition,
    SlotTexCoord,
    SlotNormal,
    SlotBirthTime,
    SlotVelocity,
};

struct ITrackball {
    virtual void MouseDown(int x, int y) = 0;
    virtual void MouseUp(int x, int y) = 0;
    virtual void MouseMove(int x, int y) = 0;
    virtual void ReturnHome() = 0;
    virtual vmath::Matrix3 GetRotation() const = 0;
    virtual void Update(unsigned int microseconds) = 0;
};

struct MeshPod {
    GLuint IndexBuffer;
    GLuint PositionsBuffer;
    GLuint NormalsBuffer;
    GLuint TexCoordsBuffer;
    GLsizei IndexCount;
    GLsizei VertexCount;
};

struct TexturePod {
    GLuint Handle;
    GLsizei Width;
    GLsizei Height;
    GLsizei Depth;
};

struct SurfacePod {
    GLuint Fbo;
    GLuint ColorTexture;
    GLuint DepthTexture;
};

// Trackball.cpp
ITrackball* CreateTrackball(float width, float height, float radius);

// Shader.cpp
GLuint LoadProgram(const char* vsKey, const char* gsKey, const char* fsKey);
void SetUniform(const char* name, int value);
void SetUniform(const char* name, float value);
void SetUniform(const char* name, float x, float y);
void SetUniform(const char* name, vmath::Matrix4 value);
void SetUniform(const char* name, vmath::Matrix3 value);
void SetUniform(const char* name, vmath::Vector3 value);
void SetUniform(const char* name, vmath::Vector4 value);

// Mesh.cpp
MeshPod CreateQuad(float left, float top, float right, float bottom);
MeshPod CreateQuad();
MeshPod LoadMesh(const char* path);
void RenderMesh(MeshPod mesh);

// Texture.cpp
TexturePod LoadTexture(const char* path);
SurfacePod CreateSurface(int width, int height);
TexturePod CreateVelocityTexture(GLsizei texWidth, GLsizei texHeight, GLsizei texDepth);

// Particles.cpp
void AdvanceTime(Particle* particleList, int maxParticles, float dt, float timeStep);
TexturePod VisualizePotential(GLsizei texWidth, GLsizei texHeight);
