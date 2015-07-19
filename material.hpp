#ifndef CS488_MATERIAL_HPP
#define CS488_MATERIAL_HPP

#include "algebra.hpp"
#include <list>

class Light;
class SceneContainer;
class PhotonMap;

class Material {
public:
  virtual ~Material();
  virtual Colour GetDiffuse() = 0;
  virtual Colour GetSpecular() = 0;
  virtual double GetShine() = 0;
  virtual bool GetRef() = 0;
  virtual double GetRefIndex() = 0;
  virtual double GetGloss() = 0;

  virtual Colour DoLighting(SceneContainer* Scene, const Ray& R, const std::list<Light*>* lights,const HitInfo& Hit, const Colour& ambient) = 0;

protected:
  Material()
  {
  }
};

class PhongMaterial : public Material {
public:
  PhongMaterial() : m_kd(1), m_ks(1), m_shininess(0), refractive(false), refractiveIndex(0) {}
  PhongMaterial(const Colour& kd, const Colour& ks, double shininess, bool refractive = false, double refractiveIndex = 0
      , double glossiness = 0.f);
  virtual ~PhongMaterial();

  virtual Colour GetDiffuse() override { return m_kd; }
  virtual Colour GetSpecular() override { return m_ks; }
  virtual double GetShine() override { return m_shininess; }
  virtual bool GetRef() override { return refractive; }
  virtual double GetRefIndex() override { return refractiveIndex; }
  virtual double GetGloss() override { return glossiness; }

  Colour DoLighting(SceneContainer* Scene, const Ray& R, const std::list<Light*>* lights,const HitInfo& Hit, const Colour& ambient);

private:
  Colour m_kd;
  Colour m_ks;

  double m_shininess;
  bool refractive;
  double refractiveIndex;
  double glossiness;
};


#endif