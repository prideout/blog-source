#include <vmath.hpp>
#include <cmath>
#include <pez.h>
#include "Common.hpp"

using namespace vmath;

typedef std::vector<Point3> PointList;
static PointList HilbertPath;
static float HilbertSegmentLength = 0.05f;

struct Turtle {
    void Move(float dx, float dy, bool changeFace = false)
    {
        if (changeFace) ++Face;
        switch (Face) {
            case 0: P += Vector3(dx, dy, 0); break;
            case 1: P += Vector3(0, dy, dx); break;
            case 2: P += Vector3(-dy, 0, dx); break;
            case 3: P += Vector3(0, -dy, dx); break;
            case 4: P += Vector3(dy, 0, dx); break;
            case 5: P += Vector3(dy, dx, 0); break;
        }

        HilbertPath.push_back(P);
    }
    Point3 P;
    int Face;
};

static Turtle HilbertTurtle;

static void HilbertU(int level);
static void HilbertD(int level);
static void HilbertC(int level);
static void HilbertA(int level);

static void HilbertU(int level)
{
    if (level == 0) return; float dist = HilbertSegmentLength;
    HilbertD(level-1);      HilbertTurtle.Move(0, -dist);
    HilbertU(level-1);      HilbertTurtle.Move(dist, 0);
    HilbertU(level-1);      HilbertTurtle.Move(0, dist);
    HilbertC(level-1);
}
 
static void HilbertD(int level)
{
    if (level == 0) return; float dist = HilbertSegmentLength;
    HilbertU(level-1);      HilbertTurtle.Move(dist, 0);
    HilbertD(level-1);      HilbertTurtle.Move(0, -dist);
    HilbertD(level-1);      HilbertTurtle.Move(-dist, 0);
    HilbertA(level-1);
}
 
static void HilbertC(int level)
{
    if (level == 0) return; float dist = HilbertSegmentLength;
    HilbertA(level-1);      HilbertTurtle.Move(-dist, 0);
    HilbertC(level-1);      HilbertTurtle.Move(0, dist);
    HilbertC(level-1);      HilbertTurtle.Move(dist, 0);
    HilbertU(level-1);
}
 
static void HilbertA(int level)
{
    if (level == 0) return; float dist = HilbertSegmentLength;
    HilbertC(level-1);      HilbertTurtle.Move(0, dist);
    HilbertA(level-1);      HilbertTurtle.Move(-dist, 0);
    HilbertA(level-1);      HilbertTurtle.Move(0, -dist);
    HilbertD(level-1);
}

Curve CreateCircle(int slices, bool adjacency)
{
    Curve curve;
    curve.Count = slices + (adjacency ? 2 : 0);
    curve.Positions = new float[7 * curve.Count];

    const float radius = 1.0f;
    const float dtheta = TwoPi / float(slices - 1);
    float theta = adjacency ? -dtheta : 0;
    float* p = curve.Positions;
    for (size_t n = 0; n < curve.Count; theta += dtheta, ++n) {
    
        // Position attribute:
        *p++ = radius * std::cos(theta);
        *p++ = radius * std::sin(theta);
        *p++ = 0;
        
        // Normal vector attribute:
        *p++ = 0;
        *p++ = 0;
        *p++ = 1;
        
        // PathCoord attribute:
        *p++ = (float) n / float(curve.Count);
    }

    glGenBuffers(1, &curve.Vbo);
    glBindBuffer(GL_ARRAY_BUFFER, curve.Vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 7 * curve.Count, curve.Positions, GL_STATIC_DRAW);

    return curve;
}

Curve CreateSuperellipse(int slices, float n, float a, float b, bool adjacency)
{
    Curve curve;
    curve.Count = slices + (adjacency ? 2 : 0);
    curve.Positions = new float[7 * curve.Count];

    const float radius = 1.0f;
    const float dtheta = TwoPi / float(slices - 1);
    float theta = adjacency ? -dtheta : 0;
    float* p = curve.Positions;
    for (size_t index = 0; index < curve.Count; theta += dtheta, ++index) {
    
        float c = std::cos(theta);
        float s = std::sin(theta);

        // Position attribute:
        *p++ = std::pow(std::abs(c), 2.0f / n) * a * sign(c);
        *p++ = std::pow(std::abs(s), 2.0f / n) * b * sign(s);
        *p++ = 0;
        
        // Normal vector attribute:
        *p++ = 0;
        *p++ = 0;
        *p++ = 1;
        
        // PathCoord attribute:
        *p++ = (float) index / float(curve.Count);
    }

    glGenBuffers(1, &curve.Vbo);
    glBindBuffer(GL_ARRAY_BUFFER, curve.Vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 7 * curve.Count, curve.Positions, GL_STATIC_DRAW);

    return curve;
}

