#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include <cmath>

#define EPSILON 0.0001
#define EPSILON2 (EPSILON * EPSILON)

namespace FastMath
{

static constexpr double PI = 3.14159265358979323846;

template<typename T>
inline bool IsNearly(const T& a, const T& comp, T EPS = EPSILON)
{
    return std::abs(a - comp) < EPS;
}

inline double DegreesToRadians(double degrees)
{
    return degrees * (PI / 180.0);
}

// ################################################################
// ### Useful TMP Functions
// ################################################################

// Multiply all elements of the object by a value
template<class Type, int N>
struct MultElements
{
    static inline void eval(Type& obj, const typename Type::value_type& mult)
    {
        obj[N] *= mult;
        MultElements < Type, N - 1 >::eval(obj, mult);
    }
};


// Multiply and add the two arrays like so:
//    a[0]*b[0] + a[1]*b[1] + ...
template<class Type, size_t N, size_t I>
struct MultArrays
{
    static_assert(N > 0, "Array must have size > 0");
    static inline Type eval(const Type(&a)[N], const Type(&b)[N])
    {
        return a[I] * b[I] + MultArrays < Type, N, I + 1 >::eval(a, b);
    }
};
// Specialize for 4D, 3D and 2D
template<> struct MultArrays<double, 4, 4>
{
    static inline double eval(const double (&)[4], const double (&)[4])
    {
        return 0.0;
    }
};
template<> struct MultArrays<double, 3, 3>
{
    static inline double eval(const double (&)[3], const double (&)[3])
    {
        return 0.0;
    }
};
template<> struct MultArrays<double, 2, 2>
{
    static inline double eval(const double (&)[2], const double (&)[2])
    {
        return 0.0;
    }
};


// Multiply every element of the array by the scalar
template<class Type, size_t N, size_t I>
struct MultArrayScalar
{
    static_assert(N > 0, "Array must have size > 0");
    static inline void eval(Type(&a)[N], const Type&& scalar)
    {
        a[I] *= std::forward<decltype(scalar)>(scalar);
        MultArrayScalar < Type, N, I + 1 >::eval(a, std::forward<decltype(scalar)>(scalar));
    }
};
// Specialize for 4D, 3D and 2D
template<> struct MultArrayScalar<double, 4, 4>
{
    static inline void eval(const double (&)[4], const double&&) {}
};
template<> struct MultArrayScalar<double, 3, 3>
{
    static inline void eval(const double (&)[3], const double&&) {}
};
template<> struct MultArrayScalar<double, 2, 2>
{
    static inline void eval(const double (&)[2], const double&&) {}
};



// Assign one array's contents to anothers
template<class Type, size_t N, size_t I>
inline void FastArrayCopy(Type(&to)[N], const Type(&from)[N])
{
    static_assert(N > 0, "Array must have size > 0");

    to[I] = from[I];
    FastArrayCopy < Type, N, I + 1 > (to, from);
}
// Specialize for 4D, 3D and 2D
template<>
inline void FastArrayCopy<double, 16, 16>(double(&)[16], const double(&)[16]) {}

template<>
inline void FastArrayCopy<double, 4, 4>(double(&)[4], const double(&)[4]) {}

template<>
inline void FastArrayCopy<double, 3, 3>(double(&)[3], const double(&)[3]) {}

template<>
inline void FastArrayCopy<double, 2, 2>(double(&)[2], const double(&)[2]) {}



// Assign one object with operator[] defined to another
template<class Type, size_t N, size_t I>
inline void FastArrayAssign(Type(&array)[N], const Type&& value)
{
    static_assert(N > 0, "Array must have size > 0");

    array[I] = std::forward<decltype(value)>(value);
    FastArrayAssign < Type, N, I + 1 > (array, std::forward<decltype(value)>(value));
}
// Specialize for 4D, 3D and 2D
template<>
inline void FastArrayAssign<double, 16, 16>(double(&)[16], const double&&) {}

template<>
inline void FastArrayAssign<double, 4, 4>(double(&)[4], const double&&) {}

template<>
inline void FastArrayAssign<double, 3, 3>(double(&)[3], const double&&) {}

template<>
inline void FastArrayAssign<double, 2, 2>(double(&)[2], const double&&) {}



// ################################################################
// ### Points
// ################################################################
template<size_t N, class StorageType = double>
class Point
{
public:
    using value_type = StorageType;
    using point_type = Point<N, value_type>;

