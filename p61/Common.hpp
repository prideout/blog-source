#pragma once
#include <vector>
#include <vmath.hpp>
#include <pez.h>
#include <glew.h>

enum { PositionSlot, NormalSlot, PathCoordSlot };

struct Curve {
    GLuint Vbo;
    float* Positions;
    size_t Count;
};

struct Surface {
    GLuint FboHandle;
    GLuint TextureHandle;
};

struct Texture {
    GLsizei Width;
    GLsizei Height;
    GLuint Handle;
    GLenum Format;
};

struct ITrackball {
    virtual void MouseDown(int x, int y) = 0;
    virtual void MouseUp(int x, int y) = 0;
    virtual void MouseMove(int x, int y) = 0;
    virtual void ReturnHome() = 0;
    virtual vmath::Matrix3 GetRotation() const = 0;
    virtual void Update(unsigned int microseconds) = 0;
};

// Trackball.cpp
ITrackball* CreateTrackball(float width, float height, float radius);

// Shader.cpp
GLuint LoadProgram(const char* vsKey, const char* gsKey, const char* fsKey, GLenum prim);
void SetUniform(const char* name, int value);
void SetUniform(const char* name, float value);
void SetUniform(const char* name, float x, float y);
void SetUniform(const char* name, vmath::Matrix4 value);
void SetUniform(const char* name, vmath::Matrix3 value);
void SetUniform(const char* name, vmath::Vector3 value);
void SetUniform(const char* name, vmath::Vector4 value);

// Curve.cpp
Curve CreateHilbertCurve(int slices, bool adjacency);
Curve CreateCircle(int slices, bool adjacency);
Curve CreateSuperellipse(int slices, float n, float a, float b, bool adjacency);
void RenderCurve(Curve c);
void RenderLines(Curve c, float time);

// CreateSurface.cpp
Surface CreateSurface(GLsizei width, GLsizei height, int numComponents);

// CreateQuad.cpp
GLuint CreateQuad();
void RenderQuad(GLuint vbo);

// Text.cpp
Texture OverlayText(std::string message);

// Texture.cpp
Texture LoadTexture(std::string ddsFilename);

// Main.cpp
float fract(float f);
float sign(float f);
