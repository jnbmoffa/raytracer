#pragma once

#include "algebra.hpp"
#include <iosfwd>

// Represents the scene camera
struct Camera {
	Camera() : eye(0.f,0.f,0.f), view(0.f,0.f,-1.f), up(0.f,1.f,0.f), fov(50.f), ApertureRadius(1.4f), FocalDistance(1.f) {}

	Point3D eye;
	Vector3D view, up;
	double fov, ApertureRadius, FocalDistance;
};

std::ostream& operator<<(std::ostream& out, const Camera& c);