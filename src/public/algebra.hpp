//---------------------------------------------------------------------------
//
// CS488 -- Introduction to Computer Graphics
//
// algebra.hpp/algebra.cpp
//
// Classes and functions for manipulating points, vectors, matrices,
// and colours.  You probably won't need to modify anything in these
// two files.
//
// University of Waterloo Computer Graphics Lab / 2003
//
//---------------------------------------------------------------------------

#ifndef CS488_ALGEBRA_HPP
#define CS488_ALGEBRA_HPP

#include <iostream>
#include <algorithm>
#include <cmath>
#include "fastmath.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using Point3D = FastMath::Point3D;
using Vector3D = FastMath::Vector3D;
using Vector4D = FastMath::Vector4D;

class Matrix4x4;
inline Matrix4x4 operator *(const Matrix4x4& a, const Matrix4x4& b);

class Matrix4x4
{
public:
    using value_type = double;

    static const Matrix4x4 Identity;

    Matrix4x4() :
        Matrix4x4(Identity)
    {}

    Matrix4x4(const Matrix4x4& other)
    {
        FastMath::AssignArrays<value_type, 16, 0>::eval(v_, other.v_);
    }

    Matrix4x4(const Vector4D& row1, const Vector4D& row2, const Vector4D& row3, const Vector4D& row4)
    {
        v_[0] = row1[0];
        v_[1] = row1[1];
        v_[2] = row1[2];
        v_[3] = row1[3];

        v_[4] = row2[0];
        v_[5] = row2[1];
        v_[6] = row2[2];
        v_[7] = row2[3];

        v_[8] = row3[0];
        v_[9] = row3[1];
        v_[10] = row3[2];
        v_[11] = row3[3];

        v_[12] = row4[0];
        v_[13] = row4[1];
        v_[14] = row4[2];
        v_[15] = row4[3];
    }

    Matrix4x4& operator=(const Matrix4x4& other)
    {
        FastMath::AssignArrays<value_type, 16, 0>::eval(v_, other.v_);
        return *this;
    }

    Vector4D getRow(size_t row) const
    {
        return Vector4D(v_[4 * row], v_[4 * row + 1], v_[4 * row + 2], v_[4 * row + 3]);
    }
    
    value_type* getRow(size_t row)
    {
        return (value_type*)v_ + 4 * row;
    }

    Vector4D getColumn(size_t col) const
    {
        return Vector4D(v_[col], v_[4 + col], v_[8 + col], v_[12 + col]);
    }

    Vector4D operator[](size_t row) const
    {
        return getRow(row);
    }

    value_type* operator[](size_t row)
    {
        return getRow(row);
    }

    void rotate(char axis, value_type angle)
    {
        const value_type rad = angle * (M_PI / 180);
        const value_type cosValue = cos(rad);
        const value_type sinValue = sin(rad);

        Vector4D R1(1.f, 0.f, 0.f, 0.f);
        Vector4D R2(0.f, 1.f, 0.f, 0.f);
        Vector4D R3(0.f, 0.f, 1.f, 0.f);
        const Vector4D R4(0.f, 0.f, 0.f, 1.f);
        switch (axis)
        {
        case 'x':
        case 'X':
            R2[1] = cosValue;
            R2[2] = -sinValue;
            R3[1] = sinValue;
            R3[2] = cosValue;
            break;
        case 'y':
        case 'Y':
            R1[0] = cosValue;
            R1[2] = sinValue;
            R3[0] = -sinValue;
            R3[2] = cosValue;
            break;
        case 'z':
        case 'Z':
            R1[0] = cosValue;
            R1[1] = -sinValue;
            R2[0] = sinValue;
            R2[1] = cosValue;
            break;
        }
        (*this) = (*this) * Matrix4x4(R1, R2, R3, R4);
    }

    void scale(const Vector3D& amount)
    {
        const Vector4D R1(amount[0], 0.f, 0.f, 0.f);
        const Vector4D R2(0.f, amount[1], 0.f, 0.f);
        const Vector4D R3(0.f, 0.f, amount[2], 0.f);
        const Vector4D R4(0.f, 0.f, 0.f, 1.f);
        (*this) = (*this) * Matrix4x4(R1, R2, R3, R4);
    }

    void translate(const Vector3D& amount)
    {
        const Vector4D R1(1.f, 0.f, 0.f, amount[0]);
        const Vector4D R2(0.f, 1.f, 0.f, amount[1]);
        const Vector4D R3(0.f, 0.f, 1.f, amount[2]);
        const Vector4D R4(0.f, 0.f, 0.f, 1.f);
        (*this) = (*this) * Matrix4x4(R1, R2, R3, R4);
    }

    Matrix4x4 transpose() const
    {
        return Matrix4x4(getColumn(0), getColumn(1),
                         getColumn(2), getColumn(3));
    }

    Matrix4x4 invert() const;

private:
    value_type v_[16];
};

