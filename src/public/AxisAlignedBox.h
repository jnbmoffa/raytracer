#pragma once

#include "fastmath.h"
#include "algebra.hpp"
#include "ray.h"
#include <iostream>
#include <cmath>

// Axis-Aliged Bounding-AxisAlignedBox (AABB)
// Optimized for fast intersection tests with rays.
template<typename T>
class AxisAlignedBox
{
    T data[6];

public:
    enum class NormalSelect
    {
        Right,
        Left,
        Top,
        Bottom,
        Front,
        Back
    };

    AxisAlignedBox();
    AxisAlignedBox(T right, T left, T top, T bottom, T front, T back);
    AxisAlignedBox(const AxisAlignedBox& other);

    AxisAlignedBox& operator=(const AxisAlignedBox& other);

    T GetRight() const noexcept;
    T GetLeft() const noexcept;
    T GetTop() const noexcept;
    T GetBottom() const noexcept;
    T GetFront() const noexcept;
    T GetBack() const noexcept;
    T GetWidth() const;
    T GetHeight() const;
    T GetDepth() const;

    void Transform(const Matrix4x4& M);

    static Vector3D GetNormal(NormalSelect type)
    {
        switch (type)
        {
        case NormalSelect::Right:
            return Vector3D(1, 0, 0);
        case NormalSelect::Left:
            return Vector3D(-1, 0, 0);
        case NormalSelect::Top:
            return Vector3D(0, 1, 0);
        case NormalSelect::Bottom:
            return Vector3D(0, -1, 0);
        case NormalSelect::Front:
            return Vector3D(0, 0, 1);
        case NormalSelect::Back:
            return Vector3D(0, 0, -1);
        }
        return Vector3D::ZeroVector;
    }
};




// Impl

template<typename T>
AxisAlignedBox<T>::AxisAlignedBox()
{}

template<typename T>
AxisAlignedBox<T>::AxisAlignedBox(T right, T left, T top, T bottom, T front, T back) :
    data{right, left, top, bottom, front, back}
{}

template<typename T>
AxisAlignedBox<T>::AxisAlignedBox(const AxisAlignedBox<T>& other)
{
    std::copy(other.data, other.data + 6, (T*)data);
}

template<typename T>
AxisAlignedBox<T>& AxisAlignedBox<T>::operator=(const AxisAlignedBox<T>& other)
{
    std::copy(other.data, other.data + 6, (T*)data);
    return *this;
}

template<typename T>
inline T AxisAlignedBox<T>::GetRight() const noexcept
{
    return data[0];
}

template<typename T>
inline T AxisAlignedBox<T>::GetLeft() const noexcept
{
    return data[1];
}

template<typename T>
inline T AxisAlignedBox<T>::GetTop() const noexcept
{
    return data[2];
}

template<typename T>
inline T AxisAlignedBox<T>::GetBottom() const noexcept
{
    return data[3];
}

template<typename T>
inline T AxisAlignedBox<T>::GetFront() const noexcept
{
    return data[4];
}

template<typename T>
inline T AxisAlignedBox<T>::GetBack() const noexcept
{
    return data[5];
}

template<typename T>
inline T AxisAlignedBox<T>::GetWidth() const
{
    return abs(data[1] - data[0]);
}

template<typename T>
inline T AxisAlignedBox<T>::GetHeight() const
{
    return abs(data[2] - data[3]);
}

template<typename T>
inline T AxisAlignedBox<T>::GetDepth() const
{
    return abs(data[4] - data[5]);
}

template<typename T>
void AxisAlignedBox<T>::Transform(const Matrix4x4& M)
{
    Point3D min(data[1], data[3], data[5]);
    Point3D max(data[0], data[2], data[4]);
    min = M * min;
    max = M * max;
    data[0] = std::max(min[0], max[0]);
    data[1] = std::min(min[0], max[0]);
    data[2] = std::max(min[1], max[1]);
    data[3] = std::min(min[1], max[1]);
    data[4] = std::max(min[2], max[2]);
    data[5] = std::min(min[2], max[2]);
}


// Check whether the point lies inside of, or on the boundary of, the box.
template<typename T>
bool Contains(const AxisAlignedBox<T>& box, const Point3D& p)
{
    return p[0] >= box.GetLeft() &&
           p[0] <= box.GetRight() &&
           p[1] >= box.GetBottom() &&
           p[1] <= box.GetTop() &&
           p[2] >= box.GetBack() &&
           p[2] <= box.GetFront();
}

