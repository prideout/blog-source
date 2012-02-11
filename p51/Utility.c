#include "Platform.h"
#include "Utility.h"
#include <glsw.h>
#include <openctm.h>
#include <stdlib.h>
#include <string.h>

Mesh CreateMesh(const char* ctmFile)
{
    Mesh mesh = {0, 0, 0, 0};
    
    char qualifiedPath[256] = {0};
    strcpy(qualifiedPath, PezResourcePath());
    strcat(qualifiedPath, "/\0");
    strcat(qualifiedPath, ctmFile);
    
    // Open the CTM file:
    CTMcontext ctmContext = ctmNewContext(CTM_IMPORT);
    ctmLoad(ctmContext, qualifiedPath);
    PezCheckCondition(ctmGetError(ctmContext) == CTM_NONE, "OpenCTM issue with loading %s", qualifiedPath);
    CTMuint vertexCount = ctmGetInteger(ctmContext, CTM_VERTEX_COUNT);
    CTMuint faceCount = ctmGetInteger(ctmContext, CTM_TRIANGLE_COUNT);
    
    // Create the VBO for positions:
    const CTMfloat* positions = ctmGetFloatArray(ctmContext, CTM_VERTICES);
    if (positions) {
        GLuint handle;
        GLsizeiptr size = vertexCount * sizeof(float) * 3;
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ARRAY_BUFFER, handle);
        glBufferData(GL_ARRAY_BUFFER, size, positions, GL_STATIC_DRAW);
        mesh.Positions = handle;
    }
    
    // Create the VBO for normals:
    const CTMfloat* normals = ctmGetFloatArray(ctmContext, CTM_NORMALS);
    if (normals) {
        GLuint handle;
        GLsizeiptr size = vertexCount * sizeof(float) * 3;
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ARRAY_BUFFER, handle);
        glBufferData(GL_ARRAY_BUFFER, size, normals, GL_STATIC_DRAW);
        mesh.Normals = handle;
    }
    
    // Create the VBO for indices:
    const CTMuint* indices = ctmGetIntegerArray(ctmContext, CTM_INDICES);
    if (indices) {
        
        GLsizeiptr bufferSize = faceCount * 3 * sizeof(unsigned short);
        
        // Convert indices from 32-bit to 16-bit:
        unsigned short* faceBuffer = (unsigned short*) malloc(bufferSize);
        unsigned short* pDest = faceBuffer;
        const CTMuint* pSrc = indices;
        unsigned int remainingFaces = faceCount;
        while (remainingFaces--)
        {
            *pDest++ = (unsigned short) *pSrc++;
            *pDest++ = (unsigned short) *pSrc++;
            *pDest++ = (unsigned short) *pSrc++;
        }
        
        GLuint handle;
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, faceBuffer, GL_STATIC_DRAW);
        mesh.Faces = handle;
        
        free(faceBuffer);
    }
    
    ctmFreeContext(ctmContext);

    mesh.FaceCount = faceCount;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return mesh;
}

GLuint CreateProgram(const char* vsKey, const char* fsKey)
{
    static int first = 1;
    if (first)
    {
        glswInit();
        glswAddPath("../", ".glsl");
        glswAddPath("./", ".glsl");

        char qualifiedPath[128];
        strcpy(qualifiedPath, PezResourcePath());
        strcat(qualifiedPath, "/");
        glswAddPath(qualifiedPath, ".glsl");

        first = 0;
    }
    
    const char* vsSource = glswGetShader(vsKey);
    const char* fsSource = glswGetShader(fsKey);
    const char* msg = "Can't find %s shader: '%s'.\n";
    PezCheckCondition(vsSource != 0, msg, "vertex", vsKey);
    PezCheckCondition(fsSource != 0, msg, "fragment", fsKey);
    
    GLint compileSuccess;
    GLchar compilerSpew[256];

    GLuint vsHandle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsHandle, 1, &vsSource, 0);
    glCompileShader(vsHandle);
    glGetShaderiv(vsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(vsHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, "Can't compile %s:\n%s", vsKey, compilerSpew);
    
    GLuint fsHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsHandle, 1, &fsSource, 0);
    glCompileShader(fsHandle);
    glGetShaderiv(fsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(fsHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, "Can't compile %s:\n%s", fsKey, compilerSpew);
    
    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vsHandle);
    glAttachShader(programHandle, fsHandle);
    glLinkProgram(programHandle);
    
    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    glGetProgramInfoLog(programHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(linkSuccess, "Can't link %s with %s:\n%s", vsKey, fsKey, compilerSpew);
    
    return programHandle;
}

GLuint CreateQuad(float left, float bottom, float right, float top)
{
    float quad[] = {
        left, bottom,
        left, top,
        right, top,
        right, top,
        right, bottom,
        left, bottom,
    };

    GLuint handle;
    glGenBuffers(1, &handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return handle;
}
