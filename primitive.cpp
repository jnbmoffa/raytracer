#include "primitive.hpp"
#include "polyroots.hpp"
#include "box.h"
#include <cmath>
#include <vector>

Primitive::~Primitive()
{
}

Sphere::~Sphere()
{
}

bool Sphere::SimpleTrace(Ray R)
{
	R.Direction.normalize();
	Vector3D deltaP = m_center - R.Start;
	return (((m_radius*m_radius) - (deltaP - (R.Direction.dot(deltaP))*R.Direction).length2()) >= 0);
}

bool Sphere::DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M)
{
	R.Direction.normalize();
	bool retry;
	do{
		retry = false;
		Vector3D deltaP = m_center - R.Start;
		double uDotDeltaP = R.Direction.dot(deltaP);
		double discriminant = (m_radius*m_radius - (deltaP - (uDotDeltaP)*R.Direction).length2());
		// std::cout << discriminant << std::endl;
		if (discriminant >=0)
		{
			double sqrtDisc = sqrt(discriminant);
			double s = std::min(uDotDeltaP + sqrtDisc, uDotDeltaP - sqrtDisc);
			if (s < 0)
			{
				if (uDotDeltaP + sqrtDisc > 0) s = uDotDeltaP + sqrtDisc;
				else if (uDotDeltaP - sqrtDisc > 0) s = uDotDeltaP - sqrtDisc;
				else return false;
			}
			Vector3D rayVec = s * R.Direction;
			Point3D hitLoc = R.Start + rayVec;
			Vector3D Normal = (hitLoc - m_center);

			Point3D WorldRay = M * R.Start;
			Point3D WorldHit = M * hitLoc;
			if (clampDist(closestDist, WorldRay, WorldHit, Normal, Hit, M))
			{
				return true;
			}

			if (CheckCloseHit(WorldRay, WorldHit))
			{
				R.Start = R.Start + EPSILON*R.Direction;
				retry = true;
			}
		}
	} while(retry);
	return false;
}

Cube::~Cube()
{
}

bool Cube::IsInside(Point3D Int, Vector3D Mask)
{
	bool xCheck = true, yCheck = true, zCheck = true;
	Point3D Extent(m_pos[0] + m_size, m_pos[1] + m_size, m_pos[2] + m_size);
	if (Mask[0] != 0) {xCheck = !(Int[0] < m_pos[0] || Int[0] > Extent[0]);}
	if (Mask[1] != 0) {yCheck = !(Int[1] < m_pos[1] || Int[1] > Extent[1]);}
	if (Mask[2] != 0) {zCheck = !(Int[2] < m_pos[2] || Int[2] > Extent[2]);}
	return xCheck && yCheck && zCheck;
}

bool Cube::DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M)
{
	R.Direction.normalize();
	Vector3D xNorm = Vector3D(1, 0, 0);
	Vector3D yNorm = Vector3D(0, 1, 0);
	Vector3D zNorm = Vector3D(0, 0, 1);

	Vector3D Norms[6];
	Norms[0] = -xNorm;
	Norms[1] = -yNorm;
	Norms[2] = -zNorm;
	Norms[3] = xNorm;
	Norms[4] = yNorm;
	Norms[5] = zNorm;

	bool ret, retry;
	do{
		ret = retry = false;
		Point3D PointOnPlane = m_pos;
		Vector3D Mask(1, 1, 1);
		for (int i=0;i<6;i++)
		{
			if (i == 3)
			{
				PointOnPlane = m_pos + Point3D(m_size, m_size, m_size);
				Mask = Vector3D(-1, -1, -1);
			}
			Vector3D Norm = Norms[i];
			double D = SolveForD(PointOnPlane, Norm);
			double S = -(D + (-SolveForD(R.Start, Norm)))/(Norm.dot(R.Direction));
			if (S <= 0) continue;
			Vector3D rayAdd = S*R.Direction;
			Point3D rayInt = R.Start + rayAdd;
			Vector3D NewMask = Mask + Norm;
			if (IsInside(rayInt, NewMask))
			{
				Point3D WorldRay = M * R.Start;
				Point3D WorldHit = M * rayInt;
				if (clampDist(closestDist, WorldRay, WorldHit, Norm, Hit, M))
				{
					ret = true;
				}

				if (CheckCloseHit(WorldRay, WorldHit))
				{
					R.Start = R.Start + EPSILON*R.Direction;
					retry = true;
				}
			}
		}
	} while(retry);

	return ret;
}

Cylinder::~Cylinder()
{
}

