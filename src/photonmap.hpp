#pragma once
#include "array.h"
#include "algebra.hpp"
#include "kdtree.h"
#include <memory>
#include <vector>

class SceneContainer;

// The actual stored data in the map
struct Photon : public KDObject
{
	Photon() {}
	Photon(Point3D Position, Colour Power, Vector3D IncidentDir) : Position(Position), Power(Power), IncidentDir(IncidentDir) {}

	Point3D Position;
	Colour Power;
	Vector3D IncidentDir;

	virtual double operator[](unsigned int Index) const override;
};

std::ostream& operator<<(std::ostream& out, const Photon& p);

class PhotonMap
{
	SceneContainer* Scene;
	std::vector<Photon*> Storage;
	KDTree<Photon> Tree;
	unsigned int NumToEmit;

	std::default_random_engine generator;
    std::uniform_real_distribution<double> PhotonDistribution;

    // Only does caustics, global illumination uses russian roulette
	void TracePhoton(const Ray& R, const Colour& Power, unsigned int depth, bool bHasRef = false);

public:
	PhotonMap(SceneContainer* Scene, unsigned int NumToEmit);
	~PhotonMap();

	// Fire NumToEmit photons into the scene and store them in a KDTree
	void BuildTree();

	// Find all photons within SearchDistSq square units from the CheckLoc
	// MaxDist2 will be the square dist to the furthest away photon returned
	void LocatePhotons(Array<Photon*>& OutArray, const Point3D& CheckLoc, const double& SearchDistSq, double& MaxDist2);
};