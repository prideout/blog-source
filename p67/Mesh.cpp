#include <string>
#include <openctm.h>
#include "Common.hpp"

using std::string;

MeshPod CreateQuad(float left, float top, float right, float bottom)
{
    MeshPod pod = {0};
    pod.VertexCount = 4;
    pod.IndexCount = 6;

    float positions[] = {
        left, top, 0,
        right, top, 0,
        right, bottom, 0,
        left, bottom, 0,
    };

    glGenBuffers(1, &pod.PositionsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, pod.PositionsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    float texcoords[] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1,
    };

    glGenBuffers(1, &pod.TexCoordsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, pod.TexCoordsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);

    int faces[] = { 3, 2, 1, 1, 0, 3 };

    glGenBuffers(1, &pod.IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pod.IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(faces), faces, GL_STATIC_DRAW);

    return pod;
}

MeshPod CreateQuad()
{
    return CreateQuad(-1, +1, +1, -1);
}

void RenderMesh(MeshPod mesh)
{
    PezCheckCondition(glGetError() == GL_NO_ERROR, "OpenGL error.");
    PezCheckCondition(mesh.IndexBuffer != 0 && mesh.PositionsBuffer != 0, "Invalid mesh.");

    glBindBuffer(GL_ARRAY_BUFFER, mesh.PositionsBuffer);
    glVertexAttribPointer(SlotPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(SlotPosition);

    if (mesh.NormalsBuffer) {
        glBindBuffer(GL_ARRAY_BUFFER, mesh.NormalsBuffer);
        glVertexAttribPointer(SlotNormal, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glEnableVertexAttribArray(SlotNormal);
    }

    if (mesh.TexCoordsBuffer) {
        glBindBuffer(GL_ARRAY_BUFFER, mesh.TexCoordsBuffer);
        glVertexAttribPointer(SlotTexCoord, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
        glEnableVertexAttribArray(SlotTexCoord);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.IndexBuffer);
    glDrawElements(GL_TRIANGLES, mesh.IndexCount, GL_UNSIGNED_INT, 0);
    PezCheckCondition(glGetError() == GL_NO_ERROR, "OpenGL error.");

    glDisableVertexAttribArray(SlotPosition);
    glDisableVertexAttribArray(SlotNormal);
    glDisableVertexAttribArray(SlotTexCoord);
}

MeshPod LoadMesh(const char* path)
{
    string fullpath;
    if (path[1] == ':' || path[0] == '/') {
        fullpath = path;
    } else {
        fullpath = string(PezGetAssetsFolder());
        char trailingChar = fullpath[fullpath.size() - 1];
        if (trailingChar != '/' && trailingChar != '\\')
        {
            fullpath = fullpath + '/';
        }
        fullpath = fullpath + path;
    }

    // Open the CTM file:
    CTMcontext ctmContext = ctmNewContext(CTM_IMPORT);
    ctmLoad(ctmContext, fullpath.c_str());
    PezCheckCondition(ctmGetError(ctmContext) == CTM_NONE, "Unable to load OpenCTM file: %s\n", fullpath.c_str());

    MeshPod pod = {0};
    pod.VertexCount = ctmGetInteger(ctmContext, CTM_VERTEX_COUNT);
    pod.IndexCount = 3 * ctmGetInteger(ctmContext, CTM_TRIANGLE_COUNT);

    // Create the VBO for positions:
    const CTMfloat* positions = ctmGetFloatArray(ctmContext, CTM_VERTICES);
    GLsizeiptr size = pod.VertexCount * sizeof(float) * 3;
    glGenBuffers(1, &pod.PositionsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, pod.PositionsBuffer);
    glBufferData(GL_ARRAY_BUFFER, size, positions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(SlotPosition);
    glVertexAttribPointer(SlotPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

    // Create the VBO for normals:
    const CTMfloat* normals = ctmGetFloatArray(ctmContext, CTM_NORMALS);
    if (normals) {
        GLsizeiptr size = pod.VertexCount * sizeof(float) * 3;
        glGenBuffers(1, &pod.NormalsBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, pod.NormalsBuffer);
        glBufferData(GL_ARRAY_BUFFER, size, normals, GL_STATIC_DRAW);
        glEnableVertexAttribArray(SlotNormal);
        glVertexAttribPointer(SlotNormal, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    }

    // Create the VBO for texcoords:
    const CTMfloat* texcoords = ctmGetFloatArray(ctmContext, CTM_UV_MAP_1);
    if (texcoords) {
        GLsizeiptr size = pod.VertexCount * sizeof(float) * 2;
        glGenBuffers(1, &pod.TexCoordsBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, pod.TexCoordsBuffer);
        glBufferData(GL_ARRAY_BUFFER, size, texcoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(SlotTexCoord);
        glVertexAttribPointer(SlotTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
    }

    // Create the VBO for indices:
    const CTMuint* indices = ctmGetIntegerArray(ctmContext, CTM_INDICES);
    size = pod.IndexCount * sizeof(CTMuint);
    glGenBuffers(1, &pod.IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pod.IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);

    ctmFreeContext(ctmContext);

    return pod;
}
