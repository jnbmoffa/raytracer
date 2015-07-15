#ifndef RENDER_HPP
#define RENDER_HPP

#include <string>
#include "algebra.hpp"
#include "scene.hpp"
#include "light.hpp"
#include "Thread.h"
#include "camera.h"
#include <random>

typedef Array<SceneNode*> NodeList;
extern double xDiv, yDiv, FocalDistance;
extern int SuperSamples, DOFRays;
extern bool bUseOctree, bUseDOF, bUseAdaptive;

class SceneContainer;

void render(// What to render
               SceneNode* root,
               // Where to output the image
               const std::string& filename,
               // Image size
               int width, int height,
               // Viewing parameters
               Camera* cam,
               // Lighting parameters
               const Colour& ambient,
               const std::list<Light*>& lights
               );

class Image;

class RenderThread : public Thread
{
protected:
     SceneContainer* Scene;
     Image* img;

     int yMin, yMax, xMin, xMax;
     double pixelWidth, pixelHeight, halfWidth, halfHeight;

     Camera* NormCam;
     Point3D RandomEyePoint;

     const Colour& ambient;
     const std::list<Light*>* lights;

     std::default_random_engine generator;
     std::uniform_real_distribution<double> RefrDistribution;

public:
     RenderThread(SceneContainer* Scene, Image* img, int yMin, int yMax, int xMin, int xMax, double pixelWidth, double pixelHeight, double halfWidth, double halfHeight,
                  Camera* Cam, const Colour& ambient, const std::list<Light*>* lights) :
               Scene(Scene), img(img), yMin(yMin), yMax(yMax), xMin(xMin), xMax(xMax), pixelWidth(pixelWidth), pixelHeight(pixelHeight),
               halfWidth(halfWidth), halfHeight(halfHeight), NormCam(Cam), ambient(ambient), lights(lights),
               RefrDistribution(0.f,1.f)
               {
                    RandomEyePoint = Cam->eye;
               }

     virtual void Main() override;

     // Do a trace through the pixel at a specific location (center is xNorm=0.5, yNorm=0.5)
     Colour TracePixelNormalized(int x, int y, double xNorm, double yNorm);
     Colour TraceRay(Ray& R, double powerCoef, unsigned int depth);
     Colour AdaptiveSuperSample(int x, int y, double xNormMin, double xNormMax, double yNormMin, double yNormMax, unsigned int depth);
};

#endif
