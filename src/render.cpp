#include "render.hpp"
#include "image.hpp"
#include <cmath>
#include <limits>
#include <memory>
#include <vector>
#include "octree.h"
#include "scenecontainer.h"

atomic_int PROGRESS(0);

// TODO: get rid of all globals
double yDiv = 1.f, xDiv = 1.f;  // threads
int SuperSamples = 1; // AA
bool bUseOctree = false, bUseAdaptive = false;

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
               )
{
  // Scene setup
  std::vector<std::unique_ptr<SceneNode>> List;
  root->FlattenScene(List);
  std::unique_ptr<SceneContainer> Scene;
  if (bUseOctree)
  {
     Scene = std::make_unique<OctreeSceneContainer>(&List, &lights, MappedPhotons);
  }
  else
  {
    Scene = std::make_unique<SceneContainer>(&List, &lights, MappedPhotons);
  }

  std::shared_ptr<Camera> cam = CreateCamera(luaCam, width, height);

  // Create img object to store the render
  Image img(width, height, 3);

  // Nice-to-have output
  std::cout.precision(2);
  std::cerr << "render("
          << filename << ", " << width << ", " << height << ",\n     "
          << *cam << ",\n     "
          << ambient << ",\n     {";
  for (auto I = lights.cbegin(); I != lights.cend(); ++I) {
    if (I != lights.cbegin()) std::cerr << ", ";
    std::cerr << **I;
  }
  std::cerr << "});" << std::endl;

  Vector4D a(0,0,0,0);
  std::cout << a.length2() << std::endl;

  // Create rendering threads
  std::cout << "Tracing rays..." << std::endl;
  std::unique_ptr<RenderThread> threads[(int)(xDiv*yDiv)];
  int threadIndex = 0;
  double  imageBlockHeight = height/yDiv,
          imageBlockWidth = width/xDiv;

  ImageBlock imgBlockParams;

  for (double y=0;y<height;y+=imageBlockHeight)
  {
    for (double x=0;x<width;x+=imageBlockWidth)
    {
      int endY = ((y+imageBlockHeight) > height) ? height : y+imageBlockHeight;
      int endX = ((x+imageBlockWidth) > width) ? width : x+imageBlockWidth;
      imgBlockParams.yMin = y; imgBlockParams.yMax = endY;
      imgBlockParams.xMin = x; imgBlockParams.xMax = endX;

      if (bUseAdaptive) threads[threadIndex++] = CreateThread<AdaptiveSampleThread> (Scene.get(), &img, imgBlockParams, cam, ambient, TimeDuration, TimeSteps);
      else if (SuperSamples > 1) threads[threadIndex++] = CreateThread<SuperSampleThread> (Scene.get(), &img, imgBlockParams, cam, ambient, TimeDuration, TimeSteps, SuperSamples);
      else threads[threadIndex++] = CreateThread<RenderThread> (Scene.get(), &img, imgBlockParams, cam, ambient, TimeDuration, TimeSteps);
    }
  }
  
  // Status bar
  while (PROGRESS/xDiv*width < width*height)
  {
    std::cout << std::fixed << (double)(PROGRESS/xDiv*width*100)/((double)(width*height)) << "\%\xd"; std::cout.flush();
    SleepMicro(50000);  // Sleep so minimal CPU is used for this
  }

  // Wait for threads to finish
  for (int j=0;j<(int)(xDiv*yDiv);j++) threads[j]->Join();

  std::cout << "Creating image file (" << filename << ")..." << std::endl;
  img.savePng(filename);
}

void RenderThread::Main()
{
  for (int y=Params.yMin;y<Params.yMax;y++)
  {
    for (int x=Params.xMin;x<Params.xMax;x++)
    {
      Colour Total;
      for (int t=0;t<TimeSteps;t++)
      {
        double Time  = TimeDuration*((double)t/(double)TimeSteps);
        Colour DOFTotal;
        for (int d=0;d<NormCam->GetDOFRays();d++)
        {
          RandomEyePoint = NormCam->GetRandomEye();
          DOFTotal = DOFTotal + TracePixelAntiAliased(x, y, Time);
        }
        // Average DOF ray colour
        Total = Total + DOFTotal / NormCam->GetDOFRays();
      }

      Total = Total / TimeSteps;

      (*img)(x, y, 0) = Total.R();
      (*img)(x, y, 1) = Total.G();
      (*img)(x, y, 2) = Total.B();
    }
    PROGRESS++;
  }
}

inline Colour RenderThread::TracePixelAntiAliased(int x, int y, const double& Time)
{
  Ray R = NormCam->GetRayThroughPixel(x, y, 0.5f, 0.5f);
  //std::cout << R.Direction << std::endl;
  return TraceRay(R, 1.f, 0, Time);
}

