#ifndef CS488_LIGHT_HPP
#define CS488_LIGHT_HPP

#include "algebra.hpp"
#include "array.h"
#include <iosfwd>
#include <random>

class SceneNode;
class SceneContainer;

// Represents a simple point light.
struct Light {
  Light();
  
  double power;
  Colour colour;
  Point3D position;
  double falloff[3];

  bool IsVisibleFrom(SceneContainer* Scene, const Point3D& LightLoc, const Point3D& TestLoc);
  virtual double GetIntensity(SceneContainer* Scene, const Point3D& TestLoc);
};

struct SphereLight : public Light
{
	SphereLight() : generator(std::random_device{}()), Distribution(-1.f,1.f) {}

	double radius;
	int RingPoints, NumRings;
	std::default_random_engine generator;
    std::uniform_real_distribution<double> Distribution;

	virtual double GetIntensity(SceneContainer* Scene, const Point3D& TestLoc) override;
};

std::ostream& operator<<(std::ostream& out, const Light& l);

#endif
