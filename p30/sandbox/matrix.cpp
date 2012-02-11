#include <algorithm>
#include <array>
#include <vector>
#include <iostream>
#include <ctime>

template <typename T>
struct StaticMatrix
{
    std::array<T, 16> M;

    StaticMatrix() { }
    StaticMatrix(T* m) { std::copy(m, m + 16, M.begin()); }
    StaticMatrix(const StaticMatrix<T>& other) { M = other.M; }
    const T operator() (int i, int j) const { return M[i * 4 + j]; }
    T& operator() (int i, int j) { return M[i * 4 + j]; }

    StaticMatrix<T> Transposed() const
    {
        StaticMatrix<T> other;
        for (int i = 0; i < 16; ++i)
        {
            int r = i / 4;
            int c = i % 4;
            other.M[r * 4 + c] = M[c * 4 + r];
        }
        return other;
    }

	StaticMatrix<T> Translated(T x, T y, T z)
    {
		M[12] += x;
		M[13] += y;
		M[14] += z;
        return *this;
    }

	StaticMatrix<T> operator + (const StaticMatrix& other)
    {
        StaticMatrix<T> sum;
        for (int i = 0; i < 16; ++i)
            sum.M[i] = M[i] + other.M[i];
        return sum;
    }

    static StaticMatrix<T> Identity()
    {
        StaticMatrix<T> m;
        for (int i = 0; i < 16; ++i)
            m.M[i] = (i / 4) == (i % 4) ? (T) 1 : (T) 0;
        return m;
    }

    static StaticMatrix<T> Random()
    {
        StaticMatrix<T> m;
        for (int i = 0; i < 16; ++i)
            m.M[i] = (T) rand() / (T) RAND_MAX;
        return m;
    }
};

template <typename T>
struct HeapMatrix
{
    std::vector<T> M;
    HeapMatrix() : M(16) { }

    HeapMatrix(const HeapMatrix<T>& other)
    {
        M = other.M;
    }

    HeapMatrix(T* m) : M(16) { std::copy(m, m + 16, M.begin()); }
    const T operator() (int i, int j) const { return M[i * 4 + j]; }
    T& operator() (int i, int j) { return M[i * 4 + j]; }

    HeapMatrix<T> Transposed() const
    {
        HeapMatrix<T> other;
        for (int i = 0; i < 16; ++i)
        {
            int r = i / 4;
            int c = i % 4;
            other.M[r * 4 + c] = M[c * 4 + r];
        }
        return other;
    }

	HeapMatrix<T> Translated(T x, T y, T z)
    {
		M[12] += x;
		M[13] += y;
		M[14] += z;
        return *this;
    }

    HeapMatrix<T> operator + (const HeapMatrix& other)
    {
        HeapMatrix<T> sum;
        for (int i = 0; i < 16; ++i)
            sum.M[i] = M[i] + other.M[i];
        return sum;
    }

    static HeapMatrix<T> Identity()
    {
        HeapMatrix<T> m;
        for (int i = 0; i < 16; ++i)
            m.M[i] = (i / 4) == (i % 4) ? (T) 1 : (T) 0;
        return m;
    }

    static HeapMatrix<T> Random()
    {
        HeapMatrix<T> m;
        for (int i = 0; i < 16; ++i)
            m.M[i] = (T) rand() / (T) RAND_MAX;
        return m;
    }
};

template <typename T>
struct MoveableHeapMatrix
{
    std::vector<T> M;
    MoveableHeapMatrix() : M(16) { }

    // Here's C++0x in action: a move constructor!
    MoveableHeapMatrix<T>(MoveableHeapMatrix<T>&& other)
    {
        M = std::move(other.M);
    }

    MoveableHeapMatrix(const MoveableHeapMatrix<T>& other)
    {
        M = other.M;
    }

    MoveableHeapMatrix(T* m) : M(16) { std::copy(m, m + 16, M.begin()); }
    const T operator() (int i, int j) const { return M[i * 4 + j]; }
    T& operator() (int i, int j) { return M[i * 4 + j]; }

    MoveableHeapMatrix<T> Transposed() const
    {
        MoveableHeapMatrix<T> other;
        for (int i = 0; i < 16; ++i)
        {
            int r = i / 4;
            int c = i % 4;
            other.M[r * 4 + c] = M[c * 4 + r];
        }
        return other;
    }

	MoveableHeapMatrix<T> Translated(T x, T y, T z)
    {
		M[12] += x;
		M[13] += y;
		M[14] += z;
        return *this;
    }

    MoveableHeapMatrix<T> operator + (const MoveableHeapMatrix& other)
    {
        MoveableHeapMatrix<T> sum;
        for (int i = 0; i < 16; ++i)
            sum.M[i] = M[i] + other.M[i];
        return sum;
    }

    static MoveableHeapMatrix<T> Identity()
    {
        MoveableHeapMatrix<T> m;
        for (int i = 0; i < 16; ++i)
            m.M[i] = (i / 4) == (i % 4) ? (T) 1 : (T) 0;
        return m;
    }

    static MoveableHeapMatrix<T> Random()
    {
        MoveableHeapMatrix<T> m;
        for (int i = 0; i < 16; ++i)
            m.M[i] = (T) rand() / (T) RAND_MAX;
        return m;
    }
};

template <typename MATRIX>
void test1()
{
    MATRIX a = MATRIX::Random();
    MATRIX b = MATRIX::Random();
    MATRIX c = MATRIX::Identity();
    for (int count = 0; count < 5; ++count) {
        clock_t start = clock();
        for (int j = 0; j < 10000; ++j) {
            MATRIX i = MATRIX::Identity();
            MATRIX d = a + b + i;
            c = c + d;
        }
        clock_t finish = clock();
        std::cout << finish - start << " milliseconds" << std::endl;
    }
    std::cout << std::endl;
}

template <typename MATRIX>
void test2()
{
    MATRIX a = MATRIX::Random();
    for (int count = 0; count < 5; ++count) {
        clock_t start = clock();
        for (int j = 0; j < 5000; ++j) {
            a.Transposed().Translated(1, 2, 3).Transposed().Translated(3, 2, 1).Transposed().Translated(8, 9, 10)
             .Transposed().Translated(10, 9, 8).Transposed().Translated(4, 5, 6).Transposed().Translated(7, 9, 8);
        }
        clock_t finish = clock();
        std::cout << finish - start << " milliseconds" << std::endl;
    }
    std::cout << std::endl;
}

void main() 
{
    std::cout << "Static Matrix (uses std::array)" << std::endl;
    std::cout << "-------------------------------" << std::endl;
    test2<StaticMatrix<float>>();

    std::cout << "Heap Matrix with C++98 (uses std::vector)" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;
    test2<HeapMatrix<float>>();

    std::cout << "Heap Matrix with C++0x (uses std::vector and std::move)" << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;
    test2<MoveableHeapMatrix<float>>();
}
