#include "scene.hpp"
#include <iostream>

SceneNode::SceneNode(const std::string& name, Matrix4x4 M)
    : m_name(name),
      m_trans(M)
{
}

bool SceneNode::SimpleTrace(Ray R)
{
    R.Transform(m_invtrans);

    for (auto iter = m_children.begin(); iter != m_children.end(); ++iter)
    {
        auto& Node = *iter;
        if (Node->SimpleTrace(R))
        {
            return true;
        }
    }
    return false;
}

bool SceneNode::DepthTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M)
{
    R.Transform(m_invtrans);
    Matrix4x4 T(M * m_trans);

    bool ret = false;
    for (auto iter = m_children.begin(); iter != m_children.end(); ++iter)
    {
        auto& Node = *iter;
        if (Node->DepthTrace(R, closestDist, Hit, T))
        {
            ret = true;
        }
    }
    return ret;
}

bool SceneNode::ColourTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M)
{
    R.Transform(m_invtrans);
    Matrix4x4 T(M * m_trans);

    bool ret = false;
    for (auto iter = m_children.begin(); iter != m_children.end(); ++iter)
    {
        auto& Node = *iter;
        if (Node->ColourTrace(R, closestDist, Hit, T))
        {
            ret = true;
        }
    }
    return ret;
}

bool SceneNode::TimeTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M, const double& Time)
{
    (void)R;
    (void)closestDist;
    (void)Hit;
    (void)M;
    (void)Time;
    return false;
}

void SceneNode::FlattenScene(std::vector<std::unique_ptr<SceneNode>>& List, Matrix4x4 M)
{
    for (auto& s : m_children)
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

GeometryNode::GeometryNode(const std::string& name, std::shared_ptr<Primitive> primitive, Vector3D Velocity)
    : SceneNode(name),
      Velocity(Velocity),
      m_primitive(primitive)
{
}

GeometryNode::GeometryNode(const std::string& name, std::shared_ptr<Primitive> primitive, std::shared_ptr<Material>& Mat, Matrix4x4 M, Vector3D Velocity)
    : SceneNode(name, M),
      Velocity(Velocity),
      m_material(Mat),
      m_primitive(primitive)
{
    m_invtrans = m_trans.invert();
}

void GeometryNode::FlattenScene(std::vector<std::unique_ptr<SceneNode>>& List, Matrix4x4 M)
{
    List.emplace_back(std::make_unique<GeometryNode>(m_name, m_primitive, m_material, M * m_trans, Velocity));
}

bool GeometryNode::SimpleTrace(Ray R)
{
    if (m_primitive->SimpleTrace(R))
    {
        return true;
    }
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
        Hit.Mat = m_material.get();
        return true;
    }
    return false;
}

bool GeometryNode::TimeTrace(Ray R, double& closestDist, HitInfo& Hit, Matrix4x4& M, const double& Time)
{
    Matrix4x4 m_timetrans = M * m_trans;
    if (Velocity != Vector3D::ZeroVector)
    {
        m_timetrans.translate(Time * Velocity);
        R.Transform(m_timetrans.invert());
    }
    else
    {
        R.Transform(m_invtrans);
    }
    // std::cout << "Scene:" << R.Start << "," << R.Direction << std::endl;

    if (m_primitive->DepthTrace(R, closestDist, Hit, m_timetrans))
    {
        Hit.Mat = m_material.get();
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

BoxF GetSceneBounds(const std::vector<std::unique_ptr<SceneNode>>& Scene)
{
    const double posInf = std::numeric_limits<double>::max();
    const double negInf = std::numeric_limits<double>::min();

    double right = negInf;
    double left = posInf;
    double top = negInf;
    double bottom = posInf;
    double front = negInf;
    double back = posInf;

    for (auto& Node : Scene)
    {
        const BoxF B = Node->GetBox();
        right = std::max(right, B.GetRight());
        left = std::min(left, B.GetLeft());
        top = std::max(top, B.GetTop());
        bottom = std::min(bottom, B.GetBottom());
        front = std::max(front, B.GetFront());
        back = std::min(back, B.GetBack());
    }
    return BoxF(right, left, top, bottom, front, back);
}