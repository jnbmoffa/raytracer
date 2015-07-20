#pragma once

#include "algebra.hpp"
#include <iosfwd>

// Represents the scene camera
struct Camera {
	Point3D eye;
	Vector3D view, up, right;
	double fov, ApertureRadius, FocalDistance;
	Vector3D xApertureRadius, yApertureRadius;
	int DOFRays;

	std::default_random_engine generator;
    std::uniform_real_distribution<double> ApertureDistribution;

	Camera() : eye(0.f,0.f,0.f), view(0.f,0.f,-1.f), up(0.f,1.f,0.f), fov(50.f), ApertureRadius(1.4f), FocalDistance(1.f)
		, ApertureDistribution(-1.f, 1.f) {}

	virtual void CalcApertureRadius() = 0;
	virtual Point3D GetRandomEye() = 0;
	virtual double GetFocalDist() = 0;
	virtual double GetDOFRays() = 0;
};

// Point camera with no aperture
struct PointCamera : public Camera
{
	virtual void CalcApertureRadius() override {}

	virtual inline Point3D GetRandomEye() override
	{
	  return eye;
	}

	virtual inline double GetFocalDist() override
	{
		return 1.f;
	}

	virtual inline double GetDOFRays() override
	{
		return 1;
	}
};

// Simulated camera with a lense and focal disance
struct LenseCamera : public Camera
{
	virtual void CalcApertureRadius() override
	{
		xApertureRadius = ApertureRadius*right;
        yApertureRadius = ApertureRadius*up;
	}

	virtual inline Point3D GetRandomEye() override
	{
	  return eye + (ApertureDistribution(generator)*xApertureRadius) + (ApertureDistribution(generator)*yApertureRadius);
	}

	virtual inline double GetFocalDist() override
	{
		return FocalDistance;
	}

	virtual inline double GetDOFRays() override
	{
		return DOFRays;
	}
};

std::ostream& operator<<(std::ostream& out, const Camera& c);