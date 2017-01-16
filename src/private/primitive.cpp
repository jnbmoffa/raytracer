#include "primitive.hpp"
#include "polyroots.hpp"
#include "AxisAlignedBox.h"
#include <limits>
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
    R.Normalize();
    const Vector3D rayDir = R.GetDirection();

    Vector3D deltaP = m_center - R.GetOrigin();
    return (((m_radius * m_radius) - (deltaP - (rayDir.dot(deltaP)) * rayDir).length2()) >= 0);
}

bool Sphere::DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M)
{
    R.Normalize();

    bool retry;
    do
    {
        const Point3D rayOrigin = R.GetOrigin();
        const Vector3D rayDir = R.GetDirection();

        retry = false;
        Vector3D deltaP = m_center - rayOrigin;
        double uDotDeltaP = rayDir.dot(deltaP);
        double discriminant = (m_radius * m_radius - (deltaP - (uDotDeltaP) * rayDir).length2());
        // std::cout << discriminant << std::endl;
        if (discriminant >= 0)
        {
            double sqrtDisc = sqrt(discriminant);
            double s = std::min(uDotDeltaP + sqrtDisc, uDotDeltaP - sqrtDisc);
            if (s < 0)
            {
                if (uDotDeltaP + sqrtDisc > 0)
                {
                    s = uDotDeltaP + sqrtDisc;
                }
                else if (uDotDeltaP - sqrtDisc > 0)
                {
                    s = uDotDeltaP - sqrtDisc;
                }
                else
                {
                    return false;
                }
            }
            Vector3D rayVec = s * rayDir;
            Point3D hitLoc = rayOrigin + rayVec;
            Vector3D Normal = (hitLoc - m_center);

            Point3D WorldRay = M * rayOrigin;
            Point3D WorldHit = M * hitLoc;
            if (clampDist(closestDist, WorldRay, WorldHit, Normal, Hit, M))
            {
                return true;
            }

            if (CheckCloseHit(WorldRay, WorldHit))
            {
                R.SetOrigin(rayOrigin + EPSILON * rayDir);
                retry = true;
            }
        }
    }
    while (retry);
    return false;
}

bool Cube::DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M)
{
    R.Normalize();

    bool ret = false, retry;
    do
    {
        const Point3D rayOrigin = R.GetOrigin();

        retry = false;
        if (GetIntersection(R, Bounds, Hit))
        {
            const Point3D WorldRay = M * rayOrigin;
            const Point3D WorldHit = M * Hit.Location;
            if (clampDist(closestDist, WorldRay, WorldHit, Hit.Normal, Hit, M))
            {
                ret = true;
            }
            else if (CheckCloseHit(WorldRay, WorldHit))
            {
                R.SetOrigin(rayOrigin + (EPSILON * R.GetDirection()));
                retry = true;
            }
        }
    }
    while (retry);

    return ret;
}

Cylinder::~Cylinder()
{
}

bool Cylinder::DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M)
{
    R.Normalize();
    bool retry;
    do
    {
        const Point3D rayOrigin = R.GetOrigin();
        const Vector3D rayDir = R.GetDirection();

        retry = false;
        double XD = rayDir[0], YD = rayDir[1], ZD = rayDir[2],
               XE = rayOrigin[0], YE = rayOrigin[1], ZE = rayOrigin[2], roots[2];
        size_t numRoots = quadraticRoots(XD * XD + YD * YD, 2 * XE * XD + 2 * YE * YD, XE * XE + YE * YE - 1.f, roots);
        if (numRoots > 0)
        {
            Vector3D Normal;
            double s = -1.f;
            if (numRoots == 1 && roots[0] > 0)
            {
                s = roots[0];
            }
            else if (numRoots == 2)
            {
                bool fPstv = roots[0] > 0, sPstv = roots[1] > 0;
                if (!fPstv && !sPstv)
                {
                    return false;    // no hits in front
                }
                s = std::min(fPstv ? roots[0] : 0, sPstv ? roots[1] : 0);
                double s2 = std::max(roots[0], roots[1]);
                Point3D hitLoc1 = rayOrigin + s * rayDir;
                Point3D hitLoc2 = rayOrigin + s2 * rayDir;
                Normal = (hitLoc1 - Point3D(0, 0, hitLoc1[2]));
                if (hitLoc1[2] > 1.f && hitLoc2[2] < 1.f)
                {
                    s = (1.f - ZE) / ZD;
                    Normal = Vector3D(0, 0, 1);
                }
                else if (hitLoc1[2] < -1.f && hitLoc2[2] > -1.f)
                {
                    s = (-1.f - ZE) / ZD;
                    Normal = Vector3D(0, 0, -1);
                }
            }
            if (s <= 0)
            {
                return false;    // no good hit
            }

            Vector3D rayVec = s * rayDir;
            Point3D hitLoc = rayOrigin + rayVec;

            Point3D WorldRay = M * rayOrigin;
            Point3D WorldHit = M * hitLoc;

            if (hitLoc[2] > -1.005f && hitLoc[2] < 1.005f)
            {
                if (clampDist(closestDist, WorldRay, WorldHit, Normal, Hit, M))
                {
                    return true;
                }

                if (CheckCloseHit(WorldRay, WorldHit))
                {
                    R.SetOrigin(rayOrigin + EPSILON * rayDir);
                    retry = true;
                }
            }
        }
    }
    while (retry);
    return false;
}

