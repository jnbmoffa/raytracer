#pragma once

#include "octree.h"
#include "array.h"
#include <list>

class SceneNode;
class Light;

class SceneContainer
{
protected:
	Array<SceneNode*>* Nodes;
	const std::list<Light*>* lights;

public:
	virtual ~SceneContainer() {}
	SceneContainer(Array<SceneNode*>* Nodes, const std::list<Light*>* lights) : Nodes(Nodes), lights(lights) {}
	virtual bool Trace(Colour& OutCol, const Ray& R, HitInfo& Hit, const Colour& ambient);
	virtual bool DepthTrace(const Ray& R, double& dist);
};

class OctreeSceneContainer : public SceneContainer
{
	OcTree<SceneNode> Tree;

 public:
 	virtual ~OctreeSceneContainer() {}
 	OctreeSceneContainer(Array<SceneNode*>* Nodes, const std::list<Light*>* lights);
 	virtual bool Trace(Colour& OutCol, const Ray& R, HitInfo& Hit, const Colour& ambient) override;
 	virtual bool DepthTrace(const Ray& R, double& dist) override;
};