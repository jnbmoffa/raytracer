#include "light.hpp"
#include "scenecontainer.h"
#include "ray.h"
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

bool Light::IsVisibleFrom(SceneContainer* Scene, const Point3D& LightLoc, const Point3D& TestLoc, const double& Time)
{
  Vector3D PtToLight = LightLoc - TestLoc;
  double LightDist = PtToLight.length();
  PtToLight.normalize();
  double dist = 1000000.f;
  bool bHit = Scene->TimeDepthTrace(Ray(TestLoc, PtToLight), dist, Time);
  if (bHit && dist > EPSILON && dist < LightDist) return false;
  return true;
}

double Light::GetIntensity(SceneContainer* Scene, const Point3D& TestLoc, const double& Time)
{
  if (IsVisibleFrom(Scene, position, TestLoc, Time)) return 1.f;
  return 0.f;
}

double SphereLight::GetIntensity(SceneContainer* Scene, const Point3D& TestLoc, const double& Time)
{
  Vector3D Normal = TestLoc - position; Normal.normalize();
  Matrix4x4 Rot; Rot.rotate('x', 45); // Potential error if Normal = x axis
  Vector3D u = cross((Rot * Normal),Normal); u.normalize(); u = radius*u;
  Vector3D v = cross(Normal,u); v.normalize(); v = radius*v;
  int NumVisiblePoints = 0;
  double theta=0.f, RingInc = 1.f/(double)NumRings, AngleInc = (2*M_PI)/(double)RingPoints;

  // Sum up the intensity of the visible points on the light
  for (double i=RingInc;i<1.f;i+=RingInc)
  {
    for (int j=0;j<4;j++)
    {
      if (IsVisibleFrom(Scene, position + i*sin(theta)*u + i*cos(theta)*v, TestLoc, Time)) NumVisiblePoints++;
      theta+=AngleInc;
    }
    theta-=3.f*(AngleInc);
  }
  return (double)NumVisiblePoints/((double)RingPoints*NumRings);
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
