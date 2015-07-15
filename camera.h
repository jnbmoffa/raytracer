#pragma once

#include "algebra.hpp"
#include <iosfwd>

// Represents the scene camera
struct Camera {
	Point3D eye;
	Vector3D view, up, right;
	double fov, ApertureRadius, FocalDistance;
	Vector3D xApertureRadius, yApertureRadius;

	std::default_random_engine generator;
    std::uniform_real_distribution<double> ApertureDistribution;

	Camera() : eye(0.f,0.f,0.f), view(0.f,0.f,-1.f), up(0.f,1.f,0.f), fov(50.f), ApertureRadius(1.4f), FocalDistance(1.f)
		, ApertureDistribution(-1.f, 1.f) {}

	void CalcApertureRadius()
	{
		xApertureRadius = ApertureRadius*right;
        yApertureRadius = ApertureRadius*up;
        std::cout << xApertureRadius << "," << yApertureRadius << std::endl;
	}

	inline Point3D GetRandomEye()
	{
	  return eye + (ApertureDistribution(generator)*xApertureRadius) + (ApertureDistribution(generator)*yApertureRadius);
	}
};

std::ostream& operator<<(std::ostream& out, const Camera& c);