template<typename T>
struct AABIntersectData
{
    T tMin;
    T tMax;
    T tx1;
    T tx2;
    T ty1;
    T ty2;
    T tz1;
    T tz2;
};

// Get the intersection min and max. (Gets rid of duplicate code)
// @return true if they intersect
template<typename T>
void DoIntersect(const Ray& ray, const AxisAlignedBox<T>& box, AABIntersectData<T>& data)
{
    const Point3D rayOrigin = ray.GetOrigin();
    const Vector3D rayAABBDiv = ray.GetAABBDiv();

    data.tx1 = (box.GetLeft() - rayOrigin[0]) * rayAABBDiv[0];
    data.tx2 = (box.GetRight() - rayOrigin[0]) * rayAABBDiv[0];

    data.tMin = std::min(data.tx1, data.tx2);
    data.tMax = std::max(data.tx1, data.tx2);

    data.ty1 = (box.GetBottom() - rayOrigin[1]) * rayAABBDiv[1];
    data.ty2 = (box.GetTop() - rayOrigin[1]) * rayAABBDiv[1];

    data.tMin = std::max(data.tMin, std::min(data.ty1, data.ty2));
    data.tMax = std::min(data.tMax, std::max(data.ty1, data.ty2));

    data.tz1 = (box.GetBack() - rayOrigin[2]) * rayAABBDiv[2];
    data.tz2 = (box.GetFront() - rayOrigin[2]) * rayAABBDiv[2];

    data.tMin = std::max(data.tMin, std::min(data.tz1, data.tz2));
    data.tMax = std::min(data.tMax, std::max(data.tz1, data.tz2));
}

// Return if the ray intersects or not. Optimized.
// Return outMult indicating the intersection point in the forward direction
template<typename T>
bool GetIntersection(const Ray& ray, const AxisAlignedBox<T>& box, HitInfo& hit)
{
    const Vector3D rayDir = ray.GetDirection();

    AABIntersectData<T> data;
    DoIntersect(ray, box, data);
    const bool haveIntersected = data.tMax >= std::max(static_cast<T>(0), data.tMin);

    if (haveIntersected)
    {
        T tUsed;

        const Point3D rayOrigin = ray.GetOrigin();
        if (Contains(box, rayOrigin))
        {
            tUsed = data.tMax;
            hit.Location = rayOrigin + ((data.tMax + EPSILON) * rayDir);
        }
        else
        {
            tUsed = data.tMin;
            hit.Location = rayOrigin + ((data.tMin - EPSILON) * rayDir);
        }

        if (tUsed == data.tx1)
        {
            hit.Normal = AxisAlignedBox<T>::GetNormal(AxisAlignedBox<T>::NormalSelect::Left);
        }
        else if (tUsed == data.tx2)
        {
            hit.Normal = AxisAlignedBox<T>::GetNormal(AxisAlignedBox<T>::NormalSelect::Right);
        }
        else if (tUsed == data.ty1)
        {
            hit.Normal = AxisAlignedBox<T>::GetNormal(AxisAlignedBox<T>::NormalSelect::Bottom);
        }
        else if (tUsed == data.ty2)
        {
            hit.Normal = AxisAlignedBox<T>::GetNormal(AxisAlignedBox<T>::NormalSelect::Top);
        }
        else if (tUsed == data.tz1)
        {
            hit.Normal = AxisAlignedBox<T>::GetNormal(AxisAlignedBox<T>::NormalSelect::Back);
        }
        else
        {
            hit.Normal = AxisAlignedBox<T>::GetNormal(AxisAlignedBox<T>::NormalSelect::Front);
        }
    }

    return haveIntersected;
}

// Return if the ray intersects or not (or starts inside of). Optimized.
template<typename T>
bool CheckIntersection(const Ray& ray, const AxisAlignedBox<T>& box)
{
    // Starts in box?
    if (Contains(box, ray.GetOrigin()))
    {
        return true;
    }

    AABIntersectData<T> data;
    DoIntersect(ray, box, data);
    return data.tMax >= data.tMin;
}

template<typename T>
inline std::ostream& operator <<(std::ostream& os, const AxisAlignedBox<T>& B)
{
    return os << "AxisAlignedBox: [" <<  B.GetLeft() << "," <<
           B.GetRight() << "," << B.GetTop() <<
           "," << B.GetBottom() << "," <<
           B.GetFront() << "," << B.GetBack() << "]" ;
}

using BoxF = AxisAlignedBox<double>;