bool Cylinder::DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M)
{
	R.Direction.normalize();
	bool retry;
	do{
		retry = false;
		double XD = R.Direction[0], YD = R.Direction[1], ZD = R.Direction[2],
			   XE = R.Start[0], YE = R.Start[1], ZE = R.Start[2], roots[2];
		size_t numRoots = quadraticRoots(XD*XD + YD*YD, 2*XE*XD + 2*YE*YD, XE*XE + YE*YE - 1.f, roots);
		if (numRoots > 0)
		{
			Vector3D Normal;
			double s = -1.f;
			if (numRoots == 1 && roots[0] > 0) s = roots[0];
			else if (numRoots == 2)
			{
				bool fPstv = roots[0] > 0, sPstv = roots[1] > 0;
				if (!fPstv && !sPstv) return false;	// no hits in front
				s = std::min(fPstv ? roots[0] : 0, sPstv ? roots[1] : 0);
				double s2 = std::max(roots[0], roots[1]);
				Point3D hitLoc1 = R.Start + s * R.Direction;
				Point3D hitLoc2 = R.Start + s2 * R.Direction;
				Normal = (hitLoc1 - Point3D(0,0,hitLoc1[2]));
				if (hitLoc1[2] > 1.f && hitLoc2[2] < 1.f)
				{
					s = (1.f - ZE)/ZD;
					Normal = Vector3D(0,0,1);
				}
				else if (hitLoc1[2] < -1.f && hitLoc2[2] > -1.f)
				{
					s = (-1.f - ZE)/ZD;
					Normal = Vector3D(0,0,-1);
				}
			}
			if (s <= 0) return false;	// no good hit

			Vector3D rayVec = s * R.Direction;
			Point3D hitLoc = R.Start + rayVec;

			Point3D WorldRay = M * R.Start;
			Point3D WorldHit = M * hitLoc;

			if (hitLoc[2] > -1.005f && hitLoc[2] < 1.005f)
			{
				if (clampDist(closestDist, WorldRay, WorldHit, Normal, Hit, M))
				{
					return true;
				}

				if (CheckCloseHit(WorldRay, WorldHit))
				{
					R.Start = R.Start + EPSILON*R.Direction;
					retry = true;
				}
			}
		}
	} while(retry);
	return false;
}

Cone::~Cone()
{
}

bool Cone::DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M)
{
	R.Direction.normalize();
	double XD = R.Direction[0], YD = R.Direction[1], ZD = R.Direction[2],
		   XE = R.Start[0], YE = R.Start[1], ZE = R.Start[2], roots[2];
	size_t numRoots = quadraticRoots(XD*XD + YD*YD - ZD*ZD, 2*XE*XD + 2*YE*YD - 2*ZE*ZD, XE*XE + YE*YE - ZE*ZE, roots);
	if (numRoots > 0)
	{
		std::vector<Point3D> Hits(3);
		std::vector<Vector3D> Normals(3);
		Vector3D rayVec = R.Direction;

		// Find cone hit
		Point3D hitLoc = R.Start + roots[0] * rayVec;
		if (hitLoc[2] > -0.0005f && hitLoc[2] < 1.0005f) 
		{
			Hits.push_back(hitLoc);
			Normals.push_back(Vector3D(2*hitLoc[0], 2*hitLoc[1], -2*hitLoc[2]));
		}
		if (numRoots == 2)
		{
			hitLoc = R.Start + roots[1] * rayVec;
			if (hitLoc[2] > -0.0005f && hitLoc[2] < 1.0005f)
			{
				Hits.push_back(hitLoc);
				Normals.push_back(Vector3D(2*hitLoc[0], 2*hitLoc[1], -2*hitLoc[2]));
			}
		}

		// Find cone cap hit
		Vector3D Normal;
		double S = (1.f - ZE)/ZD;
		hitLoc = R.Start + S * rayVec;
		if (hitLoc[2] > -0.0005f && hitLoc[2] < 1.0005f && (hitLoc[0]*hitLoc[0] + hitLoc[1]*hitLoc[1]) <= 1.f)
		{
			Hits.push_back(hitLoc);
			Normals.push_back(Vector3D(0,0,1));
		}

		if (Hits.size() > 0)
		{
			hitLoc = Point3D(100000000.f, 100000000.f, 100000000.f);
			int i = 0;
			for (Point3D& P : Hits)
			{
				if ((P - R.Start).length2() < (hitLoc - R.Start).length2())
				{
					hitLoc = P;
					Normal = Normals[i];
				}
				i++;
			}

			Point3D WorldRay = M * R.Start;
			Point3D WorldHit = M * hitLoc;

			if (clampDist(closestDist, WorldRay, WorldHit, Normal, Hit, M))
			{
				return true;
			}
		}
	}
	return false;
}

NonhierSphere::~NonhierSphere()
{
}

