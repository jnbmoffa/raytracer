#ifndef CS488_MESH_HPP
#define CS488_MESH_HPP

#include <vector>
#include <iosfwd>
#include "primitive.hpp"
#include "algebra.hpp"

// A polygonal mesh.
class Mesh : public Primitive {
public:
	typedef std::vector<int> Face;

  Mesh(const std::vector<Point3D>& verts,
       const std::vector< Face >& faces);

  virtual bool DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M);
  
private:
	Sphere BoundingSphere;

	std::vector<Point3D> m_verts;
	std::vector<Face> m_faces;

	friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
};

#endif
