#include "scene.hpp"
#include <iostream>

SceneNode::SceneNode(const std::string& name, Matrix4x4 M)
  : m_name(name),
    m_trans(M)
{
}

SceneNode::~SceneNode()
{
}

bool SceneNode::SimpleTrace(Ray R)
{
  R.Transform(m_invtrans);

  for (ChildList::iterator iter = m_children.begin(); iter != m_children.end(); ++iter)
  {
    SceneNode* Node = *iter;
    if (Node->SimpleTrace(R)) return true;
  }
  return false;
}

bool SceneNode::DepthTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M)
{
  R.Transform(m_invtrans); Matrix4x4 T(M * m_trans);
  
  bool ret = false;
  for (ChildList::iterator iter = m_children.begin(); iter != m_children.end(); ++iter)
  {
    SceneNode* Node = *iter;
    if (Node->DepthTrace(R, closestDist, Hit, T)) ret = true;
  }
  return ret;
}

bool SceneNode::ColourTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M)
{
  R.Transform(m_invtrans); Matrix4x4 T(M * m_trans);

  bool ret = false;
  for (ChildList::iterator iter = m_children.begin(); iter != m_children.end(); ++iter)
  {
    SceneNode* Node = *iter;
    if (Node->ColourTrace(R, closestDist, Hit, T))
    {
      ret = true;
    }
  }
  return ret;
}

void SceneNode::FlattenScene(Array<SceneNode*>& List, Matrix4x4 M)
{
  for (SceneNode* s : m_children)
  {
    s->FlattenScene(List, M * m_trans);
  }
}

void SceneNode::rotate(char axis, double angle)
{
  //std::cerr << "Stub: Rotate " << m_name << " around " << axis << " by " << angle << std::endl;
  m_trans.rotate(axis, angle);
  m_invtrans = m_trans.invert();
}

void SceneNode::scale(const Vector3D& amount)
{
  //std::cerr << "Stub: Scale " << m_name << " by " << amount << std::endl;
  m_trans.scale(amount);
  m_invtrans = m_trans.invert();
}

void SceneNode::translate(const Vector3D& amount)
{
  //std::cerr << "Stub: Translate " << m_name << " by " << amount << std::endl;
  m_trans.translate(amount);
  m_invtrans = m_trans.invert();
}

bool SceneNode::is_joint() const
{
  return false;
}

JointNode::JointNode(const std::string& name)
  : SceneNode(name)
{
}

JointNode::~JointNode()
{
}

bool JointNode::is_joint() const
{
  return true;
}

void JointNode::set_joint_x(double min, double init, double max)
{
  m_joint_x.min = min;
  m_joint_x.init = init;
  m_joint_x.max = max;
}

void JointNode::set_joint_y(double min, double init, double max)
{
  m_joint_y.min = min;
  m_joint_y.init = init;
  m_joint_y.max = max;
}

GeometryNode::GeometryNode(const std::string& name, Primitive* primitive)
  : SceneNode(name),
    m_primitive(primitive)
{
}

GeometryNode::GeometryNode(const std::string& name, Primitive* primitive, Material* Mat, Matrix4x4 M)
  : SceneNode(name, M),
    m_material(Mat),
    m_primitive(primitive)
{
  m_invtrans = m_trans.invert();
}

GeometryNode::~GeometryNode()
{
  if (m_primitive) delete m_primitive;
}

void GeometryNode::FlattenScene(Array<SceneNode*>& List, Matrix4x4 M)
{
  List.Add(new GeometryNode(m_name, m_primitive, m_material, M * m_trans));
}

bool GeometryNode::SimpleTrace(Ray R)
{
  if (m_primitive->SimpleTrace(R)) return true;
  return false;
}

bool GeometryNode::DepthTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M)
{
  R.Transform(m_invtrans);

  bool ret = false;
  if (m_primitive->DepthTrace(R, closestDist, Hit, M * m_trans))
  {
    ret = true;
  }
  return ret;
}

bool GeometryNode::ColourTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M)
{
  R.Transform(m_invtrans);

  if (m_primitive->DepthTrace(R, closestDist, Hit, M * m_trans))
  {
    Hit.Mat = m_material;
    return true;
  }
  return false;
}

BoxF GeometryNode::GetBox()
{
  BoxF Bounds = m_primitive->GetBox();
  Bounds.Transform(m_trans);
  return Bounds;
}