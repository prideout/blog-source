#include <vmath.hpp>
#include <cmath>
#include <vector>
#include <pez.h>
#include "noise.h"
#include "Common.hpp"

using namespace vmath;

static const Point3 SphereCenter(0, 0, 0);
static const float SphereRadius = 1.0f;
static const float Epsilon = 1e-10f;
static const float NoiseLengthScale[] = {0.4f, 0.23f, 0.11f};
static const float NoiseGain[] = {1.0f, 0.5f, 0.25f};
static const float PlumeCeiling(3);
static const float PlumeBase(-3);
static const float PlumeHeight(8);
static const float RingRadius(1.25f);
static const float RingSpeed(0.3f);
static const float RingsPerSecond(0.125f);
static const float RingMagnitude(10);
static const float RingFalloff(0.7f);
static const float ParticlesPerSecond(1000);
static const float SeedRadius(0.125f);
static const float InitialBand(0.1f);
extern bool ShowStreamlines;

static float Time = 0;
static unsigned int Seed(0);

static FlowNoise3 noise;

inline float noise0(Vector3 s) { return noise(s.getX(), s.getY(), s.getZ()); }
inline float noise1(Vector3 s) { return noise(s.getY() + 31.416f, s.getZ() - 47.853f, s.getX() + 12.793f); }
inline float noise2(Vector3 s) { return noise(s.getZ() - 233.145f, s.getX() - 113.408f, s.getY() - 185.31f); }
inline Vector3 noise3d(Vector3 s) { return Vector3(noise0(s), noise1(s), noise2(s)); };

static Vector3 SamplePotential(Point3 p);
static float SampleDistance(Point3 p);
static Vector3 SampleCachedCurl(Point3 p);

static Vector3 ComputeGradient(Point3 p)
{
    const float e = 0.01f;
    Vector3 dx(e, 0, 0);
    Vector3 dy(0, e, 0);
    Vector3 dz(0, 0, e);

    float d =    SampleDistance(p);
    float dfdx = SampleDistance(p + dx) - d;
    float dfdy = SampleDistance(p + dy) - d;
    float dfdz = SampleDistance(p + dz) - d;

    return normalize(Vector3(dfdx, dfdy, dfdz));
}

static Vector3 ComputeCurl(Point3 p)
{
    const float e = 1e-4f;
    Vector3 dx(e, 0, 0);
    Vector3 dy(0, e, 0);
    Vector3 dz(0, 0, e);

    float x = SamplePotential(p + dy)[2] - SamplePotential(p - dy)[2]
            - SamplePotential(p + dz)[1] + SamplePotential(p - dz)[1];

    float y = SamplePotential(p + dz)[0] - SamplePotential(p - dz)[0]
            - SamplePotential(p + dx)[2] + SamplePotential(p - dx)[2];

    float z = SamplePotential(p + dx)[1] - SamplePotential(p - dx)[1]
            - SamplePotential(p + dy)[0] + SamplePotential(p - dy)[0];

    return Vector3(x, y, z) / (2*e);
}

static Vector3 BlendVectors(Vector3 potential, float alpha, Vector3 distanceGradient)
{
    float dp = dot(potential, distanceGradient);
    return alpha * potential + (1-alpha) * dp * distanceGradient;
}

static float SampleDistance(Point3 p)
{
    float phi = p.getY();
    Vector3 u = p - SphereCenter;
    float d = length(u);
    return d - SphereRadius;
}

static Vector3 SamplePotential(Point3 p)
{
   Vector3 psi(0,0,0);
   Vector3 gradient = ComputeGradient(p);

   float obstacleDistance = SampleDistance(p);

   // add turbulence octaves that respect boundaries, increasing upwards
   float height_factor = ramp((p.getY() - PlumeBase) / PlumeHeight);
   for (unsigned int i=0; i < countof(NoiseLengthScale); ++i) {
        Vector3 s = Vector3(p) / NoiseLengthScale[i];
        float d = ramp(std::fabs(obstacleDistance) / NoiseLengthScale[i]);
        Vector3 psi_i = BlendVectors(noise3d(s), d, gradient);
        psi += height_factor*NoiseGain[i]*psi_i;
   }

   Vector3 risingForce = Point3(0, 0, 0) - p;
   risingForce = Vector3(-risingForce[2], 0, risingForce[0]);

   // add rising vortex rings
   float ring_y = PlumeCeiling;
   float d = ramp(std::fabs(obstacleDistance) / RingRadius);
   while (ring_y > PlumeBase) {
      float ry = p.getY() - ring_y;
      float rr = std::sqrt(p.getX()*p.getX()+p.getZ()*p.getZ());
      float rmag = RingMagnitude / (sqr(rr-RingRadius)+sqr(rr+RingRadius)+sqr(ry)+RingFalloff);
      Vector3 rpsi = rmag * risingForce;
      psi += BlendVectors(rpsi, d, gradient);
      ring_y -= RingSpeed / RingsPerSecond;
   }

   return psi;
}