Colour RenderThread::TraceRay(Ray& R, double powerCoef, unsigned int depth, const double& Time)
{
  if (powerCoef <= 0.05 || depth >= 9) return Colour();

  Colour TraceColour(0, 0, 0);
  HitInfo Hit;
  if (!Scene->TimeRayTrace(TraceColour, R, Hit, ambient, Time)) return Colour();

  // Refraction enabled?
  if (Hit.Mat->GetRef())
  {
    unsigned int nextDepth = depth+1;
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
    double nint = ni/nt;
    double sin2t = nint*nint*(1.f-NdotR*NdotR); // assumes R.Direction and Hit.Normal are normalized

    if (sin2t <= 1.f)
    {
      // Fresnel
      double cost = sqrt(1.f-sin2t);
      double R1 = (ni*NdotR - nt*cost)/(ni*NdotR + nt*cost); R1 *= R1;
      double R2 = (nt*NdotR - ni*cost)/(nt*NdotR + ni*cost); R2 *= R2;
      double Reflectance = (R1 + R2)/2.f;

      // Reflected ray creation
      Ray ReflectedRay = R.Reflect(Hit, NdotR);
      Colour reflCol;
      if (Hit.Mat->GetGloss() > 0.f)
      {
        // Glossy
        Ray GlossRay; GlossRay.Start = ReflectedRay.Start;
        Vector3D u = ReflectedRay.Direction.cross(Hit.Normal);
        Vector3D v = ReflectedRay.Direction.cross(u);

        // TODO: remove magic number 8
        Colour TotalGloss;
        for (int i=0;i<8;i++)
        {
          double theta = acos(pow((1.f - RefrDistribution(generator)), 1.f/(Hit.Mat->GetGloss()+1.f)));
          double phi = 2.f * M_PI * RefrDistribution(generator);
          double x = sin(theta) * cos(phi);
          double y = sin(theta) * sin(phi);

          // Perturb ray
          GlossRay.Direction = x * u + y * v + ReflectedRay.Direction;
          GlossRay.Direction.normalize();
          TotalGloss = TotalGloss + TraceRay(GlossRay, powerCoef * Reflectance, nextDepth, Time);
        }
        reflCol = TotalGloss / 8.f;
      }
      else
      {
        reflCol = TraceRay(ReflectedRay, powerCoef * Reflectance, nextDepth, Time);
      }

      // Refracted ray creation
      Ray RefractedRay = R.Refract(ni, nt, NdotR, sin2t, Hit);
      double refrCoef = powerCoef * (1.f - Reflectance);
      Colour refrCol =  refrCoef * TraceColour + TraceRay(RefractedRay, refrCoef, nextDepth, Time);

      return reflCol + refrCol;
    }
    else
    {
      // Total Internal Reflection
      Ray ReflectedRay = R.Reflect(Hit, (-R.Direction).dot(Hit.Normal));
      return TraceRay(ReflectedRay, powerCoef, nextDepth, Time);
    }
  }
  else
  {
    // Diffuse surface
    return powerCoef * TraceColour;
  }
}

Colour SuperSampleThread::TracePixelAntiAliased(int x, int y, const double& Time)
{
  // Equally distributed super-samples (grid)
  Colour SuperTotal;
  double numRays = (double)SuperSamples*SuperSamples;
  Colour Cols[(int)numRays];
  double halfSubWidth = 1.f/((double)SuperSamples*2.f); int index = 0;
  for (double i=halfSubWidth;i<1;i+=halfSubWidth*2)
  {
    for (double j=halfSubWidth;j<1;j+=halfSubWidth*2)
    {
      Ray R = NormCam->GetRayThroughPixel(x, y, i, j);
      Cols[index++] = TraceRay(R, 1.f, 0, Time);
    }
  }

  for (int i=0;i<(int)numRays;i++)
  {
    SuperTotal  = SuperTotal + Cols[i];
  }
  return SuperTotal / numRays;
}

inline Colour AdaptiveSampleThread::TracePixelAntiAliased(int x, int y, const double& Time)
{
  return AdaptiveSuperSample(x, y, 0.f, 1.f, 0.f, 1.f, 0, Time);
}

Colour AdaptiveSampleThread::AdaptiveSuperSample(int x, int y, double xNormMin, double xNormMax, double yNormMin, double yNormMax, unsigned int depth, const double& Time)
{
  Colour Cols[4];
  double QuartX = (xNormMax - xNormMin)/4.f;
  double QuartY = (yNormMax - yNormMin)/4.f;
  Ray R0 = NormCam->GetRayThroughPixel(x, y, xNormMin + QuartX, yNormMin + QuartY);
  Ray R1 = NormCam->GetRayThroughPixel(x, y, xNormMin + QuartX, yNormMax - QuartY);
  Ray R2 = NormCam->GetRayThroughPixel(x, y, xNormMax - QuartX, yNormMin + QuartY);
  Ray R3 = NormCam->GetRayThroughPixel(x, y, xNormMax - QuartX, yNormMax - QuartY);
  Cols[0] = TraceRay(R0, 1.f, 0, Time);
  Cols[1] = TraceRay(R1, 1.f, 0, Time);
  Cols[2] = TraceRay(R2, 1.f, 0, Time);
  Cols[3] = TraceRay(R3, 1.f, 0, Time);
  Colour Total = Cols[0] + Cols[1] + Cols[2] + Cols[3];

  // Not the same colours - recurse
  if (!(Cols[0] == Cols[1] && Cols[0] == Cols[2] && Cols[0] == Cols[3]) && depth < 2)
  {
    double HalfX = QuartX*2, HalfY = QuartY*2;
    unsigned int newDepth = depth+1;
    Total = Total + AdaptiveSuperSample(x, y, xNormMin, xNormMin + HalfX, yNormMin, yNormMin + HalfY, newDepth, Time) +
                    AdaptiveSuperSample(x, y, xNormMin + HalfX, xNormMax, yNormMin, yNormMin + HalfY, newDepth, Time) +
                    AdaptiveSuperSample(x, y, xNormMin, xNormMin + HalfX, yNormMin + HalfY, yNormMax, newDepth, Time) +
                    AdaptiveSuperSample(x, y, xNormMin + HalfX, xNormMax, yNormMin + HalfY, yNormMax, newDepth, Time);
    Total = Total / 8.f;
  }
  // Same colours or too deep
  else
  {
    Total = Total / 4.f;
  }

  return Total;
}