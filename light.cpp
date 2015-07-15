#include "light.hpp"
#include "scenecontainer.h"
#include <iostream>

Light::Light()
  : colour(0.0, 0.0, 0.0),
    position(0.0, 0.0, 0.0)
{
  falloff[0] = 1.0;
  falloff[1] = 0.0;
  falloff[2] = 0.0;
}

bool Light::IsVisibleFrom(SceneContainer* Scene, const Point3D& TestLoc)
{
  Vector3D PtToLight = position - TestLoc;
  double LightDist = PtToLight.length();
  PtToLight.normalize();
  double dist = 1000000.f;
  bool bHit = Scene->DepthTrace(Ray(TestLoc, PtToLight), dist);
  if (bHit && dist > EPSILON && dist < LightDist) return false;
  return true;
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
