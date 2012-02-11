#include "Utility.h"
#include <vectormath.h>
#include <stdlib.h>
#include <math.h>

static Point3 granny_path(float t);
static Vector3 V3Perp(Vector3 v);
static void RedistributePath(const float* pSrc, float* pDest, float length, int width);

Texture CreatePathTexture()
{
    GLuint handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    
    int width = 128;
    int height = 128;
    float* pData = (float*) malloc(sizeof(float) * 3 * width * height);
    float* pDest = pData;

    int row = 0;

    // Ellipse
    {
        float theta = 0;
        const float dtheta = 2 * Pi / (float) width;
        float* pTemp = (float*) malloc(width * 3 * sizeof(float));
        pDest = pTemp;
        float length = 0;

        // Path Centers:
        float px, py;
        for (int slice = 0; slice < width; ++slice, theta += dtheta) {
            float x = 2 * cos(theta);
            float y = 1 * sin(theta);
            *pDest++ = x;
            *pDest++ = 0;
            *pDest++ = y;
            if (slice > 1) {
                length += sqrt((x - px) * (x - px) + (y - py) * (y - py));
            }
            px = x; py = y;
        }

        // Restribute:
        pDest = pData;
        const float* pSrc = pTemp;
        RedistributePath(pSrc, pDest, length, width);
        pDest += width * 3;
        free(pTemp);
    
        // Path Normals:
        for (int slice = 0; slice < width; ++slice) {
            *pDest++ = 0;
            *pDest++ = 1;
            *pDest++ = 0;
        }
        row++;
    }
    
    // Upside-down Pacman Ghost
    {
        float dt = 1.0f / (float) (width - 1);
        float t = 0;
        float length = 0;
        float px, py;
        float* pTemp = (float*) malloc(sizeof(float) * 3 * width);
        float* pStart = pDest;
        pDest = pTemp;
        for (int slice = 0; slice < width; ++slice, t += dt) {
            float tt = (fmod(t, 1.0f) * 8.0f);
            int stage = (int) tt;
            float fraction = tt - (float) stage;
            float x, y;
            switch (stage) {
            case 0:
                x = -6 + fraction * 12.0f;
                y = -7;
                break;
            case 1:
                tt = (1 - fraction) * Pi * 0.5f;
                x = +6 + 6 * cos(tt);
                y = -1 - 6 * sin(tt);
                break;
            case 2:
                x = 12;
                y = -1 + 6 * fraction;
                break;
            case 3:
            case 4:
            case 5:
                fraction = ((stage - 3) + fraction) / 3.0f;
                x = 12 - 24 * fraction;
                y = 5 + 3 * sin(fraction * 4.9f * Pi);
                break;
            case 6:
                x = -12;
                y = 5 - 6 * fraction;
                break;
            case 7:
                tt = fraction * Pi * 0.5f;
                x = -6 - 6 * cos(tt);
                y = -1 - 6 * sin(tt);
                break;
            }
            x /= 4.0f; y /= 5.0;
            *pDest++ = x;
            *pDest++ = 0;
            *pDest++ = y;
            if (slice > 1) {
                length += sqrt((x - px) * (x - px) + (y - py) * (y - py));
            }
            px = x; py = y;
        }

        // Redistribute the path nodes:
        pDest = pStart;
        const float* pSrc = pTemp;
        RedistributePath(pSrc, pDest, length, width);
        free(pTemp);
        pDest += width * 3;
        
        // Orientation vectors:
        dt = 1.0f / (float) (width - 1);
        t = 0;
        for (int slice = 0; slice < width; ++slice, t += dt) {
            float tt = (fmod(t, 1.0f) * 8.0f);
            int stage = (int) tt;
            float fraction = tt - (float) stage;
            float x, y;
            switch (stage) {
            case 0:
                *pDest++ = 0;
                *pDest++ = cos(fraction * Pi);
                *pDest++ = sin(fraction * Pi);
                break;
            case 1: case 2: case 3:
            case 4: case 5: case 6:
                *pDest++ = 0;
                *pDest++ = -1;
                *pDest++ = 0;
                break;
            case 7:
                *pDest++ = 0;
                *pDest++ = cos(Pi + fraction * Pi);
                *pDest++ = sin(Pi + fraction * Pi);
                break;
            }
        }
        row++;
    }

    // Granny Knot
    for (; row < height / 2; row++)
    {
        float initial = (float) rand() / RAND_MAX;
        float scale = 1.0f + (float) rand() / RAND_MAX;

        float ax = -1.0f + 2.0f * (float) rand() / RAND_MAX;
        float ay = -1.0f + 2.0f * (float) rand() / RAND_MAX;
        float az = -1.0f + 2.0f * (float) rand() / RAND_MAX;
        Vector3 axis = V3Normalize(V3MakeFromElems(ax, ay, az));
        float theta = Pi * (float) rand() / RAND_MAX;
        Transform3 rotation = T3MakeRotationAxis(theta, axis);

        float t = initial;
        const float dt = 1.0f / (float) width;
        
        // Path Centers:
        for (int slice = 0; slice < width; ++slice, t += dt) {
            Point3 p = T3MulP3(rotation, granny_path(t));
            *pDest++ = p.x * scale;
            *pDest++ = p.y * scale;
            *pDest++ = p.z * scale;
        }
    
        // Path Normals:
        t = initial;
        for (int slice = 0; slice < width; ++slice, t += dt) {
            Point3 p0 = granny_path(t);
            Point3 p1 = granny_path(t + dt / 2);
            Vector3 a = V3Normalize(P3Sub(p1, p0));
            Vector3 b = V3Normalize(V3Perp(a));
            Vector3 n = T3MulV3(rotation, b);
            *pDest++ = n.x;
            *pDest++ = n.y;
            *pDest++ = n.z;
        }
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, pData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    free(pData);
    
    Texture texture;
    texture.Handle = handle;
    texture.Width = width;
    texture.Height = height;
    return texture;
}

Point3 granny_path(float t)
{
    t = 2 * Pi * t;
    float x = -0.22 * cos(t) - 1.28 * sin(t) - 0.44 * cos(3 * t) - 0.78 * sin(3 * t);
    float y = -0.1 * cos(2 * t) - 0.27 * sin(2 * t) + 0.38 * cos(4 * t) + 0.46 * sin(4 * t);
    float z = 0.7 * cos(3 * t) - 0.4 * sin(3 * t);
    return P3MakeFromElems(x, y, z);
}

Vector3 V3Perp(Vector3 u)
{
    Vector3 v = V3MakeFromElems(0, 0, 1);
    Vector3 u_prime = V3Cross(u, v);
    if (V3LengthSqr(u_prime) < 0.01f) {
        v = V3MakeFromElems(0, 1, 0);
        u_prime = V3Cross(u, v);
        if (V3LengthSqr(u_prime) < 0.01f) {
            v = V3MakeFromElems(1, 0, 0);
            u_prime = V3Cross(u, v);
        }
    }
    return V3Normalize(u_prime);
}

void RedistributePath(const float* pSrc, float* pDest, float length, int width)
{
    int sourceCount = width;
    int destCount = width;
        
    float sourceCursor = 0;
    float destCursor = 0;
    float destSegment = length / (destCount - 1);
    int sourceIndex = 0;
        
    for (int i = 0; i < destCount; i++) {
        
        Point3 s0 = *((Point3*) (pSrc + sourceIndex * 3));
        Point3 s1 = *((Point3*) (pSrc + ((sourceIndex+1) % sourceCount) * 3));

        float nextDestCursor = destCursor + destSegment;
        float sourceSegment = P3Dist(s0, s1);
        float nextSourceCursor = sourceCursor + sourceSegment;
            
        while (nextSourceCursor < nextDestCursor && sourceIndex < sourceCount - 2) {
            sourceCursor = nextSourceCursor;
            sourceIndex = sourceIndex + 1;
                
            s0 = *((Point3*) (pSrc + sourceIndex * 3));
            s1 = *((Point3*) (pSrc + ((sourceIndex+1) % sourceCount) * 3));
            sourceSegment = P3Dist(s0, s1);
                
            nextSourceCursor = sourceCursor + sourceSegment;
        }
            
        float t = (sourceSegment - nextSourceCursor + nextDestCursor) / sourceSegment;
        Point3 d = P3Lerp(t, s0, s1);
        *pDest++ = d.x; *pDest++ = d.y; *pDest++ = d.z;
    
        destCursor = nextDestCursor;
        sourceCursor = nextSourceCursor - sourceSegment;
    }
}
