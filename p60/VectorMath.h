#pragma once
#include <vectormath_aos.h>

namespace VectorMath
{
    using namespace Vectormath::Aos;

    inline Point3 project(Point3 p, Point3 a, Point3 b)
    {
        float t = dot(b - a, p - a) / distSqr(a, b);
        return a + t * (b - a);
    }

    inline Matrix4 PickMatrix(float centerX, float centerY, float width, float height, int viewport[4])
    {
        float sx = viewport[2] / width;
        float sy = viewport[3] / height;
        float tx = (viewport[2] + 2.0f * (viewport[0] - centerX)) / width;
        float ty = (viewport[3] + 2.0f * (viewport[1] - centerY)) / height;

        Vector4 c0(sx, 0, 0, tx);
        Vector4 c1(0, sy, 0, ty);
        Vector4 c2(0, 0, 1, 0);
        Vector4 c3(0, 0, 0, 1);

        return transpose(Matrix4(c0, c1, c2, c3));
    }

    inline Point3 perspective(Vector4 v)
    {
        return Point3(v.getX() / v.getW(), v.getY() / v.getW(), v.getZ() / v.getW());
    }

    inline Vector3 perp(Vector3 a)
    {
        Vector3 c = Vector3(1, 0, 0);
        Vector3 b = cross(a, c);
        if (lengthSqr(b) < 0.01f)
        {
            c = Vector3(0, 1, 0);
            b = cross(a, c);
        }
        return b;
    }

    inline Quat rotate(Quat a, Quat b)
    {
        float w = a.getW() * b.getW() - a.getX() * b.getX() - a.getY() * b.getY() - a.getZ() * b.getZ();
        float x = a.getW() * b.getX() + a.getX() * b.getW() + a.getY() * b.getZ() - a.getZ() * b.getY();
        float y = a.getW() * b.getY() + a.getY() * b.getW() + a.getZ() * b.getX() - a.getX() * b.getZ();
        float z = a.getW() * b.getZ() + a.getZ() * b.getW() + a.getX() * b.getY() - a.getY() * b.getX();
        Quat q(x, y, z, w);
        return normalize(q);
    }
}