    Point()
    {
        FastArrayAssign<value_type, N, 0>(v_, 0);
    }

    Point(const point_type& other)
    {
        FastArrayCopy<value_type, N, 0>(v_, other.v_);
    }

    // Creates a ctor that takes N elements to initialize v_ with
    // i,e: Point3D(double x, double y, double z){ ... }
    // static_asserts are used to enforce the correct number of arguments
    template<typename... Args>
    explicit Point(const value_type& x, Args&& ... args)
    {
        v_[0] = x;
        ctorHelper(std::forward<decltype(args)>(args)...);
    }
    template<size_t Idx = 1, typename... Args>
    void ctorHelper(const value_type& x, Args && ... args)
    {
        static_assert(Idx < N, "Too many args passed to Point constructor");
        v_[Idx] = x;
        ctorHelper < Idx + 1 > (std::forward<decltype(args)>(args)...);
    }
    template<size_t Idx>
    void ctorHelper()
    {
        static_assert(Idx == N, "Too few args passed to Point constructor");
    }

    point_type& operator=(const point_type& other)
    {
        FastArrayCopy<value_type, N, 0>(v_, other.v_);
        return *this;
    }

    value_type& operator[](size_t idx)
    {
        return v_[ idx ];
    }

    value_type operator[](size_t idx) const
    {
        return v_[ idx ];
    }

private:
    value_type v_[N];
};

using Point3D = Point<3, double>;



// ################################################################
// ### Vectors
// ################################################################
template<size_t N, class StorageType = double>
class Vector
{
public:
    using value_type = StorageType;
    using vec_type = Vector<N, value_type>;

    static const Vector ZeroVector;

    Vector()
    {
        FastArrayAssign<value_type, N, 0>(v_, 0);
    }

    Vector(const vec_type& other)
    {
        FastArrayCopy<value_type, N, 0>(v_, other.v_);
    }

    // Creates a ctor that takes N elements to initialize v_ with
    // i,e: Vector4D(double x, double y, double z, double w){ ... }
    // static_asserts are used to enforce the correct number of arguments
    template<typename... Args>
    explicit Vector(const value_type& x, Args&& ... args)
    {
        v_[0] = x;
        ctorHelper(std::forward<decltype(args)>(args)...);
    }
    template<size_t Idx = 1, typename... Args>
    void ctorHelper(const value_type& x, Args && ... args)
    {
        static_assert(Idx < N, "Too many args passed to Vector constructor");
        v_[Idx] = x;
        ctorHelper < Idx + 1 > (std::forward<decltype(args)>(args)...);
    }
    template<size_t Idx>
    void ctorHelper()
    {
        static_assert(Idx == N, "Too few args passed to Vector constructor");
    }

    inline vec_type& operator =(const vec_type& other)
    {
        FastArrayCopy<value_type, N, 0>(v_, other.v_);
        return *this;
    }

    inline value_type& operator[](size_t idx)
    {
        return v_[idx];
    }

    inline value_type operator[](size_t idx) const
    {
        return v_[idx];
    }

    value_type dot(const vec_type& other) const
    {
        return MultArrays<value_type, N, 0>::eval(v_, other.v_);
    }

    value_type length2() const
    {
        return MultArrays<value_type, N, 0>::eval(v_, v_);
    }

    value_type length() const
    {
        return std::sqrt(length2());
    }

