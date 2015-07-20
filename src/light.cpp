#include "light.hpp"
#include "scenecontainer.h"
#include <iostream>
#include <cmath>

Light::Light()
  : colour(0.0, 0.0, 0.0),
    position(0.0, 0.0, 0.0)
{
  falloff[0] = 1.0;
  falloff[1] = 0.0;
  falloff[2] = 0.0;
}

bool Light::IsVisibleFrom(SceneContainer* Scene, const Point3D& LightLoc, const Point3D& TestLoc)
{
  Vector3D PtToLight = LightLoc - TestLoc;
  double LightDist = PtToLight.length();
  PtToLight.normalize();
  double dist = 1000000.f;
  bool bHit = Scene->DepthTrace(Ray(TestLoc, PtToLight), dist);
  if (bHit && dist > EPSILON && dist < LightDist) return false;
  return true;
}

double Light::GetIntensity(SceneContainer* Scene, const Point3D& TestLoc)
{
  if (IsVisibleFrom(Scene, position, TestLoc)) return 1.f;
  return 0.f;
}

double SphereLight::GetIntensity(SceneContainer* Scene, const Point3D& TestLoc)
{
  Vector3D Normal = TestLoc - position; Normal.normalize();
  Matrix4x4 Rot; Rot.rotate('x', 45); // Potential error if Normal = x axis
  Vector3D u = (Rot * Normal).cross(Normal); u.normalize(); u = radius*u;
  Vector3D v = Normal.cross(u); v.normalize(); v = radius*v;
  int NumVisiblePoints = 0;
  double theta=0.f, RingInc = 1.f/(double)NumRings, AngleInc = (2*M_PI)/(double)RingPoints;
  for (double i=RingInc;i<1.f;i+=RingInc)
  {
    for (int j=0;j<4;j++)
    {
      if (IsVisibleFrom(Scene, position + i*sin(theta)*u + i*cos(theta)*v, TestLoc)) NumVisiblePoints++;
      theta+=AngleInc;
    }
    theta-=3.f*(AngleInc);
  }
  return (double)NumVisiblePoints/((double)RingPoints*4);
}

std::ostream& operator<<(std::ostream& out, const Light& l)
{
  out << "L[" << l.colour << ", " << l.position << ", ";
  for (int i = 0; i < 3; i++) {
    if (i > 0) out << ", ";
    out << l.falloff[i];
  }
  out << "]";
  return out;
}