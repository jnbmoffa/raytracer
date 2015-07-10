#ifndef SCENE_HPP
#define SCENE_HPP

#include <list>
#include "algebra.hpp"
#include "primitive.hpp"
#include "material.hpp"

class SceneNode {
public:
  SceneNode(const std::string& name, Matrix4x4 M = Matrix4x4());
  virtual ~SceneNode();

  virtual bool SimpleTrace(Ray R);
  virtual bool DepthTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M);
  virtual bool ColourTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M);

  virtual void FlattenScene(std::list<SceneNode*>& List, Matrix4x4 M = Matrix4x4());

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

  void add_child(SceneNode* child)
  {
    m_children.push_back(child);
  }

  void remove_child(SceneNode* child)
  {
    m_children.remove(child);
  }

  // Callbacks to be implemented.
  // These will be called from Lua.
  void rotate(char axis, double angle);
  void scale(const Vector3D& amount);
  void translate(const Vector3D& amount);

  // Returns true if and only if this node is a JointNode
  virtual bool is_joint() const;

  std::string m_name;
protected:
  
  // Useful for picking
  int m_id;

  // Transformations
  Matrix4x4 m_trans;
  Matrix4x4 m_invtrans;

  // Hierarchy
  typedef std::list<SceneNode*> ChildList;
  ChildList m_children;
};

class JointNode : public SceneNode {
public:
  JointNode(const std::string& name);
  virtual ~JointNode();

  virtual bool is_joint() const;

  void set_joint_x(double min, double init, double max);
  void set_joint_y(double min, double init, double max);

  struct JointRange {
    double min, init, max;
  };

  
protected:

  JointRange m_joint_x, m_joint_y;
};

class GeometryNode : public SceneNode {
public:
  GeometryNode(const std::string& name, Primitive* primitive);
  GeometryNode(const std::string& name, Primitive* primitive, Material* Mat, Matrix4x4 M = Matrix4x4());
  virtual ~GeometryNode();

  virtual void FlattenScene(std::list<SceneNode*>& List, Matrix4x4 M = Matrix4x4());

  virtual bool SimpleTrace(Ray R);
  virtual bool DepthTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M);
  virtual bool ColourTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M);

  const Material* get_material() const;
  Material* get_material();

  void set_material(Material* material)
  {
    m_material = material;
  }

protected:
  Material* m_material;
  Primitive* m_primitive;
};

#endif