Cone::~Cone()
{
}

bool Cone::DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M)
{
    R.Normalize();
    const Point3D rayOrigin = R.GetOrigin();
    const Vector3D rayDir = R.GetDirection();

    if (CheckIntersection(R, Bounds))
    {
        double XD = rayDir[0], YD = rayDir[1], ZD = rayDir[2],
               XE = rayOrigin[0], YE = rayOrigin[1], ZE = rayOrigin[2], roots[2];
        size_t numRoots = quadraticRoots(XD * XD + YD * YD - ZD * ZD, 2 * XE * XD + 2 * YE * YD - 2 * ZE * ZD, XE * XE + YE * YE - ZE * ZE, roots);
        if (numRoots > 0)
        {
            std::vector<Point3D> Hits;      // do not use reserve
            std::vector<Vector3D> Normals;  // do not use reserve
            Vector3D rayVec = rayDir;

            // Find cone hit
            Point3D hitLoc = rayOrigin + roots[0] * rayVec;
            if (hitLoc[2] > -0.0005f && hitLoc[2] < 1.0005f)
            {
                Hits.push_back(hitLoc);
                Normals.push_back(Vector3D(2 * hitLoc[0], 2 * hitLoc[1], -2 * hitLoc[2]));
            }
            if (numRoots == 2)
            {
                hitLoc = rayOrigin + roots[1] * rayVec;
                if (hitLoc[2] > -0.0005f && hitLoc[2] < 1.0005f)
                {
                    Hits.push_back(hitLoc);
                    Normals.push_back(Vector3D(2 * hitLoc[0], 2 * hitLoc[1], -2 * hitLoc[2]));
                }
            }

            // Find cone cap hit
            Vector3D Normal;
            double S = (1.f - ZE) / ZD;
            hitLoc = rayOrigin + S * rayVec;
            if (hitLoc[2] > -0.0005f && hitLoc[2] < 1.0005f && (hitLoc[0]*hitLoc[0] + hitLoc[1]*hitLoc[1]) <= 1.f)
            {
                Hits.push_back(hitLoc);
                Normals.push_back(Vector3D(0, 0, 1));
            }

            if (Hits.size() > 0)
            {
                hitLoc = Point3D(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
                int i = 0;
                for (Point3D& P : Hits)
                {
                    if ((P - rayOrigin).length2() < (hitLoc - rayOrigin).length2())
                    {
                        hitLoc = P;
                        Normal = Normals[i];
                    }
                    ++i;
                }

                Point3D WorldRay = M * rayOrigin;
                Point3D WorldHit = M * hitLoc;

                if (clampDist(closestDist, WorldRay, WorldHit, Normal, Hit, M))
                {
                    return true;
                }
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
    R.Normalize();
    const Vector3D rayDir = R.GetDirection();

    Vector3D deltaP = m_pos - R.GetOrigin();
    return (((m_radius * m_radius) - (deltaP - (rayDir.dot(deltaP)) * rayDir).length2()) >= 0);
}

bool NonhierSphere::DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M)
{
    bool retry;
    Matrix4x4 Mat = M * m_trans;
    R.Transform(m_invtrans);
    R.Normalize();

    do
    {
        const Point3D rayOrigin = R.GetOrigin();
        const Vector3D rayDir = R.GetDirection();

        retry = false;
        Vector3D deltaP = m_pos - rayOrigin;
        double uDotDeltaP = rayDir.dot(deltaP);
        double discriminant = (m_radius * m_radius - (deltaP - (uDotDeltaP) * rayDir).length2());
        if (discriminant >= 0)
        {
            double sqrtDisc = sqrt(discriminant);
            double s = std::min(uDotDeltaP + sqrtDisc, uDotDeltaP - sqrtDisc);
            if (s < 0)
            {
                if (uDotDeltaP + sqrtDisc > 0)
                {
                    s = uDotDeltaP + sqrtDisc;
                }
                else if (uDotDeltaP - sqrtDisc > 0)
                {
                    s = uDotDeltaP - sqrtDisc;
                }
                else
                {
                    return false;
                }
            }
            Vector3D rayVec = s * rayDir;
            Point3D hitLoc = rayOrigin + rayVec;
            Vector3D Normal = (hitLoc - m_pos);

            Point3D WorldRay = Mat * rayOrigin;
            Point3D WorldHit = Mat * hitLoc;

            if (clampDist(closestDist, WorldRay, WorldHit, Normal, Hit, Mat))
            {
                return true;
            }

            if (CheckCloseHit(WorldRay, WorldHit))
            {
                R.SetOrigin(rayOrigin + EPSILON * rayDir);
                retry = true;
            }
        }
    }
    while (retry);
    return false;
}

inline bool CheckCloseHit(const Point3D& WorldRay, const Point3D& WorldHit)
{
    return (WorldHit - WorldRay).length2() < EPSILON2;
}