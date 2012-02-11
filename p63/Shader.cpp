#include <string>
#include <glsw.h>
#include "Common.hpp"

using namespace vmath;
using std::string;

GLuint LoadProgram(const char* pV, const char* pG, const char* pF)
{
    GLint compileSuccess, linkSuccess;
    GLchar compilerSpew[256];

    static bool first = true;
    if (first) {
        string prefix (PezGetAssetsFolder());
        if (prefix[prefix.size() - 1] != '/')
            prefix = prefix + "/";
        glswInit();
        glswAddPath(prefix .c_str(), ".glsl");
        first = false;
    }

    string vsString = "Shaders." + string(pV);
    string gsString = pG ? ("Shaders." + string(pG)) : "";
    string fsString = "Shaders." + string(pF);
    const char* vsKey = vsString.c_str();
    const char* gsKey = pG ? gsString.c_str() : 0;
    const char* fsKey = fsString.c_str();

    GLuint programHandle = glCreateProgram();

    const char* vsSource = glswGetShader(vsKey);
    PezCheckCondition(vsSource != 0, "Can't find vertex shader: %s\n", vsKey);
    GLuint vsHandle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsHandle, 1, &vsSource, 0);
    glCompileShader(vsHandle);
    glGetShaderiv(vsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(vsHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, ("Can't compile %s:\n%s", vsKey, compilerSpew));
    glAttachShader(programHandle, vsHandle);

    const char* fsSource = glswGetShader(fsKey);
    PezCheckCondition(fsSource != 0, "Can't find fragment shader: %s\n", fsKey);
    GLuint fsHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsHandle, 1, &fsSource, 0);
    glCompileShader(fsHandle);
    glGetShaderiv(fsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(fsHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, ("Can't compile %s:\n%s", fsKey, compilerSpew));
    glAttachShader(programHandle, fsHandle);

    if (pG) {
        const char* gsSource = glswGetShader(gsKey);
        PezCheckCondition(gsSource != 0, "Can't find geometry shader: %s\n", gsKey);
        GLuint gsHandle = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gsHandle, 1, &gsSource, 0);
        glCompileShader(gsHandle);
        glGetShaderiv(gsHandle, GL_COMPILE_STATUS, &compileSuccess);
        glGetShaderInfoLog(gsHandle, sizeof(compilerSpew), 0, compilerSpew);
        PezCheckCondition(compileSuccess, ("Can't compile %s:\n%s", gsKey, compilerSpew));
        glAttachShader(programHandle, gsHandle);

        glProgramParameteriEXT(programHandle, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
        glProgramParameteriEXT(programHandle, GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS);
        glProgramParameteriEXT(programHandle, GL_GEOMETRY_VERTICES_OUT_EXT, 4);
    }

    glBindAttribLocation(programHandle, SlotPosition, "Position");
    glBindAttribLocation(programHandle, SlotTexCoord, "TexCoord");
    glBindAttribLocation(programHandle, SlotNormal, "Normal");
    glBindAttribLocation(programHandle, SlotBirthTime, "BirthTime");
    glBindAttribLocation(programHandle, SlotVelocity, "Velocity");
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    glGetProgramInfoLog(programHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(linkSuccess, compilerSpew);

    glUseProgram(programHandle);
    return programHandle;
}

void SetUniform(const char* name, int value)
{
    GLuint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &program);
    GLint location = glGetUniformLocation(program, name);
    glUniform1i(location, value);
}

void SetUniform(const char* name, float value)
{
    GLuint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &program);
    GLint location = glGetUniformLocation(program, name);
    glUniform1f(location, value);
}

void SetUniform(const char* name, float x, float y)
{
    GLuint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &program);
    GLint location = glGetUniformLocation(program, name);
    glUniform2f(location, x, y);
}

void SetUniform(const char* name, Matrix4 value)
{
    GLuint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &program);
    GLint location = glGetUniformLocation(program, name);
    glUniformMatrix4fv(location, 1, 0, (float*) &value);
}

void SetUniform(const char* name, Matrix3 nm)
{
    GLuint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &program);
    GLint location = glGetUniformLocation(program, name);
    float packed[9] = {
        nm.getRow(0).getX(), nm.getRow(1).getX(), nm.getRow(2).getX(),
        nm.getRow(0).getY(), nm.getRow(1).getY(), nm.getRow(2).getY(),
        nm.getRow(0).getZ(), nm.getRow(1).getZ(), nm.getRow(2).getZ() };
    glUniformMatrix3fv(location, 1, 0, &packed[0]);
}

void SetUniform(const char* name, Vector3 value)
{
    GLuint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &program);
    GLint location = glGetUniformLocation(program, name);
    glUniform3f(location, value.getX(), value.getY(), value.getZ());
}

void SetUniform(const char* name, Vector4 value)
{
    GLuint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &program);
    GLint location = glGetUniformLocation(program, name);
    glUniform4f(location, value.getX(), value.getY(), value.getZ(), value.getW());
}
