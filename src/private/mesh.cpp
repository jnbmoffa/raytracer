#include "mesh.hpp"
#include <iostream>
#include <limits>

Mesh::Mesh(const std::vector<Point3D>& verts, const std::vector<std::vector<int>>& faces) :
    m_verts(verts),
    m_faces(faces)
{
    double MaxX, MaxY, MaxZ, MinX, MinY, MinZ;
    MaxX = MaxY = MaxZ = -1000000.0;
    MinX = MinY = MinZ =  1000000.0;
    for (std::vector<Point3D>::const_iterator iter = verts.begin(); iter != verts.end(); ++iter)
    {
        Point3D P = *iter;
        MaxX = std::max<double>(MaxX, P[0]);
        MaxY = std::max<double>(MaxY, P[1]);
        MaxZ = std::max<double>(MaxZ, P[2]);
        MinX = std::min<double>(MinX, P[0]);
        MinY = std::min<double>(MinY, P[1]);
        MinZ = std::min<double>(MinZ, P[2]);
    }
    const Point3D center((MaxX + MinX) * 0.5, (MaxY + MinY) * 0.5, (MaxZ + MinZ) * 0.5);
    const double radius = std::max(std::abs(center[0] - MaxX), std::max(std::abs(center[0] - MinX),
                                   std::max(std::abs(center[1] - MaxY), std::max(std::abs(center[1] - MinY),
                                            std::max(std::abs(center[2] - MaxZ), std::abs(center[2] - MinZ))))));
    Bounds = BoxF(center[0] - radius, center[0] + radius, center[1] + radius, center[1] - radius, center[2] + radius, center[2] - radius);
}

bool Mesh::DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M)
{
    bool ret = false;
    R.Direction.normalize();
    if (Bounds.Intersects(R))
    {
        for (std::vector<Face>::iterator iter = m_faces.begin(); iter != m_faces.end(); ++iter)
        {
            Face F = *iter;
            if (F.size() > 2)
            {
                Vector3D Norm = cross((m_verts[F[1]] - m_verts[F[0]]), m_verts[F[2]] - m_verts[F[0]]);
                Norm.normalize();
                double D = SolveForD(m_verts[F[0]], Norm);
                double S = -(D + (-SolveForD(R.Start, Norm))) / (Norm.dot(R.Direction));
                if (S <= 0)
                {
                    continue;
                }
                Vector3D rayAdd = S * R.Direction;
                Point3D rayInt = R.Start + rayAdd;

                // Flatten all vectors based on largest normal component
                int IgnoreIdx = 0;
                for (int i = 0; i < 3; i++)
                {
                    if (std::abs(Norm[i]) > std::abs(Norm[(i + 1) % 3]) && std::abs(Norm[i]) > std::abs(Norm[(i + 2) % 3]))
                    {
                        IgnoreIdx = i;
                        break;
                    }
                }

                // Get projected hit point
                Point3D ProjHit = rayInt;
                ProjHit[IgnoreIdx] = 0;
                unsigned int count = 0;
                for (unsigned int it = 0; it < F.size(); it++)
                {
                    Point3D L1 = m_verts[F[it]];
                    L1[IgnoreIdx] = 0;
                    Point3D L2 = m_verts[F[(it + 1) % F.size()]];
                    L2[IgnoreIdx] = 0;
                    Vector3D LDir = L2 - L1;
                    Vector3D Cross = cross(Norm, LDir);

                    if ((ProjHit - L1).dot(Cross) > 0)
                    {
                        count++;
                    }
                    else
                    {
                        break;
                    }
                }

                // On the same side of every line?
                if (count == F.size())
                {
                    Point3D WorldRay = M * R.Start;
                    Point3D WorldHit = M * rayInt;
                    if (clampDist(closestDist, WorldRay, WorldHit, Norm, Hit, M))
                    {
                        ret = true;
                    }
                }
            }
        }
    }
    return ret;
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
    std::cerr << "mesh({";
    for (std::vector<Point3D>::const_iterator I = mesh.m_verts.begin(); I != mesh.m_verts.end(); ++I)
    {
        if (I != mesh.m_verts.begin())
        {
            std::cerr << ",\n      ";
        }
        std::cerr << *I;
    }
    std::cerr << "},\n\n     {";

    for (std::vector<Mesh::Face>::const_iterator I = mesh.m_faces.begin(); I != mesh.m_faces.end(); ++I)
    {
        if (I != mesh.m_faces.begin())
        {
            std::cerr << ",\n      ";
        }
        std::cerr << "[";
        for (Mesh::Face::const_iterator J = I->begin(); J != I->end(); ++J)
        {
            if (J != I->begin())
            {
                std::cerr << ", ";
            }
            std::cerr << *J;
        }
        std::cerr << "]";
    }
    std::cerr << "});" << std::endl;
    return out;
}
