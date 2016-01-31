#pragma once

#include "algebra.hpp"

class Material;

struct HitInfo
{
  HitInfo() : Mat(nullptr) {}

  Point3D Location;
  Vector3D Normal;
  Material* Mat;
};

struct Ray
{
  Ray() {}
  Ray(const Point3D& S, const Vector3D& D) : Start(S), Direction(D)
  {
    invDirection = Vector3D(1.f/D[0], 1.f/D[1], 1.f/D[2]);
  }

  inline void Transform(const Matrix4x4& M)
  {
      Direction = M * Direction;
      Start = M * Start;
      invDirection = Vector3D(1.f/Direction[0], 1.f/Direction[1], 1.f/Direction[2]);
  }

  inline Ray Reflect(const HitInfo& Hit, const double cosi) const
  {
    Ray ReflectedRay(Hit.Location, Direction + ((2*cosi)*Hit.Normal)); ReflectedRay.Direction.normalize();
    return ReflectedRay;
  }

  inline Ray Refract(const double ni, const double nt, const double cosi, const double sin2t, const HitInfo& Hit) const
  {
    Ray RefractedRay(Hit.Location, (ni/nt)*Direction + (ni/nt*cosi -sqrt(1-sin2t))*Hit.Normal); RefractedRay.Direction.normalize();
    return RefractedRay;
  }

  Point3D Start;
  Vector3D Direction;
  Vector3D invDirection;  // AABB optimization
};