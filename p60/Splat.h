#include "Pez.h"
#include "VectorMath.h"
#include <vector>

typedef std::vector<VectorMath::Point3> PointList;

struct Surface {
    GLuint FboHandle;
    GLuint TextureHandle[2];
};

static const int PositionSlot = 0;
static const int GridDensity = 16;

GLuint CreateQuad();
GLuint CreatePoints();
GLuint CreateCube();
GLuint CreateProgram(const char* vsKey, const char* gsKey, const char* fsKey);
PointList CreatePathline();
GLuint CreateSplat(GLuint quadVao, PointList positions);
GLuint CreateNoise();
Surface CreateSurface(GLsizei width, GLsizei height, int numComponents, int numTargets = 1);
Surface CreateVolume(GLsizei width, GLsizei height, GLsizei depth);
