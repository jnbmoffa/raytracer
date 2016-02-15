#pragma once

#include "luacamera.h"
#include "algebra.hpp"
#include "ray.h"
#include <iosfwd>
#include <memory>

// The parameters of the plane that rays are being shot through
struct RenderPlaneParams
{
     double pixelWidth;
     double pixelHeight;
     double halfWidth;
     double halfHeight;
};

// Represents the scene camera
class Camera {
protected:
	LuaCamera m_data;	// Cache camera data for speed
	RenderPlaneParams m_planeParams;
	Vector3D m_xApertureRadius, m_yApertureRadius;

	std::default_random_engine generator;
    std::uniform_real_distribution<double> ApertureDistribution;

	// Protected ctor, use factory
	explicit Camera(const std::shared_ptr<LuaCamera>& initData,
					const int width,
					const int height) : 	m_data(*initData),
											ApertureDistribution(-1.f, 1.f)
	{
		// Set up the camera direction vectors
		m_data.view.normalize();
		m_data.up = -m_data.up; m_data.up.normalize();
		m_data.right = cross(m_data.up,m_data.view); m_data.right.normalize();
		
		// Calculate aperture radius
		m_xApertureRadius = m_data.ApertureRadius * m_data.right;
        m_yApertureRadius = m_data.ApertureRadius * m_data.up;

        // Determine the render plane parameters
		double radFOV = m_data.fov * (M_PI / 180.f);
		m_planeParams.halfWidth = m_data.FocalDistance*tan(radFOV/2);

		double heightWidthRatio = (double)width / (double)height;
		m_planeParams.halfHeight = heightWidthRatio * m_planeParams.halfWidth;

		double 	cameraWidth = m_planeParams.halfWidth * 2,
				cameraHeight = m_planeParams.halfHeight * 2;

		m_planeParams.pixelWidth = cameraWidth / (width - 1);
		m_planeParams.pixelHeight = cameraHeight / (height - 1);
	}

public:
	Camera() = delete;
	virtual ~Camera() = default;

	Ray GetRayThroughPixel(const int x, const int y, const double& xNorm, const double& yNorm)
	{
		// Determine the location on the render plane to fire the ray through
		// xNorm == 0 && yNorm == 0 -> bottom left of pixel (x,y)
		// xNorm == 1 && yNorm == 1 -> top right of pixel (x,y)
		Vector3D rightComp = ((x * m_planeParams.pixelWidth) + (m_planeParams.pixelWidth*xNorm) - m_planeParams.halfWidth) * m_data.right;
		Vector3D upComp = ((y * m_planeParams.pixelHeight) + (m_planeParams.pixelHeight*yNorm) - m_planeParams.halfHeight) * m_data.up;
		Vector3D View = m_data.FocalDistance*m_data.view;
		Point3D ViewPlanePoint = m_data.eye + View + rightComp + upComp;

		Point3D RandomEyePoint = GetRandomEye();
		Vector3D RayDir = ViewPlanePoint - RandomEyePoint; RayDir.normalize();
		return Ray(RandomEyePoint, RayDir);
	}

	virtual Point3D GetRandomEye() = 0;
	virtual double GetDOFRays() const = 0;

	friend std::unique_ptr<Camera> CreateCamera(const std::shared_ptr<LuaCamera>& luaCam,
												const int width,
												const int height);
	friend std::ostream& operator<<(std::ostream& out, const Camera& c);
};

// Simulated camera with a lense and focal disance
class LenseCamera : public Camera
{
protected:
	explicit LenseCamera(	const std::shared_ptr<LuaCamera>& initData,
							const int width,
							const int height) : 	Camera(initData, width, height) {}
public:	
	virtual inline Point3D GetRandomEye() override
	{
	  return m_data.eye + (ApertureDistribution(generator)*m_xApertureRadius) + (ApertureDistribution(generator)*m_yApertureRadius);
	}

	virtual inline double GetDOFRays() const override
	{
		return m_data.DOFRays;
	}

	friend std::unique_ptr<Camera> CreateCamera(const std::shared_ptr<LuaCamera>& luaCam,
												const int width,
												const int height);
};

// Point camera with no aperture
class PointCamera : public Camera
{
protected:
	explicit PointCamera(	const std::shared_ptr<LuaCamera>& initData,
							const int width,
							const int height) : 	Camera(initData, width, height) {}
public:
	virtual inline Point3D GetRandomEye() override
	{
	  return m_data.eye;
	}

	virtual inline double GetDOFRays() const override
	{
		return 1;
	}

	friend std::unique_ptr<Camera> CreateCamera(const std::shared_ptr<LuaCamera>& luaCam,
												const int width,
												const int height);
};

std::unique_ptr<Camera> CreateCamera(   const std::shared_ptr<LuaCamera>& luaCam,
                                        const int width,
                                        const int height);

std::ostream& operator<<(std::ostream& out, const Camera& c);                                        