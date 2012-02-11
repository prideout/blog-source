#pragma once
#include "Vector.hpp"

template <typename T>
struct Matrix2 {
    Matrix2()
    {
        x.x = 1; x.y = 0;
        y.x = 0; y.y = 1;
    }
    Matrix2(const T* m)
    {
        x.x = m[0]; x.y = m[1];
        y.x = m[2]; y.y = m[3];
    }
    vec2 x;
    vec2 y;
};

template <typename T>
struct Matrix3 {
    Matrix3()
    {
        x.x = 1; x.y = 0; x.z = 0;
        y.x = 0; y.y = 1; y.z = 0;
        z.x = 0; z.y = 0; z.z = 1;
    }
    Matrix3(const T* m)
    {
        x.x = m[0]; x.y = m[1]; x.z = m[2];
        y.x = m[3]; y.y = m[4]; y.z = m[5];
        z.x = m[6]; z.y = m[7]; z.z = m[8];
    }
    Matrix3(vec3 x, vec3 y, vec3 z) : x(x), y(y), z(z)
    {
    }
    Matrix3 Transposed() const
    {
        Matrix3 m;
        m.x.x = x.x; m.x.y = y.x; m.x.z = z.x;
        m.y.x = x.y; m.y.y = y.y; m.y.z = z.y;
        m.z.x = x.z; m.z.y = y.z; m.z.z = z.z;
        return m;
    }
    Vector3<T> operator * (const Vector3<T>& b) const
    {
        Vector3<T> v;
        v.x = x.x * b.x + y.x * b.y + z.x * b.z;
        v.y = x.y * b.x + y.y * b.y + z.y * b.z;
        v.z = x.z * b.x + y.z * b.y + z.z * b.z;
        return v;
    }
    const T* Pointer() const
    {
        return &x.x;
    }
    vec3 x;
    vec3 y;
    vec3 z;
};

