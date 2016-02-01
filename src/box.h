#pragma once
#include "algebra.hpp"
#include "ray.h"
#include <iostream>
#include <cmath>

// Axis-Aliged Bounding-Box (AABB)
// Optimized for fast intersection tests with rays.
template<typename T>
class Box
{
	T data[6];

public:
	Box() {}

	Box(T left, T right, T top, T bottom, T front, T back)
	{
		data[0] = left;
		data[1] = right;
		data[2] = top;
		data[3] = bottom;
		data[4] = front;
		data[5] = back;
	}

	Box(const Box& other)
	{
		std::copy(other.data, other.data+6, (T*)data);
	}

	Box& operator=(const Box& other)
	{
		std::copy(other.data, other.data+6, (T*)data);
		return *this;
	}

	inline T GetLeft() const noexcept
	{
		return data[0];
	}

	inline T GetRight() const noexcept
	{
		return data[1];
	}

	inline T GetTop() const noexcept
	{
		return data[2];
	}

	inline T GetBottom() const noexcept
	{
		return data[3];
	}

	inline T GetFront() const noexcept
	{
		return data[4];
	}

	inline T GetBack() const noexcept
	{
		return data[5];
	}

	inline T& GetLeft() noexcept
	{
		return data[0];
	}

	inline T& GetRight() noexcept
	{
		return data[1];
	}

	inline T& GetTop() noexcept
	{
		return data[2];
	}

	inline T& GetBottom() noexcept
	{
		return data[3];
	}

	inline T& GetFront() noexcept
	{
		return data[4];
	}

	inline T& GetBack() noexcept
	{
		return data[5];
	}

	inline T GetWidth() const
	{
		return abs(data[1] - data[0]);
	}

	inline T GetHeight() const
	{
		return abs(data[2] - data[3]);
	}

	inline T GetDepth() const
	{
		return abs(data[4] - data[5]);
	}

	// Point lies inside or on the boundary of the box?
	inline bool Contains( const Point3D& p ) const
	{
		return p[0] >= data[0] &&
				p[0] <= data[1] &&
				p[1] >= data[3] &&
				p[1] <= data[2] &&
				p[2] >= data[5] &&
				p[2] <= data[4];
	}

	// Return if the ray intersects or not (or starts inside of). Optimized.
	bool Intersects( const Ray& ray ) const
	{
		// Starts in box?
		if( Contains( ray.Start ) ) return true;

		T tx1 = (data[0] - ray.Start[0]) * ray.invDirection[0];
		T tx2 = (data[1] - ray.Start[0]) * ray.invDirection[0];

		T tmin = std::min(tx1, tx2);
		T tmax = std::max(tx1, tx2);

		T ty1 = (data[3] - ray.Start[1]) * ray.invDirection[1];
		T ty2 = (data[2] - ray.Start[1]) * ray.invDirection[1];

		tmin = std::max(tmin, std::min(ty1, ty2));
		tmax = std::min(tmax, std::max(ty1, ty2));

		T tz1 = (data[5] - ray.Start[2]) * ray.invDirection[2];
		T tz2 = (data[4] - ray.Start[2]) * ray.invDirection[2];

		tmin = std::max(tmin, std::min(tz1, tz2));
		tmax = std::min(tmax, std::max(tz1, tz2));

		return tmax >= std::max(static_cast<T>(0), tmin);
	}

	void Transform( const Matrix4x4& M )
	{
		Point3D min(data[0], data[3], data[5]);
		Point3D max(data[1], data[2], data[4]);
		min = M * min;
		max = M * max;
		data[0] = std::min(min[0], max[0]);
		data[1] = std::max(min[0], max[0]);
		data[2] = std::max(min[1], max[1]);
		data[3] = std::min(min[1], max[1]);
		data[4] = std::max(min[2], max[2]);
		data[5] = std::min(min[2], max[2]);
	}
};

template<typename T>
inline std::ostream& operator <<(std::ostream& os, const Box<T>& B)
{
  return os << "Box: [" <<  B.GetLeft() << "," <<
  			   B.GetRight() << "," << B.GetTop() << 
  			   "," << B.GetBottom() << "," <<
  			   B.GetFront() << "," << B.GetBack() << "]" ;
}

using BoxF = Box<double>;