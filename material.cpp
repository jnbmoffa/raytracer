#include "material.hpp"

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

void PhongMaterial::apply_gl() const
{
  // Perform OpenGL calls necessary to set up this material.
}