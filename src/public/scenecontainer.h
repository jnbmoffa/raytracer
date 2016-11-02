#pragma once

#include "octree.h"
#include <vector>
#include <list>
#include "photonmap.hpp"
#include "ray.h"

class SceneNode;
class Light;

// Holds all of the scene objects and provides trace functions against them
// Allows for subclasses to optimize traces using bounding volumes/octrees/etc.
class SceneContainer
{
protected:
	std::vector<std::unique_ptr<SceneNode>>* Nodes;
	PhotonMap PMap;
	virtual bool ContainerSpecificTimeTrace(const Ray& R, HitInfo& Hit, const double& Time) const;
	virtual bool ContainerSpecificColourTrace(const Ray& R, HitInfo& Hit) const;
	virtual bool ContainerSpecificDepthTrace(const Ray& R, double& dist) const;

public:
	const std::list<std::unique_ptr<Light>>* lights;
	virtual ~SceneContainer() {}
	SceneContainer(std::vector<std::unique_ptr<SceneNode>>* Nodes, const std::list<std::unique_ptr<Light>>* lights, unsigned int Photons) : Nodes(Nodes), PMap(this, Photons), lights(lights) 
	{
		// Map photons
  		PMap.BuildTree();
	}

	// Find all photons within SearchDistSq square units from the CheckLoc
	// MaxDist2 will be the square dist to the furthest away photon returned
	void LocatePhotons(Array<Photon*>& OutArray, const Point3D& CheckLoc, const double& SearchDistSq, double& MaxDist2) const;

	inline unsigned int MappedPhotons() const { return PMap.NumPhotons(); }

	// Trace types
	bool TimeRayTrace(Colour& OutCol, const Ray& R, HitInfo& Hit, const Colour& ambient, const double& Time) const;
	bool RayTrace(Colour& OutCol, const Ray& R, HitInfo& Hit, const Colour& ambient) const;
	bool PhotonTrace(const Ray& R, HitInfo& Hit) const;
	bool TimeDepthTrace(const Ray& R, double& dist, const double& Time) const;
	bool DepthTrace(const Ray& R, double& dist) const;
};

class OctreeSceneContainer : public SceneContainer
{
	OcTree<SceneNode> Tree;
protected:
	virtual bool ContainerSpecificTimeTrace(const Ray& R, HitInfo& Hit, const double& Time) const override;
	virtual bool ContainerSpecificColourTrace(const Ray& R, HitInfo& Hit) const override;
	virtual bool ContainerSpecificDepthTrace(const Ray& R, double& dist) const override;

public:
 	virtual ~OctreeSceneContainer() {}
 	OctreeSceneContainer(std::vector<std::unique_ptr<SceneNode>>* Nodes, const std::list<std::unique_ptr<Light>>* lights, unsigned int Photons);
};