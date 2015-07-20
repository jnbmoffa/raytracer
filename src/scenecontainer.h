#pragma once

#include "octree.h"
#include "array.h"
#include <list>
#include "photonmap.hpp"

class SceneNode;
class Light;

class SceneContainer
{
protected:
	Array<SceneNode*>* Nodes;
	PhotonMap PMap;
	virtual bool ContainerSpecificColourTrace(const Ray& R, HitInfo& Hit);
	virtual bool ContainerSpecificDepthTrace(const Ray& R, double& dist);

public:
	const std::list<Light*>* lights;
	virtual ~SceneContainer() {}
	SceneContainer(Array<SceneNode*>* Nodes, const std::list<Light*>* lights) : Nodes(Nodes), PMap(this, 500000), lights(lights) {}

	// Init
	void MapPhotons();

	inline PhotonMap* GetPhotonMap() { return &PMap; }

	// Trace types
	bool RayTrace(Colour& OutCol, const Ray& R, HitInfo& Hit, const Colour& ambient);
	bool PhotonTrace(const Ray& R, HitInfo& Hit);
	bool DepthTrace(const Ray& R, double& dist);
};

class OctreeSceneContainer : public SceneContainer
{
	OcTree<SceneNode> Tree;
protected:
	virtual bool ContainerSpecificColourTrace(const Ray& R, HitInfo& Hit) override;
	virtual bool ContainerSpecificDepthTrace(const Ray& R, double& dist) override;

public:
 	virtual ~OctreeSceneContainer() {}
 	OctreeSceneContainer(Array<SceneNode*>* Nodes, const std::list<Light*>* lights);
};