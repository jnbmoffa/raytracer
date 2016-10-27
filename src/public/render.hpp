#ifndef RENDER_HPP
#define RENDER_HPP

#include <string>
#include "algebra.hpp"
#include "scene.hpp"
#include "light.hpp"
#include "Thread.h"
#include "luacamera.h"
#include "camera.h"
#include <random>
#include <memory>

typedef Array<SceneNode*> NodeList;
extern double xDiv, yDiv, FocalDistance;
extern int SuperSamples;
extern bool bUseOctree, bUseAdaptive;

class SceneContainer;

struct ImageBlock
{
     int yMin;
     int yMax;
     int xMin;
     int xMax;
};

void render(// What to render
               std::unique_ptr<SceneNode>&& root,
               // Where to output the image
               const std::string& filename,
               // Image size
               int width, int height,
               // Viewing parameters
               std::shared_ptr<LuaCamera> luaCam,
               // Lighting parameters
               const Colour& ambient,
               const std::list<std::unique_ptr<Light>>& lights,
               int MappedPhotons,
               // Time stepping
               double TimeDuration, int TimeSteps
               );

class Image;

class RenderThread : public Thread
{
protected:
     SceneContainer* Scene;
     Image* img;

     ImageBlock Params;

     std::shared_ptr<Camera> NormCam;
     Point3D RandomEyePoint;

     const Colour& ambient;

     std::default_random_engine generator;
     std::uniform_real_distribution<double> RefrDistribution;

     double TimeDuration;
     int TimeSteps;

public:
     RenderThread(SceneContainer* Scene, Image* img, ImageBlock Params, std::shared_ptr<Camera>& Cam, const Colour& ambient, double TimeDuration,
               int TimeSteps) :
               Scene(Scene), img(img), Params(Params), NormCam(Cam), ambient(ambient), generator(std::random_device{}()),
               RefrDistribution(0.f,1.f), TimeDuration(TimeDuration), TimeSteps(TimeSteps)
               {}

     virtual void Main() override;

     virtual Colour TracePixelAntiAliased(int x, int y, const double& Time);

     Colour TraceRay(Ray& R, double powerCoef, unsigned int depth, const double& Time);
};

class SuperSampleThread : public RenderThread
{
     int SuperSamples;
public:
     SuperSampleThread(SceneContainer* Scene, Image* img, ImageBlock Params, std::shared_ptr<Camera>& Cam, const Colour& ambient, double TimeDuration,
          int TimeSteps, int SuperSamples) :
          RenderThread(Scene, img, Params, Cam, ambient, TimeDuration, TimeSteps), SuperSamples(SuperSamples) {}

     virtual Colour TracePixelAntiAliased(int x, int y, const double& Time) override;
};

class AdaptiveSampleThread : public RenderThread
{
public:
     AdaptiveSampleThread(SceneContainer* Scene, Image* img, ImageBlock Params, std::shared_ptr<Camera>& Cam, const Colour& ambient, double TimeDuration,
          int TimeSteps) :
          RenderThread(Scene, img, Params, Cam, ambient, TimeDuration, TimeSteps) {}

     virtual Colour TracePixelAntiAliased(int x, int y, const double& Time) override;
     Colour AdaptiveSuperSample(int x, int y, double xNormMin, double xNormMax, double yNormMin, double yNormMax, unsigned int depth, const double& Time);
};

#endif