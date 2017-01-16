#include "photonmap.hpp"
#include "scenecontainer.h"
#include "light.hpp"
#include "material.hpp"
#include <iostream>
#include "progressthread.h"

PhotonMap::PhotonMap(SceneContainer* Scene, size_type NumToEmit) : Scene(Scene), Tree(3), NumToEmit(NumToEmit), generator(std::random_device{}()),
	PhotonDistribution(-1.f,1.f)
	{}

PhotonMap::~PhotonMap()
{
	for (Photon* P : Storage)
	{
		if (P) delete P;
	}
}

// Only does caustics
void PhotonMap::TracePhoton(const Ray& R, const Colour& Power, unsigned int depth, bool bHasRef)
{
	if (depth >= 9) return;

	HitInfo Hit;
	if (!Scene->PhotonTrace(R, Hit)) return;

	const Vector3D rayDir = R.GetDirection();

	// Refraction enabled?
	if (Hit.Mat->GetRef())
	{
		unsigned int nextDepth = depth+1;
		double NdotR = rayDir.dot(Hit.Normal);
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
			NdotR = (-rayDir).dot(Hit.Normal);
		}
		double sin2t = (ni/nt)*(ni/nt)*(1.f-NdotR*NdotR); // assumes rayDir and Hit.Normal are normalized

		if (sin2t <= 1.f)
		{
			// Fresnel
			double cost = sqrt(1.f-sin2t);
			double R1 = (ni*NdotR - nt*cost)/(ni*NdotR + nt*cost); R1 *= R1;
			double R2 = (nt*NdotR - ni*cost)/(nt*NdotR + ni*cost); R2 *= R2;
			double Reflectance = (R1 + R2) * 0.5;

			// Photons don't do glossy reflection
			Ray ReflectedRay = R.Reflect(Hit, NdotR);
			TracePhoton(ReflectedRay, Reflectance*Power, nextDepth, bHasRef);

			Ray RefractedRay = R.Refract(ni, nt, NdotR, sin2t, Hit);
			TracePhoton(RefractedRay, (1.f-Reflectance)*Power, nextDepth, true);
		}
		else
		{
			// Total internal reflection
			Ray ReflectedRay = R.Reflect(Hit, NdotR);
			TracePhoton(ReflectedRay, Power, nextDepth, bHasRef);
		}
	}
	else if (bHasRef)
	{
		// Stick if this trace has refracted at least once
		Storage.emplace_back(new Photon(Hit.Location, Power, rayDir));
	}
}

void PhotonMap::BuildTree()
{
	if (NumToEmit > 0)
	{
		std::cout << "Mapping " << NumToEmit * Scene->lights->size() << " photons..." << std::endl;
		std::unique_ptr<ProgressThread> Status(CreateThread<ProgressThread>((double)NumToEmit * Scene->lights->size()));
		Storage.reserve(NumToEmit * Scene->lights->size());
		for (auto& L : *(Scene->lights))
		{
			Colour Power = (L->power * L->colour) / NumToEmit;
			for (size_type n=0;n<NumToEmit;n++)
			{
				// Random photon directions
				Vector3D Dir(PhotonDistribution(generator), PhotonDistribution(generator), PhotonDistribution(generator)); Dir.normalize();
				TracePhoton(Ray(L->position, Dir), Power, 0);
				Status->PROGRESS++;
			}
		}

		Tree.MakeTree(Storage);
	}
}

void PhotonMap::LocatePhotons(Array<Photon*>& OutArray, const Point3D& CheckLoc, const double& SearchDistSq, double& MaxDist2) const
{
	Photon P(CheckLoc, Colour(), Vector3D());
	Tree.LocateNearby(OutArray, P, SearchDistSq, MaxDist2);
}