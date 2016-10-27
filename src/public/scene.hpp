#ifndef SCENE_HPP
#define SCENE_HPP

#include <list>
#include <memory>
#include "algebra.hpp"
#include "primitive.hpp"
#include "material.hpp"
#include "octree.h"
#include <vector>

class SceneNode {
public:
  SceneNode(const std::string& name, Matrix4x4 M = Matrix4x4());
  virtual ~SceneNode() = default;

  virtual bool SimpleTrace(Ray R);
  virtual bool DepthTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M);
  virtual bool ColourTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M);
  virtual bool TimeTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M, const double& Time);

  virtual void FlattenScene(std::vector<std::unique_ptr<SceneNode>>& List, Matrix4x4 M = Matrix4x4());

  const Matrix4x4& get_transform() const { return m_trans; }
  const Matrix4x4& get_inverse() const { return m_invtrans; }
  
  void set_transform(const Matrix4x4& m)
  {
    m_trans = m;
    m_invtrans = m.invert();
  }

  void set_transform(const Matrix4x4& m, const Matrix4x4& i)
  {
    m_trans = m;
    m_invtrans = i;
  }

  void add_child(std::shared_ptr<SceneNode>& child)
  {
    m_children.emplace_back(child);
  }

  void remove_child(std::shared_ptr<SceneNode>& child)
  {
    m_children.remove(child);
  }

  // These are called from Lua.
  void rotate(char axis, double angle);
  void scale(const Vector3D& amount);
  void translate(const Vector3D& amount);

  // Returns true if and only if this node is a JointNode
  virtual bool is_joint() const;

  virtual BoxF GetBox() { return BoxF(); }

  std::string m_name;
protected:
  
  // Useful for picking
  int m_id;

  // Transformations
  Matrix4x4 m_trans;
  Matrix4x4 m_invtrans;

  // Hierarchy
  typedef std::list<std::shared_ptr<SceneNode>> ChildList;
  ChildList m_children;
};

class JointNode : public SceneNode {
public:
  JointNode(const std::string& name);
  virtual ~JointNode() = default;

  virtual bool is_joint() const;

  void set_joint_x(double min, double init, double max);
  void set_joint_y(double min, double init, double max);

  struct JointRange {
  double min, init, max;
};

  
protected:

  JointRange m_joint_x, m_joint_y;
};

class GeometryNode : public SceneNode, OcTreeObject {
public:
  GeometryNode(const std::string& name, std::shared_ptr<Primitive> primitive, Vector3D Velocity = Vector3D());
  GeometryNode(const std::string& name, std::shared_ptr<Primitive> primitive, std::shared_ptr<Material>& Mat, Matrix4x4 M = Matrix4x4(), Vector3D Velocity = Vector3D());
  virtual ~GeometryNode() = default;

  virtual void FlattenScene(std::vector<std::unique_ptr<SceneNode>>& List, Matrix4x4 M = Matrix4x4());

  virtual bool SimpleTrace(Ray R) override;
  virtual bool DepthTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M) override;
  virtual bool ColourTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M) override;
  virtual bool TimeTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M, const double& Time) override;

  const Material* get_material() const;
  Material* get_material();

  virtual BoxF GetBox() override;

  void set_material(std::shared_ptr<Material>& material)
  {
    m_material = material;
  }

protected:
  // Linear veloctiy for motion blur (units/second)
  Vector3D Velocity;

  std::shared_ptr<Material> m_material;
  std::shared_ptr<Primitive> m_primitive;
};

BoxF GetSceneBounds(const std::vector<std::unique_ptr<SceneNode>>& Scene);

#endif