void AdvanceTime(Particle* particleList, int maxParticles, float dt, float timeStep)
{
    Time += dt;
    noise.set_time(0.5f*NoiseGain[0]/NoiseLengthScale[0]*Time);

    // Advect alive particles:
    Particle* pNode = particleList;
    int numAlive = 0;
    for (int i = 0; i < maxParticles; ++i, ++pNode) {
        if (pNode->ToB == 0)
            continue;

        ++numAlive;
        Point3 p(pNode->Px, pNode->Py, pNode->Pz);
        Vector3 v = ComputeCurl(p);
        Point3 midx = p + 0.5f * timeStep * v;
        
        v = ComputeCurl(midx);
        p += timeStep * v;
        
        pNode->Px = p.getX();
        pNode->Py = p.getY();
        pNode->Pz = p.getZ();
        pNode->Vx = v.getX();
        pNode->Vy = v.getY();
        pNode->Vz = v.getZ();
    }

    static float PreviousUpdate = Time;
    if (Time > PreviousUpdate + 0.1f)
    {
        PezDebugString("%d particles\n", numAlive);
        PreviousUpdate = Time;
    }

    // Kill particles that rise far enough:
    pNode = particleList;
    for (int i = 0; i < maxParticles; ++i, ++pNode) {
        if (pNode->ToB != 0 && pNode->Py > PlumeCeiling)
            pNode->ToB = 0;
    }

    // Introduce new particles into the system:
    if (!ShowStreamlines || Time < 0.1f) {
        static float time = 0;
        time += dt;
        int numParticlesToAdd = (int) (time * ParticlesPerSecond);
        if (numParticlesToAdd == 0)
            return;

        time = 0;
        Particle* pNode = particleList;
        for (int i = 0; i < maxParticles && numParticlesToAdd > 0; ++i, ++pNode) {
            if (pNode->ToB != 0)
                continue;

            float theta = randhashf(Seed++, 0, TwoPi);
            float r = randhashf(Seed++, 0, SeedRadius);
            float y = randhashf(Seed++, 0, InitialBand);
            pNode->Px = r*std::cos(theta);
            pNode->Py = PlumeBase + y;
            pNode->Pz = r*std::sin(theta) + 0.125f; // Nudge the emitter towards the viewer ever so slightly
            pNode->ToB = Time;
            numParticlesToAdd--;
        }
    }
}