namespace FastMath
{
// Specialization needed by fastmath.h
template<>
struct MultMtxRowCol<Matrix4x4, 4>
{
    static inline auto eval(const Matrix4x4&, const Matrix4x4&, const size_t, const size_t)
    {
        return 0.0;
    }
};

// Specialization needed by fastmath.h
template<>
struct MultMtxImpl<Matrix4x4, 4, 0, 0, 4 * 4>
{
    static inline void eval(Matrix4x4&, const Matrix4x4&, const Matrix4x4&) {}
};
} // namespace FastMath

inline Matrix4x4 operator*(const Matrix4x4& a, const Matrix4x4& b)
{
    Matrix4x4 ret;
    FastMath::MultMtx<Matrix4x4, 4>::eval(ret, a, b);
    return ret;
}

inline Vector3D operator*(const Matrix4x4& M, const Vector3D& v)
{
    return Vector3D(
               v[0] * M[0][0] + v[1] * M[0][1] + v[2] * M[0][2],
               v[0] * M[1][0] + v[1] * M[1][1] + v[2] * M[1][2],
               v[0] * M[2][0] + v[1] * M[2][1] + v[2] * M[2][2]);
}

inline Point3D operator*(const Matrix4x4& M, const Point3D& p)
{
    return Point3D(
               p[0] * M[0][0] + p[1] * M[0][1] + p[2] * M[0][2] + M[0][3],
               p[0] * M[1][0] + p[1] * M[1][1] + p[2] * M[1][2] + M[1][3],
               p[0] * M[2][0] + p[1] * M[2][1] + p[2] * M[2][2] + M[2][3]);
}

inline Vector3D transNorm(const Matrix4x4& M, const Vector3D& n)
{
    return Vector3D(
               n[0] * M[0][0] + n[1] * M[1][0] + n[2] * M[2][0],
               n[0] * M[0][1] + n[1] * M[1][1] + n[2] * M[2][1],
               n[0] * M[0][2] + n[1] * M[1][2] + n[2] * M[2][2]);
}

inline std::ostream& operator <<(std::ostream& os, const Matrix4x4& M)
{
    return os << "[" << M[0][0] << " " << M[0][1] << " "
           << M[0][2] << " " << M[0][3] << "]" << std::endl

           << "[" << M[1][0] << " " << M[1][1] << " "
           << M[1][2] << " " << M[1][3] << "]" << std::endl

           << "[" << M[2][0] << " " << M[2][1] << " "
           << M[2][2] << " " << M[2][3] << "]" << std::endl

           << "[" << M[3][0] << " " << M[3][1] << " "
           << M[3][2] << " " << M[3][3] << "]";
}

class Colour
{
public:
    Colour(double r, double g, double b)
        : r_(r)
        , g_(g)
        , b_(b)
    {}

    Colour(double c)
        : r_(c)
        , g_(c)
        , b_(c)
    {}

    Colour()
        : r_(0.0)
        , g_(0.0)
        , b_(0.0)
    {}

    Colour(const Colour& other)
        : r_(other.r_)
        , g_(other.g_)
        , b_(other.b_)
    {}

    static const Colour Black;
    static const Colour White;

    Colour& operator=(const Colour& other)
    {
        r_ = other.r_;
        g_ = other.g_;
        b_ = other.b_;
        return *this;
    }

    double R() const
    {
        return r_;
    }

    double G() const
    {
        return g_;
    }

    double B() const
    {
        return b_;
    }

    double& R()
    {
        return r_;
    }

    double& G()
    {
        return g_;
    }

    double& B()
    {
        return b_;
    }

private:
    double r_;
    double g_;
    double b_;
};

inline bool operator==(const Colour& a, const Colour& b)
{
    constexpr double localEpsilon = 0.05;
    return FastMath::IsNearly(a.R(), b.R(), localEpsilon)
           &&
           FastMath::IsNearly(a.G(), b.G(), localEpsilon)
           &&
           FastMath::IsNearly(a.B(), b.B(), localEpsilon);
}

inline bool operator!=(const Colour& a, const Colour& b)
{
    return !(a == b);
}

inline Colour operator*(double s, const Colour& a)
{
    return Colour(s * a.R(), s * a.G(), s * a.B());
}

inline Colour operator*(const Colour& a, const Colour& b)
{
    return Colour(a.R() * b.R(), a.G() * b.G(), a.B() * b.B());
}

inline Colour operator/(const Colour& a, const double& b)
{
    return Colour(a.R() / b, a.G() / b, a.B() / b);
}

inline Colour operator+(const Colour& a, const Colour& b)
{
    return Colour(a.R() + b.R(), a.G() + b.G(), a.B() + b.B());
}

inline std::ostream& operator <<(std::ostream& os, const Colour& c)
{
    return os << "c<" << c.R() << "," << c.G() << "," << c.B() << ">";
}

#endif // CS488_ALGEBRA_HPP