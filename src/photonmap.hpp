#pragma once
#include "array.h"
#include "algebra.hpp"
#include "ray.h"
#include "kdtree.h"
#include "photon.h"
#include <memory>
#include <vector>

class SceneContainer;

class PhotonMap
{
public:
	using photon_stg_type = std::vector<Photon*>;
	using size_type = photon_stg_type::size_type;

private:
	SceneContainer* Scene;
	photon_stg_type Storage;
	KDTree<Photon> Tree;
	size_type NumToEmit;

	std::default_random_engine generator;
    std::uniform_real_distribution<double> PhotonDistribution;

	// Only does caustics, global illumination uses russian roulette
    void TracePhoton(const Ray& R, const Colour& Power, unsigned int depth, bool bHasRef = false);

public:
	PhotonMap(SceneContainer* Scene, size_type NumToEmit);
	~PhotonMap();

	inline unsigned int NumPhotons() const { return NumToEmit; }

	// Fire NumToEmit photons into the scene and store them in a KDTree
	void BuildTree();

	// Find all photons within SearchDistSq square units from the CheckLoc
	// MaxDist2 will be the square dist to the furthest away photon returned
	void LocatePhotons(Array<Photon*>& OutArray, const Point3D& CheckLoc, const double& SearchDistSq, double& MaxDist2);
};