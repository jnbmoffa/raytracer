#ifndef RENDER_HPP
#define RENDER_HPP

#include <string>
#include "algebra.hpp"
#include "scene.hpp"
#include "light.hpp"
#include "Thread.h"
#include "luacamera.h"
#include "camera.h"
#include "RenderData.h"
#include <random>
#include <memory>

typedef Array<SceneNode*> NodeList;
extern uint16_t numThreads, SuperSamples;
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
     RenderData m_renderData;

     std::default_random_engine generator;
     std::uniform_real_distribution<double> RefrDistribution;

public:
     RenderThread(RenderData renderData) :
          m_renderData(renderData),
          generator(std::random_device{}()),
          RefrDistribution(0.f, 1.f)
     {}

     virtual void Main() override;

     virtual Colour TracePixelAntiAliased(int x, int y, const double& Time);

     Colour TraceRay(Ray& R, double powerCoef, unsigned int depth, const double& Time);
};

class SuperSampleThread : public RenderThread
{
     int SuperSamples;
public:
     SuperSampleThread(RenderData renderData, int SuperSamples) :
          RenderThread(renderData),
          SuperSamples(SuperSamples)
     {}

     virtual Colour TracePixelAntiAliased(int x, int y, const double& Time) override;
};

class AdaptiveSampleThread : public RenderThread
{
public:
     AdaptiveSampleThread(RenderData renderData) :
          RenderThread(renderData)
     {}

     virtual Colour TracePixelAntiAliased(int x, int y, const double& Time) override;
     Colour AdaptiveSuperSample(int x, int y, double xNormMin, double xNormMax, double yNormMin, double yNormMax, unsigned int depth, const double& Time);
};

#endif