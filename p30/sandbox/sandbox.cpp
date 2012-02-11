#include <windows.h> // For GetTickCount
#include <vector>
#include <array>
#include <iostream>
#pragma auto_inline(off)

template <typename T>
struct StaticVector
{
    StaticVector(T x, T y, T z, T w)
    {
        v[0] = x;
        v[1] = y;
        v[2] = z;
        v[3] = w;
    }
    
    StaticVector(const StaticVector<T>& other)
    {
        v = other.v;
    }

    StaticVector<T> Swizzled() const
    {
        return StaticVector<T>(v[3], v[0], v[1], v[2]);
    }

    const T x() { return v[0]; }
    const T y() { return v[1]; }
    const T z() { return v[2]; }
    const T w() { return v[3]; }

    std::array<T, 4> v;
};

template <typename T>
struct HeapVector
{
    HeapVector(T x, T y, T z, T w) : v(4)
    {
        v[0] = x;
        v[1] = y;
        v[2] = z;
        v[3] = w;
    }
    
    HeapVector(const HeapVector<T>& other)
    {
        v = other.v;
    }
    
    HeapVector<T> Swizzled() const
    {
        return HeapVector<T>(v[3], v[0], v[1], v[2]);
    }

#if 1
    HeapVector& operator=(HeapVector&& other)
    {
        if (this != &other)
        {
            v = std::move(other.v);
        }
        return *this;
    }

    HeapVector(HeapVector<T>&& other)
    {
        *this = std::move(other);
    }
#endif

    const T x() { return v[0]; }
    const T y() { return v[1]; }
    const T z() { return v[2]; }
    const T w() { return v[3]; }

    // std::array<T> doesn't have a move constructor because its contents are not heap-allocated.
    // So, we choose to use std::vector<T>

    std::vector<T> v;
};

HeapVector<int> GenerateHeapA()
{
    HeapVector<int> v(1, 2, 3, 4);
    return v;
}

HeapVector<int> GenerateHeapB()
{
    HeapVector<int> v(9, 8, -1, -2);
    return v;
}

StaticVector<int> GenerateStaticA()
{
    StaticVector<int> v(1, 2, 3, 4);
    return v;
}

StaticVector<int> GenerateStaticB()
{
    StaticVector<int> v(9, 8, -1, -2);
    return v;
}

void vectormain()
{
    int tally = 0;
    while (true)
    {
        unsigned int previous = GetTickCount();

        for (int i = 0; i < 100000; i++)
        {
            HeapVector<int> v1 = GenerateHeapA();
            HeapVector<int> v2 = GenerateHeapB();
            tally += v1.Swizzled().x() + v2.Swizzled().w();
        }
        unsigned int current = GetTickCount();

        std::cout << std::endl << current - previous << " milliseconds. " << tally;
    }

    HeapVector<int> v = GenerateHeapA();
    HeapVector<int> a(v);
    std::cout << std::endl << a.x() << ' ' << a.y() << ' ' << a.z() << ' ' << a.w();
    std::cout << std::endl;
}
