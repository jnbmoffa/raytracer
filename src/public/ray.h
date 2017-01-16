#pragma once

#include "algebra.hpp"

class Material;

struct HitInfo
{
    HitInfo() :
        Mat(nullptr)
    {}

    Point3D Location;
    Vector3D Normal;
    Material* Mat;
};

class Ray
{
public:
    Ray()
    {}

    Ray(const Point3D& origin, const Vector3D& direction) :
        m_origin(origin),
        m_direction(direction)
    {
        m_AABBDivisors = Vector3D(1.0 / direction[0], 1.0 / direction[1], 1.0 / direction[2]);
    }

    void Normalize()
    {
        m_direction.normalize();
        m_AABBDivisors = Vector3D(1.0 / m_direction[0], 1.0 / m_direction[1], 1.0 / m_direction[2]);
    }

    void Transform(const Matrix4x4& M)
    {
        SetDirection(M * m_direction);
        m_origin = M * m_origin;
    }

    Ray Reflect(const HitInfo& Hit, const double cosi) const
    {
        Ray ReflectedRay(Hit.Location, m_direction + ((2 * cosi)*Hit.Normal));
        ReflectedRay.Normalize();
        return ReflectedRay;
    }

    Ray Refract(const double ni, const double nt, const double cosi, const double sin2t, const HitInfo& Hit) const
    {
        Ray RefractedRay(Hit.Location, (ni / nt) * m_direction + (ni / nt * cosi - sqrt(1 - sin2t)) * Hit.Normal);
        RefractedRay.Normalize();
        return RefractedRay;
    }

    void SetOrigin(const Point3D& newOrig)
    {
        m_origin = newOrig;
    }

    Point3D GetOrigin() const
    {
        return m_origin;
    }

    void SetDirection(const Vector3D& newDir)
    {
        m_direction = newDir;
        Normalize();
    }

    Vector3D GetDirection() const
    {
        return m_direction;
    }

    Vector3D GetAABBDiv() const
    {
        return m_AABBDivisors;
    }

private:
    Point3D m_origin;
    Vector3D m_direction;
    Vector3D m_AABBDivisors;  // AABB optimization
};