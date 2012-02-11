#pragma once
#include <cmath>
#include <array>

const float Pi = 4 * std::atan(1.0f);
const float TwoPi = 2 * Pi;

template <typename T>
struct Vector2 {
    Vector2() {}
    Vector2(T x, T y) : x(x), y(y) {}
    T Dot(const Vector2& v) const
    {
        return x * v.x + y * v.y;
    }
    Vector2 operator+(const Vector2& v) const
    {
        return Vector2(x + v.x, y + v.y);
    }
    Vector2 operator-(const Vector2& v) const
    {
        return Vector2(x - v.x, y - v.y);
    }
    void operator+=(const Vector2& v)
    {
        *this = Vector2(x + v.x, y + v.y);
    }
    void operator-=(const Vector2& v)
    {
        *this = Vector2(x - v.x, y - v.y);
    }
    Vector2 operator/(float s) const
    {
        return Vector2(x / s, y / s);
    }
    Vector2 operator*(float s) const
    {
        return Vector2(x * s, y * s);
    }
    void operator/=(float s)
    {
        *this = Vector2(x / s, y / s);
    }
    void operator*=(float s)
    {
        *this = Vector2(x * s, y * s);
    }
    void Normalize()
    {
        float s = 1.0f / Length();
        x *= s;
        y *= s;
    }
    Vector2 Normalized() const 
    {
        Vector2 v = *this;
        v.Normalize();
        return v;
    }
    T LengthSquared() const
    {
        return x * x + y * y;
    }
    T Length() const
    {
        return sqrt(LengthSquared());
    }
    const T* Pointer() const
    {
        return &x;
    }
    operator Vector2<float>() const
    {
        return Vector2<float>(x, y);
    }
    bool operator==(const Vector2& v) const
    {
        return x == v.x && y == v.y;
    }
    Vector2 Lerp(float t, const Vector2& v) const
    {
        return Vector2(x * (1 - t) + v.x * t,
                       y * (1 - t) + v.y * t);
    }
    Vector2 Min(const Vector2& v) const
    {
        return Vector2(x < v.x ? x : v.x, y < v.y ? y : v.y);
    }
    Vector2 Max(const Vector2& v) const
    {
        return Vector2(x >= v.x ? x : v.x, y >= v.y ? y : v.y);
    }
    template <typename P>
    P* Write(P* pData)
    {
        Vector2* pVector = (Vector2*) pData;
        *pVector++ = *this;
        return (P*) pVector;
    }
    T x;
    T y;
};

template <typename T>
struct Vector3 {
    Vector3() {}
    Vector3(T x, T y, T z) : x(x), y(y), z(z) {}
    T Length()
    {
        return std::sqrt(x * x + y * y + z * z);
    }
    void Normalize()
    {
        float s = 1.0f / Length();
        x *= s;
        y *= s;
        z *= s;
    }
    Vector3 Normalized() const 
    {
        Vector3 v = *this;
        v.Normalize();
        return v;
    }
    Vector3 Cross(const Vector3& v) const
    {
        return Vector3(y * v.z - z * v.y,
                       z * v.x - x * v.z,
                       x * v.y - y * v.x);
    }
    T Dot(const Vector3& v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }
    Vector3 operator+(const Vector3& v) const
    {
        return Vector3(x + v.x, y + v.y,  z + v.z);
    }
    void operator+=(const Vector3& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
    }
    void operator-=(const Vector3& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
    }
    void operator/=(T s)
    {
        x /= s;
        y /= s;
        z /= s;
    }
    Vector3 operator-(const Vector3& v) const
    {
        return Vector3(x - v.x, y - v.y,  z - v.z);
    }
    Vector3 operator-() const
    {
        return Vector3(-x, -y, -z);
    }
    Vector3 operator*(T s) const
    {
        return Vector3(x * s, y * s, z * s);
    }
    Vector3 operator/(T s) const
    {
        return Vector3(x / s, y / s, z / s);
    }
    bool operator==(const Vector3& v) const
    {
        return x == v.x && y == v.y && z == v.z;
    }
    Vector3 Lerp(float t, const Vector3& v) const
    {
        return Vector3(x * (1 - t) + v.x * t,
                       y * (1 - t) + v.y * t,
                       z * (1 - t) + v.z * t);
    }
    const T* Pointer() const
    {
        return &x;
    }
    template <typename P>
    P* Write(P* pData)
    {
        Vector3<T>* pVector = (Vector3<T>*) pData;
        *pVector++ = *this;
        return (P*) pVector;
    }
    T x;
    T y;
    T z;
};

template <typename T>
struct Vector4 {

    Vector4() {}
    
    Vector4(T x, T y, T z, T w)
    {
        v[0] = x;
        v[1] = y;
        v[2] = z;
        v[3] = w;
    }

    Vector4(const Vector3<T>& v3, T w)
    {
        v[0] = v3.x;
        v[1] = v3.y;
        v[2] = v3.z;
        v[3] = w;
    }

    T Dot(const Vector4<T>& v4) const
    {
        return
            v[0] * v4.v[0] +
            v[1] * v4.v[1] +
            v[2] * v4.v[2] +
            v[3] * v4.v[3];
    }

    Vector4 Lerp(float t, const Vector4& v4) const
    {
        return Vector4(v[0] * (1 - t) + v4.v[0] * t,
                       v[1] * (1 - t) + v4.v[1] * t,
                       v[2] * (1 - t) + v4.v[2] * t,
                       v[3] * (1 - t) + v4.v[3] * t);
    }
    const T* Pointer() const
    {
        return &v[0];
    }

    T x() const { return v[0]; }
    T y() const { return v[1]; }
    T z() const { return v[2]; }
    T w() const { return v[3]; }

    std::array<T, 4> v;
};

typedef Vector2<bool> bvec2;

typedef Vector2<int> ivec2;
typedef Vector3<int> ivec3;
typedef Vector4<int> ivec4;

typedef Vector2<float> vec2;
typedef Vector3<float> vec3;
typedef Vector4<float> vec4;
