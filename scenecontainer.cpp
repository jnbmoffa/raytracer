#include "scenecontainer.h"
#include "scene.hpp"

bool SceneContainer::Trace(Colour& OutCol, const Ray& R, HitInfo& Hit, const Colour& ambient)
{
  double closestDist = 10000000.f; Matrix4x4 M;
  bool bHit = false;
  // std::cout << "Nodes:" <<  (*Nodes).Num() << std::endl;
  for (SceneNode* S : *Nodes)
  {
    if(S->ColourTrace(R, closestDist, Hit, M)) bHit = true;
  }

  if (bHit)
  {
    Hit.Normal.normalize();

    OutCol = Hit.Mat->DoLighting(this, R, lights, Hit, ambient);
    return true;
  }
  return false;
}

bool SceneContainer::DepthTrace(const Ray& R, double& dist)
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

OctreeSceneContainer::OctreeSceneContainer(Array<SceneNode*>* Nodes, const std::list<Light*>* lights) : SceneContainer(Nodes, lights)
{
  std::cout << "Building octree..." << std::endl;
  Tree = OcTree<SceneNode>(GetSceneBounds(*Nodes));
  for (SceneNode* s : *Nodes)
  {
    Tree.Insert(s);
  }
}

bool OctreeSceneContainer::Trace(Colour& OutCol, const Ray& R, HitInfo& Hit, const Colour& ambient)
{
  double closestDist = 10000000.f; Matrix4x4 M;
  bool bHit = false;
  Array<SceneNode*> OctList;
  Tree.Trace(OctList, R);
  for (SceneNode* S : OctList)
  {
    if(S->ColourTrace(R, closestDist, Hit, M)) bHit = true;
  }

  if (bHit)
  {
    Hit.Normal.normalize();

    OutCol = Hit.Mat->DoLighting(this, R, lights, Hit, ambient);
    return true;
  }
  return false;
}

bool OctreeSceneContainer::DepthTrace(const Ray& R, double& dist)
{
  bool bHit = false;
  HitInfo Hit; Matrix4x4 M;
  dist = 1000000.f;
  Array<SceneNode*> OctList;
  Tree.Trace(OctList, R);
  for (SceneNode* S : OctList)
  {
    if(S->ColourTrace(R, dist, Hit, M)) bHit = true;
  }
  return bHit;
}