Curve CreateHilbertCurve(int slices, bool adjacency)
{
    HilbertPath.clear();
    
    const int lod = 3;
    const float dist = HilbertSegmentLength;
    HilbertU(lod); HilbertTurtle.Move(dist, 0, true);
    HilbertU(lod); HilbertTurtle.Move(0, dist, true);
    HilbertC(lod); HilbertTurtle.Move(0, dist, true);
    HilbertC(lod); HilbertTurtle.Move(0, dist, true);
    HilbertC(lod); HilbertTurtle.Move(dist, 0, true);
    HilbertD(lod);

    Point3 minCorner = HilbertPath.front();
    Point3 maxCorner = HilbertPath.front();
    for (PointList::const_iterator i = ++HilbertPath.begin(); i != HilbertPath.end(); ++i) {
        minCorner = minPerElem(*i, minCorner);
        maxCorner = maxPerElem(*i, maxCorner);
    }
    Vector3 offset ( lerp(0.5f, minCorner, maxCorner) );
    for (PointList::iterator i = HilbertPath.begin(); i != HilbertPath.end(); ++i) {
        *i -= offset;
        *i = Point3(1.25f * normalize(Vector3(*i)));
    }

    if (adjacency) {
        Point3 A = *HilbertPath.begin();
        Point3 B = *(++HilbertPath.begin());
        Point3 C = *(--HilbertPath.end());
        Point3 D = *(--(--HilbertPath.end()));
        HilbertPath.insert(HilbertPath.begin(), HilbertPath.front() + (A - B));
        HilbertPath.insert(HilbertPath.begin(), HilbertPath.front() + (A - B));
        HilbertPath.insert(HilbertPath.end(),  HilbertPath.back() + (C - D));
        HilbertPath.insert(HilbertPath.end(),  HilbertPath.back() + (C - D));
    }

    Curve curve;
    curve.Count = HilbertPath.size();
    curve.Positions = new float[7 * curve.Count];

    const float radius = 1.0f;
    const float dtheta = TwoPi / float(slices - 1);
    float theta = adjacency ? -dtheta : 0;
    float* p = curve.Positions;
    PointList::const_iterator pNode = HilbertPath.begin();
    for (size_t n = 0; n < curve.Count; theta += dtheta, ++n, ++pNode) {

        // Position attribute:
        *p++ = pNode->getX();
        *p++ = pNode->getY();
        *p++ = pNode->getZ();

        // Normal vector attribute:
        Vector3 guide( normalize(Vector3(*pNode)) );
        *p++ = guide.getX();
        *p++ = guide.getY();
        *p++ = guide.getZ();
        
        // PathCoord attribute:
        *p++ = (float) n / float(curve.Count);
    }

    glGenBuffers(1, &curve.Vbo);
    glBindBuffer(GL_ARRAY_BUFFER, curve.Vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 7 * curve.Count, curve.Positions, GL_STATIC_DRAW);

    return curve;
}

void RenderCurve(Curve curve)
{
    glBindBuffer(GL_ARRAY_BUFFER, curve.Vbo);
    glEnableVertexAttribArray(PositionSlot);
    glEnableVertexAttribArray(NormalSlot);
    glEnableVertexAttribArray(PathCoordSlot);

    GLsizei stride = sizeof(float) * 7;
    const GLvoid* normalOffset = (GLvoid*) (sizeof(float) * 3);
    const GLvoid* pathCoordOffset = (GLvoid*) (sizeof(float) * 6);

    glVertexAttribPointer(PositionSlot, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glVertexAttribPointer(NormalSlot, 3, GL_FLOAT, GL_FALSE, stride, normalOffset);
    glVertexAttribPointer(PathCoordSlot, 1, GL_FLOAT, GL_FALSE, stride, pathCoordOffset);

    glDrawArrays(GL_LINE_STRIP_ADJACENCY_EXT, 0, curve.Count);

    glDisableVertexAttribArray(PositionSlot);
    glDisableVertexAttribArray(NormalSlot);
    glDisableVertexAttribArray(PathCoordSlot);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RenderLines(Curve curve, float time)
{
    glBindBuffer(GL_ARRAY_BUFFER, curve.Vbo);
    glEnableVertexAttribArray(PositionSlot);
    glEnableVertexAttribArray(NormalSlot);
    glEnableVertexAttribArray(PathCoordSlot);

    GLsizei stride = sizeof(float) * 7;
    const GLvoid* normalOffset = (GLvoid*) (sizeof(float) * 3);
    const GLvoid* pathCoordOffset = (GLvoid*) (sizeof(float) * 6);

    glVertexAttribPointer(PositionSlot, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glVertexAttribPointer(NormalSlot, 3, GL_FLOAT, GL_FALSE, stride, normalOffset);
    glVertexAttribPointer(PathCoordSlot, 1, GL_FLOAT, GL_FALSE, stride, pathCoordOffset);

    for (float offset = 0; offset < 1.0f; offset += 0.25f) {
        float percentage = fmod(offset + time / 4.0f, 1.0f); 
        size_t count = curve.Count - 3;
        float index = floor(percentage * count);
        SetUniform("Time", fract(percentage * count));
        glDrawArrays(GL_LINES, GLint(index), 2);
    }

    glDisableVertexAttribArray(PositionSlot);
    glDisableVertexAttribArray(NormalSlot);
    glDisableVertexAttribArray(PathCoordSlot);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