TexturePod VisualizePotential(GLsizei texWidth, GLsizei texHeight)
{
    std::vector<unsigned char> data(texWidth * texHeight * 3);

    const float W = 2.0f;
    const float H = W * texHeight / texWidth;

    Vector3 minV(100,100,100);
    Vector3 maxV(-100,-100,-100);
    for (GLsizei row = 0; row < texHeight; ++row) {
        for (GLsizei col = 0; col < texWidth; ++col) {
            float x = -W + 2 * W * col / texWidth;
            float y = -H + 2 * H * row / texHeight;
            Point3 p(x, y, 0);
            Vector3 v = SamplePotential(p);
            minV = minPerElem(v, minV);
            maxV = maxPerElem(v, maxV);
        }
    }

    std::vector<unsigned char>::iterator pData = data.begin();
    for (GLsizei row = 0; row < texHeight; ++row) {
        for (GLsizei col = 0; col < texWidth; ++col) {
            unsigned char red = 255;
            unsigned char grn = 0;
            unsigned char blu = 0;

            if (row == 0 || col == 0 || row == texHeight-1 || col == texWidth-1)
                grn = 255;

            float x = -W + 2 * W * col / texWidth;
            float y = -H + 2 * H * row / texHeight;
            Point3 p(x, y, 0);
            Vector3 v = SamplePotential(p);

            v = divPerElem(v - minV, maxV - minV);

            *pData++ = (unsigned char) (v.getX() * 255);
            *pData++ = (unsigned char) (v.getY() * 255);
            *pData++ = (unsigned char) (v.getZ() * 255);
        }
    }

    GLuint handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, &data[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    TexturePod pod;
    pod.Handle = handle;
    pod.Width = texWidth;
    pod.Height = texHeight;
    pod.Depth = 1;

    return pod;
}

static struct
{
    std::vector<float> Data;
    TexturePod Description;
} VelocityCache;

float fract(float f)
{
    return f - floor(f);
}

template<class S,class T>
inline S lerp(const S &value0, const S &value1, T f)
{ return (1-f)*value0 + f*value1; }

static Vector3 SampleCachedCurl(Point3 p)
{
    GLsizei width = VelocityCache.Description.Width;
    GLsizei height = VelocityCache.Description.Height;
    GLsizei depth = VelocityCache.Description.Depth;
    const float W = 2.0f;
    const float H = W * height / width;
    const float D = W;
    float x = p[0];
    float y = p[1];
    float z = p[2];
    x = width * (x + W) / (2 * W);
    y = height * (y + H) / (2 * H);
    z = depth * (z + D) / (2 * D);
    x = std::min(x, float(width-2));
    y = std::min(y, float(height-2));
    z = std::min(z, float(depth-2));
    x = std::max(x, 0.0f);
    y = std::max(y, 0.0f);
    z = std::max(z, 0.0f);

#define V(x,y,z) Vector3( \
    VelocityCache.Data[int(z) * width * height * 3 + int(y) * width * 3 + int(x) * 3 + 0],   \
    VelocityCache.Data[int(z) * width * height * 3 + int(y) * width * 3 + int(x) * 3 + 1],   \
    VelocityCache.Data[int(z) * width * height * 3 + int(y) * width * 3 + int(x) * 3 + 2] )
    Vector3 v000 = V(floor(x), floor(y), floor(z));
    Vector3 v001 = V(floor(x), floor(y), ceil(z));
    Vector3 v010 = V(floor(x), ceil(y), floor(z));
    Vector3 v011 = V(floor(x), ceil(y), ceil(z));
    Vector3 v100 = V(ceil(x), floor(y), floor(z));
    Vector3 v101 = V(ceil(x), floor(y), ceil(z));
    Vector3 v110 = V(ceil(x), ceil(y), floor(z));
    Vector3 v111 = V(ceil(x), ceil(y), ceil(z));
#undef V

    float u = fract(x); float v = fract(y); float w = fract(z);

    Vector3 v00_ = lerp(v000,v001,w);
    Vector3 v01_ = lerp(v010,v011,w);
    Vector3 v10_ = lerp(v100,v101,w);
    Vector3 v11_ = lerp(v110,v111,w);

    Vector3 v0__ = lerp(v00_,v01_,v);
    Vector3 v1__ = lerp(v10_,v11_,v);

    return lerp(v0__,v1__,u);
}

TexturePod CreateVelocityTexture(GLsizei texWidth, GLsizei texHeight, GLsizei texDepth)
{
    VelocityCache.Data.resize(texWidth * texHeight * texDepth * 3);

    const float W = 2.0f;
    const float H = W * texHeight / texWidth;
    const float D = W;

    std::vector<float>::iterator pData = VelocityCache.Data.begin();
    for (GLsizei slice = 0; slice < texDepth; ++slice) {
        for (GLsizei row = 0; row < texHeight; ++row) {
            for (GLsizei col = 0; col < texWidth; ++col) {

                float x = -W + 2 * W * col / texWidth;
                float y = -H + 2 * H * row / texHeight;
                float z = -D + 2 * D * slice / texDepth;

                Point3 p(x, y, z);
                Vector3 v = ComputeCurl(p);

                *pData++ = v[0];
                *pData++ = v[1];
                *pData++ = v[2];
            }
        }
        PezDebugString("%d slices remaining\n", texDepth - 1- slice);
    }

    GLuint handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_3D, handle);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, texWidth, texHeight, texDepth, 0, GL_RGB, GL_FLOAT, &VelocityCache.Data[0]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    VelocityCache.Description.Handle = handle;
    VelocityCache.Description.Width = texWidth;
    VelocityCache.Description.Height = texHeight;
    VelocityCache.Description.Depth = texDepth;
    VelocityCache.Data.clear();

    return VelocityCache.Description;
}

