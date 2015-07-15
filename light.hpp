#ifndef CS488_LIGHT_HPP
#define CS488_LIGHT_HPP

#include "algebra.hpp"
#include "array.h"
#include <iosfwd>

class SceneNode;
class SceneContainer;

// Represents a simple point light.
struct Light {
  Light();
  
  Colour colour;
  Point3D position;
  double falloff[3];

  bool IsVisibleFrom(SceneContainer* Scene, const Point3D& TestLoc);
};

std::ostream& operator<<(std::ostream& out, const Light& l);

#endif
