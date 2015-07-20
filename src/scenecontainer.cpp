#include "scenecontainer.h"
#include "scene.hpp"

bool SceneContainer::ContainerSpecificColourTrace(const Ray& R, HitInfo& Hit)
{
  double closestDist = 10000000.f; Matrix4x4 M;
  bool bHit = false;
  for (SceneNode* S : *Nodes)
  {
    if(S->ColourTrace(R, closestDist, Hit, M)) bHit = true;
  }
  return bHit;
}

bool SceneContainer::ContainerSpecificDepthTrace(const Ray& R, double& dist)
{
  bool bHit = false;
  HitInfo ShadowHit; Matrix4x4 M;
  dist = 1000000.f;
  for (SceneNode* S : *Nodes)
  {
    if (S->DepthTrace(R, dist, ShadowHit, M)) bHit = true;
  }
  return bHit;
}

void SceneContainer::MapPhotons()
{
  PMap.BuildTree();
}

bool SceneContainer::RayTrace(Colour& OutCol, const Ray& R, HitInfo& Hit, const Colour& ambient)
{
  if (ContainerSpecificColourTrace(R, Hit))
  {
    Hit.Normal.normalize();

    OutCol = Hit.Mat->DoLighting(this, R, lights, Hit, ambient);
    return true;
  }
  return false;
}

bool SceneContainer::PhotonTrace(const Ray& R, HitInfo& Hit)
{
  return ContainerSpecificColourTrace(R, Hit);
}

bool SceneContainer::DepthTrace(const Ray& R, double& dist)
{
  return ContainerSpecificDepthTrace(R, dist);
}

OctreeSceneContainer::OctreeSceneContainer(Array<SceneNode*>* Nodes, const std::list<Light*>* lights, unsigned int Photons) : SceneContainer(Nodes, lights, Photons)
{
  std::cout << "Building octree..." << std::endl;
  Tree = OcTree<SceneNode>(GetSceneBounds(*Nodes));
  for (SceneNode* s : *Nodes)
  {
    Tree.Insert(s);
  }
}

bool OctreeSceneContainer::ContainerSpecificColourTrace(const Ray& R, HitInfo& Hit)
{
  double closestDist = 10000000.f; Matrix4x4 M;
  bool bHit = false;
  Array<SceneNode*> OctList;
  Tree.Trace(OctList, R);
  for (SceneNode* S : OctList)
  {
    if(S->ColourTrace(R, closestDist, Hit, M)) bHit = true;
  }
  return bHit;
}

bool OctreeSceneContainer::ContainerSpecificDepthTrace(const Ray& R, double& dist)
{
  bool bHit = false;
  HitInfo Hit; Matrix4x4 M;
  dist = 1000000.f;
  Array<SceneNode*> OctList;
  Tree.Trace(OctList, R);
  for (SceneNode* S : OctList)
  {
    if(S->DepthTrace(R, dist, Hit, M)) bHit = true;
  }
  return bHit;
}