bool NonhierSphere::SimpleTrace(Ray R)
{
	R.Direction.normalize();
	Vector3D deltaP = m_pos - R.Start;
	return (((m_radius*m_radius) - (deltaP - (R.Direction.dot(deltaP))*R.Direction).length2()) >= 0);
}

bool NonhierSphere::DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M)
{
	bool retry;
	Matrix4x4 Mat = M * m_trans;
	R.Transform(m_invtrans);
	R.Direction.normalize();
	do{
		retry = false;
		Vector3D deltaP = m_pos - R.Start;
		double uDotDeltaP = R.Direction.dot(deltaP);
		double discriminant = (m_radius*m_radius - (deltaP - (uDotDeltaP)*R.Direction).length2());
		if (discriminant >=0)
		{
			double sqrtDisc = sqrt(discriminant);
			double s = std::min(uDotDeltaP + sqrtDisc, uDotDeltaP - sqrtDisc);
			if (s < 0)
			{
				if (uDotDeltaP + sqrtDisc > 0) s = uDotDeltaP + sqrtDisc;
				else if (uDotDeltaP - sqrtDisc > 0) s = uDotDeltaP - sqrtDisc;
				else return false;
			}
			Vector3D rayVec = s * R.Direction;
			Point3D hitLoc = R.Start + rayVec;
			Vector3D Normal = (hitLoc - m_pos);

			Point3D WorldRay = Mat * R.Start;
			Point3D WorldHit = Mat * hitLoc;

			if (clampDist(closestDist, WorldRay, WorldHit, Normal, Hit, Mat))
			{
				return true;
			}

			if (CheckCloseHit(WorldRay, WorldHit))
			{
				R.Start = R.Start + EPSILON*R.Direction;
				retry = true;
			}
		}
	} while(retry);
	return false;
}

NonhierBox::~NonhierBox()
{
}

bool NonhierBox::SimpleTrace(Ray R)
{
	(void)R; return false;
}

bool NonhierBox::IsInside(Point3D Int, Vector3D Mask)
{
	bool xCheck = true, yCheck = true, zCheck = true;
	Point3D Extent(m_pos[0] + m_size, m_pos[1] + m_size, m_pos[2] + m_size);
	if (Mask[0] != 0) {xCheck = !(Int[0] < m_pos[0] || Int[0] > Extent[0]);}
	if (Mask[1] != 0) {yCheck = !(Int[1] < m_pos[1] || Int[1] > Extent[1]);}
	if (Mask[2] != 0) {zCheck = !(Int[2] < m_pos[2] || Int[2] > Extent[2]);}
	return xCheck && yCheck && zCheck;
}

bool NonhierBox::DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M)
{
	R.Direction.normalize();
	Vector3D xNorm = Vector3D(1, 0, 0);
	Vector3D yNorm = Vector3D(0, 1, 0);
	Vector3D zNorm = Vector3D(0, 0, 1);

	Vector3D Norms[6];
	Norms[0] = -xNorm;
	Norms[1] = -yNorm;
	Norms[2] = -zNorm;
	Norms[3] = xNorm;
	Norms[4] = yNorm;
	Norms[5] = zNorm;

	bool ret = false;
	Point3D PointOnPlane = m_pos;
	Vector3D Mask(1, 1, 1);
	for (int i=0;i<6;i++)
	{
		if (i == 3)
		{
			PointOnPlane = m_pos + Point3D(m_size, m_size, m_size);
			Mask = Vector3D(-1, -1, -1);
		}
		Vector3D Norm = Norms[i];
		double D = SolveForD(PointOnPlane, Norm);
		double S = -(D + (-SolveForD(R.Start, Norm)))/(Norm.dot(R.Direction));
		if (S <= 0) continue;
		Vector3D rayAdd = S*R.Direction;
		Point3D rayInt = R.Start + rayAdd;
		Vector3D NewMask = Mask + Norm;
		if (IsInside(rayInt, NewMask))
		{
			Point3D WorldRay = M * R.Start;
			Point3D WorldHit = M * rayInt;
			if (clampDist(closestDist, WorldRay, WorldHit, Norm, Hit, M))
			{
				ret = true;
			}
		}
	}

	return ret;
}

bool clampDist(double& closestDist, const Point3D& WorldRay, const Point3D& WorldHit, const Vector3D& Normal, HitInfo& Hit, const Matrix4x4& M)
{
	double Dist = (WorldHit - WorldRay).length();
	if (closestDist > Dist && Dist > EPSILON)
	{
		closestDist = Dist;
		Hit.Location = WorldHit;
		Hit.Normal = Normal;
		Hit.Normal = transNorm(M.invert(), Hit.Normal);
		return true;
	}
	return false;
}

bool CheckCloseHit(const Point3D& WorldRay, const Point3D& WorldHit)
{
	return (WorldHit - WorldRay).length() < EPSILON;
}