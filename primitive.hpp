#ifndef CS488_PRIMITIVE_HPP
#define CS488_PRIMITIVE_HPP

#include "algebra.hpp"

#define EPSILON 0.005

class Primitive {
public:
  virtual ~Primitive();
  virtual bool SimpleTrace(Ray R) { (void)R; return false; }
  virtual bool DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M)
    { (void)R; (void)closestDist; (void)M; (void)Hit; return false; }
};

class Sphere : public Primitive {
public:
  Sphere() : m_center(0, 0, 0), m_radius(1) {}
  Sphere(const Point3D& C, double R) : m_center(C), m_radius(R) {}
  virtual ~Sphere();

  virtual bool SimpleTrace(Ray R);
  virtual bool DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M);
private:
  Point3D m_center;
  double m_radius;
};

class Cube : public Primitive {
public:
  Cube() : m_pos(0,0,0), m_size(1.f) {}
  virtual ~Cube();
  bool IsInside(Point3D Int, Vector3D Mask);
  virtual bool DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M);

private:
  Point3D m_pos;
  double m_size;
};

class Cylinder : public Primitive {
public:
  virtual ~Cylinder();
  virtual bool DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M);
};

class Cone : public Primitive {
public:
  virtual ~Cone();
  virtual bool DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M);
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const Point3D& pos, double radius)
    : m_pos(0, 0, 0), m_radius(1)
  {
    m_trans.translate(Vector3D(pos[0], pos[1], pos[2]));
    m_trans.scale(Vector3D(radius, radius, radius));
    m_invtrans = m_trans.invert();
  }
  virtual ~NonhierSphere();

  virtual bool SimpleTrace(Ray R);
  virtual bool DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M);

private:
  Point3D m_pos;
  double m_radius;
  Matrix4x4 m_trans;
  Matrix4x4 m_invtrans;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const Point3D& pos, double size)
    : m_pos(pos), m_size(size)
  {
  }

  bool IsInside(Point3D Int, Vector3D Mask);
  virtual bool SimpleTrace(Ray R);
  virtual bool DepthTrace(Ray R, double& closestDist, HitInfo& Hit, const Matrix4x4& M);
  
  virtual ~NonhierBox();

private:
  Point3D m_pos;
  double m_size;
};

bool clampDist(double& closestDist, const Point3D& WorldRay, const Point3D& WorldHit, const Vector3D& Normal, HitInfo& Hit, const Matrix4x4& M);

bool CheckCloseHit(const Point3D& WorldRay, const Point3D& WorldHit);

inline double SolveForD(Point3D P, Vector3D N)
{
  return -(P[0]*N[0] + P[1]*N[1] +P[2]*N[2]);
}

#endif
