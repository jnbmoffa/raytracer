#include "photonmap.hpp"
#include "scenecontainer.h"
#include "light.hpp"
#include "material.hpp"
#include <iostream>
#include "progressthread.h"

std::ostream& operator<<(std::ostream& out, const Photon& p)
{
	return out << "Photon[" << p.Position << "," << p.Power << "," << p.IncidentDir;
}

double Photon::operator[](unsigned int Index) const
{
	return Position[Index];
}

PhotonMap::PhotonMap(SceneContainer* Scene, unsigned int NumToEmit) : Scene(Scene), Tree(3), NumToEmit(NumToEmit), generator(std::random_device{}()),
	PhotonDistribution(-1.f,1.f)
	{}

PhotonMap::~PhotonMap()
{
	for (Photon* P : Storage)
	{
		if (P) delete P;
	}
}

// Only does caustics, make different to do global illumination as well
void PhotonMap::TracePhoton(const Ray& R, const Colour& Power, unsigned int depth, bool bHasRef)
{
	if (depth >= 9) return;

	HitInfo Hit;
	if (!Scene->PhotonTrace(R, Hit)) return;

	// Refraction enabled?
	if (Hit.Mat->GetRef())
	{
		double NdotR = R.Direction.dot(Hit.Normal);
		double ni, nt;
		if (NdotR > 0)
		{
			// Leaving object
			ni = Hit.Mat->GetRefIndex(); nt = 1.f;
			Hit.Normal = -Hit.Normal;
		}
		else
		{
			// Entering object
			ni = 1.f; nt = Hit.Mat->GetRefIndex();
			NdotR = (-R.Direction).dot(Hit.Normal);
		}
		double sin2t = (ni/nt)*(ni/nt)*(1.f-NdotR*NdotR); // assumes R.Direction and Hit.Normal are normalized

		if (sin2t <= 1.f)
		{
			// Fresnel
			double cost = sqrt(1.f-sin2t);
			double R1 = (ni*NdotR - nt*cost)/(ni*NdotR + nt*cost); R1 *= R1;
			double R2 = (nt*NdotR - ni*cost)/(nt*NdotR + ni*cost); R2 *= R2;
			double Reflectance = (R1 + R2)/2.f;

			Ray ReflectedRay = R.Reflect(Hit, NdotR);
			TracePhoton(ReflectedRay, Reflectance*Power, depth+1, bHasRef);

			Ray RefractedRay = R.Refract(ni, nt, NdotR, sin2t, Hit);
			TracePhoton(RefractedRay, (1.f-Reflectance)*Power, depth+1, true);
			// std::cout << "Refract" << std::endl;
		}
		else
		{
			// Total internal reflection
			// std::cout << "Reflect" << std::endl;
			Ray ReflectedRay = R.Reflect(Hit, NdotR);
			TracePhoton(ReflectedRay, Power, depth+1, bHasRef);
		}
	}
	else if (bHasRef)
	{
		// Stick
		// std::cout << "Stick" << std::endl;
		Storage.push_back(new Photon(Hit.Location, Power, R.Direction));
	}
}

void PhotonMap::BuildTree()
{
	std::cout << "Mapping " << NumToEmit * Scene->lights->size() << " photons..." << std::endl;
	std::shared_ptr<ProgressThread> Status(CreateThread<ProgressThread>((double)NumToEmit * Scene->lights->size()));
	Storage.reserve(NumToEmit * Scene->lights->size());
	for (Light* L : *(Scene->lights))
	{
		Colour Power = (L->power * L->colour) / NumToEmit;
		for (unsigned int n=0;n<NumToEmit;n++)
		{
			Vector3D Dir(PhotonDistribution(generator), PhotonDistribution(generator), PhotonDistribution(generator)); Dir.normalize();
			TracePhoton(Ray(L->position, Dir), Power, 0);
			Status->PROGRESS++;
		}
	}

	// std::cout << "Stored Photons:" <<  Storage.size() << std::endl;
	Tree.MakeTree(Storage);
	// std::cout << "Actualy stored Photons:" <<  Tree.CountNodes() << std::endl;
}

void PhotonMap::LocatePhotons(Array<Photon*>& OutArray, const Point3D& CheckLoc, const double& SearchDistSq, double& MaxDist2)
{
	Photon P(CheckLoc, Colour(), Vector3D());
	Tree.LocateNearby(OutArray, P, SearchDistSq, MaxDist2);
}