    value_type normalize()
    {
        if (length2() > 0.0)
        {
            value_type invlen = 1.0 / length();
            MultElements < vec_type, N - 1 >::eval(*this, invlen);
            return invlen;
        }
        return 0.0;
    }

private:
    value_type v_[N];
};

template<size_t N, class StorageType>
const Vector<N,StorageType> Vector<N,StorageType>::ZeroVector = Vector<N,StorageType>();

// Common Types
using Vector4D = FastMath::Vector<4>;
using Vector3D = FastMath::Vector<3>;

template<> struct MultElements < Vector3D, -1 >
{
    static inline void eval(Vector3D&, const typename Vector3D::value_type&) {}
};

template<> struct MultElements < Vector4D, -1 >
{
    static inline void eval(Vector4D&, const typename Vector4D::value_type&) {}
};

inline Vector3D operator *(double s, const Vector3D& v)
{
    return Vector3D(s * v[0], s * v[1], s * v[2]);
}

inline Vector3D operator +(const Vector3D& a, const Vector3D& b)
{
    return Vector3D(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}

inline Point3D operator +(const Point3D& a, const Vector3D& b)
{
    return Point3D(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}

inline Point3D operator +(const Point3D& a, const Point3D& b)
{
    return Point3D(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}

inline Vector3D operator -(const Point3D& a, const Point3D& b)
{
    return Vector3D(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
}

inline Vector3D operator -(const Vector3D& a, const Vector3D& b)
{
    return Vector3D(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
}

inline Vector3D operator -(const Vector3D& a)
{
    return Vector3D(-a[0], -a[1], -a[2]);
}

inline Point3D operator -(const Point3D& a, const Vector3D& b)
{
    return Point3D(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
}

// Cross product specific to 3D vectors
inline Vector3D cross(const Vector3D& a, const Vector3D& b)
{
    return Vector3D(a[1] * b[2] - a[2] * b[1],
                    a[2] * b[0] - a[0] * b[2],
                    a[0] * b[1] - a[1] * b[0]);
}

inline bool operator!=(const Vector3D& a, const Vector3D& b)
{
    return !IsNearly(a[0], b[0]) || !IsNearly(a[1], b[1]) || !IsNearly(a[2], b[2]);
}

inline std::ostream& operator <<(std::ostream& os, const Point3D& p)
{
    return os << "p<" << p[0] << "," << p[1] << "," << p[2] << ">";
}

inline std::ostream& operator <<(std::ostream& os, const Vector3D& v)
{
    return os << "v<" << v[0] << "," << v[1] << "," << v[2] << ">";
}



// ################################################################
// ### Matrix Operations
// ################################################################

// Unrolls the inner loop of matrix mult into:
// a[R][0] * b[0][C] + a[R][1] * b[1][C] + ...
template<class Mtx, size_t I>
struct MultMtxRowCol
{
    enum : size_t { NxtI = I + 1  /* Counter */ };
    static inline auto eval(const Mtx& a, const Mtx& b, const size_t& R, const size_t& C)
    {
        return a[R][I] * b[I][C] + MultMtxRowCol<Mtx, NxtI>::eval(a, b, R, C);
    }
};

// Multiplication of matricies
// Must specialize this class to terminate for each type of matrix
template<class Mtx, size_t N, size_t C, size_t R, size_t I>
struct MultMtxImpl
{
    enum : size_t
    {
        NxtI = I + 1,          // Counter
        NxtC = NxtI % N,       // Column
        NxtR = NxtI / N % N    // Row
    };
    static inline void eval(Mtx& ret, const Mtx& a, const Mtx& b)
    {
        ret[R][C] = MultMtxRowCol<Mtx, 0>::eval(a, b, R, C);
        MultMtxImpl<Mtx, N, NxtC, NxtR, NxtI>::eval(ret, a, b);
    }
};

template<class Mtx, size_t N>
struct FastMatrixMultiply
{
    static inline void eval(Mtx& ret, const Mtx& a, const Mtx& b)
    {
        MultMtxImpl<Mtx, N, 0, 0, 0>::eval(ret, a, b);
    }
};



// ################################################################
// ### Factorial
// ################################################################

template<uint64_t N>
struct Factorial
{
    enum : uint64_t { val = N * Factorial < N - 1 >::val };
};
// Base case
template<> struct Factorial<1>
{
    enum : uint64_t { val = 1 };
};



// ################################################################
// ### Fibonacci
// ################################################################

template<uint64_t N>
struct Fibonacci
{
    enum : uint64_t { val = Fibonacci < N - 1 >::val + Fibonacci < N - 2 >::val };
};
// Base cases
template<> struct Fibonacci<0>
{
    enum : uint64_t { val = 0 };
};
template<> struct Fibonacci<1>
{
    enum : uint64_t { val = 1 };
};

} // namespace FastMath