template <typename T>
struct Matrix4 {
    Matrix4()
    {
        x = vec4(1, 0, 0, 0);
        y = vec4(0, 1, 0, 0);
        z = vec4(0, 0, 1, 0);
        w = vec4(0, 0, 0, 1);
    }
    Matrix4(const Matrix3<T>& m)
    {
        x.x = m.x.x; x.y = m.x.y; x.z = m.x.z; x.w = 0;
        y.x = m.y.x; y.y = m.y.y; y.z = m.y.z; y.w = 0;
        z.x = m.z.x; z.y = m.z.y; z.z = m.z.z; z.w = 0;
        w.x = 0; w.y = 0; w.z = 0; w.w = 1;
    }
    Matrix4(const T* m)
    {
        x.x = m[0];  x.y = m[1];  x.z = m[2];  x.w = m[3];
        y.x = m[4];  y.y = m[5];  y.z = m[6];  y.w = m[7];
        z.x = m[8];  z.y = m[9];  z.z = m[10]; z.w = m[11];
        w.x = m[12]; w.y = m[13]; w.z = m[14]; w.w = m[15];
    }
    Matrix4 operator * (const Matrix4& b) const
    {
        Matrix4 m;
        const Matrix4& a = *this;
        m.x.x = b.x.x * a.x.x + b.x.y * a.y.x + b.x.z * a.z.x + b.x.w * a.w.x;
        m.x.y = b.x.x * a.x.y + b.x.y * a.y.y + b.x.z * a.z.y + b.x.w * a.w.y;
        m.x.z = b.x.x * a.x.z + b.x.y * a.y.z + b.x.z * a.z.z + b.x.w * a.w.z;
        m.x.w = b.x.x * a.x.w + b.x.y * a.y.w + b.x.z * a.z.w + b.x.w * a.w.w;
        m.y.x = b.y.x * a.x.x + b.y.y * a.y.x + b.y.z * a.z.x + b.y.w * a.w.x;
        m.y.y = b.y.x * a.x.y + b.y.y * a.y.y + b.y.z * a.z.y + b.y.w * a.w.y;
        m.y.z = b.y.x * a.x.z + b.y.y * a.y.z + b.y.z * a.z.z + b.y.w * a.w.z;
        m.y.w = b.y.x * a.x.w + b.y.y * a.y.w + b.y.z * a.z.w + b.y.w * a.w.w;
        m.z.x = b.z.x * a.x.x + b.z.y * a.y.x + b.z.z * a.z.x + b.z.w * a.w.x;
        m.z.y = b.z.x * a.x.y + b.z.y * a.y.y + b.z.z * a.z.y + b.z.w * a.w.y;
        m.z.z = b.z.x * a.x.z + b.z.y * a.y.z + b.z.z * a.z.z + b.z.w * a.w.z;
        m.z.w = b.z.x * a.x.w + b.z.y * a.y.w + b.z.z * a.z.w + b.z.w * a.w.w;
        m.w.x = b.w.x * a.x.x + b.w.y * a.y.x + b.w.z * a.z.x + b.w.w * a.w.x;
        m.w.y = b.w.x * a.x.y + b.w.y * a.y.y + b.w.z * a.z.y + b.w.w * a.w.y;
        m.w.z = b.w.x * a.x.z + b.w.y * a.y.z + b.w.z * a.z.z + b.w.w * a.w.z;
        m.w.w = b.w.x * a.x.w + b.w.y * a.y.w + b.w.z * a.z.w + b.w.w * a.w.w;
        return m;
    }
    Vector4<T> operator * (const Vector4<T>& b) const
    {
        Vector4<T> v;
        v.x = x.x * b.x + y.x * b.y + z.x * b.z + w.x * b.w;
        v.y = x.y * b.x + y.y * b.y + z.y * b.z + w.y * b.w;
        v.z = x.z * b.x + y.z * b.y + z.z * b.z + w.z * b.w;
        v.w = x.w * b.x + y.w * b.y + z.w * b.z + w.w * b.w;
        return v;
    }
    Matrix4& operator *= (const Matrix4& b)
    {
        Matrix4 m = *this * b;
        return (*this = m);
    }
    Matrix4 Transposed() const
    {
        Matrix4 m;
        m.x.x = x.x; m.x.y = y.x; m.x.z = z.x; m.x.w = w.x;
        m.y.x = x.y; m.y.y = y.y; m.y.z = z.y; m.y.w = w.y;
        m.z.x = x.z; m.z.y = y.z; m.z.z = z.z; m.z.w = w.z;
        m.w.x = x.w; m.w.y = y.w; m.w.z = z.w; m.w.w = w.w;
        return m;
    }
    Matrix3<T> ToMat3() const
    {
        Matrix3<T> m;
        m.x.x = x.x; m.y.x = y.x; m.z.x = z.x;
        m.x.y = x.y; m.y.y = y.y; m.z.y = z.y;
        m.x.z = x.z; m.y.z = y.z; m.z.z = z.z;
        return m;
    }
    const T* Pointer() const
    {
        return &x.x;
    }
    static Matrix4<T> Identity()
    {
        return Matrix4();
    }
    static Matrix4<T> Translate(const Vector3<T>& v)
    {
        Matrix4 m;
        m.x.x = 1; m.x.y = 0; m.x.z = 0; m.x.w = 0;
        m.y.x = 0; m.y.y = 1; m.y.z = 0; m.y.w = 0;
        m.z.x = 0; m.z.y = 0; m.z.z = 1; m.z.w = 0;
        m.w.x = v.x; m.w.y = v.y; m.w.z = v.z; m.w.w = 1;
        return m;
    }
    static Matrix4<T> Translate(T x, T y, T z)
    {
        Matrix4 m;
        m.x.x = 1; m.x.y = 0; m.x.z = 0; m.x.w = 0;
        m.y.x = 0; m.y.y = 1; m.y.z = 0; m.y.w = 0;
        m.z.x = 0; m.z.y = 0; m.z.z = 1; m.z.w = 0;
        m.w.x = x; m.w.y = y; m.w.z = z; m.w.w = 1;
        return m;
    }
    static Matrix4<T> Scale(T s)
    {
        Matrix4 m;
        m.x.x = s; m.x.y = 0; m.x.z = 0; m.x.w = 0;
        m.y.x = 0; m.y.y = s; m.y.z = 0; m.y.w = 0;
        m.z.x = 0; m.z.y = 0; m.z.z = s; m.z.w = 0;
        m.w.x = 0; m.w.y = 0; m.w.z = 0; m.w.w = 1;
        return m;
    }
    static Matrix4<T> Scale(T x, T y, T z)
    {
        Matrix4 m;
        m.x.x = x; m.x.y = 0; m.x.z = 0; m.x.w = 0;
        m.y.x = 0; m.y.y = y; m.y.z = 0; m.y.w = 0;
        m.z.x = 0; m.z.y = 0; m.z.z = z; m.z.w = 0;
        m.w.x = 0; m.w.y = 0; m.w.z = 0; m.w.w = 1;
        return m;
    }
    static Matrix4<T> Rotate(T degrees)
    {
        T radians = degrees * 3.14159f / 180.0f;
        T s = std::sin(radians);
        T c = std::cos(radians);
        
        Matrix4 m;
        m.x.x =  c; m.x.y = s; m.x.z = 0; m.x.w = 0;
        m.y.x = -s; m.y.y = c; m.y.z = 0; m.y.w = 0;
        m.z.x =  0; m.z.y = 0; m.z.z = 1; m.z.w = 0;
        m.w.x =  0; m.w.y = 0; m.w.z = 0; m.w.w = 1;
        return m;
    }
    static Matrix4<T> Rotate(T degrees, const vec3& axis)
    {
        T radians = degrees * 3.14159f / 180.0f;
        T s = std::sin(radians);
        T c = std::cos(radians);
        
        Matrix4 m = Identity();
        m.x.x = c + (1 - c) * axis.x * axis.x;
        m.x.y = (1 - c) * axis.x * axis.y - axis.z * s;
        m.x.z = (1 - c) * axis.x * axis.z + axis.y * s;
        m.y.x = (1 - c) * axis.x * axis.y + axis.z * s;
        m.y.y = c + (1 - c) * axis.y * axis.y;
        m.y.z = (1 - c) * axis.y * axis.z - axis.x * s;
        m.z.x = (1 - c) * axis.x * axis.z - axis.y * s;
        m.z.y = (1 - c) * axis.y * axis.z + axis.x * s;
        m.z.z = c + (1 - c) * axis.z * axis.z;
        return m;
    }
    static Matrix4<T> Ortho(T left, T right, T bottom, T top, T hither, T yon)
    {
        T a = 2.0f / (right - left);
        T b = 2.0f / (top - bottom);
        T c = -2.0f / (yon - hither);
        T tx = (right + left) / (right - left);
        T ty = (top + bottom) / (top - bottom);
        T tz = (yon + hither) / (yon - hither);
        Matrix4 m;
        m.x.x = a; m.x.y = 0; m.x.z = 0; m.x.w = -tx;
        m.y.x = 0; m.y.y = b; m.y.z = 0; m.y.w = -ty;
        m.z.x = 0; m.z.y = 0; m.z.z = c; m.z.w = -tz;
        m.w.x = 0; m.w.y = 0; m.w.z = 0; m.w.w = 1;
        return m.Transposed();
    }
    static Matrix4<T> Frustum(T left, T right, T bottom, T top, T hither, T yon)
    {
        T a = 2 * hither / (right - left);
        T b = 2 * hither / (top - bottom);
        T c = (right + left) / (right - left);
        T d = (top + bottom) / (top - bottom);
        T e = - (yon + hither) / (yon - hither);
        T f = -2 * yon * hither / (yon - hither);
        Matrix4 m;
        m.x.x = a; m.x.y = 0; m.x.z = 0; m.x.w = 0;
        m.y.x = 0; m.y.y = b; m.y.z = 0; m.y.w = 0;
        m.z.x = c; m.z.y = d; m.z.z = e; m.z.w = -1;
        m.w.x = 0; m.w.y = 0; m.w.z = f; m.w.w = 1;
        return m;
    }
    static Matrix4<T> LookAt(const Vector3<T>& eye,
                             const Vector3<T>& target,
                             const Vector3<T>& up)
    {
        Vector3<T> z = (eye - target).Normalized();
        Vector3<T> x = up.Cross(z).Normalized();
        Vector3<T> y = z.Cross(x).Normalized();
        
        Matrix4<T> m;
        m.x = Vector4<T>(x, 0);
        m.y = Vector4<T>(y, 0);
        m.z = Vector4<T>(z, 0);
        m.w = Vector4<T>(0, 0, 0, 1);
        
        Vector4<T> eyePrime = m * Vector4<T>(-eye, 1);
        m = m.Transposed();
        m.w = eyePrime;
        
        return m;
    }
    std::array<T, 4> 
    vec4 x;
    vec4 y;
    vec4 z;
    vec4 w;
};

typedef Matrix2<float> mat2;
typedef Matrix3<float> mat3;
typedef Matrix4<float> mat4;
