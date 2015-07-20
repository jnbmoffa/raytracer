#include "material.hpp"

#include "light.hpp"
#include "photonmap.hpp"
#include "scenecontainer.h"

Material::~Material()
{
}

PhongMaterial::PhongMaterial(const Colour& kd, const Colour& ks, double shininess, bool refractive, double refractiveIndex
	, double glossiness)
  : m_kd(kd), m_ks(ks), m_shininess(shininess), refractive(refractive), refractiveIndex(refractiveIndex), glossiness(glossiness)
{
}

PhongMaterial::~PhongMaterial()
{
}

Colour PhongMaterial::DoLighting(SceneContainer* Scene, const Ray& R, const std::list<Light*>* lights, const HitInfo& Hit, const Colour& ambient)
{
	// Lighting
	// Ambient
	Colour OutCol = ambient*m_kd;
	for (Light* L : *lights)
	{
		// Caustics
		Array<Photon*> Photons;
		double radius = 2, MaxDist2 = -1;
		Scene->GetPhotonMap()->LocatePhotons(Photons, Hit.Location, radius*radius, MaxDist2);	// search 5 units around hit
		Colour CausticTotal;
		for (Photon* P : Photons)
		{
			CausticTotal = CausticTotal + (-P->IncidentDir).dot(Hit.Normal) * m_kd * P->Power;
		}
		if (MaxDist2 > 0) OutCol = OutCol + CausticTotal / (M_PI * MaxDist2); // Colour(1, 0, 1);

		// Shadows
		double LightIntensity = L->GetIntensity(Scene, Hit.Location);
		if (IsNearly(LightIntensity, 0.f)) continue;

		// Diffuse
		Vector3D LightDir = L->position - Hit.Location; LightDir.normalize();
		double LdotN = LightDir.dot(Hit.Normal);
		if (m_kd != Colour(0, 0, 0))
		{
			if (LdotN < 0) LdotN = 0;
			Colour Diffuse = m_kd * L->colour * LdotN;
			OutCol = OutCol + LightIntensity*Diffuse;
		}

		// Specular
		if (m_ks != Colour(0, 0, 0))
		{
			Vector3D TempEye = R.Start - Hit.Location; TempEye.normalize();
			Vector3D Reflection = ((2*LdotN)*Hit.Normal) - LightDir; Reflection.normalize();
			double RdotE = Reflection.dot(TempEye);
			if (RdotE > 0) OutCol = OutCol + (L->colour * m_ks * pow(RdotE, m_shininess));
		}
	}
	return OutCol;
}