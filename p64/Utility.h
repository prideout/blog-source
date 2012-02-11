#pragma once
#include <vector>
#include <vmath.hpp>
#include <pez.h>
#include <glew.h>

enum AttributeSlot {
    SlotPosition,
    SlotTexCoord,
};

struct ITrackball {
    virtual void MouseDown(int x, int y) = 0;
    virtual void MouseUp(int x, int y) = 0;
    virtual void MouseMove(int x, int y) = 0;
    virtual void ReturnHome() = 0;
    virtual vmath::Matrix3 GetRotation() const = 0;
    virtual float GetZoom() const = 0;
    virtual void Update(unsigned int microseconds) = 0;
};

struct TexturePod {
    GLuint Handle;
    GLsizei Width;
    GLsizei Height;
};

struct SurfacePod {
    GLuint FboHandle;
    GLuint ColorTexture;
};

struct SlabPod {
    SurfacePod Ping;
    SurfacePod Pong;
};

ITrackball* CreateTrackball(float width, float height, float radius);
GLuint LoadProgram(const char* vsKey, const char* gsKey, const char* fsKey);
void SetUniform(const char* name, int value);
void SetUniform(const char* name, float value);
void SetUniform(const char* name, float x, float y);
void SetUniform(const char* name, vmath::Matrix4 value);
void SetUniform(const char* name, vmath::Matrix3 value);
void SetUniform(const char* name, vmath::Vector3 value);
void SetUniform(const char* name, vmath::Point3 value);
void SetUniform(const char* name, vmath::Vector4 value);
TexturePod LoadTexture(const char* path);
SurfacePod CreateSurface(int width, int height);
GLuint CreatePointVbo(float x, float y, float z);
