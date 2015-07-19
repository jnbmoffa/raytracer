#pragma once
#include "array.h"
#include "algebra.hpp"
#include "kdtree.h"
#include <memory>
#include <vector>

class SceneContainer;

// For storing photons in the map
struct Photon : public KDObject
{
	Photon() {}
	Photon(Point3D Position, Colour Power, Vector3D IncidentDir) : Position(Position), Power(Power), IncidentDir(IncidentDir) {}

	Point3D Position;
	Colour Power;
	Vector3D IncidentDir;
	short flag;

	virtual double operator[](unsigned int Index) const override;
};

std::ostream& operator<<(std::ostream& out, const Photon& p);

class PhotonMap
{
	// KD Tree?
	SceneContainer* Scene;
	std::vector<Photon*> Storage;
	KDTree<Photon> Tree;
	unsigned int NumToEmit;

	std::default_random_engine generator;
    std::uniform_real_distribution<double> PhotonDistribution;

	void TracePhoton(const Ray& R, const Colour& Power, unsigned int depth, bool bHasRef = false);

public:
	PhotonMap(SceneContainer* Scene, unsigned int NumToEmit);
	~PhotonMap();
	void BuildTree();
	void LocatePhotons(Array<Photon*>& OutArray, const Point3D& CheckLoc, const double& SearchDistSq, double& MaxDist2);
};